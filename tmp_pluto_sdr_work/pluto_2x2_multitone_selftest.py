import argparse
import json
import os
import time

import adi
import numpy as np


URI = "ip:192.168.2.1"

C = 299_792_458.0
VF_CABLE = 0.66

LO_HZ = int(100e6)
SAMPLE_RATE = int(30.72e6)
RF_BW = int(28e6)
N = 131072
N_AVG = 16

TX_GAIN_DB = -45
RX_GAIN_DB = 10
TOTAL_SCALE = 0.06
TONE_START_HZ = 500_000
TONE_STOP_HZ = 6_250_000
TONE_COUNT = 32

MIN_DUT_AMP_DB = -30.0
MIN_REF_AMP_DB = -30.0
MIN_VALID_TONES = 12
MIN_FIT_TONES = 12
MIN_WINDOW_TONES = 18
MIN_WINDOW_BW_HZ = 3e6
MAX_RESIDUAL_DEG = 3.0
MAX_RESIDUAL_PEAK_DEG = 6.0
MAX_WINDOW_LENGTH_SPAN_M = 0.75
WINDOW_TOP_CANDIDATES = 6
FIT_OUTLIER_RESIDUAL_DEG = 8.0
MAX_FIT_OUTLIER_REJECTIONS = 4
MAX_REPEAT_SPAN_M = 0.10
LOW_BAND_DEMO_START_HZ = 0.5e6
LOW_BAND_DEMO_STOP_HZ = 6.25e6
MIN_LOW_BAND_DEMO_TONES = 6
MIN_LOW_BAND_DEMO_BW_HZ = 1.25e6
MAX_LOW_BAND_DEMO_RESIDUAL_DEG = 2.5
MAX_LOW_BAND_DEMO_PEAK_DEG = 5.5

CAL_FILE = "cal_through.json"


def db20(value):
    value = abs(value)
    if value <= 0.0:
        return -600.0
    return 20.0 * np.log10(value)


def make_bin_aligned_tones(sample_rate, n, start_hz, stop_hz, count):
    bin_hz = sample_rate / n
    start_bin = int(round(start_hz / bin_hz))
    stop_bin = int(round(stop_hz / bin_hz))
    bins = np.linspace(start_bin, stop_bin, count)
    bins = np.unique(np.round(bins).astype(int))
    return bins.astype(np.float64) * bin_hz


def evaluate_quality(dut_amp_db, ref_amp_db, min_dut_amp_db, min_ref_amp_db, min_valid_tones):
    dut_amp_db = np.asarray(dut_amp_db, dtype=np.float64)
    ref_amp_db = np.asarray(ref_amp_db, dtype=np.float64)

    valid_mask = (dut_amp_db >= min_dut_amp_db) & (ref_amp_db >= min_ref_amp_db)
    valid_count = int(np.count_nonzero(valid_mask))

    if valid_count < min_valid_tones:
        verdict = "LOW_SIGNAL"
    else:
        verdict = "OK"

    return {
        "verdict": verdict,
        "valid_mask": valid_mask,
        "valid_count": valid_count,
        "dut_min_db": float(np.min(dut_amp_db)),
        "dut_max_db": float(np.max(dut_amp_db)),
        "ref_min_db": float(np.min(ref_amp_db)),
        "ref_max_db": float(np.max(ref_amp_db)),
    }


def apply_fit_quality(
    quality,
    residual_deg,
    max_residual_deg=MAX_RESIDUAL_DEG,
    peak_residual_deg=None,
    max_peak_residual_deg=MAX_RESIDUAL_PEAK_DEG,
):
    quality = dict(quality)
    quality["residual_deg"] = float(residual_deg)
    if peak_residual_deg is not None:
        quality["peak_residual_deg"] = float(peak_residual_deg)
    if quality["verdict"] == "OK" and residual_deg > max_residual_deg:
        quality["verdict"] = "HIGH_RESIDUAL"
    if (
        quality["verdict"] == "OK"
        and peak_residual_deg is not None
        and peak_residual_deg > max_peak_residual_deg
    ):
        quality["verdict"] = "HIGH_RESIDUAL"
    return quality


