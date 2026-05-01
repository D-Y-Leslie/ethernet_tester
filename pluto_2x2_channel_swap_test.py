import argparse
import csv
import math
import pathlib
import time
from datetime import datetime
from typing import NamedTuple

import numpy as np


URI = "ip:192.168.2.1"

C = 299_792_458.0
VF_CABLE = 0.66

LO_HZ = int(400e6)
SAMPLE_RATE = int(30.72e6)
RF_BW = int(28e6)
N = 131072
N_AVG = 16
N_STAGE_RUNS = 3
DEFAULT_ZERO_OFFSET_M = 0.055

TX_GAIN_DB = -45
RX_GAIN_DB = 10
TOTAL_SCALE = 0.06

MIN_DUT_AMP_DB = -30.0
MIN_REF_AMP_DB = -30.0
MIN_VALID_TONES = 12
MAX_RESIDUAL_DEG_RMS = 20.0
MAX_STAGE_SPAN_M = 0.30


def make_bin_aligned_tones(sample_rate, n, start_hz, stop_hz, count):
    bin_hz = sample_rate / n
    start_bin = int(round(start_hz / bin_hz))
    stop_bin = int(round(stop_hz / bin_hz))
    bins = np.linspace(start_bin, stop_bin, count)
    bins = np.unique(np.round(bins).astype(int))
    return bins.astype(np.float64) * bin_hz


TONES_HZ = make_bin_aligned_tones(SAMPLE_RATE, N, 0.5e6, 14e6, 55)


class Stage(NamedTuple):
    name: str
    title: str
    instructions: tuple[str, ...]
    expect_near_zero: bool


class FitResult(NamedTuple):
    tau_ns: float
    signed_length_m: float
    length_m: float
    slope_rad_per_hz: float
    intercept_rad: float
    residual_deg_rms: float


class RunResult(NamedTuple):
    stage: str
    run: int
    tau_ns: float
    signed_length_m: float
    length_m: float
    valid_tones: int
    min_dut_amp_db: float
    min_ref_amp_db: float
    residual_deg_rms: float
    verdict: str


def db20(value):
    value = abs(value)
    if value <= 0.0:
        return -600.0
    return 20.0 * np.log10(value)


def apply_zero_offset(signed_length_m, zero_offset_m):
    corrected_signed = signed_length_m - zero_offset_m
    return corrected_signed, abs(corrected_signed)


def default_stages():
    return [
        Stage(
            name="normal_loopback",
            title="Normal 2T2R loopback",
            instructions=(
                "TX1A -> RX1A",
                "TX2A -> RX2A",
                "Use the two short SMA paths without RJ45 cable.",
            ),
            expect_near_zero=True,
        ),
        Stage(
            name="rx_swapped_loopback",
            title="RX channel swapped loopback",
            instructions=(
                "TX1A -> RX2A",
                "TX2A -> RX1A",
                "This exposes TX/RX channel bias and sign changes.",
            ),
            expect_near_zero=True,
        ),
        Stage(
            name="sma_a_dut_b_ref",
            title="SMA A on DUT, SMA B on REF",
            instructions=(
                "SMA A: TX1A -> RX1A",
                "SMA B: TX2A -> RX2A",
                "Mark the physical SMA cables before this run.",
            ),
            expect_near_zero=True,
        ),
        Stage(
            name="sma_b_dut_a_ref",
            title="SMA B on DUT, SMA A on REF",
            instructions=(
                "SMA B: TX1A -> RX1A",
                "SMA A: TX2A -> RX2A",
                "The signed result should roughly flip from the previous stage.",
            ),
            expect_near_zero=True,
        ),
        Stage(
            name="dut_forward",
            title="DUT cable forward",
            instructions=(
                "DUT path: TX1A -> SMA/RJ45 fixture -> cable -> SMA/RJ45 fixture -> RX1A",
                "Reference path: TX2A -> RX2A with the chosen reference SMA cable.",
                "Keep the reference SMA cable fixed for all DUT cable runs.",
            ),
            expect_near_zero=False,
        ),
        Stage(
            name="dut_reversed",
            title="DUT cable reversed",
            instructions=(
                "Reverse the same RJ45 cable end-to-end on the DUT path.",
                "Keep TX1A/RX1A and TX2A/RX2A channel roles unchanged.",
                "This checks connector/fixture direction sensitivity.",
            ),
            expect_near_zero=False,
        ),
    ]


def make_multitone():
    t = np.arange(N) / SAMPLE_RATE
    sig = np.zeros(N, dtype=np.complex128)
    rng = np.random.default_rng(1234)
    phases = rng.uniform(0, 2.0 * np.pi, len(TONES_HZ))

    for freq_hz, phase in zip(TONES_HZ, phases):
        sig += np.exp(1j * (2.0 * np.pi * freq_hz * t + phase))

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
    ref = np.exp(-1j * 2.0 * np.pi * freq_hz * t)
    return np.mean(rx * ref)


