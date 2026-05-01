import importlib.util
import io
import math
import pathlib
import unittest
from contextlib import redirect_stdout

import numpy as np


SCRIPT_PATH = pathlib.Path(__file__).resolve().parents[1] / "pluto_2x2_channel_swap_test.py"
SPEC = importlib.util.spec_from_file_location("pluto_2x2_channel_swap_test", SCRIPT_PATH)
channel_swap = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(channel_swap)


class ChannelSwapExperimentTest(unittest.TestCase):
    def test_default_stage_plan_names_and_wiring(self):
        stages = channel_swap.default_stages()

        self.assertEqual(
            [stage.name for stage in stages],
            [
                "normal_loopback",
                "rx_swapped_loopback",
                "sma_a_dut_b_ref",
                "sma_b_dut_a_ref",
                "dut_forward",
                "dut_reversed",
            ],
        )
        self.assertIn("TX1A -> RX1A", stages[0].instructions[0])
        self.assertIn("TX1A -> RX2A", stages[1].instructions[0])
        self.assertTrue(stages[0].expect_near_zero)
        self.assertFalse(stages[-1].expect_near_zero)

    def test_stage_start_prompt_requires_enter_with_tx_off(self):
        stage = channel_swap.default_stages()[1]

        prompt = channel_swap.stage_start_prompt(stage)

        self.assertIn("RX channel swapped loopback", prompt)
        self.assertIn("TX output is OFF", prompt)
        self.assertIn("change hardware", prompt)
        self.assertIn("Press Enter to START this test", prompt)

    def test_fit_delay_keeps_signed_result(self):
        tones_hz = np.linspace(0.5e6, 14e6, 55)
        tau_s = -8.0e-9
        ratios = np.exp(-1j * 2.0 * np.pi * tones_hz * tau_s)

        result = channel_swap.fit_delay(tones_hz, ratios, vf=0.66)

        self.assertAlmostEqual(result.tau_ns, -8.0, places=9)
        self.assertLess(result.signed_length_m, 0.0)
        self.assertAlmostEqual(result.length_m, abs(result.signed_length_m), places=12)
        self.assertLess(result.residual_deg_rms, 1e-9)

    def test_summarize_runs_reports_mean_span_and_verdict(self):
        rows = [
            channel_swap.RunResult("stage", 1, 1.0, 0.20, 0.20, 55, -5.0, -4.0, 0.4, "OK"),
            channel_swap.RunResult("stage", 2, 2.0, 0.30, 0.30, 55, -5.0, -4.0, 0.6, "OK"),
            channel_swap.RunResult("stage", 3, 3.0, 0.40, 0.40, 54, -6.0, -4.0, 0.8, "OK"),
        ]

        summary = channel_swap.summarize_runs(rows)

        self.assertEqual(summary["stage"], "stage")
        self.assertEqual(summary["runs"], 3)
        self.assertAlmostEqual(summary["mean_tau_ns"], 2.0)
        self.assertAlmostEqual(summary["span_signed_length_m"], 0.20)
        self.assertEqual(summary["min_valid_tones"], 54)
        self.assertEqual(summary["verdict"], "OK")

    def test_summarize_runs_reports_zero_offset_corrected_mean(self):
        rows = [
            channel_swap.RunResult("stage", 1, 1.0, 0.20, 0.20, 55, -5.0, -4.0, 0.4, "OK"),
            channel_swap.RunResult("stage", 2, 2.0, 0.30, 0.30, 55, -5.0, -4.0, 0.6, "OK"),
        ]

        summary = channel_swap.summarize_runs(rows, zero_offset_m=0.05)

        self.assertAlmostEqual(summary["mean_corrected_signed_length_m"], 0.20)
        self.assertAlmostEqual(summary["mean_corrected_length_m"], 0.20)

    def test_summarize_runs_marks_unstable_when_span_is_large(self):
        rows = [
            channel_swap.RunResult("stage", 1, 1.0, 0.20, 0.20, 55, -5.0, -4.0, 0.4, "OK"),
            channel_swap.RunResult("stage", 2, 6.0, 1.20, 1.20, 55, -5.0, -4.0, 0.6, "OK"),
        ]

        summary = channel_swap.summarize_runs(rows, max_span_m=0.50)

        self.assertEqual(summary["verdict"], "UNSTABLE")

    def test_quality_verdict_rejects_low_signal_or_few_tones(self):
        low_signal = channel_swap.quality_verdict(
            dut_amp_db=np.array([-45.0, -42.0, -41.0]),
            ref_amp_db=np.array([-4.0, -4.0, -4.0]),
            residual_deg_rms=1.0,
            min_valid_tones=2,
        )
        few_tones = channel_swap.quality_verdict(
            dut_amp_db=np.array([-20.0, -45.0, -45.0]),
            ref_amp_db=np.array([-4.0, -4.0, -4.0]),
            residual_deg_rms=1.0,
            min_valid_tones=2,
        )

        self.assertEqual(low_signal["verdict"], "LOW_SIGNAL")
        self.assertEqual(few_tones["verdict"], "FEW_VALID_TONES")

    def test_apply_zero_offset_subtracts_fixed_loopback_bias(self):
        corrected_signed, corrected_abs = channel_swap.apply_zero_offset(1.120, 0.055)

        self.assertAlmostEqual(corrected_signed, 1.065)
        self.assertAlmostEqual(corrected_abs, 1.065)

    def test_result_to_dict_includes_zero_offset_corrected_lengths(self):
        row = channel_swap.RunResult(
            "dut_forward",
            1,
            5.0,
            1.120,
            1.120,
            55,
            -6.0,
            -4.0,
            0.5,
            "OK",
        )

        data = channel_swap.result_to_dict(row, zero_offset_m=0.055)

        self.assertEqual(data["zero_offset_m"], "0.055000")
        self.assertEqual(data["corrected_signed_length_m"], "1.065000")
        self.assertEqual(data["corrected_length_m"], "1.065000")

    def test_print_run_includes_dut_and_ref_minimum_amplitudes(self):
        row = channel_swap.RunResult(
            "rx_swapped_loopback",
            1,
            -14.524,
            -2.874,
            2.874,
            0,
            -49.90,
            -3.25,
            151.22,
            "LOW_SIGNAL",
        )
        output = io.StringIO()

        with redirect_stdout(output):
            channel_swap.print_run(row, zero_offset_m=0.055)

        text = output.getvalue()
        self.assertIn("dut_min= -49.90 dB", text)
        self.assertIn("ref_min=  -3.25 dB", text)
        self.assertIn("corr_signed=  -2.929 m", text)


if __name__ == "__main__":
    unittest.main()
