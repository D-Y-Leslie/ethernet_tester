# SDR Length Module Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a demonstrable Ethernet twisted-pair length measurement path using the 7020-SDR / AD9361 dual-transmit dual-receive method, then connect the result back into the ESP32-S3 + TJC tester UI.

**Architecture:** Keep ESP32-S3 responsible for wire-map, type, DCR, short detection, TJC UI, and measurement triggering. Use the 7020-SDR / AD9361 as the RF measurement subsystem: it emits identical multitone IQ on TX1/TX2, receives DUT/reference IQ on RX1/RX2, estimates phase slope, converts group delay to cable length, and returns a compact text result. Do not move AD9361 high-speed IQ acquisition onto ESP32.

**Tech Stack:** Arduino C++ on ESP32-S3-N16R8, TJC UART HMI at 115200 baud, Python + pyadi-iio/libiio for PC-side SDR validation, and arm-linux-gnueabihf C + libiio for the final SDR-board standalone program.

---

## Context Summary

Read sources:

- `SDR方案.md`: Current active Length plan has switched from AD9959/AD8310 to 7020-SDR / AD9361 2T2R phase-slope measurement.
- `AD8310方案.md` and `1.md`: AD8310 TPVout did not respond reliably to Pluto TX ON/OFF or TX gain changes; AD9959 output was verified, but the AD8310 path is paused.
- `ethernet_tester1/ethernet_tester1.ino`: Main ESP32 project already has Wire Map, Cable Type, DCR, and Short Detect pages and logic.
- `ethernet_tester2.ino/ethernet_tester2.ino.ino`: AD8310 diagnostic sketch only, not the main application path.
- `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`: Multitone self-test script already exists.
- `D:\Python\Python File\Professional Experiments\pluto_2x2_cable_length.py`: Cable length sweep script still uses per-tone TX buffer refresh and should not be trusted until dual-loopback multitone passes.
- Task PDF: The thesis task requires literature review, AC/DC twisted-pair measurement methods, MCU-controlled signal amplitude/phase, output acquisition/analysis, and final parameter display.
- TJC PDF: Screen model is `TJC4827X543_011C_Y`, resolution is `480*272`, power pin is `5V`, UART pins are `RX` and `TX`; RX connects to MCU TX, TX connects to MCU RX.

Current hard gate:

```text
Before connecting a real cable to TX1A/RX1A, dual short-loopback must estimate near 0 m:
TX1A -> RX1A
TX2A -> RX2A
Estimated extra length <= 1.0 m
No large multi-meter false result such as 5 m or 10 m
```

## File Structure

No existing file should be deleted or renamed.

Version-control note: at plan-writing time, neither `C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester` nor `D:\Python\Python File\Professional Experiments` is a git repository. Run the git checkpoint steps only after initializing or moving the work into a git repository.

- Modify in Task 3: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
  - Owns 2T2R multitone self-test, tone extraction, phase-slope fitting, and self-test pass/fail output.
- Modify in Task 3: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`
  - Owns unit tests for synthetic delay estimation and self-test quality helpers.
- Modify in Task 5: `D:\Python\Python File\Professional Experiments\pluto_2x2_cable_length.py`
  - Owns real cable length measurement only after self-test passes.
- Modify in Tasks 6-7: `ethernet_tester1/ethernet_tester1.ino`
  - Add Length page state, TJC events, and an SDR result receiver after PC-side algorithm is stable.
- Create in Task 2: `docs/experiments/sdr_length_log.md`
  - Human-readable experiment record for dual-loopback, 1-2 pair tests, and VF calibration.
- Create in Task 5: `docs/experiments/sdr_length_calibration.csv`
  - Calibration rows: date, known length, measured length, VF, notes.

## Scope Check

This is one plan for the Length module MVP only. Keep PoE detection as a separate plan after Length has a demonstrable result. Keep final SDR-board C port as a separate implementation plan after the PC Python algorithm passes dual-loopback and real cable tests.

---

### Task 1: Freeze the AD8310 Path and Prepare the 2T2R Bench

**Files:**
- Read: `SDR方案.md`
- Read: `AD8310方案.md`
- Read: `D:\Python\Python File\Professional Experiments\Pre.md`
- No code changes

- [ ] **Step 1: Physically remove AD8310 from the current Length experiment path**

Disconnect AD8310, AD9959, and GPIO2 TPVout sampling from the active Length measurement bench.

Expected active RF wiring for this task:

```text
TX1A -> short SMA cable -> RX1A
TX2A -> short SMA cable -> RX2A
```

- [ ] **Step 2: Confirm SDR address from PC**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe scan_iio.py
```

