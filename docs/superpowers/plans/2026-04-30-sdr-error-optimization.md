# SDR Error Optimization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Optimize the SDR 2T2R cable-length script so it reports trustworthy length estimates for the contest-basic target range instead of returning low-SNR phase-fit artifacts.

**Architecture:** Keep the working 2T2R multitone architecture: TX1/RX1 is the DUT cable path and TX2/RX2 is the reference loopback path. Add signal-quality gates, FFT-bin-aligned tones, complex-ratio averaging, robust phase fitting, repeatability reporting, and experiment logging around the existing script. Do not change the ESP32/TJC application until the PC-side SDR measurement is stable.

**Tech Stack:** Python 3, pyadi-iio, NumPy, unittest, local Markdown experiment logs.

---

## Current Evidence

Existing files:

- Main SDR script: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- SDR tests: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`
- Debug notes: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\SDR方案Debug.md`
- Experiment log: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\docs\experiments\sdr_length_log.md`

Current script already includes these partial optimizations:

```python
RF_BW = int(28e6)
TONES_HZ = np.linspace(0.5e6, 14e6, 55)
N = 131072
N_AVG = 16
TOTAL_SCALE = 0.06
```

New physical-length estimate from user:

```text
Long coiled cable physical estimate = 4 * 1.10m + 3 * 1.12m + 0.15m = 7.91m
Current SDR estimate for that cable = 24.717m
Absolute error = 16.807m
Relative error = 212.5%
Measured / physical ratio = 3.12x
```

Interpretation:

```text
The long-cable trend is correct, but the 24.717m result is not a calibrated length.
The DUT tones were about -60.94dB to -40.11dB, so the phase fit was low-SNR and should have been rejected or marked LOW_SIGNAL/UNSTABLE.
```

Target aligned to contest-basic requirements:

```text
Primary target: 10m to 50m cable length range
Primary error target: absolute relative error <= 5%
Display/decision target: reject low-quality measurements instead of reporting a false precise length
```

The 7.91m cable is below the 10m basic range, but it is still useful as a low-end sanity check.

## File Structure

- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
  - Add reusable measurement helpers, quality gates, robust fit, CLI options, and repeatability summary.
- Modify: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`
  - Add unit tests for bin-aligned tones, valid-tone filtering, robust fitting, and quality verdicts.
- Modify: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\docs\experiments\sdr_length_log.md`
  - Add the 7.91m physical estimate and revised interpretation of the 24.717m measurement.
- Create: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\docs\experiments\sdr_error_optimization_trials.md`
  - Record each optimization trial and the accepted/rejected verdict.

Version-control note:

```text
At the previous check, neither the Arduino project directory nor the Python experiment directory was a git repository.
Run git checkpoint steps only if `git rev-parse --show-toplevel` succeeds in that directory.
```

---

### Task 1: Record the Revised Long-Cable Ground Truth

**Files:**
- Modify: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\docs\experiments\sdr_length_log.md`

- [ ] **Step 1: Append the revised physical estimate**

Add this block after the existing "Long Coiled Cable Trial" section:

```markdown
### Revised Physical Estimate

User compared the long coiled cable against labeled short cables:

```text
4 * 1.10m + 3 * 1.12m + 0.15m = 7.91m
```

The previous SDR result was:

```text
Estimated extra length = 24.717m
```

Revised interpretation:

```text
This run proves the long cable is much longer than the 1m cables, but the numeric result is rejected for calibration because DUT tones were below the reliable signal-quality gate.
The optimization target is to make the script output LOW_SIGNAL or UNSTABLE for this condition, unless a lower-frequency profile yields a stable fit.
```
```

- [ ] **Step 2: Verify the log contains the revised estimate**

Run:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
Select-String -Path "docs\experiments\sdr_length_log.md" -Pattern "7.91m","24.717m","LOW_SIGNAL"
```

Expected:

```text
At least one match for each pattern.
```

---