def fit_delay(tones_hz, ratios, vf=VF_CABLE, valid_mask=None):
    tones_hz = np.asarray(tones_hz, dtype=np.float64)
    ratios = np.asarray(ratios, dtype=np.complex128)
    finite_mask = np.isfinite(tones_hz) & np.isfinite(ratios.real) & np.isfinite(ratios.imag)
    if valid_mask is None:
        valid_mask = finite_mask
    else:
        valid_mask = np.asarray(valid_mask, dtype=bool) & finite_mask

    if int(np.count_nonzero(valid_mask)) < 2:
        raise ValueError("At least two valid tones are required for delay fitting.")

    freqs = tones_hz[valid_mask]
    phase = np.unwrap(np.angle(ratios[valid_mask]))
    slope, intercept = np.polyfit(freqs, phase, 1)
    fitted = slope * freqs + intercept
    residual = phase - fitted
    residual_deg_rms = math.sqrt(float(np.mean(np.square(np.rad2deg(residual)))))

    tau_s = -slope / (2.0 * np.pi)
    signed_length_m = tau_s * vf * C
    return FitResult(
        tau_ns=tau_s * 1e9,
        signed_length_m=signed_length_m,
        length_m=abs(signed_length_m),
        slope_rad_per_hz=float(slope),
        intercept_rad=float(intercept),
        residual_deg_rms=residual_deg_rms,
    )


def quality_verdict(
    dut_amp_db,
    ref_amp_db,
    residual_deg_rms,
    min_dut_amp_db=MIN_DUT_AMP_DB,
    min_ref_amp_db=MIN_REF_AMP_DB,
    min_valid_tones=MIN_VALID_TONES,
    max_residual_deg_rms=MAX_RESIDUAL_DEG_RMS,
):
    dut_amp_db = np.asarray(dut_amp_db, dtype=np.float64)
    ref_amp_db = np.asarray(ref_amp_db, dtype=np.float64)
    valid_mask = (dut_amp_db >= min_dut_amp_db) & (ref_amp_db >= min_ref_amp_db)
    valid_count = int(np.count_nonzero(valid_mask))

    if dut_amp_db.size == 0 or float(np.max(dut_amp_db)) < min_dut_amp_db:
        verdict = "LOW_SIGNAL"
    elif valid_count < min_valid_tones:
        verdict = "FEW_VALID_TONES"
    elif math.isfinite(residual_deg_rms) and residual_deg_rms > max_residual_deg_rms:
        verdict = "HIGH_RESIDUAL"
    else:
        verdict = "OK"

    return {
        "verdict": verdict,
        "valid_count": valid_count,
        "valid_mask": valid_mask,
    }


def measure_ratios(sdr):
    ratios_all = np.zeros((N_AVG, len(TONES_HZ)), dtype=np.complex128)
    dut_all = np.zeros((N_AVG, len(TONES_HZ)), dtype=np.complex128)
    ref_all = np.zeros((N_AVG, len(TONES_HZ)), dtype=np.complex128)

    for avg_idx in range(N_AVG):
        rx = sdr.rx()
        rx0, rx1 = unpack_rx(rx)
        for tone_idx, freq_hz in enumerate(TONES_HZ):
            h_dut = extract_tone(rx0, freq_hz)
            h_ref = extract_tone(rx1, freq_hz)
            dut_all[avg_idx, tone_idx] = h_dut
            ref_all[avg_idx, tone_idx] = h_ref
            ratios_all[avg_idx, tone_idx] = h_dut / (h_ref + 1e-30)

    ratios = np.mean(ratios_all, axis=0)
    dut_amp_db = np.array([db20(v) for v in np.mean(np.abs(dut_all), axis=0)])
    ref_amp_db = np.array([db20(v) for v in np.mean(np.abs(ref_all), axis=0)])
    return ratios, dut_amp_db, ref_amp_db


def tx_destroy(sdr):
    try:
        sdr.tx_destroy_buffer()
    except Exception:
        pass


def configure_sdr(uri):
    import adi

    print(f"Connecting to {uri}...")
    sdr = adi.ad9361(uri=uri)

    print("Configuring AD9361 2T2R...")
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


def start_tx(sdr):
    iq = make_multitone()
    tx_destroy(sdr)
    time.sleep(0.5)
    sdr.tx([iq, iq])
    time.sleep(1.0)
    for _ in range(3):
        _ = sdr.rx()