Expected output includes:

```text
ip:pluto.local => 192.168.2.1
```

- [ ] **Step 3: Run the existing test suite**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest discover -s tests
```

Expected:

```text
Ran 16 tests
OK
```

Commit only if files changed during this task. This task should normally have no commit.

---

### Task 2: Run the Current Multitone Dual-Loopback Self-Test

**Files:**
- Run: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Create if missing: `docs/experiments/sdr_length_log.md`

- [ ] **Step 1: Run the self-test**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_multitone_selftest.py
```

When prompted, confirm this wiring:

```text
TX1A -> RX1A
TX2A -> RX2A
```

- [ ] **Step 2: Judge the amplitude table**

Accept only if all three conditions are true:

```text
Each printed tone has both dut_amp_dB and ref_amp_dB stronger than -30 dB.
The two channels are in the same rough amplitude range, with no channel consistently 20 dB weaker.
There is no pattern where 1 MHz is strong and most higher-frequency tones fall below -35 dB.
```

- [ ] **Step 3: Judge the length result**

Accept only if:

```text
abs(Extra delay relative to reference) <= 5.1 ns
Estimated extra length <= 1.0 m
```

Rationale:

```text
1.0 m / (0.66 * 299792458 m/s) = about 5.1 ns
```

- [ ] **Step 4: Record the result**

Append this exact record shape to `docs/experiments/sdr_length_log.md`:

```markdown
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
- Extra delay relative to reference = 0.000 ns
- Estimated extra length = 0.000 m
- Verdict = PASS or FAIL

Tone table:
```text
freq_MHz, dut_amp_dB, ref_amp_dB, rel_phase_deg
1.000, 0.00, 0.00, 0.00
```
```

Do not keep the example values in the experiment log. Enter the exact values and tone table printed by the console.

---

### Task 3: If Dual-Loopback Fails, Tune Only One Gain Variable at a Time

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_multitone_selftest.py`
- Test: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_multitone_selftest.py`

- [ ] **Step 1: Try stronger TX with the same RX gain**

Change:

```python
TX_GAIN_DB = -45
RX_GAIN_DB = 10
```

to:

```python
TX_GAIN_DB = -35
RX_GAIN_DB = 10
```

- [ ] **Step 2: Update the matching default test**

In `tests\test_pluto_2x2_multitone_selftest.py`, change:

```python
self.assertEqual(selftest.TX_GAIN_DB, -45)
self.assertEqual(selftest.RX_GAIN_DB, 10)
```

to:

```python
self.assertEqual(selftest.TX_GAIN_DB, -35)
self.assertEqual(selftest.RX_GAIN_DB, 10)
```

- [ ] **Step 3: Run tests**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_multitone_selftest
```

Expected:

```text
Ran 3 tests
OK
```

- [ ] **Step 4: Re-run self-test and record result**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_multitone_selftest.py
```

Append the result to `docs/experiments/sdr_length_log.md` using the Task 2 record shape.

- [ ] **Step 5: If TX -35 dB still fails, restore TX and raise RX gain**

Change:

```python
TX_GAIN_DB = -35
RX_GAIN_DB = 10
```

to:

```python
TX_GAIN_DB = -45
RX_GAIN_DB = 20
```

Update the test to:

```python
self.assertEqual(selftest.TX_GAIN_DB, -45)
self.assertEqual(selftest.RX_GAIN_DB, 20)
```

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest discover -s tests
```

Expected:

```text
OK
```

- [ ] **Step 6: Version checkpoint for the selected gain setting**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
git rev-parse --show-toplevel
```