### Task 2: Add Pure Helper Functions for Tone Bins and dB Conversion

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Modify: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`

- [ ] **Step 1: Write failing tests for bin-aligned tones**

Add to `test_pluto_2x2_multitone_selftest.py`:

```python
    def test_make_bin_aligned_tones_returns_integer_fft_bins(self):
        tones = selftest.make_bin_aligned_tones(
            sample_rate=30_720_000,
            n=131072,
            start_hz=500_000,
            stop_hz=14_000_000,
            count=55,
        )

        bin_hz = 30_720_000 / 131072
        bins = tones / bin_hz

        self.assertEqual(len(tones), 55)
        self.assertAlmostEqual(float(tones[0]), round(500_000 / bin_hz) * bin_hz)
        self.assertAlmostEqual(float(tones[-1]), round(14_000_000 / bin_hz) * bin_hz)
        self.assertTrue(np.allclose(bins, np.round(bins)))
        self.assertTrue(np.all(np.diff(tones) > 0))

    def test_db20_handles_zero_without_inf(self):
        self.assertEqual(selftest.db20(0.0), -600.0)
        self.assertAlmostEqual(selftest.db20(1.0), 0.0)
        self.assertAlmostEqual(selftest.db20(10.0), 20.0)
```

- [ ] **Step 2: Run tests and verify failure**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
FAILED
AttributeError: module 'pluto_2x2_multitone_selftest' has no attribute 'make_bin_aligned_tones'
```

- [ ] **Step 3: Add helper functions**

Add these functions after constants in `pluto_2x2_multitone_selftest.py`:

```python
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
```

- [ ] **Step 4: Replace `TONES_HZ` with bin-aligned tones**

Change:

```python
TONES_HZ = np.linspace(0.5e6, 14e6, 55)
```

to:

```python
TONES_HZ = make_bin_aligned_tones(SAMPLE_RATE, N, 0.5e6, 14e6, 55)
```

If the helper must be defined before `TONES_HZ`, move the helper functions above the `TONES_HZ` assignment.

- [ ] **Step 5: Run tests and verify pass**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
OK
```

---

### Task 3: Add Signal-Quality Gates and Verdicts

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Modify: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`

- [ ] **Step 1: Add failing tests for quality verdicts**

Add:

```python
    def test_evaluate_quality_accepts_enough_valid_tones(self):
        dut_amp_db = np.array([-20.0, -25.0, -28.0, -29.0])
        ref_amp_db = np.array([-5.0, -5.0, -6.0, -6.0])

        result = selftest.evaluate_quality(
            dut_amp_db,
            ref_amp_db,
            min_dut_amp_db=-30.0,
            min_ref_amp_db=-30.0,
            min_valid_tones=3,
        )

        self.assertEqual(result["verdict"], "OK")
        self.assertEqual(result["valid_count"], 4)
        self.assertTrue(np.all(result["valid_mask"]))

    def test_evaluate_quality_rejects_low_dut_signal(self):
        dut_amp_db = np.array([-60.0, -50.0, -45.0, -42.0])
        ref_amp_db = np.array([-4.0, -4.0, -4.0, -4.0])

        result = selftest.evaluate_quality(
            dut_amp_db,
            ref_amp_db,
            min_dut_amp_db=-30.0,
            min_ref_amp_db=-30.0,
            min_valid_tones=3,
        )

        self.assertEqual(result["verdict"], "LOW_SIGNAL")
        self.assertEqual(result["valid_count"], 0)
        self.assertFalse(np.any(result["valid_mask"]))
```

- [ ] **Step 2: Run tests and verify failure**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
FAILED
AttributeError: module 'pluto_2x2_multitone_selftest' has no attribute 'evaluate_quality'
```

- [ ] **Step 3: Add constants and quality function**

Add constants:

```python
MIN_DUT_AMP_DB = -30.0
MIN_REF_AMP_DB = -30.0
MIN_VALID_TONES = 12
```

Add function:

```python
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
```

- [ ] **Step 4: Run tests and verify pass**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
OK
```

---