def measure_once(sdr, stage_name, run_index, vf):
    ratios, dut_amp_db, ref_amp_db = measure_ratios(sdr)
    signal_quality = quality_verdict(dut_amp_db, ref_amp_db, residual_deg_rms=float("nan"))
    fit_mask = signal_quality["valid_mask"]
    if int(np.count_nonzero(fit_mask)) < 2:
        fit_mask = np.isfinite(ratios.real) & np.isfinite(ratios.imag)

    fit = fit_delay(TONES_HZ, ratios, vf=vf, valid_mask=fit_mask)
    quality = quality_verdict(dut_amp_db, ref_amp_db, fit.residual_deg_rms)

    return RunResult(
        stage=stage_name,
        run=run_index,
        tau_ns=fit.tau_ns,
        signed_length_m=fit.signed_length_m,
        length_m=fit.length_m,
        valid_tones=quality["valid_count"],
        min_dut_amp_db=float(np.min(dut_amp_db)),
        min_ref_amp_db=float(np.min(ref_amp_db)),
        residual_deg_rms=fit.residual_deg_rms,
        verdict=quality["verdict"],
    )


def summarize_runs(rows, max_span_m=MAX_STAGE_SPAN_M, zero_offset_m=0.0):
    if not rows:
        raise ValueError("Cannot summarize an empty run list.")

    signed_lengths = np.array([row.signed_length_m for row in rows], dtype=np.float64)
    corrected_signed_lengths = signed_lengths - zero_offset_m
    tau_ns = np.array([row.tau_ns for row in rows], dtype=np.float64)
    residuals = np.array([row.residual_deg_rms for row in rows], dtype=np.float64)
    span = float(np.max(signed_lengths) - np.min(signed_lengths))
    corrected_span = float(np.max(corrected_signed_lengths) - np.min(corrected_signed_lengths))
    verdicts = {row.verdict for row in rows}

    if verdicts != {"OK"}:
        verdict = "+".join(sorted(verdicts))
    elif span > max_span_m:
        verdict = "UNSTABLE"
    else:
        verdict = "OK"

    return {
        "stage": rows[0].stage,
        "runs": len(rows),
        "mean_tau_ns": float(np.mean(tau_ns)),
        "mean_signed_length_m": float(np.mean(signed_lengths)),
        "mean_length_m": float(np.mean(np.abs(signed_lengths))),
        "mean_corrected_signed_length_m": float(np.mean(corrected_signed_lengths)),
        "mean_corrected_length_m": float(np.mean(np.abs(corrected_signed_lengths))),
        "span_signed_length_m": span,
        "span_corrected_signed_length_m": corrected_span,
        "min_valid_tones": min(row.valid_tones for row in rows),
        "max_residual_deg_rms": float(np.max(residuals)),
        "verdict": verdict,
    }


def result_to_dict(row, zero_offset_m=0.0):
    corrected_signed, corrected_abs = apply_zero_offset(row.signed_length_m, zero_offset_m)
    return {
        "stage": row.stage,
        "run": row.run,
        "tau_ns": f"{row.tau_ns:.6f}",
        "signed_length_m": f"{row.signed_length_m:.6f}",
        "length_m": f"{row.length_m:.6f}",
        "zero_offset_m": f"{zero_offset_m:.6f}",
        "corrected_signed_length_m": f"{corrected_signed:.6f}",
        "corrected_length_m": f"{corrected_abs:.6f}",
        "valid_tones": row.valid_tones,
        "min_dut_amp_db": f"{row.min_dut_amp_db:.2f}",
        "min_ref_amp_db": f"{row.min_ref_amp_db:.2f}",
        "residual_deg_rms": f"{row.residual_deg_rms:.3f}",
        "verdict": row.verdict,
    }


def default_output_path():
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    return pathlib.Path.cwd() / f"channel_swap_results_{stamp}.csv"


def write_results_csv(path, rows, zero_offset_m=0.0):
    path = pathlib.Path(path)
    fieldnames = [
        "stage",
        "run",
        "tau_ns",
        "signed_length_m",
        "length_m",
        "zero_offset_m",
        "corrected_signed_length_m",
        "corrected_length_m",
        "valid_tones",
        "min_dut_amp_db",
        "min_ref_amp_db",
        "residual_deg_rms",
        "verdict",
    ]
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(result_to_dict(row, zero_offset_m=zero_offset_m))
    return path


def print_stage(stage):
    print()
    print("=" * 72)
    print(stage.title)
    print("=" * 72)
    for line in stage.instructions:
        print(f"- {line}")
    print()


def stage_start_prompt(stage):
    return (
        "\n"
        f"Ready for test: {stage.title}\n"
        "TX output is OFF. change hardware now, then keep your hands off the wiring.\n"
        "Press Enter to START this test..."
    )