If the command prints a repository path, run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
git status --short
git add pluto_2x2_multitone_selftest.py tests/test_pluto_2x2_multitone_selftest.py
git commit -m "test: tune sdr multitone selftest gain"
```

If the command prints `fatal: not a git repository`, skip the commit step for this workspace.

Checkpoint condition:

```text
Dual loopback has Estimated extra length <= 1.0 m.
```

---

### Task 4: Measure RJ45 1-2 Pair Only After Self-Test Passes

**Files:**
- Run: `D:\Python\Python File\Professional Experiments\pluto_2x2_cable_length.py`
- Modify in Task 5: `D:\Python\Python File\Professional Experiments\pluto_2x2_cable_length.py`
- Create/append: `docs/experiments/sdr_length_log.md`

- [ ] **Step 1: Wire the DUT path**

Use only the 1-2 pair for the first demo:

```text
TX1A center -> near RJ45 Pin 1
TX1A shield -> near RJ45 Pin 2 or fixture ground reference, matching the balun/fixture wiring
Remote RJ45 Pin 1/Pin 2 -> RX1A through the same fixture style
TX2A -> short SMA cable -> RX2A
```

If there is no balun or controlled fixture yet, make a temporary two-wire fixture and keep it physically short and repeatable. Do not connect all eight RJ45 pins to the SDR during the first Length demo.

- [ ] **Step 2: Run the cable script**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe pluto_2x2_cable_length.py
```

Expected console prompt:

```text
DUT channel: TX1A -> cable pair under test -> RX1A
Reference:   TX2A -> short SMA loopback -> RX2A
```

- [ ] **Step 3: Accept only trend correctness at first**

Test at least two cables:

```text
Known short cable: result must be smaller
Known longer cable: result must be larger
```

Do not require exact meter-level accuracy before calibration.

- [ ] **Step 4: Record the first cable data**

Append:

```markdown
## 2026-04-30 RJ45 1-2 Pair First Cable Test

Wiring:
- DUT: TX1A -> RJ45 1-2 pair -> RX1A
- Reference: TX2A -> RX2A short SMA loopback

Cable:
- Known length = 10.00 m

Result:
- Extra delay relative to reference = 50.540 ns
- Estimated cable length = 10.00 m
- Trend verdict = PASS or FAIL
```

Do not keep the example values in the experiment log. Enter the known cable length and the exact SDR result from the console.

---

### Task 5: Calibrate `VF_CABLE` With a Known Cable

**Files:**
- Modify: `D:\Python\Python File\Professional Experiments\pluto_2x2_cable_length.py`
- Test: `D:\Python\Python File\Professional Experiments\tests\test_pluto_2x2_cable_length.py`
- Create/append: `docs/experiments/sdr_length_calibration.csv`

- [ ] **Step 1: Compute calibrated velocity factor**

Use this formula:

```text
VF_new = VF_old * known_length_m / measured_length_m
```

Example:

```text
VF_old = 0.66
known_length_m = 10.00
measured_length_m = 8.50
VF_new = 0.66 * 10.00 / 8.50 = 0.776
```

- [ ] **Step 2: Update the script constant**

Change:

```python
VF_CABLE = 0.66
```

to the measured value, for example:

```python
VF_CABLE = 0.776
```

- [ ] **Step 3: Update the test expectation**

In `tests\test_pluto_2x2_cable_length.py`, change the synthetic delay test only if the expected `VF_CABLE` default is asserted. If there is no explicit `VF_CABLE` default assertion, leave the test unchanged because it derives `tau` from the script constant.

- [ ] **Step 4: Run tests**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
.\.venv\Scripts\python.exe -B -m unittest tests.test_pluto_2x2_cable_length
```

Expected:

```text
Ran 3 tests
OK
```

- [ ] **Step 5: Save calibration CSV**

Append one row to `docs/experiments/sdr_length_calibration.csv`:

```csv
date,known_length_m,measured_length_m,vf_old,vf_new,notes
2026-04-30,10.00,8.50,0.66,0.776,"RJ45 1-2 pair first calibration"
```

Use actual measured values instead of the example values.

- [ ] **Step 6: Version checkpoint for calibration**

Run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
git rev-parse --show-toplevel
```

If the command prints a repository path, run:

```powershell
cd "D:\Python\Python File\Professional Experiments"
git status --short
git add pluto_2x2_cable_length.py tests/test_pluto_2x2_cable_length.py
git commit -m "calibrate sdr cable velocity factor"
```

