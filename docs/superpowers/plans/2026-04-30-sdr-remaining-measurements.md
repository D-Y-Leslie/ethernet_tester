# SDR Remaining Measurements Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish the SDR 2T2R cable-length validation with a fixed normal topology, zero-offset correction, and enough known/unknown cable trials to decide whether the method meets the contest-basic accuracy target.

**Architecture:** Keep `TX1A -> RX1A` as the DUT path and `TX2A -> RX2A` as the reference path. Use the B-group loopback result as a fixed zero offset (`0.055 m`) and reject runs marked `LOW_SIGNAL`, `FEW_VALID_TONES`, `HIGH_RESIDUAL`, or `UNSTABLE`. Do not use the A2 crossed topology for calibration because both received paths were low signal.

**Tech Stack:** AD9361 2T2R, Python 3, pyadi-iio, NumPy, CSV logs, Markdown experiment notes.

---

## Current Calibration Facts

Use these facts for the next measurement round:

```text
A1 normal loopback mean signed length = 0.050 m
B1 SMA1 DUT / SMA2 REF mean signed length = 0.057 m
B2 SMA2 DUT / SMA1 REF mean signed length = 0.053 m
Recommended zero offset = 0.055 m
SMA line mismatch estimate = about 0.002 m
```

Run the script with:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
& "D:\Python\Python File\Professional Experiments\.venv\Scripts\python.exe" -B .\pluto_2x2_channel_swap_test.py --runs 3 --zero-offset-m 0.055
```

Read `corrected signed len` and `corrected_signed_length_m` first. Keep raw `signed_length_m` in the log for traceability.

## Measurement Sequence

### Task 1: Reconfirm Zero Offset Before DUT

**Files:**
- Output: `channel_swap_results_YYYYMMDD_HHMMSS.csv`
- Append evidence to: `SDR方案Debug_GPT.md`

- [ ] **Step 1: Run one normal loopback check**

Command:

```powershell
& "D:\Python\Python File\Professional Experiments\.venv\Scripts\python.exe" -B .\pluto_2x2_channel_swap_test.py --runs 3 --zero-offset-m 0.055 --stages sma_a_dut_b_ref
```

Wiring:

```text
SMA1: TX1A -> RX1A
SMA2: TX2A -> RX2A
No RJ45 cable.
```

Acceptance:

```text
verdict = OK
valid tones = 55
corrected signed len should be near 0 m
span should be <= 0.03 m
```

### Task 2: Measure 1.12 m Cable Forward and Reversed

**Files:**
- Output: `channel_swap_results_YYYYMMDD_HHMMSS.csv`
- Append evidence to: `SDR方案Debug_GPT.md`

- [ ] **Step 1: Run DUT forward/reversed stages**

Command:

```powershell
& "D:\Python\Python File\Professional Experiments\.venv\Scripts\python.exe" -B .\pluto_2x2_channel_swap_test.py --runs 5 --zero-offset-m 0.055 --stages dut_forward dut_reversed
```

Wiring for `dut_forward`:

```text
DUT: TX1A -> SMA/RJ45 fixture -> 1.12 m cable -> SMA/RJ45 fixture -> RX1A
REF: TX2A -> RX2A
```

Wiring for `dut_reversed`:

```text
Reverse only the RJ45 cable end-to-end.
Keep TX1A/RX1A and TX2A/RX2A unchanged.
```

Acceptance:

```text
verdict = OK
valid tones >= 45
corrected length mean is close to 1.12 m
forward/reversed difference <= 0.10 m is good; >0.20 m means connector or fixture direction sensitivity.
```

### Task 3: Measure 1.10 m Cable Forward and Reversed

**Files:**
- Output: `channel_swap_results_YYYYMMDD_HHMMSS.csv`
- Append evidence to: `SDR方案Debug_GPT.md`

- [ ] **Step 1: Repeat Task 2 using the 1.10 m cable**

Command:

```powershell
& "D:\Python\Python File\Professional Experiments\.venv\Scripts\python.exe" -B .\pluto_2x2_channel_swap_test.py --runs 5 --zero-offset-m 0.055 --stages dut_forward dut_reversed
```

Acceptance:

```text
Do not require 1.10 m and 1.12 m to be distinguishable.
For contest-basic alignment, the important result is stable short-cable behavior and no false long-cable result.
```

### Task 4: Measure the Approximate 7.91 m Cable

**Files:**
- Output: `channel_swap_results_YYYYMMDD_HHMMSS.csv`
- Append evidence to: `SDR方案Debug_GPT.md`

- [ ] **Step 1: Run the long coiled cable**

Command:

```powershell
& "D:\Python\Python File\Professional Experiments\.venv\Scripts\python.exe" -B .\pluto_2x2_channel_swap_test.py --runs 5 --zero-offset-m 0.055 --stages dut_forward dut_reversed
```

Known physical estimate:

```text
4 * 1.10 m + 3 * 1.12 m + 0.15 m = 7.91 m
```

Acceptance:

```text
If LOW_SIGNAL appears again, reject the numeric length and keep it as evidence that the fixture/coupling is not reliable for that cable.
If verdict = OK, compare corrected length against 7.91 m.
```

### Task 5: Decide Whether to Improve Hardware or Calibrate VF

**Files:**
- Append summary to: `SDR方案Debug_GPT.md`

- [ ] **Step 1: Use this decision table**

```text
If 1.12 m and 1.10 m are stable but not distinguishable:
  Accept this for basic-requirement direction; short cables are below the 10 m contest-basic range.

If 7.91 m is LOW_SIGNAL:
  Improve RJ45/SMA fixture before further length calibration.

If 7.91 m is OK and near the physical estimate:
  Keep zero offset = 0.055 m and estimate VF from the known cables only after collecting one cable >= 10 m.

If forward/reversed differs by >0.20 m:
  Fix connector contact, ground return, and fixture symmetry before changing software.
```