TONES_HZ = make_bin_aligned_tones(SAMPLE_RATE, N, TONE_START_HZ, TONE_STOP_HZ, TONE_COUNT)


def make_multitone():
    t = np.arange(N) / SAMPLE_RATE
    sig = np.zeros(N, dtype=np.complex128)

    rng = np.random.default_rng(1234)
    phases = rng.uniform(0, 2 * np.pi, len(TONES_HZ))

    for freq_hz, phase in zip(TONES_HZ, phases):
        sig += np.exp(1j * (2 * np.pi * freq_hz * t + phase))

    sig = sig / np.max(np.abs(sig))
    sig = sig * TOTAL_SCALE * (2**14)
    return sig.astype(np.complex64)


def unpack_rx(rx):
    if isinstance(rx, (list, tuple)):
        return np.array(rx[0]), np.array(rx[1])

    arr = np.array(rx)
    if arr.ndim == 2:
        return arr[0], arr[1]

    raise RuntimeError("RX data is not two-channel data. Check rx_enabled_channels = [0, 1].")


def extract_tone(rx, freq_hz):
    rx = np.asarray(rx)
    rx = rx - np.mean(rx)

    n = len(rx)
    t = np.arange(n) / SAMPLE_RATE
    ref = np.exp(-1j * 2 * np.pi * freq_hz * t)
    return np.mean(rx * ref)


def tx_destroy(sdr):
    try:
        sdr.tx_destroy_buffer()
    except Exception:
        pass


def fit_phase_slope(ratios, fit_mask):
    ratios = np.asarray(ratios)
    fit_mask = np.asarray(fit_mask, dtype=bool)

    if np.count_nonzero(fit_mask) < 2:
        raise ValueError("Need at least two valid tones for phase-slope fitting.")

    freqs = TONES_HZ[fit_mask]
    phase = np.unwrap(np.angle(ratios[fit_mask]))
    slope, intercept = np.polyfit(freqs, phase, 1)

    fitted = slope * freqs + intercept
    residual = phase - fitted
    residual_deg = float(np.sqrt(np.mean(np.square(residual))) * 180.0 / np.pi)
    residual_abs_deg = np.abs(residual) * 180.0 / np.pi

    tau = -slope / (2 * np.pi)
    signed_length_m = tau * VF_CABLE * C
    length_m = abs(signed_length_m)
    return {
        "length_m": length_m,
        "signed_length_m": signed_length_m,
        "tau": tau,
        "slope": slope,
        "phase": phase,
        "residual_deg": residual_deg,
        "peak_residual_deg": float(np.max(residual_abs_deg)),
        "tone_count": int(np.count_nonzero(fit_mask)),
        "start_hz": float(freqs[0]),
        "stop_hz": float(freqs[-1]),
        "bandwidth_hz": float(freqs[-1] - freqs[0]),
    }


def estimate_length_fit(ratios, valid_mask=None):
    ratios = np.asarray(ratios)
    if valid_mask is None:
        valid_mask = np.ones(len(ratios), dtype=bool)
    valid_mask = np.asarray(valid_mask, dtype=bool)
    fit_mask = valid_mask.copy()

    if np.count_nonzero(fit_mask) < 2:
        raise ValueError("Need at least two valid tones for phase-slope fitting.")

    rejected = 0
    while True:
        fit = fit_phase_slope(ratios, fit_mask)
        freqs = TONES_HZ[fit_mask]
        phase = fit["phase"]
        fitted = fit["slope"] * freqs + np.mean(phase - fit["slope"] * freqs)
        residual_abs_deg = np.abs(phase - fitted) * 180.0 / np.pi

        worst_local_idx = int(np.argmax(residual_abs_deg))
        worst_residual_deg = float(residual_abs_deg[worst_local_idx])

        if worst_residual_deg <= FIT_OUTLIER_RESIDUAL_DEG:
            break
        if rejected >= MAX_FIT_OUTLIER_REJECTIONS:
            break
        if np.count_nonzero(fit_mask) - 1 < MIN_FIT_TONES:
            break

        fit_indices = np.flatnonzero(fit_mask)
        fit_mask[fit_indices[worst_local_idx]] = False
        rejected += 1

    fit = fit_phase_slope(ratios, fit_mask)
    fit["fit_mask"] = fit_mask
    fit["rejected_count"] = rejected
    return fit