### Task 4: Fit Only Valid Tones and Report Residuals

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Modify: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`

- [ ] **Step 1: Add failing tests for valid-tone fitting**

Add:

```python
    def test_estimate_length_uses_valid_mask(self):
        length_m = 2.5
        tau = length_m / (selftest.VF_CABLE * selftest.C)
        ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * tau)
        bad = ratios.copy()
        bad[0] = np.exp(1j * 2.5)
        bad[-1] = np.exp(-1j * 1.7)
        valid_mask = np.ones(len(bad), dtype=bool)
        valid_mask[0] = False
        valid_mask[-1] = False

        estimated, tau_extra, _slope, _phase, residual_deg = selftest.estimate_length(
            bad,
            valid_mask=valid_mask,
        )

        self.assertAlmostEqual(estimated, length_m, places=6)
        self.assertAlmostEqual(tau_extra, tau, places=15)
        self.assertLess(residual_deg, 1e-6)
```

- [ ] **Step 2: Run tests and verify failure**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
FAILED
TypeError: estimate_length() got an unexpected keyword argument 'valid_mask'
```

- [ ] **Step 3: Replace `estimate_length`**

Replace the current function with:

```python
def estimate_length(ratios, valid_mask=None):
    ratios = np.asarray(ratios)
    if valid_mask is None:
        valid_mask = np.ones(len(ratios), dtype=bool)
    valid_mask = np.asarray(valid_mask, dtype=bool)

    if np.count_nonzero(valid_mask) < 2:
        raise ValueError("Need at least two valid tones for phase-slope fitting.")

    freqs = TONES_HZ[valid_mask]
    phase = np.unwrap(np.angle(ratios[valid_mask]))
    slope, intercept = np.polyfit(freqs, phase, 1)

    fitted = slope * freqs + intercept
    residual = phase - fitted
    residual_deg = float(np.sqrt(np.mean(np.square(residual))) * 180.0 / np.pi)

    tau = -slope / (2 * np.pi)
    length_m = abs(tau) * VF_CABLE * C
    return length_m, tau, slope, phase, residual_deg
```

- [ ] **Step 4: Update existing tests for new return shape**

Change:

```python
estimated, tau_extra, _slope, _phase = selftest.estimate_length(ratios)
```

to:

```python
estimated, tau_extra, _slope, _phase, residual_deg = selftest.estimate_length(ratios)
self.assertLess(residual_deg, 1e-6)
```

- [ ] **Step 5: Run tests and verify pass**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
OK
```

---

### Task 5: Make `run_selftest` Use Mean Amplitudes and Quality Gate

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`

- [ ] **Step 1: Replace capture aggregation logic**

Inside `run_selftest`, replace the current `ratios_all` block with:

```python
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
    quality = evaluate_quality(
        dut_amp_db,
        ref_amp_db,
        MIN_DUT_AMP_DB,
        MIN_REF_AMP_DB,
        MIN_VALID_TONES,
    )
```

- [ ] **Step 2: Replace the print loop**

Replace the extra `rx = sdr.rx()` print section with:

```python
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
```

- [ ] **Step 3: Return quality-aware fit result**

Change:

```python
    return estimate_length(ratios)
```

to:

```python
    if quality["verdict"] != "OK":
        return None, None, None, None, None, quality

    length_m, tau, slope, phase, residual_deg = estimate_length(
        ratios,
        valid_mask=quality["valid_mask"],
    )
    quality["residual_deg"] = residual_deg
    return length_m, tau, slope, phase, residual_deg, quality
```

- [ ] **Step 4: Update `main` result handling**

Replace:

```python
        length_m, tau, _slope, _phase = run_selftest(sdr)
```

with:

```python
        length_m, tau, _slope, _phase, residual_deg, quality = run_selftest(sdr)
```

Replace final print block with:

```python
        print()
        print("========================================")
        print("Measurement result")
        print("========================================")
        print(f"Quality verdict                  = {quality['verdict']}")
        print(f"Valid tones                      = {quality['valid_count']} / {len(TONES_HZ)}")
        print(f"DUT amplitude range              = {quality['dut_min_db']:.2f} to {quality['dut_max_db']:.2f} dB")
        print(f"REF amplitude range              = {quality['ref_min_db']:.2f} to {quality['ref_max_db']:.2f} dB")
        if quality["verdict"] == "OK":
            print(f"Fit residual RMS                 = {residual_deg:.3f} deg")
            print(f"Extra delay relative to reference = {tau * 1e9:.3f} ns")
            print(f"Estimated extra length            = {length_m:.3f} m")
        else:
            print("Estimated extra length            = INVALID")
        print("========================================")
```