Check whether the Arduino project is a git repository:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
git rev-parse --show-toplevel
```

If the command prints a repository path, commit the experiment records:

```powershell
git add docs/experiments/sdr_length_calibration.csv docs/experiments/sdr_length_log.md
git commit -m "docs: record sdr length calibration"
```

If either command prints `fatal: not a git repository`, skip that commit step for that workspace.

---

### Task 6: Add a Minimal Length Result Page to ESP32/TJC After PC Measurement Works

**Files:**
- Modify: `ethernet_tester1/ethernet_tester1.ino`
- TJC screen project: add page `p_length` with `b0 Back`, `b1 Start`, `t0` to `t9`

- [ ] **Step 1: Add TJC events**

Configure TJC button events:

```text
main Length button release event: prints "lengthenter",0
p_length.b0 release event: prints "back",0
p_length.b1 release event: prints "lengthstart",0
```

- [ ] **Step 2: Extend page enum**

In `ethernet_tester1.ino`, change:

```cpp
enum UiPage : uint8_t {
  PAGE_MAIN = 0,
  PAGE_WIRE,
  PAGE_DCR,
  PAGE_TYPE,
  PAGE_SHORT
};
```

to:

```cpp
enum UiPage : uint8_t {
  PAGE_MAIN = 0,
  PAGE_WIRE,
  PAGE_DCR,
  PAGE_TYPE,
  PAGE_SHORT,
  PAGE_LENGTH
};
```

- [ ] **Step 3: Add Length flags**

Add near the other page flags:

```cpp
bool lengthEnterFlag = false;
bool lengthStartFlag = false;
bool lengthBusy = false;
uint32_t lastLengthStartEventMs = 0;
```

- [ ] **Step 4: Add object names**

Add near the Short page object names:

```cpp
static const char* LENGTH_T0 = "p_length.t0";
static const char* LENGTH_T1 = "p_length.t1";
static const char* LENGTH_T2 = "p_length.t2";
static const char* LENGTH_T3 = "p_length.t3";
static const char* LENGTH_T4 = "p_length.t4";
static const char* LENGTH_T5 = "p_length.t5";
static const char* LENGTH_T6 = "p_length.t6";
static const char* LENGTH_T7 = "p_length.t7";
static const char* LENGTH_T8 = "p_length.t8";
static const char* LENGTH_T9 = "p_length.t9";
```

- [ ] **Step 5: Add reset and enter functions**

Add forward declarations:

```cpp
void lengthPageReset();
void enterLengthPage();
void runLengthTest();
```

Add implementations:

```cpp
void enterLengthPage() {
  lengthPageReset();
  currentPage = PAGE_LENGTH;
}

void lengthPageReset() {
  tjcPage("p_length");
  delay(60);

  tjcSetText(LENGTH_T0, "Cable Length");
  tjcSetText(LENGTH_T1, "WAIT TEST");
  tjcSetText(LENGTH_T2, "PAIR");
  tjcSetText(LENGTH_T3, "1-2");
  tjcSetText(LENGTH_T4, "LENGTH");
  tjcSetText(LENGTH_T5, "--");
  tjcSetText(LENGTH_T6, "METHOD");
  tjcSetText(LENGTH_T7, "SDR PHASE");
  tjcSetText(LENGTH_T8, "INFO");
  tjcSetText(LENGTH_T9, "PRESS START");
}
```

- [ ] **Step 6: Parse Length events**

In `processAsciiTokenBuffer()`, add before the generic `start` check:

```cpp
if (tjcAsciiBuf.endsWith("lengthenter")) {
  lengthEnterFlag = true;
  tjcAsciiBuf = "";
  Serial.println("[EVENT] lengthenter");
  return;
}

if (tjcAsciiBuf.endsWith("lengthstart")) {
  if (!lengthBusy && (now - lastLengthStartEventMs >= START_DEBOUNCE_MS)) {
    lengthStartFlag = true;
    lastLengthStartEventMs = now;
    Serial.println("[EVENT] lengthstart accepted");
  } else {
    Serial.println("[EVENT] lengthstart ignored");
  }
  tjcAsciiBuf = "";
  return;
}
```

- [ ] **Step 7: Add page transition and stub run**

In `loop()`, add:

```cpp
if (lengthEnterFlag && !wireBusy && !dcrBusy && !typeBusy && !shortBusy && !lengthBusy) {
  lengthEnterFlag = false;
  enterLengthPage();
}

