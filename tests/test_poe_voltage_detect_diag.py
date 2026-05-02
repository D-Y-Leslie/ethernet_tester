import pathlib
import re
import unittest


SKETCH_PATH = (
    pathlib.Path(__file__).resolve().parents[1]
    / "poe_voltage_detect_diag"
    / "poe_voltage_detect_diag.ino"
)


class PoeVoltageDetectDiagSketchTest(unittest.TestCase):
    def setUp(self):
        self.assertTrue(
            SKETCH_PATH.exists(),
            f"Expected diagnostic sketch at {SKETCH_PATH}",
        )
        self.source = SKETCH_PATH.read_text(encoding="utf-8")

    def test_mode_a_gpio2_and_current_divider_are_configured(self):
        self.assertIn("static const int PIN_POE_ADC_A = 2;", self.source)
        self.assertIn("static const int PIN_POE_ADC_B = 13;", self.source)
        self.assertIn("static const float POE_DIVIDER_RATIO = 20.6078f;", self.source)
        self.assertIn("static const uint16_t POE_ADC_SAMPLES = 120;", self.source)
        self.assertIn("analogSetPinAttenuation(PIN_POE_ADC_A, ADC_11db);", self.source)
        self.assertIn("analogSetPinAttenuation(PIN_POE_ADC_B, ADC_11db);", self.source)

    def test_status_thresholds_match_mvp_design(self):
        self.assertIn("static const float NO_POE_MAX_V = 5.0f;", self.source)
        self.assertIn("static const float POE_PRESENT_MIN_V = 30.0f;", self.source)
        self.assertIn("static const float POE_OVER_RANGE_V = 60.0f;", self.source)
        self.assertRegex(self.source, r"if\s*\(va\s*>\s*POE_OVER_RANGE_V\s*\|\|\s*vb\s*>\s*POE_OVER_RANGE_V\)")
        self.assertIn("return \"OVER_RANGE\";", self.source)
        self.assertIn("return \"NO_POE\";", self.source)
        self.assertIn("return \"UNSAFE\";", self.source)
        self.assertIn("return \"A+B\";", self.source)
        self.assertIn("return \"B\";", self.source)

    def test_serial_output_uses_poe_diag_csv_format(self):
        expected_tokens = [
            '"POE_DIAG,ms="',
            '",adcA="',
            '",adcB="',
            '",VA="',
            '",VB="',
            '",mode="',
            '",status="',
            '",unit=V"',
        ]
        for token in expected_tokens:
            self.assertIn(token, self.source)

    def test_diagnostic_sketch_does_not_touch_tjc_or_wire_map_pins(self):
        forbidden = [
            "Serial1",
            "WiFi",
            "TJC",
            "GPIO4",
            "GPIO5",
            "GPIO6",
            "GPIO8",
            "GPIO9",
            "GPIO10",
            "GPIO11",
        ]
        for token in forbidden:
            self.assertNotIn(token, self.source)


if __name__ == "__main__":
    unittest.main()
