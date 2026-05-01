import json
import os
import sys
import tempfile
import types
import unittest

import numpy as np

sys.modules.setdefault("adi", types.SimpleNamespace(ad9361=lambda *args, **kwargs: None))

import pluto_2x2_multitone_selftest as selftest


def latest_8m_demo_measurement():
    length_m = 8.0
    tau = length_m / (selftest.VF_CABLE * selftest.C)
    rng = np.random.default_rng(42)
    ratios = np.exp(1j * rng.uniform(-np.pi, np.pi, len(selftest.TONES_HZ)))

    stable_slice = slice(4, 15)
    ratios[stable_slice] = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ[stable_slice] * tau)

    valid_mask = np.ones(len(selftest.TONES_HZ), dtype=bool)
    valid_mask[[0, 1, 2, 3, 18, 23, 27, 31]] = False
    dut_amp_db = np.where(valid_mask, -20.0, -35.0)
    ref_amp_db = np.where(valid_mask, -20.0, -35.0)
    return ratios, valid_mask, dut_amp_db, ref_amp_db


class Pluto2x2MultitoneSelftestTest(unittest.TestCase):
    def test_defaults_match_pre_md(self):
        self.assertEqual(selftest.URI, "ip:192.168.2.1")
        self.assertEqual(selftest.SAMPLE_RATE, 30_720_000)
        self.assertEqual(selftest.LO_HZ, 100_000_000)
        self.assertEqual(selftest.TX_GAIN_DB, -45)
        self.assertEqual(selftest.RX_GAIN_DB, 10)
        self.assertEqual(selftest.RF_BW, 28_000_000)
        self.assertEqual(selftest.N, 131072)
        self.assertEqual(selftest.N_AVG, 16)
        self.assertEqual(selftest.TONE_START_HZ, 500_000)
        self.assertEqual(selftest.TONE_STOP_HZ, 6_250_000)
        self.assertEqual(selftest.TONE_COUNT, 32)
        self.assertEqual(len(selftest.TONES_HZ), 32)
        bin_hz = selftest.SAMPLE_RATE / selftest.N
        self.assertAlmostEqual(float(selftest.TONES_HZ[0]), round(500_000.0 / bin_hz) * bin_hz)
        self.assertAlmostEqual(float(selftest.TONES_HZ[-1]), round(6_250_000.0 / bin_hz) * bin_hz)

    def test_make_multitone_returns_complex64_and_is_scaled(self):
        tone = selftest.make_multitone()

        self.assertEqual(tone.dtype, np.complex64)
        self.assertEqual(tone.shape, (selftest.N,))
        self.assertLessEqual(float(np.max(np.abs(tone))), selftest.TOTAL_SCALE * (2**14) + 1e-3)
        self.assertGreater(float(np.max(np.abs(tone))), 0.0)

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

    def test_apply_fit_quality_rejects_high_residual(self):
        quality = {
            "verdict": "OK",
            "valid_mask": np.ones(4, dtype=bool),
            "valid_count": 4,
        }

        result = selftest.apply_fit_quality(quality, residual_deg=3.5, max_residual_deg=3.0)

        self.assertEqual(result["verdict"], "HIGH_RESIDUAL")
        self.assertAlmostEqual(result["residual_deg"], 3.5)

    def test_estimate_length_from_synthetic_phase_slope(self):
        length_m = 2.5
        tau = length_m / (selftest.VF_CABLE * selftest.C)
        ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * tau)

        estimated, signed_length_m, tau_extra, _slope, _phase, residual_deg = selftest.estimate_length(ratios)

        self.assertAlmostEqual(estimated, length_m, places=6)
        self.assertAlmostEqual(signed_length_m, length_m, places=6)
        self.assertAlmostEqual(tau_extra, tau, places=15)
        self.assertLess(residual_deg, 1e-6)

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

        estimated, signed_length_m, tau_extra, _slope, _phase, residual_deg = selftest.estimate_length(
            bad,
            valid_mask=valid_mask,
        )

        self.assertAlmostEqual(estimated, length_m, places=6)
        self.assertAlmostEqual(signed_length_m, length_m, places=6)
        self.assertAlmostEqual(tau_extra, tau, places=15)
        self.assertLess(residual_deg, 1e-6)

    def test_estimate_length_rejects_single_phase_outlier(self):
        length_m = 2.5
        tau = length_m / (selftest.VF_CABLE * selftest.C)
        ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * tau)
        outlier_idx = 8
        ratios[outlier_idx] = np.exp(1j * np.deg2rad(67.61))

        estimated, signed_length_m, tau_extra, _slope, _phase, residual_deg = selftest.estimate_length(ratios)

        self.assertAlmostEqual(estimated, length_m, places=5)
        self.assertAlmostEqual(signed_length_m, length_m, places=5)
        self.assertAlmostEqual(tau_extra, tau, places=14)
        self.assertLess(residual_deg, 1e-6)

    def test_window_fit_recovers_from_bad_phase_point(self):
        length_m = 8.0
        tau = length_m / (selftest.VF_CABLE * selftest.C)
        ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * tau)
        ratios[8] *= -1
        valid_mask = np.ones(len(ratios), dtype=bool)
        valid_mask[[5, 16, 28]] = False

        result = selftest.find_stable_window_fit(ratios, valid_mask)

        self.assertEqual(result["verdict"], "OK")
        self.assertAlmostEqual(result["length_m"], length_m, places=1)
        self.assertGreaterEqual(result["tone_count"], selftest.MIN_WINDOW_TONES)
        self.assertGreaterEqual(result["bandwidth_hz"], selftest.MIN_WINDOW_BW_HZ)

    def test_window_fit_rejects_inconsistent_phase_regions(self):
        tau_short = 8.0 / (selftest.VF_CABLE * selftest.C)
        tau_long = 12.0 / (selftest.VF_CABLE * selftest.C)
        ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * tau_short)
        ratios[16:] = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ[16:] * tau_long)
        valid_mask = np.ones(len(ratios), dtype=bool)

        result = selftest.find_stable_window_fit(ratios, valid_mask)

        self.assertEqual(result["verdict"], "UNSTABLE_PHASE")
        self.assertGreaterEqual(result["candidate_count"], 0)

    def test_low_band_demo_fit_accepts_latest_8m_low_frequency_window(self):
        ratios, valid_mask, _dut_amp_db, _ref_amp_db = latest_8m_demo_measurement()

        result = selftest.find_low_band_demo_fit(ratios, valid_mask)

        self.assertEqual(result["verdict"], "OK")
        self.assertAlmostEqual(result["length_m"], 8.0, delta=0.25)
        self.assertLessEqual(result["stop_hz"], selftest.LOW_BAND_DEMO_STOP_HZ)
        self.assertGreaterEqual(result["bandwidth_hz"], selftest.MIN_LOW_BAND_DEMO_BW_HZ)

    def test_calibration_cancels_fixture_delay(self):
        fixture_m = 0.5
        cable_m = 1.12
        tau_fix = fixture_m / (selftest.VF_CABLE * selftest.C)
        tau_cable = cable_m / (selftest.VF_CABLE * selftest.C)

        cal_ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * tau_fix)
        meas_ratios = np.exp(-1j * 2 * np.pi * selftest.TONES_HZ * (tau_fix + tau_cable))

        calibrated = meas_ratios / cal_ratios
        estimated, signed_length_m, _, _, _, residual_deg = selftest.estimate_length(calibrated)

        self.assertAlmostEqual(estimated, cable_m, places=5)
        self.assertAlmostEqual(signed_length_m, cable_m, places=5)
        self.assertLess(residual_deg, 1e-6)

    def test_run_selftest_rejects_low_signal_before_fitting(self):
        ratios = np.ones(len(selftest.TONES_HZ), dtype=np.complex128)
        dut_amp_db = np.full(len(selftest.TONES_HZ), -60.0)
        ref_amp_db = np.full(len(selftest.TONES_HZ), -5.0)

        original_measure_ratios = selftest.measure_ratios
        original_measure_tones = getattr(selftest, "measure_tones", None)
        original_print_tone_table = selftest.print_tone_table

        try:
            selftest.measure_ratios = lambda _sdr: ratios
            selftest.measure_tones = lambda _sdr: (ratios, dut_amp_db, ref_amp_db)
            selftest.print_tone_table = lambda *args, **kwargs: None

            result, returned_ratios = selftest.run_selftest(object())
        finally:
            selftest.measure_ratios = original_measure_ratios
            if original_measure_tones is None:
                delattr(selftest, "measure_tones")
            else:
                selftest.measure_tones = original_measure_tones
            selftest.print_tone_table = original_print_tone_table

        length_m, signed_length_m, tau, _slope, _phase, residual_deg, quality = result
        self.assertIsNone(length_m)
        self.assertIsNone(signed_length_m)
        self.assertIsNone(tau)
        self.assertIsNone(residual_deg)
        self.assertEqual(quality["verdict"], "LOW_SIGNAL")
        np.testing.assert_allclose(returned_ratios, ratios)

    def test_run_selftest_accepts_nearly_complete_linear_loopback(self):
        ratios = np.ones(len(selftest.TONES_HZ), dtype=np.complex128)
        dut_amp_db = np.full(len(selftest.TONES_HZ), -20.0)
        ref_amp_db = np.full(len(selftest.TONES_HZ), -20.0)
        dut_amp_db[[1, 6, 25, 29, 30, 31]] = -31.0

        original_measure_ratios = selftest.measure_ratios
        original_measure_tones = getattr(selftest, "measure_tones", None)
        original_print_tone_table = selftest.print_tone_table

        try:
            selftest.measure_ratios = lambda _sdr: ratios
            selftest.measure_tones = lambda _sdr: (ratios, dut_amp_db, ref_amp_db)
            selftest.print_tone_table = lambda *args, **kwargs: None

            result, returned_ratios = selftest.run_selftest(object())
        finally:
            selftest.measure_ratios = original_measure_ratios
            if original_measure_tones is None:
                delattr(selftest, "measure_tones")
            else:
                selftest.measure_tones = original_measure_tones
            selftest.print_tone_table = original_print_tone_table

        length_m, _signed_length_m, _tau, _slope, _phase, _residual_deg, quality = result
        self.assertEqual(quality["verdict"], "OK")
        self.assertEqual(quality["valid_count"], 26)
        self.assertAlmostEqual(length_m, 0.0)
        np.testing.assert_allclose(returned_ratios, ratios)

    def test_run_selftest_rejects_too_few_default_tones_valid(self):
        ratios = np.ones(len(selftest.TONES_HZ), dtype=np.complex128)
        dut_amp_db = np.full(len(selftest.TONES_HZ), -20.0)
        ref_amp_db = np.full(len(selftest.TONES_HZ), -20.0)
        dut_amp_db[: len(selftest.TONES_HZ) - 11] = -31.0

        original_measure_ratios = selftest.measure_ratios
        original_measure_tones = getattr(selftest, "measure_tones", None)
        original_print_tone_table = selftest.print_tone_table

        try:
            selftest.measure_ratios = lambda _sdr: ratios
            selftest.measure_tones = lambda _sdr: (ratios, dut_amp_db, ref_amp_db)
            selftest.print_tone_table = lambda *args, **kwargs: None

            result, returned_ratios = selftest.run_selftest(object())
        finally:
            selftest.measure_ratios = original_measure_ratios
            if original_measure_tones is None:
                delattr(selftest, "measure_tones")
            else:
                selftest.measure_tones = original_measure_tones
            selftest.print_tone_table = original_print_tone_table

        length_m, _signed_length_m, _tau, _slope, _phase, _residual_deg, quality = result
        self.assertIsNone(length_m)
        self.assertEqual(quality["verdict"], "LOW_SIGNAL")
        self.assertEqual(quality["valid_count"], 11)
        np.testing.assert_allclose(returned_ratios, ratios)

    def test_run_selftest_uses_low_band_demo_after_full_window_rejects(self):
        ratios, _valid_mask, dut_amp_db, ref_amp_db = latest_8m_demo_measurement()

        original_measure_ratios = selftest.measure_ratios
        original_measure_tones = getattr(selftest, "measure_tones", None)
        original_print_tone_table = selftest.print_tone_table

        try:
            selftest.measure_ratios = lambda _sdr: ratios
            selftest.measure_tones = lambda _sdr: (ratios, dut_amp_db, ref_amp_db)
            selftest.print_tone_table = lambda *args, **kwargs: None

            result, returned_ratios = selftest.run_selftest(object())
        finally:
            selftest.measure_ratios = original_measure_ratios
            if original_measure_tones is None:
                delattr(selftest, "measure_tones")
            else:
                selftest.measure_tones = original_measure_tones
            selftest.print_tone_table = original_print_tone_table

        length_m, _signed_length_m, _tau, _slope, _phase, _residual_deg, quality = result
        self.assertEqual(quality["verdict"], "OK")
        self.assertEqual(quality["fit_mode"], "low_band_demo")
        self.assertTrue(quality["demo_warning"])
        self.assertAlmostEqual(length_m, 8.0, delta=0.25)
        np.testing.assert_allclose(returned_ratios, ratios)

    def test_summarize_lengths_reports_mean_span_and_relative_error(self):
        summary = selftest.summarize_lengths([1.009, 1.122, 1.013], known_length_m=1.12)

        self.assertAlmostEqual(summary["mean_m"], 1.048, places=3)
        self.assertAlmostEqual(summary["span_m"], 0.113, places=3)
        self.assertAlmostEqual(summary["relative_error_pct"], -6.428571, places=5)
        self.assertEqual(summary["count"], 3)

    def test_evaluate_repeatability_marks_large_span_unstable(self):
        summary = selftest.summarize_lengths([1.009, 1.122, 1.013], known_length_m=1.12)

        verdict = selftest.evaluate_repeatability(summary, max_span_m=0.10)

        self.assertEqual(verdict, "UNSTABLE")

    def test_parse_args_defaults_to_measure_single_run(self):
        args = selftest.parse_args([])

        self.assertEqual(args.mode, "measure")
        self.assertEqual(args.runs, 1)
        self.assertIsNone(args.known_length)

    def test_parse_args_accepts_runs_known_length_and_selftest(self):
        args = selftest.parse_args(["selftest", "--runs", "3", "--known-length", "0"])

        self.assertEqual(args.mode, "selftest")
        self.assertEqual(args.runs, 3)
        self.assertAlmostEqual(args.known_length, 0.0)

    def test_save_load_cal_roundtrip(self):
        ratios = np.array([1 + 2j, 3 + 4j, 5 + 6j])
        ref_length_m = 1.10
        with tempfile.NamedTemporaryFile(suffix=".json", delete=False) as f:
            path = f.name
        try:
            selftest.save_cal(ratios, ref_length_m, path)
            loaded = selftest.load_cal(path)
            np.testing.assert_allclose(loaded["ratios"], ratios)
            self.assertAlmostEqual(loaded["ref_length_m"], ref_length_m)
        finally:
            os.unlink(path)

    def test_load_cal_returns_none_when_missing(self):
        self.assertIsNone(selftest.load_cal("/nonexistent/path.json"))


if __name__ == "__main__":
    unittest.main()