def estimate_length(ratios, valid_mask=None):
    fit = estimate_length_fit(ratios, valid_mask=valid_mask)
    return (
        fit["length_m"],
        fit["signed_length_m"],
        fit["tau"],
        fit["slope"],
        fit["phase"],
        fit["residual_deg"],
    )


def find_stable_window_fit(
    ratios,
    valid_mask,
    min_window_tones=MIN_WINDOW_TONES,
    min_window_bw_hz=MIN_WINDOW_BW_HZ,
    max_residual_deg=MAX_RESIDUAL_DEG,
    max_peak_residual_deg=MAX_RESIDUAL_PEAK_DEG,
    max_length_span_m=MAX_WINDOW_LENGTH_SPAN_M,
):
    valid_indices = np.flatnonzero(np.asarray(valid_mask, dtype=bool))
    candidates = []

    for start_pos in range(len(valid_indices)):
        for stop_pos in range(start_pos + min_window_tones, len(valid_indices) + 1):
            indices = valid_indices[start_pos:stop_pos]
            bandwidth_hz = TONES_HZ[indices[-1]] - TONES_HZ[indices[0]]
            if bandwidth_hz < min_window_bw_hz:
                continue

            window_mask = np.zeros(len(valid_mask), dtype=bool)
            window_mask[indices] = True
            fit = fit_phase_slope(ratios, window_mask)
            if fit["residual_deg"] > max_residual_deg:
                continue
            if fit["peak_residual_deg"] > max_peak_residual_deg:
                continue

            fit["window_mask"] = window_mask
            candidates.append(fit)

    if not candidates:
        return {
            "verdict": "UNSTABLE_PHASE",
            "candidate_count": 0,
        }

    candidates.sort(key=lambda item: (item["residual_deg"], -item["tone_count"]))
    signed_lengths = np.array([item["signed_length_m"] for item in candidates], dtype=np.float64)
    length_span_m = float(np.max(signed_lengths) - np.min(signed_lengths))

    if len(candidates) < 2 or length_span_m > max_length_span_m:
        best = candidates[0]
        return {
            "verdict": "UNSTABLE_PHASE",
            "candidate_count": len(candidates),
            "length_span_m": length_span_m,
            "best_length_m": best["length_m"],
            "best_residual_deg": best["residual_deg"],
            "best_start_hz": best["start_hz"],
            "best_stop_hz": best["stop_hz"],
        }

    best = candidates[0]
    best = dict(best)
    best["verdict"] = "OK"
    best["candidate_count"] = len(candidates)
    best["length_span_m"] = length_span_m
    return best