def print_run(row, zero_offset_m=0.0):
    corrected_signed, _corrected_abs = apply_zero_offset(row.signed_length_m, zero_offset_m)
    print(
        f"{row.stage:20s} run {row.run}: "
        f"tau={row.tau_ns:9.3f} ns, "
        f"signed_len={row.signed_length_m:8.3f} m, "
        f"corr_signed={corrected_signed:8.3f} m, "
        f"abs_len={row.length_m:8.3f} m, "
        f"valid={row.valid_tones:2d}, "
        f"dut_min={row.min_dut_amp_db:7.2f} dB, "
        f"ref_min={row.min_ref_amp_db:7.2f} dB, "
        f"resid={row.residual_deg_rms:6.2f} deg, "
        f"{row.verdict}"
    )


def print_summary(summary):
    print()
    print("Stage summary:")
    print(f"  stage                = {summary['stage']}")
    print(f"  runs                 = {summary['runs']}")
    print(f"  mean tau             = {summary['mean_tau_ns']:.3f} ns")
    print(f"  mean signed length   = {summary['mean_signed_length_m']:.3f} m")
    print(f"  corrected signed len = {summary['mean_corrected_signed_length_m']:.3f} m")
    print(f"  signed length span   = {summary['span_signed_length_m']:.3f} m")
    print(f"  corrected span       = {summary['span_corrected_signed_length_m']:.3f} m")
    print(f"  min valid tones      = {summary['min_valid_tones']}")
    print(f"  max residual         = {summary['max_residual_deg_rms']:.2f} deg")
    print(f"  verdict              = {summary['verdict']}")


def run_stage(sdr, stage, runs, vf, zero_offset_m):
    print_stage(stage)
    tx_destroy(sdr)
    input(stage_start_prompt(stage))
    print("TX multitone ON")
    start_tx(sdr)

    rows = []
    for run_index in range(1, runs + 1):
        row = measure_once(sdr, stage.name, run_index, vf)
        rows.append(row)
        print_run(row, zero_offset_m=zero_offset_m)

    summary = summarize_runs(rows, zero_offset_m=zero_offset_m)
    print_summary(summary)
    tx_destroy(sdr)
    print("TX output is OFF. You may change hardware before the next test prompt.")
    return rows


def parse_args():
    parser = argparse.ArgumentParser(
        description="Interactive AD9361 2T2R channel/SMA swap test for cable-length error analysis."
    )
    parser.add_argument("--uri", default=URI, help=f"pyadi-iio URI, default: {URI}")
    parser.add_argument("--runs", type=int, default=N_STAGE_RUNS, help="Repeated measurements per wiring stage.")
    parser.add_argument("--vf", type=float, default=VF_CABLE, help="Velocity factor used for length conversion.")
    parser.add_argument(
        "--zero-offset-m",
        type=float,
        default=DEFAULT_ZERO_OFFSET_M,
        help=f"Signed loopback zero offset to subtract from results, default: {DEFAULT_ZERO_OFFSET_M:.3f} m.",
    )
    parser.add_argument("--output", default=None, help="CSV output path. Default: channel_swap_results_TIMESTAMP.csv")
    parser.add_argument(
        "--stages",
        nargs="*",
        default=None,
        help="Optional stage names to run. Default: all stages in the recommended order.",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    stages = default_stages()
    if args.stages:
        wanted = set(args.stages)
        stages = [stage for stage in stages if stage.name in wanted]
        missing = wanted - {stage.name for stage in stages}
        if missing:
            raise SystemExit(f"Unknown stage name(s): {', '.join(sorted(missing))}")

    print("AD9361 2T2R channel/SMA swap experiment")
    print()
    print("Recommended stage order:")
    for stage in stages:
        mark = "near-zero" if stage.expect_near_zero else "DUT-length"
        print(f"- {stage.name:20s} ({mark})")
    print()
    print("Keep signed_length_m. A sign flip after swapping cables/channels is useful evidence.")
    print(f"Using zero offset correction: {args.zero_offset_m:.3f} m")
    input("Press Enter after checking that RF cables are safe to connect...")

    sdr = configure_sdr(args.uri)
    all_rows = []

    try:
        for stage in stages:
            all_rows.extend(run_stage(sdr, stage, args.runs, args.vf, args.zero_offset_m))
    finally:
        print()
        print("Stopping TX...")
        tx_destroy(sdr)

    output = pathlib.Path(args.output) if args.output else default_output_path()
    saved = write_results_csv(output, all_rows, zero_offset_m=args.zero_offset_m)
    print(f"Saved CSV: {saved}")
    print("Done.")


if __name__ == "__main__":
    main()