if (lengthStartFlag && !lengthBusy) {
  runLengthTest();
}
```

Add a temporary PC-SDR status function while the PC script still owns the real measurement:

```cpp
void runLengthTest() {
  lengthBusy = true;
  lengthStartFlag = false;

  Serial.println("====================================");
  Serial.println("Length test PC-SDR status start");

  tjcSetText(LENGTH_T1, "TEST OK");
  tjcSetText(LENGTH_T5, "PC SDR READY");
  tjcSetText(LENGTH_T9, "RUN SDR SCRIPT ON PC");

  Serial.println("Length PC-SDR status: PC SDR script still owns measurement.");
  Serial.println("Length test PC-SDR status end");
  Serial.println("====================================");

  lengthBusy = false;
}
```

- [ ] **Step 8: Compile in Arduino IDE**

Expected:

```text
Compilation finished successfully.
```

- [ ] **Step 9: Version checkpoint**

Run:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
git rev-parse --show-toplevel
```

If the command prints a repository path, run:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
git add ethernet_tester1/ethernet_tester1.ino
git commit -m "feat: add length page pc status"
```

If the command prints `fatal: not a git repository`, skip the commit step for this workspace.

---

### Task 7: Replace Length Stub With SDR Result Protocol

**Files:**
- Modify: `ethernet_tester1/ethernet_tester1.ino`
- Modify in this task: SDR-side script or C program that emits line protocol

- [ ] **Step 1: Use a line protocol**

Protocol:

```text
ESP32 -> SDR: START\n
SDR   -> ESP32: LEN,8.43\n
SDR   -> ESP32: ERR,NO_SIGNAL\n
SDR   -> ESP32: ERR,SELFTEST_FAIL\n
```

- [ ] **Step 2: Select UART pins only after confirming the ESP32-S3 board pinout**

Current reserved pins:

```text
GPIO1: DCR ADC
GPIO2: old AD8310 ADC, keep free unless repurposed
GPIO4/5/6/8/9/7/10/11: RJ45 wire map
GPIO12: cable type
GPIO13/14/15/16/21: old AD9959 control, can be freed after AD9959 is fully removed
GPIO17/18: TJC UART
GPIO35/36/37: avoid because PSRAM-related risk on this board
GPIO38: board RGB LED according to board silkscreen/photo
GPIO40: used by AD8310 diagnostic sketch, available after AD8310 path is retired
```

Recommended first wiring for ESP32-SDR UART after pinout confirmation:

```text
ESP32 GPIO39 <- SDR TX
ESP32 GPIO40 -> SDR RX
ESP32 GND  <-> SDR GND
```

- [ ] **Step 3: Add Serial2 setup**

After pinout confirmation, add constants:

```cpp
static const uint32_t SDR_BAUD = 115200;
static const int SDR_RX_PIN = 39;
static const int SDR_TX_PIN = 40;
```

In `setup()`, add:

```cpp
Serial2.begin(SDR_BAUD, SERIAL_8N1, SDR_RX_PIN, SDR_TX_PIN);
```

- [ ] **Step 4: Implement request-response read**

Replace the PC-SDR status `runLengthTest()` with:

```cpp
void runLengthTest() {
  lengthBusy = true;
  lengthStartFlag = false;

  tjcSetText(LENGTH_T1, "TESTING");
  tjcSetText(LENGTH_T5, "--");
  tjcSetText(LENGTH_T9, "WAIT SDR");

  while (Serial2.available()) {
    Serial2.read();
  }

  Serial2.print("START\n");

  String line = "";
  uint32_t startMs = millis();
  const uint32_t timeoutMs = 15000;

  while (millis() - startMs < timeoutMs) {
    while (Serial2.available()) {
      char c = (char)Serial2.read();
      if (c == '\n' || c == '\r') {
        if (line.length() > 0) {
          if (line.startsWith("LEN,")) {
            String value = line.substring(4);
            tjcSetText(LENGTH_T1, "TEST OK");
            tjcSetText(LENGTH_T5, value + " m");
            tjcSetText(LENGTH_T9, "SDR DONE");
            lengthBusy = false;
            return;
          }

          if (line.startsWith("ERR,")) {
            tjcSetText(LENGTH_T1, "TEST FAIL");
            tjcSetText(LENGTH_T5, "--");
            tjcSetText(LENGTH_T9, line);
            lengthBusy = false;
            return;
          }

          line = "";
        }
      } else if (line.length() < 48) {
        line += c;
      }
    }
    delay(5);
  }

  tjcSetText(LENGTH_T1, "TEST FAIL");
  tjcSetText(LENGTH_T5, "--");
  tjcSetText(LENGTH_T9, "ERR,TIMEOUT");
  lengthBusy = false;
}
```

- [ ] **Step 5: Compile and bench-test with a USB serial adapter**

Before wiring the real SDR board, emulate SDR output from PC:

```text
Type into USB-serial terminal:
LEN,8.43
```

Expected TJC:

```text
p_length.t1 = TEST OK
p_length.t5 = 8.43 m
p_length.t9 = SDR DONE
```

- [ ] **Step 6: Version checkpoint**

Run:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
git rev-parse --show-toplevel
```