def find_low_band_demo_fit(
    ratios,
    valid_mask,
    start_hz=LOW_BAND_DEMO_START_HZ,
    stop_hz=LOW_BAND_DEMO_STOP_HZ,
    min_window_tones=MIN_LOW_BAND_DEMO_TONES,
    min_window_bw_hz=MIN_LOW_BAND_DEMO_BW_HZ,
    max_residual_deg=MAX_LOW_BAND_DEMO_RESIDUAL_DEG,
    max_peak_residual_deg=MAX_LOW_BAND_DEMO_PEAK_DEG,
):
    valid_mask = np.asarray(valid_mask, dtype=bool)
    band_mask = (TONES_HZ >= start_hz) & (TONES_HZ <= stop_hz)
    valid_indices = np.flatnonzero(valid_mask & band_mask)
    candidates = []

    for start_pos in range(len(valid_indices)):
        for stop_pos in range(start_pos + min_window_tones, len(valid_indices) + 1):
            indices = valid_indices[start_pos:stop_pos]
            bandwidth_hz = TONES_HZ[indices[-1]] - TONES_HZ[indices[0]]
            if bandwidth_hz < min_window_bw_hz:
                continue

            window_mask = np.zeros(len(valid_mask), dtype=bool)
            window_mask[indices] = True
            fit = fit_phase_slope(ratios, window_mask)
            if fit["residual_deg"] > max_residual_deg:
                continue
            if fit["peak_residual_deg"] > max_peak_residual_deg:
                continue

            fit["window_mask"] = window_mask
            candidates.append(fit)

    if not candidates:
        return {
            "verdict": "UNSTABLE_PHASE",
            "candidate_count": 0,
            "demo_band_start_hz": float(start_hz),
            "demo_band_stop_hz": float(stop_hz),
        }

    candidates.sort(
        key=lambda item: (
            -item["bandwidth_hz"],
            -item["tone_count"],
            item["residual_deg"],
            item["peak_residual_deg"],
        )
    )
    signed_lengths = np.array([item["signed_length_m"] for item in candidates], dtype=np.float64)

    best = dict(candidates[0])
    best["verdict"] = "OK"
    best["candidate_count"] = len(candidates)
    best["demo_band_start_hz"] = float(start_hz)
    best["demo_band_stop_hz"] = float(stop_hz)
    best["demo_length_span_m"] = float(np.max(signed_lengths) - np.min(signed_lengths))
    return best


def summarize_lengths(lengths_m, known_length_m=None):
    lengths = np.asarray(lengths_m, dtype=np.float64)
    summary = {
        "count": int(len(lengths)),
        "mean_m": float(np.mean(lengths)),
        "std_m": float(np.std(lengths, ddof=1)) if len(lengths) > 1 else 0.0,
        "span_m": float(np.max(lengths) - np.min(lengths)) if len(lengths) else 0.0,
    }
    if known_length_m is not None:
        summary["known_length_m"] = float(known_length_m)
        summary["error_m"] = float(summary["mean_m"] - known_length_m)
        summary["relative_error_pct"] = float((summary["mean_m"] - known_length_m) / known_length_m * 100.0)
    return summary


def evaluate_repeatability(summary, max_span_m=MAX_REPEAT_SPAN_M):
    if summary["count"] <= 0:
        return "NO_ACCEPTED_RUNS"
    if summary["span_m"] > max_span_m:
        return "UNSTABLE"
    return "OK"


def measure_tones(sdr):
    iq = make_multitone()

    tx_destroy(sdr)
    time.sleep(0.5)

    print("TX multitone ON")
    sdr.tx([iq, iq])
    time.sleep(1.0)

    for _ in range(3):
        _ = sdr.rx()

    print(f"Averaging {N_AVG} captures...")
    ratios_all = np.zeros((N_AVG, len(TONES_HZ)), dtype=np.complex128)
    dut_all = np.zeros((N_AVG, len(TONES_HZ)), dtype=np.complex128)
    ref_all = np.zeros((N_AVG, len(TONES_HZ)), dtype=np.complex128)

    for i in range(N_AVG):
        rx = sdr.rx()
        rx0, rx1 = unpack_rx(rx)
        for k, freq_hz in enumerate(TONES_HZ):
            h_dut = extract_tone(rx0, freq_hz)
            h_ref = extract_tone(rx1, freq_hz)
            dut_all[i, k] = h_dut
            ref_all[i, k] = h_ref
            ratios_all[i, k] = h_dut / (h_ref + 1e-30)

    dut_mean = np.mean(dut_all, axis=0)
    ref_mean = np.mean(ref_all, axis=0)
    ratios = np.mean(ratios_all, axis=0)
    dut_amp_db = np.array([db20(v) for v in dut_mean])
    ref_amp_db = np.array([db20(v) for v in ref_mean])
    return ratios, dut_amp_db, ref_amp_db


