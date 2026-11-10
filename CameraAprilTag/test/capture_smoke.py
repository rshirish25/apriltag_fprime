#!/usr/bin/env python3
"""Exercise the real deployment with temporary fake Pi capture executables.

Usage: python3 CameraAprilTag/test/capture_smoke.py /absolute/path/to/CameraAprilTag
Uses the deployment's default F Prime framing and Unix serialization types.
"""
import csv
import os
from pathlib import Path
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import time
import zlib


def wait_for(predicate, process, timeout=10):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise AssertionError("Deployment exited unexpectedly")
        if predicate():
            return
        time.sleep(0.05)
    raise AssertionError("Timed out waiting for deployment output")


def send_command(connection, opcode, arguments):
    # FprimeProtocol header, U16 command descriptor, U32 opcode, CRC32 trailer.
    body = struct.pack(">HI", 0, opcode) + arguments
    frame = struct.pack(">II", 0xDEADBEEF, len(body)) + body
    connection.sendall(frame + struct.pack(">I", zlib.crc32(frame)))


def string_argument(value):
    data = str(value).encode()
    return struct.pack(">Q", len(data)) + data  # Unix FwSizeStoreType = U64


def main():
    binary = Path(sys.argv[1]).resolve()
    fixture = Path(__file__).resolve().parents[2] / "images/four_320x240.png"
    with tempfile.TemporaryDirectory(prefix="camera-apriltag-") as temporary:
        work = Path(temporary)
        tools = work / "bin"
        tools.mkdir()
        images = work / ("images_" + "x" * 28)
        images.mkdir()
        (tools / "sudo").write_text("#!/bin/sh\nexit 0\n")
        (tools / "rpicam-still").write_text(
            '#!/bin/sh\n'
            'printf incomplete > "$2"\n'
            'printf "%s" "$2" > "$TEST_WORK/capture_started"\n'
            'while [ ! -f "$TEST_WORK/finish_capture" ]; do sleep 0.05; done\n'
            'cp "$TEST_IMAGE" "$2"\n'
            'if [ -f "$TEST_WORK/fail_capture" ]; then exit 1; fi\n'
        )
        for tool in tools.iterdir():
            tool.chmod(0o755)
        environment = dict(os.environ, PATH=str(tools) + os.pathsep + os.environ["PATH"],
                           TEST_WORK=str(work), TEST_IMAGE=str(fixture))
        with socket.socket() as server, (work / "deployment.log").open("w+") as log:
            server.bind(("127.0.0.1", 0))
            server.listen(1)
            server.settimeout(10)
            process = subprocess.Popen(
                [str(binary), "-a", "127.0.0.1", "-p", str(server.getsockname()[1])],
                cwd=work, env=environment, stdout=log, stderr=subprocess.STDOUT,
            )
            try:
                connection, _ = server.accept()
                with connection:
                    poses = work / "poses.csv"
                    send_command(connection, 0x10030000,
                                 struct.pack(">I", 0) + string_argument(images.name))
                    wait_for(lambda: (work / "capture_started").exists(), process)
                    time.sleep(0.2)
                    assert not poses.exists(), "AprilTag ran before capture completed"
                    (work / "finish_capture").touch()

                    def rows():
                        if not poses.exists():
                            return []
                        with poses.open() as stream:
                            return list(csv.DictReader(stream))

                    wait_for(lambda: len(rows()) == 4, process)
                    assert {int(row["tag_id"]) for row in rows()} == {0, 1, 2, 3}
                    assert {row["frame"] for row in rows()} == {"0"}
                    assert len((work / "capture_started").read_text()) > 40

                    # A failed capture must not send a path, even if it wrote a valid image.
                    (work / "capture_started").unlink()
                    (work / "fail_capture").touch()
                    send_command(connection, 0x10030000,
                                 struct.pack(">I", 0) + string_argument(images.name))
                    wait_for(lambda: (work / "capture_started").exists(), process)
                    time.sleep(0.5)
                    assert len(rows()) == 4, "Failed capture was forwarded"

                    # The original manual command still processes images.
                    (work / "input.png").symlink_to(fixture)
                    send_command(connection, 0x10020000, string_argument("input.png"))
                    wait_for(lambda: len(rows()) == 8, process)
                    assert {row["frame"] for row in rows()[4:]} == {"1"}
                    send_command(connection, 0x10020000, string_argument("missing.png"))
                    time.sleep(0.3)
                    assert process.poll() is None and len(rows()) == 8
                print("PASS: completed capture -> async image path -> four tag poses")
                print("PASS: incomplete/failed captures do not produce pose rows")
                print("PASS: manual image command and missing-image handling")
            except BaseException:
                log.flush()
                log.seek(0)
                print(log.read(), file=sys.stderr)
                raise
            finally:
                process.send_signal(signal.SIGINT)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


if __name__ == "__main__":
    main()