If the command prints a repository path, run:

```powershell
cd "C:\Users\Leslie\Documents\Arduino\Project\ethernet_tester"
git add ethernet_tester1/ethernet_tester1.ino
git commit -m "feat: receive sdr length result"
```

If the command prints `fatal: not a git repository`, skip the commit step for this workspace.

---

### Task 8: Thesis and Demo Alignment

**Files:**
- Modify in this task: thesis document under `F:\毕设\论文`
- Read: `F:\毕设\2026届_2022210914_以太网双绞线测试仪的设计与实现.pdf`
- Append: `docs/experiments/sdr_length_log.md`

- [ ] **Step 1: Keep thesis wording honest**

Use this system wording:

```text
ESP32-S3 is the main control and human-machine interaction controller.
Z7020-AD9361 is the RF measurement subsystem for cable length.
The two modules communicate through UART or GPIO trigger/result lines.
```

Do not write:

```text
ESP32 directly drives AD9361.
```

- [ ] **Step 2: Record deliverable status**

Use this status table in the thesis experiment chapter:

```text
Wire Map: implemented and demonstrated
Cable Type: implemented as UTP / shielded preliminary identification
DCR: implemented as four-pair manual loop resistance measurement
Short Detect: implemented for arbitrary wire-to-wire short detection
Length: SDR phase-slope method, first demonstration on RJ45 1-2 pair
PoE: separate final task after Length MVP
```

- [ ] **Step 3: Demo order**

Use this demonstration order:

```text
1. Wire Map pass/fail
2. Short Detect with one intentional short
3. DCR 1-2 / 3-6 / 4-5 / 7-8
4. Type UTP vs shielded
5. Length 1-2 pair with short cable and longer cable trend
```

This order keeps the SDR portion last, so the core tester remains demonstrable even if RF setup needs extra time.

---

## Self-Review

Spec coverage:

- Task PDF asks for AC/DC parameter measurement and display. This plan preserves DC modules and advances AC phase-based Length.
- Current SDR scheme requires dual-loopback first. Tasks 1-3 enforce that gate.
- Real cable measurement and VF calibration are covered by Tasks 4-5.
- ESP32/TJC integration is covered by Tasks 6-7.
- Thesis/demo alignment is covered by Task 8.

Red-flag scan:

- No task depends on an undefined file.
- Experiment log examples use concrete sample numbers and explicitly require replacing them with exact console output.
- PoE and final C port are intentionally out of this plan and should each get their own plan.

Type and name consistency:

- `lengthEnterFlag`, `lengthStartFlag`, and `lengthBusy` match the existing `wire/dcr/type/short` naming pattern.
- `p_length.t0` to `p_length.t9` match the existing TJC object naming style.
- `START`, `LEN,xx.xx`, and `ERR,...` match the SDR protocol described in `SDR方案.md`.

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-04-30-sdr-length-mvp.md`. Two execution options:

1. Subagent-Driven (recommended) - dispatch a fresh worker per task, review between tasks, fast iteration.

2. Inline Execution - execute tasks in this session using executing-plans, batch execution with checkpoints.

Recommended immediate choice for this hardware-heavy project: Inline Execution for Tasks 1-2, because the next blocker is a physical SDR dual-loopback run that needs the bench in front of you.
