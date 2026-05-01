import importlib.util
import os
import pathlib
import tempfile
import unittest


SCRIPT_PATH = pathlib.Path(__file__).resolve().parents[1] / "sdr_length_tcp_server.py"


class SdrLengthTcpServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        spec = importlib.util.spec_from_file_location("sdr_length_tcp_server", SCRIPT_PATH)
        cls.server = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.server)

    def test_protocol_formats_success_and_errors(self):
        self.assertEqual(self.server.format_success(8.4321), "LEN,8.43\n")
        self.assertEqual(self.server.format_error("LOW_SIGNAL"), "ERR,LOW_SIGNAL\n")
        self.assertEqual(self.server.format_error("bad value!"), "ERR,BAD_VALUE\n")

    def test_start_command_runs_measurement_callback(self):
        calls = []

        def measure():
            calls.append("measure")
            return 12.345

        reply = self.server.handle_command("START", measure)

        self.assertEqual(calls, ["measure"])
        self.assertEqual(reply, "LEN,12.35\n")

    def test_unknown_command_does_not_run_measurement(self):
        def measure():
            raise AssertionError("measurement should not run")

        reply = self.server.handle_command("HELLO", measure)

        self.assertEqual(reply, "ERR,BAD_CMD\n")

    def test_missing_measurement_dependency_returns_specific_error(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            script_path = pathlib.Path(temp_dir) / "missing_dep_measure.py"
            script_path.write_text("import not_a_real_adi_dep\n", encoding="utf-8")

            reply = self.server.handle_command(
                "START",
                lambda: self.server.measure_length_once(script_path=script_path, runs=1),
            )

        self.assertEqual(reply, "ERR,MISSING_NOT_A_REAL_ADI_DEP\n")

    def test_mock_measurement_function_returns_fixed_length(self):
        measure = self.server.build_measure_func(
            script_path=pathlib.Path("unused.py"),
            runs=3,
            mock_length=7.91,
        )

        self.assertEqual(measure(), 7.91)

    def test_parse_args_accepts_mock_length(self):
        args = self.server.parse_args(["--mock-length", "8.43"])

        self.assertEqual(args.mock_length, 8.43)

    def test_measure_length_runs_from_measurement_script_directory(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            script_path = pathlib.Path(temp_dir) / "fake_measure.py"
            marker_path = pathlib.Path(temp_dir) / "marker.txt"
            marker_path.write_text("calibration-here", encoding="utf-8")
            script_path.write_text(
                "\n".join(
                    [
                        "import pathlib",
                        "def configure_sdr():",
                        "    return object()",
                        "def load_cal():",
                        "    return None",
                        "def run_selftest(_sdr, _cal):",
                        "    assert pathlib.Path('marker.txt').exists()",
                        "    quality = {'verdict': 'OK'}",
                        "    return (1.25, 1.25, 0, 0, 0, 0, quality), None",
                        "def tx_destroy(_sdr):",
                        "    pass",
                    ]
                ),
                encoding="utf-8",
            )

            original_cwd = pathlib.Path.cwd()
            measured = self.server.measure_length_once(script_path=script_path, runs=1)

            self.assertEqual(pathlib.Path.cwd(), original_cwd)
            self.assertEqual(measured, 1.25)


if __name__ == "__main__":
    unittest.main()