- [ ] **Step 5: Run tests**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
OK
```

---

### Task 6: Add Repeatability Mode

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Modify: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`

- [ ] **Step 1: Add tests for repeatability summary**

Add:

```python
    def test_summarize_lengths_reports_mean_span_and_relative_error(self):
        summary = selftest.summarize_lengths([1.009, 1.122, 1.013], known_length_m=1.12)

        self.assertAlmostEqual(summary["mean_m"], 1.048, places=3)
        self.assertAlmostEqual(summary["span_m"], 0.113, places=3)
        self.assertAlmostEqual(summary["relative_error_pct"], -6.428571, places=5)
        self.assertEqual(summary["count"], 3)
```

- [ ] **Step 2: Run tests and verify failure**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
FAILED
AttributeError: module 'pluto_2x2_multitone_selftest' has no attribute 'summarize_lengths'
```

- [ ] **Step 3: Add summary helper**

Add:

```python
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
```

- [ ] **Step 4: Add CLI options**

Add imports:

```python
import argparse
```

Add:

```python
def parse_args():
    parser = argparse.ArgumentParser(description="SDR 2T2R multitone cable length measurement")
    parser.add_argument("--runs", type=int, default=1, help="repeat measurement count")
    parser.add_argument("--known-length", type=float, default=None, help="known cable length in meters")
    return parser.parse_args()
```

At start of `main()`, add:

```python
    args = parse_args()
```

Wrap the call to `run_selftest` so repeated OK lengths are summarized:

```python
        ok_lengths = []
        last_quality = None
        for run_idx in range(args.runs):
            print()
            print(f"Run {run_idx + 1} / {args.runs}")
            length_m, tau, _slope, _phase, residual_deg, quality = run_selftest(sdr)
            last_quality = quality
            if quality["verdict"] == "OK":
                ok_lengths.append(length_m)
                print(f"Run length = {length_m:.3f} m")
            else:
                print(f"Run rejected = {quality['verdict']}")

        if ok_lengths:
            summary = summarize_lengths(ok_lengths, known_length_m=args.known_length)
            print()
            print("Repeatability summary")
            print(f"Accepted runs = {summary['count']} / {args.runs}")
            print(f"Mean length   = {summary['mean_m']:.3f} m")
            print(f"Std length    = {summary['std_m']:.3f} m")
            print(f"Span length   = {summary['span_m']:.3f} m")
            if args.known_length is not None:
                print(f"Known length  = {summary['known_length_m']:.3f} m")
                print(f"Error         = {summary['error_m']:.3f} m")
                print(f"Rel error     = {summary['relative_error_pct']:.2f} %")
        else:
            print("Repeatability summary")
            print(f"Accepted runs = 0 / {args.runs}")
            print(f"Last verdict  = {last_quality['verdict'] if last_quality else 'NO_DATA'}")
```

This step will require removing the old single-run final print block to avoid duplicate output.

- [ ] **Step 5: Run tests**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
OK
```

---

### Task 7: Run the Optimization Experiment Sequence

**Files:**
- Run: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Append: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\docs\experiments\sdr_error_optimization_trials.md`

- [ ] **Step 1: Self-test dual loopback**

Wiring:

```text
TX1A -> RX1A short SMA
TX2A -> RX2A short SMA
```

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_multitone_selftest.py --runs 3 --known-length 0
```

Expected:

```text
Quality verdict = OK
Mean length <= 0.10 m
```

- [ ] **Step 2: 1.12m cable repeatability**

Wiring:

```text
TX1A/RX1A through RJ45 Pin1/Pin2 cable fixture
TX2A -> RX2A short SMA
```

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_multitone_selftest.py --runs 5 --known-length 1.12
```

Target:

```text
Accepted runs >= 4 / 5
Span length <= 0.15 m
Relative error within +/-10% before final calibration
```

- [ ] **Step 3: 1.10m cable repeatability**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_multitone_selftest.py --runs 5 --known-length 1.10
```

