# SDR Length Experiment Log

## 2026-04-30 Dual Loopback Multitone Self-Test

Wiring:
- TX1A -> RX1A short SMA cable
- TX2A -> RX2A short SMA cable

Script:
- `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`

Configuration:
- LO_HZ = 400000000
- SAMPLE_RATE = 30720000
- RF_BW = 20000000
- TX_GAIN_DB = -45
- RX_GAIN_DB = 10
- TOTAL_SCALE = 0.04

Result:
- Extra delay relative to reference = 0.140 ns
- Estimated extra length = 0.028 m
- Verdict = PASS

Acceptance checks:
- All DUT and reference tone amplitudes are stronger than -30 dB.
- DUT/reference amplitudes are close at each tone, with no high-frequency collapse.
- Estimated extra length is below the 1.0 m self-test gate.

Tone table:

```text
freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   1.000,     -2.38,     -2.07,    168.61
   1.500,     -1.09,     -0.69,    168.29
   2.000,     -2.49,     -2.16,    168.99
   2.500,     -2.48,     -2.11,    168.81
   3.000,     -1.13,     -0.81,    168.59
   3.500,     -2.37,     -2.07,    168.40
   4.000,     -2.47,     -2.12,    167.71
   4.500,     -1.21,     -0.97,    168.94
   5.000,     -2.56,     -2.25,    167.89
   5.500,     -2.78,     -2.36,    168.33
   6.000,     -1.30,     -0.97,    167.56
   6.500,     -2.74,     -2.48,    168.60
   7.000,     -2.88,     -2.63,    168.39
   7.500,     -1.72,     -1.37,    168.49
   8.000,     -3.23,     -2.86,    168.63
   8.500,     -3.36,     -3.12,    168.15
   9.000,     -2.38,     -1.97,    168.68
   9.500,     -3.98,     -3.66,    168.00
  10.000,     -4.33,     -3.89,    167.77
```

## 2026-04-30 RJ45 1-2 Pair First Cable Tests

Wiring:
- DUT: TX1A/RX1A SMA center conductors connected through RJ45 Pin 1.
- DUT return: RJ45 Pin 2 tied to the SMA outer conductor at the TX and RX fixtures.
- Reference: TX2A -> RX2A short SMA cable.

Interpretation:
- This is a single-ended temporary fixture that uses Pin 1 as signal and Pin 2 as RF return.
- The method is acceptable for the current MVP trend test if the cable is otherwise floating and the same fixture style is used at both ends.
- This is not a final production-grade 100-ohm differential Ethernet-pair interface.

### Cable A

Known length:
- 1.12 m

Result:
- Extra delay relative to reference = 5.253 ns
- Estimated extra length = 1.039 m
- Verdict = PASS for first trend/accuracy check

Tone table:

```text
freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   1.000,     -5.07,     -1.19,     -1.25
   1.500,     -3.67,      0.06,     -1.93
   2.000,     -5.15,     -1.32,     -2.94
   2.500,     -4.87,     -1.34,     -3.43
   3.000,     -3.56,      0.10,     -4.13
   3.500,     -4.78,     -1.26,     -5.20
   4.000,     -4.79,     -1.23,     -5.61
   4.500,     -3.57,     -0.07,     -6.72
   5.000,     -4.66,     -1.41,     -7.29
   5.500,     -4.80,     -1.51,     -9.05
   6.000,     -3.39,     -0.15,     -9.89
   6.500,     -4.79,     -1.66,    -10.03
   7.000,     -4.78,     -1.85,    -10.74
   7.500,     -3.53,     -0.61,    -12.73
   8.000,     -4.85,     -2.10,    -12.82
   8.500,     -5.01,     -2.26,    -14.40
   9.000,     -3.79,     -1.16,    -15.66
   9.500,     -5.30,     -2.69,    -17.50
  10.000,     -5.48,     -3.12,    -19.18
```

### Cable B

Known length:
- Confirmed by physical comparison as about 1.10 m and slightly shorter than Cable A.