def measure_ratios(sdr):
    ratios, _dut_amp_db, _ref_amp_db = measure_tones(sdr)
    return ratios


def print_tone_table(ratios, dut_amp_db, ref_amp_db, quality):
    print()
    print("freq_MHz, dut_amp_dB, ref_amp_dB, valid, rel_phase_deg")

    for k, freq_hz in enumerate(TONES_HZ):
        rel_phase = np.angle(ratios[k], deg=True)
        valid = "Y" if quality["valid_mask"][k] else "N"
        print(
            f"{freq_hz / 1e6:8.3f}, "
            f"{dut_amp_db[k]:9.2f}, "
            f"{ref_amp_db[k]:9.2f}, "
            f"{valid:>5}, "
            f"{rel_phase:9.2f}"
        )


def print_fit_diagnostics(quality):
    if "fit_mode" not in quality:
        return

    print()
    print("Fit diagnostics")
    print(f"Fit mode                 = {quality['fit_mode']}")
    if quality.get("demo_warning"):
        print("Demo fallback            = YES (low-band)")
    if "residual_deg" in quality:
        print(f"Residual RMS             = {quality['residual_deg']:.3f} deg")
    if "peak_residual_deg" in quality:
        print(f"Peak residual            = {quality['peak_residual_deg']:.3f} deg")
    if "fit_start_hz" in quality and "fit_stop_hz" in quality:
        print(
            "Fit frequency window     = "
            f"{quality['fit_start_hz'] / 1e6:.3f} to {quality['fit_stop_hz'] / 1e6:.3f} MHz"
        )
    if "window_candidate_count" in quality:
        print(f"Window candidates        = {quality['window_candidate_count']}")
    if "window_length_span_m" in quality:
        print(f"Window length span       = {quality['window_length_span_m']:.3f} m")
    if "demo_candidate_count" in quality:
        print(f"Low-band demo candidates = {quality['demo_candidate_count']}")
    if "demo_band_start_hz" in quality and "demo_band_stop_hz" in quality:
        print(
            "Low-band demo search     = "
            f"{quality['demo_band_start_hz'] / 1e6:.3f} to "
            f"{quality['demo_band_stop_hz'] / 1e6:.3f} MHz"
        )
    if "demo_length_span_m" in quality:
        print(f"Low-band length span     = {quality['demo_length_span_m']:.3f} m")
    if "best_window_length_m" in quality:
        print(f"Best window length       = {quality['best_window_length_m']:.3f} m")
    if "best_window_start_hz" in quality and "best_window_stop_hz" in quality:
        print(
            "Best window frequency    = "
            f"{quality['best_window_start_hz'] / 1e6:.3f} to "
            f"{quality['best_window_stop_hz'] / 1e6:.3f} MHz"
        )


def save_cal(ratios, ref_length_m, path=CAL_FILE):
    data = {
        "real": [float(r.real) for r in ratios],
        "imag": [float(r.imag) for r in ratios],
        "tones_hz": [float(f) for f in TONES_HZ],
        "ref_length_m": float(ref_length_m),
    }
    with open(path, "w") as f:
        json.dump(data, f)
    print(f"Calibration saved to {path}")


def load_cal(path=CAL_FILE):
    if not os.path.exists(path):
        return None
    with open(path) as f:
        data = json.load(f)
    real = np.array(data["real"])
    imag = np.array(data["imag"])
    return {
        "ratios": real + 1j * imag,
        "ref_length_m": float(data.get("ref_length_m", 0.0)),
    }


def configure_sdr():
    print("Connecting...")
    sdr = adi.ad9361(uri=URI)

    print("Configuring 2T2R...")
    sdr.tx_enabled_channels = [0, 1]
    sdr.rx_enabled_channels = [0, 1]

    sdr.sample_rate = SAMPLE_RATE
    sdr.tx_lo = LO_HZ
    sdr.rx_lo = LO_HZ
    sdr.tx_rf_bandwidth = RF_BW
    sdr.rx_rf_bandwidth = RF_BW

    sdr.tx_hardwaregain_chan0 = TX_GAIN_DB
    sdr.tx_hardwaregain_chan1 = TX_GAIN_DB

    sdr.gain_control_mode_chan0 = "manual"
    sdr.gain_control_mode_chan1 = "manual"
    sdr.rx_hardwaregain_chan0 = RX_GAIN_DB
    sdr.rx_hardwaregain_chan1 = RX_GAIN_DB

    sdr.rx_buffer_size = N
    sdr.tx_cyclic_buffer = True
    return sdr