Target:

```text
Accepted runs >= 4 / 5
Mean length should be close to 1m, but 1.10m vs 1.12m ordering is not required for contest-basic alignment.
```

- [ ] **Step 4: 7.91m long cable sanity check**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_multitone_selftest.py --runs 3 --known-length 7.91
```

Target:

```text
If quality verdict is LOW_SIGNAL, the script is behaving better than before because it rejects a false 24.717m fit.
If quality verdict is OK, relative error should be recorded and compared against the 5% contest-basic target only as a pre-10m sanity check.
```

- [ ] **Step 5: Log trial results**

Create or append `docs\experiments\sdr_error_optimization_trials.md` with:

```markdown
# SDR Error Optimization Trials

## 2026-04-30 Optimized Script Trial

Configuration:
- SAMPLE_RATE = 30720000
- RF_BW = 28000000
- N = 131072
- N_AVG = 16
- Valid tone gate = DUT >= -30dB and REF >= -30dB

Results:
- Dual loopback: Accepted runs = 3 / 3, Mean length = 0.028 m, Quality = OK
- 1.12m cable: Accepted runs = 5 / 5, Mean length = 1.048 m, Span = 0.113 m, Rel error = -6.43 %
- 1.10m cable: Accepted runs = 5 / 5, Mean length = 1.562 m, Quality = OK, Decision = reject for short-cable ordering
- 7.91m cable: Accepted runs = 0 / 3, Quality = LOW_SIGNAL, Decision = reject false 24.717 m fit

Decision:
- Adjust frequency profile before using the long cable for calibration.
```

Do not keep the example values in the trial log. Replace them with the exact console summaries from the optimized script run.

---

### Task 8: Decide the Next Error-Reduction Lever

**Files:**
- Read: `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester\docs\experiments\sdr_error_optimization_trials.md`

- [ ] **Step 1: If 7.91m is LOW_SIGNAL**

Use this next lever:

```text
Try a lower-frequency profile before increasing gain:
0.2MHz to 6MHz, 24 tones, same N and N_AVG.
Reason: long cable attenuation is severe above a few MHz in the current single-ended fixture.
```

- [ ] **Step 2: If 1.12m repeatability span remains above 0.15m**

Use this next lever:

```text
Do not tune software first.
Fix mechanical repeatability:
- shorten RJ45-to-SMA pigtails
- strain-relieve the two RJ45 adapters
- keep Pin1 signal and Pin2 return geometry symmetric at TX and RX ends
- avoid touching the cable or adapters during repeated runs
```

- [ ] **Step 3: If all short runs are stable and 7.91m is accepted**

Use this next lever:

```text
Find or estimate one cable in the 10m to 50m contest-basic range.
Run --runs 5 --known-length <known_length_m>.
Only after that calibrate VF_CABLE.
```

---

## Self-Review

Spec coverage:

- Uses the revised 7.91m physical estimate.
- Aligns optimization with contest-basic 10m to 50m, <=5% error goal.
- Keeps 1.10m/1.12m data as repeatability diagnostics, not as final precision requirements.
- Rejects false low-SNR long-cable estimates instead of treating them as valid.

Red-flag scan:

- No banned filler tokens or undefined helper names are left without a task that defines them.
- Each code-changing task includes the exact functions or test code to add.
- Version-control steps are conditional because this workspace was previously not a git repository.

Type consistency:

- `evaluate_quality`, `estimate_length`, `summarize_lengths`, and `make_bin_aligned_tones` are named consistently across implementation and tests.
- `quality["verdict"]`, `quality["valid_mask"]`, and `quality["valid_count"]` are introduced before use.

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-04-30-sdr-error-optimization.md`.

Recommended execution mode:

```text
Inline Execution
```

Reason:

```text
This task is tightly coupled to hardware bench results. It is better to change one script, run one bench trial, inspect the output, and adjust deliberately.
```
