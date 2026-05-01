import pathlib
import re
import unittest


SKETCH_PATH = (
    pathlib.Path(__file__).resolve().parents[1]
    / "ethernet_tester1"
    / "ethernet_tester1.ino"
)


class EthernetTesterPoePageTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = SKETCH_PATH.read_text(encoding="utf-8")

    def test_poe_page_is_registered_with_mode_a_state(self):
        self.assertRegex(self.source, r"PAGE_LENGTH,\s*PAGE_POE")
        self.assertIn("bool poeEnterFlag = false;", self.source)
        self.assertIn("bool poeStartFlag = false;", self.source)
        self.assertIn("bool poeBusy = false;", self.source)
        self.assertIn("uint32_t lastPoeStartEventMs = 0;", self.source)

        for idx in range(12):
            self.assertIn(f'static const char* POE_T{idx} = "p_poe.t{idx}";', self.source)

    def test_poe_adc_constants_match_verified_mode_a_hardware(self):
        required_snippets = [
            "static const int PIN_POE_ADC_A = 2;",
            "static const bool POE_MODE_B_AVAILABLE = false;",
            "static const float POE_DIVIDER_RATIO = 20.6078f;",
            "static const float POE_BRIDGE_DROP_COMP_V = 1.2f;",
            "static const float NO_POE_MAX_V = 5.0f;",
            "static const float POE_PRESENT_MIN_V = 30.0f;",
            "static const float POE_OVER_RANGE_V = 60.0f;",
            "static const uint16_t POE_ADC_SAMPLES = 120;",
            "analogSetPinAttenuation(PIN_POE_ADC_A, ADC_11db);",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_poe_page_reset_initializes_tjc_fields(self):
        required_snippets = [
            'tjcPage("p_poe");',
            'tjcSetText(POE_T0, "PoE Detect");',
            'tjcSetText(POE_T1, "WAIT TEST");',
            'tjcSetText(POE_T3, "--");',
            'tjcSetText(POE_T5, "--");',
            'tjcSetText(POE_T7, "-- V");',
            'tjcSetText(POE_T9, "N/A");',
            'tjcSetText(POE_T11, "--");',
            "currentPage = PAGE_POE;",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_poe_events_are_parsed_before_generic_start(self):
        self.assertIn('endsWith("poeenter")', self.source)
        self.assertIn('endsWith("poestart")', self.source)
        poe_enter_at = self.source.index('endsWith("poeenter")')
        poe_start_at = self.source.index('endsWith("poestart")')
        generic_start_at = self.source.index('endsWith("start")')

        self.assertLess(poe_enter_at, generic_start_at)
        self.assertLess(poe_start_at, generic_start_at)
        self.assertIn("poeEnterFlag = true;", self.source)
        self.assertIn("poeStartFlag = true;", self.source)
        self.assertIn('Serial.println("[EVENT] poestart accepted");', self.source)

    def test_loop_enters_poe_page_and_runs_poe_test(self):
        required_snippets = [
            "if (poeEnterFlag &&",
            "enterPoePage();",
            "if (poeStartFlag && !poeBusy)",
            "runPoeTest();",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_poe_run_reports_mode_a_and_mode_b_not_available(self):
        required_snippets = [
            "void runPoeTest()",
            "poeBusy = true;",
            "poeStartFlag = false;",
            "float va = estimatePoeInputVoltageV(readPoeAdcVoltageV(PIN_POE_ADC_A));",
            'String vbText = POE_MODE_B_AVAILABLE ? "-- V" : "N/A";',
            "poeShowResult(va, vbText, mode, status);",
            "poeBusy = false;",
        ]

        for snippet in required_snippets:
            self.assertIn(snippet, self.source)

    def test_poe_status_thresholds_match_mvp_design(self):
        self.assertRegex(self.source, r"if\s*\(va\s*>\s*POE_OVER_RANGE_V\)")
        self.assertRegex(self.source, r"if\s*\(va\s*>=\s*POE_PRESENT_MIN_V\)")
        self.assertRegex(self.source, r"if\s*\(va\s*<=\s*NO_POE_MAX_V\)")
        self.assertIn('return "OVER_RANGE";', self.source)
        self.assertIn('return "OK";', self.source)
        self.assertIn('return "NO_POE";', self.source)
        self.assertIn('return "UNSAFE";', self.source)
        self.assertIn('return "A";', self.source)
        self.assertIn('return "UNKNOWN";', self.source)


if __name__ == "__main__":
    unittest.main()
