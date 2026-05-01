#!/usr/bin/env python3
import argparse
import contextlib
import importlib.util
import os
import pathlib
import re
import socket
import sys
import traceback


DEFAULT_SCRIPT = pathlib.Path(__file__).resolve().parent / "tmp_pluto_sdr_work" / "pluto_2x2_multitone_selftest.py"
DEFAULT_HOST = "0.0.0.0"
DEFAULT_PORT = 9000


class MeasurementRejected(RuntimeError):
    pass


@contextlib.contextmanager
def script_working_directory(script_path):
    original_cwd = pathlib.Path.cwd()
    os.chdir(pathlib.Path(script_path).resolve().parent)
    try:
        yield
    finally:
        os.chdir(original_cwd)


def sanitize_reason(reason):
    text = str(reason).strip().upper()
    text = re.sub(r"[^A-Z0-9_]+", "_", text).strip("_")
    return (text or "UNKNOWN")[:32]


def format_success(length_m):
    return f"LEN,{float(length_m):.2f}\n"


def format_error(reason):
    return f"ERR,{sanitize_reason(reason)}\n"


def load_measurement_module(script_path):
    path = pathlib.Path(script_path).resolve()
    spec = importlib.util.spec_from_file_location("pluto_2x2_multitone_selftest", path)
    module = importlib.util.module_from_spec(spec)
    try:
        spec.loader.exec_module(module)
    except ModuleNotFoundError as exc:
        raise MeasurementRejected(f"MISSING_{exc.name}") from exc
    return module


def measure_length_once(script_path=DEFAULT_SCRIPT, runs=1):
    with script_working_directory(script_path):
        module = load_measurement_module(script_path)
        sdr = module.configure_sdr()
        accepted_lengths = []
        last_quality = None

        try:
            cal_data = module.load_cal()
            if cal_data is None:
                cal_ratios = None
                ref_length_m = 0.0
            else:
                cal_ratios = cal_data["ratios"]
                ref_length_m = cal_data["ref_length_m"]

            for _idx in range(runs):
                result, _ratios = module.run_selftest(sdr, cal_ratios)
                length_m, signed_length_m, _tau, _slope, _phase, _residual_deg, quality = result
                last_quality = quality

                if quality["verdict"] != "OK":
                    continue

                if cal_ratios is None:
                    accepted_lengths.append(length_m)
                else:
                    accepted_lengths.append(ref_length_m + signed_length_m)

            if not accepted_lengths:
                verdict = last_quality["verdict"] if last_quality else "NO_DATA"
                raise MeasurementRejected(verdict)

            return sum(accepted_lengths) / len(accepted_lengths)
        finally:
            module.tx_destroy(sdr)


def handle_command(command, measure_func):
    if command.strip().upper() != "START":
        return format_error("BAD_CMD")

    try:
        return format_success(measure_func())
    except MeasurementRejected as exc:
        return format_error(str(exc))
    except Exception:
        traceback.print_exc()
        return format_error("MEASURE_FAIL")


def read_command(conn):
    data = b""
    while b"\n" not in data and len(data) < 64:
        chunk = conn.recv(1)
        if not chunk:
            break
        data += chunk
    return data.decode("ascii", errors="ignore").strip()


def build_measure_func(script_path, runs, mock_length=None):
    if mock_length is not None:
        return lambda: float(mock_length)

    return lambda: measure_length_once(script_path=script_path, runs=runs)


def serve_forever(host, port, script_path, runs, mock_length=None):
    measure = build_measure_func(script_path, runs, mock_length=mock_length)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, port))
        sock.listen(1)
        print(f"SDR length TCP server listening on {host}:{port}")
        if mock_length is not None:
            print(f"Mock length mode: {mock_length:.2f} m")

        while True:
            conn, addr = sock.accept()
            with conn:
                print(f"Client connected: {addr[0]}:{addr[1]}")
                command = read_command(conn)
                reply = handle_command(command, measure)
                conn.sendall(reply.encode("ascii"))
                print(f"Command={command!r} Reply={reply.strip()}")


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description="TCP bridge for ESP32-triggered SDR length measurement.")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--script", default=str(DEFAULT_SCRIPT))
    parser.add_argument("--runs", type=int, default=1)
    parser.add_argument("--mock-length", type=float, default=None, help="return a fixed LEN value without SDR hardware")
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    if args.runs < 1:
        raise SystemExit("--runs must be at least 1")
    serve_forever(args.host, args.port, pathlib.Path(args.script), args.runs, mock_length=args.mock_length)


if __name__ == "__main__":
    main(sys.argv[1:])