def run_selftest(sdr, cal_ratios=None):
    ratios, dut_amp_db, ref_amp_db = measure_tones(sdr)

    if cal_ratios is not None:
        ratios = ratios / (cal_ratios + 1e-30)

    quality = evaluate_quality(
        dut_amp_db,
        ref_amp_db,
        MIN_DUT_AMP_DB,
        MIN_REF_AMP_DB,
        MIN_VALID_TONES,
    )
    print_tone_table(ratios, dut_amp_db, ref_amp_db, quality)

    if quality["verdict"] != "OK":
        return (None, None, None, None, None, None, quality), ratios

    fit = estimate_length_fit(
        ratios,
        valid_mask=quality["valid_mask"],
    )
    quality.update(
        {
            "fit_mode": "full",
            "fit_start_hz": fit["start_hz"],
            "fit_stop_hz": fit["stop_hz"],
            "fit_tone_count": fit["tone_count"],
        }
    )
    quality = apply_fit_quality(
        quality,
        fit["residual_deg"],
        peak_residual_deg=fit["peak_residual_deg"],
    )
    if quality["verdict"] == "OK":
        print_fit_diagnostics(quality)
        return (
            fit["length_m"],
            fit["signed_length_m"],
            fit["tau"],
            fit["slope"],
            fit["phase"],
            fit["residual_deg"],
            quality,
        ), ratios

    window_fit = find_stable_window_fit(ratios, quality["valid_mask"])
    if window_fit["verdict"] == "OK":
        quality.update(
            {
                "verdict": "OK",
                "fit_mode": "window",
                "residual_deg": window_fit["residual_deg"],
                "peak_residual_deg": window_fit["peak_residual_deg"],
                "fit_start_hz": window_fit["start_hz"],
                "fit_stop_hz": window_fit["stop_hz"],
                "fit_tone_count": window_fit["tone_count"],
                "window_candidate_count": window_fit["candidate_count"],
                "window_length_span_m": window_fit["length_span_m"],
            }
        )
        print_fit_diagnostics(quality)
        return (
            window_fit["length_m"],
            window_fit["signed_length_m"],
            window_fit["tau"],
            window_fit["slope"],
            window_fit["phase"],
            window_fit["residual_deg"],
            quality,
        ), ratios

    low_band_fit = find_low_band_demo_fit(ratios, quality["valid_mask"])
    if low_band_fit["verdict"] == "OK":
        quality.update(
            {
                "verdict": "OK",
                "fit_mode": "low_band_demo",
                "demo_warning": True,
                "residual_deg": low_band_fit["residual_deg"],
                "peak_residual_deg": low_band_fit["peak_residual_deg"],
                "fit_start_hz": low_band_fit["start_hz"],
                "fit_stop_hz": low_band_fit["stop_hz"],
                "fit_tone_count": low_band_fit["tone_count"],
                "window_candidate_count": window_fit.get("candidate_count", 0),
                "demo_candidate_count": low_band_fit["candidate_count"],
                "demo_band_start_hz": low_band_fit["demo_band_start_hz"],
                "demo_band_stop_hz": low_band_fit["demo_band_stop_hz"],
                "demo_length_span_m": low_band_fit["demo_length_span_m"],
            }
        )
        print_fit_diagnostics(quality)
        return (
            low_band_fit["length_m"],
            low_band_fit["signed_length_m"],
            low_band_fit["tau"],
            low_band_fit["slope"],
            low_band_fit["phase"],
            low_band_fit["residual_deg"],
            quality,
        ), ratios

    quality.update(
        {
            "verdict": window_fit["verdict"],
            "fit_mode": "window_rejected",
            "window_candidate_count": window_fit.get("candidate_count", 0),
        }
    )
    if "length_span_m" in window_fit:
        quality["window_length_span_m"] = window_fit["length_span_m"]
    if "best_length_m" in window_fit:
        quality["best_window_length_m"] = window_fit["best_length_m"]
        quality["best_window_start_hz"] = window_fit["best_start_hz"]
        quality["best_window_stop_hz"] = window_fit["best_stop_hz"]
    print_fit_diagnostics(quality)
    return (None, None, None, None, None, None, quality), ratios


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="SDR 2T2R multitone cable length measurement")
    parser.add_argument("mode", nargs="?", default="measure", choices=["measure", "selftest", "cal"])
    parser.add_argument("ref_length_m", nargs="?", type=float)
    parser.add_argument("--runs", type=int, default=1, help="repeat measurement count")
    parser.add_argument("--known-length", type=float, default=None, help="known cable length in meters")
    return parser.parse_args(argv)


