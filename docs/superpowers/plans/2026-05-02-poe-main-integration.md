# PoE Main Integration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a first PoE page to the main ESP32/TJC firmware using the verified Mode A ADC path on GPIO2.

**Architecture:** Keep the standalone diagnostic sketch as hardware proof, then copy only the stable Mode A ADC reading and threshold logic into `ethernet_tester1/ethernet_tester1.ino`. Mode B is not wired yet, so the main page must show `VB=N/A` and only report `MODE=A` when Mode A voltage is above the PoE threshold.

**Tech Stack:** Arduino C++ for ESP32-S3, TJC UART text commands, Python `unittest` static firmware checks.

---

### Task 1: Add Static Coverage For PoE Main Page

**Files:**
- Create: `tests/test_ethernet_tester_poe_page.py`
- Read: `ethernet_tester1/ethernet_tester1.ino`

- [ ] **Step 1: Write the failing test**

Create `tests/test_ethernet_tester_poe_page.py` with checks for `PAGE_POE`, `poeenter`, `poestart`, `POE_T0` through `POE_T11`, GPIO2 constants, Mode A threshold logic, and `VB=N/A`.

- [ ] **Step 2: Run test to verify it fails**

Run:

```powershell
python -B -m unittest tests.test_ethernet_tester_poe_page
```

Expected: FAIL because `ethernet_tester1.ino` does not yet contain `PAGE_POE`.

### Task 2: Implement Mode A PoE Page In Main Firmware

**Files:**
- Modify: `ethernet_tester1/ethernet_tester1.ino`
- Test: `tests/test_ethernet_tester_poe_page.py`

- [ ] **Step 1: Add page state and TJC objects**

Add `PAGE_POE`, `poeEnterFlag`, `poeStartFlag`, `poeBusy`, `lastPoeStartEventMs`, and `POE_T0` through `POE_T11`.

- [ ] **Step 2: Add Mode A ADC helpers**

Add `PIN_POE_ADC_A = 2`, `POE_DIVIDER_RATIO = 20.6078f`, `POE_BRIDGE_DROP_COMP_V = 1.2f`, `NO_POE_MAX_V = 5.0f`, `POE_PRESENT_MIN_V = 30.0f`, and `POE_OVER_RANGE_V = 60.0f`.

- [ ] **Step 3: Add page reset and run flow**

Add `poePageReset()`, `enterPoePage()`, `readPoeAdcVoltageV()`, `estimatePoeInputVoltageV()`, `poeStatusText()`, `poeModeText()`, `poeShowTesting()`, `poeShowResult()`, and `runPoeTest()`.

- [ ] **Step 4: Add event parsing before generic start**

Parse `poeenter` and `poestart` before the generic `start` token so `poestart` is not consumed as Wire Map start.

- [ ] **Step 5: Run focused and full tests**

Run:

```powershell
python -B -m unittest tests.test_ethernet_tester_poe_page
python -B -m unittest discover -s tests
```

Expected: all tests pass.

### Task 3: Commit And Push

**Files:**
- Modify: `docs/superpowers/plans/2026-05-02-poe-main-integration.md`
- Modify: `ethernet_tester1/ethernet_tester1.ino`
- Create: `tests/test_ethernet_tester_poe_page.py`

- [ ] **Step 1: Review git status**

Run:

```powershell
git status --short
```

Expected: only the plan, PoE test, and main firmware are changed.

- [ ] **Step 2: Commit**

Run:

```powershell
git add docs/superpowers/plans/2026-05-02-poe-main-integration.md ethernet_tester1/ethernet_tester1.ino tests/test_ethernet_tester_poe_page.py
git commit -m "feat: add poe page to main firmware"
```

- [ ] **Step 3: Push**

Run:

```powershell
git push origin main
```

Expected: local `main` and `origin/main` are synchronized.