Result:
- Extra delay relative to reference = 7.892 ns
- Estimated extra length = 1.562 m
- Verdict = PASS for signal quality, but FAIL for near-equal-length consistency against Cable A. This result should not be used for VF calibration.

Interpretation:
- Since Cable B is physically slightly shorter than Cable A but measures much longer, the current temporary RJ45/SMA fixture still has enough connection/contact/phase-offset sensitivity to swamp differences at the 1.1 m scale.
- Use repeated measurements of the same cable and much longer cables to characterize the method before calibrating.

Tone table:

```text
freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
   1.000,     -5.40,      0.16,   -104.76
   1.500,     -3.97,      1.45,   -103.15
   2.000,     -5.47,      0.03,   -101.31
   2.500,     -5.10,     -0.04,   -101.65
   3.000,     -3.59,      1.38,   -100.03
   3.500,     -4.70,      0.04,   -100.81
   4.000,     -4.33,      0.00,   -101.04
   4.500,     -2.68,      1.25,   -102.14
   5.000,     -3.72,     -0.07,   -104.28
   5.500,     -3.74,     -0.24,   -105.79
   6.000,     -2.08,      1.14,   -108.28
   6.500,     -3.44,     -0.40,   -110.09
   7.000,     -3.48,     -0.39,   -112.42
   7.500,     -2.29,      0.71,   -116.08
   8.000,     -3.77,     -0.74,   -117.74
   8.500,     -3.86,     -0.93,   -119.84
   9.000,     -2.84,      0.13,   -122.42
   9.500,     -4.49,     -1.39,   -123.33
  10.000,     -4.76,     -1.70,   -125.61
```

## 2026-04-30 1.12 m Cable Repeatability Check

Known length:
- 1.12 m

Setup:
- Same 1.12 m cable.
- Same RJ45/SMA temporary fixture.
- User reports no intentional movement of solder points or fixture between the three runs.

Summary:

```text
Run 1: Extra delay = 5.100 ns, Estimated length = 1.009 m
Run 2: Extra delay = 5.672 ns, Estimated length = 1.122 m
Run 3: Extra delay = 5.120 ns, Estimated length = 1.013 m
Mean estimated length = 1.048 m
Sample stdev = 0.064 m
Span = 0.113 m
Mean error vs 1.12 m = -6.43%
VF from mean = 0.705
```

Verdict:
- Repeatability is usable for a first demo, with about 0.11 m span on a 1.12 m cable.
- Run 2 matches the known length closely, while runs 1 and 3 are about 10% low.
- Do not finalize `VF_CABLE` from only this cable. Use at least 3 known lengths after the fixture is mechanically fixed.

## 2026-04-30 Long Coiled Cable Trial

Known length:
- Not yet measured; visually much longer than 1.1 m cables.

Result:
- Extra delay relative to reference = -124.919 ns
- Estimated extra length = 24.717 m

Amplitude quality:
- DUT amplitude range is about -60.94 dB to -40.11 dB.
- Reference amplitude remains about -4.26 dB to 0.34 dB.

Verdict:
- PASS for coarse trend: a visibly long cable produced a much larger length result.
- FAIL for reliable phase-quality gate: DUT amplitude is below the earlier -30 dB acceptance threshold at every tone, so the fitted phase is likely noise-sensitive.

Interpretation:
- The long-cable result supports the thesis demo claim that the SDR method can distinguish short and long cables.
- It should not be used for calibration until the DUT path amplitude is improved or low-SNR tones are filtered out of the fit.

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

### Script Optimization Applied

The SDR measurement script now applies these quality controls before accepting a length estimate:

```text
DUT amplitude gate: DUT >= -30 dB
Reference amplitude gate: REF >= -30 dB
Minimum valid tones: 12
Fit residual gate: residual <= 3 deg
Repeatability gate: repeat span <= 0.10 m
```

Rejected fit verdicts:

```text
LOW_SIGNAL: too few tones pass the DUT/REF amplitude gates
HIGH_RESIDUAL: phase-vs-frequency fit residual is above 3 deg
UNSTABLE: accepted repeated runs span more than 0.10 m
```