def print_repeatability_summary(ok_lengths, total_runs, known_length_m, last_quality):
    print()
    print("Repeatability summary")
    print(f"Accepted runs = {len(ok_lengths)} / {total_runs}")

    if ok_lengths:
        summary = summarize_lengths(ok_lengths, known_length_m=known_length_m)
        verdict = evaluate_repeatability(summary)
        print(f"Repeatability verdict = {verdict}")
        print(f"Mean length   = {summary['mean_m']:.3f} m")
        print(f"Std length    = {summary['std_m']:.3f} m")
        print(f"Span length   = {summary['span_m']:.3f} m")
        if known_length_m is not None:
            print(f"Known length  = {summary['known_length_m']:.3f} m")
            print(f"Error         = {summary['error_m']:.3f} m")
            print(f"Rel error     = {summary['relative_error_pct']:.2f} %")
    else:
        verdict = last_quality["verdict"] if last_quality else "NO_DATA"
        print(f"Last verdict  = {verdict}")


def main(argv=None):
    args = parse_args(argv)
    if args.runs < 1:
        raise SystemExit("--runs must be at least 1")

    sdr = configure_sdr()

    try:
        if args.mode == "cal":
            if args.ref_length_m is None:
                raise SystemExit("Usage: python pluto_2x2_multitone_selftest.py cal <ref_length_m>")
            ref_length_m = args.ref_length_m

            print()
            print("=== REFERENCE CABLE CALIBRATION ===")
            print(f"Reference cable length = {ref_length_m:.3f} m")
            print("Connect DUT path WITH the known reference cable:")
            print("TX1A -> SMA -> adapter -> [REFERENCE CABLE] -> adapter -> SMA -> RX1A")
            print("TX2A -> RX2A (reference, unchanged)")
            print()
            input("Press Enter to calibrate...")

            result, ratios = run_selftest(sdr, cal_ratios=None)
            length_m, _signed_length_m, tau, _slope, _phase, residual_deg, quality = result
            if quality["verdict"] != "OK":
                print()
                print(f"Calibration rejected: {quality['verdict']}")
                print(f"Valid tones = {quality['valid_count']} / {len(TONES_HZ)}")
                raise SystemExit(1)

            save_cal(ratios, ref_length_m)
            print()
            print(f"Quality verdict            = {quality['verdict']}")
            print(f"Valid tones                = {quality['valid_count']} / {len(TONES_HZ)}")
            print(f"Fit residual RMS           = {residual_deg:.3f} deg")
            print(f"Reference path delay (raw)  = {tau * 1e9:.3f} ns")
            print(f"Reference path length (raw) = {length_m:.3f} m")
            print("Calibration done. Now run: python pluto_2x2_multitone_selftest.py measure")

        elif args.mode == "selftest":
            print()
            print("=== DUAL LOOPBACK SELF-TEST (raw, no calibration) ===")
            print("Connect dual loopback:")
            print("TX1A -> short SMA -> RX1A")
            print("TX2A -> SMA-REF  -> RX2A")
            print()
            input("Press Enter to start self-test...")

            ok_lengths = []
            last_quality = None
            for run_idx in range(args.runs):
                print()
                print(f"Run {run_idx + 1} / {args.runs}")
                result, _ratios = run_selftest(sdr, cal_ratios=None)
                length_m, _signed_length_m, tau, _slope, _phase, residual_deg, quality = result
                last_quality = quality

                print()
                print("========================================")
                print("Self-test result (raw)")
                print("========================================")
                print(f"Quality verdict              = {quality['verdict']}")
                print(f"Valid tones                  = {quality['valid_count']} / {len(TONES_HZ)}")
                if quality["verdict"] == "OK":
                    ok_lengths.append(length_m)
                    print(f"Fit residual RMS             = {residual_deg:.3f} deg")
                    print(f"Extra delay between channels = {tau * 1e9:.3f} ns")
                    print(f"Equivalent length offset     = {length_m:.3f} m")
                    if length_m < 1.0:
                        print("PASS - channels are balanced")
                    else:
                        print("WARN - large offset, check wiring")
                else:
                    print(f"Run rejected                 = {quality['verdict']}")
                    print("Equivalent length offset     = INVALID")
                print("========================================")

            print_repeatability_summary(ok_lengths, args.runs, args.known_length, last_quality)

        else:
            cal_data = load_cal()
            if cal_data is not None:
                cal_ratios = cal_data["ratios"]
                ref_length_m = cal_data["ref_length_m"]
                print(f"Loaded calibration from {CAL_FILE}")
                print(f"Reference cable length = {ref_length_m:.3f} m")
            else:
                cal_ratios = None
                ref_length_m = 0.0
                print("No calibration file found, measuring raw.")

            print()
            print("Connect cable under test:")
            print("TX1A -> SMA -> adapter -> [CABLE] -> adapter -> SMA -> RX1A")
            print("TX2A -> RX2A (reference, unchanged)")
            print()
            input("Press Enter to measure...")

            ok_lengths = []
            last_quality = None
            for run_idx in range(args.runs):
                print()
                print(f"Run {run_idx + 1} / {args.runs}")
                result, _ratios = run_selftest(sdr, cal_ratios)
                length_m, signed_length_m, tau, _slope, _phase, residual_deg, quality = result
                last_quality = quality

                print()
                print("========================================")
                cal_str = " (calibrated)" if cal_ratios is not None else " (raw)"
                print(f"Result{cal_str}")
                print("========================================")
                print(f"Quality verdict                  = {quality['verdict']}")
                print(f"Valid tones                      = {quality['valid_count']} / {len(TONES_HZ)}")
                print(f"DUT amplitude range              = {quality['dut_min_db']:.2f} to {quality['dut_max_db']:.2f} dB")
                print(f"REF amplitude range              = {quality['ref_min_db']:.2f} to {quality['ref_max_db']:.2f} dB")
                if quality["verdict"] == "OK":
                    print(f"Fit residual RMS                 = {residual_deg:.3f} deg")
                    print(f"Extra delay relative to reference = {tau * 1e9:.3f} ns")
                    print(f"Delta length relative to ref      = {signed_length_m:.3f} m")
                    if cal_ratios is not None:
                        reported_length_m = ref_length_m + signed_length_m
                        print(f"Estimated absolute cable length   = {reported_length_m:.3f} m")
                    else:
                        reported_length_m = length_m
                        print(f"Estimated cable length            = {reported_length_m:.3f} m")
                    ok_lengths.append(reported_length_m)
                else:
                    print(f"Run rejected                     = {quality['verdict']}")
                    print("Estimated cable length            = INVALID")
                print("========================================")

            print_repeatability_summary(ok_lengths, args.runs, args.known_length, last_quality)
    finally:
        print("Stopping TX...")
        tx_destroy(sdr)
        print("Done.")


if __name__ == "__main__":
    main()
