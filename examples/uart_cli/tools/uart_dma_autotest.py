#!/usr/bin/env python3
"""Automated binary UART0/EDMA loopback test for the AM335x UART CLI."""

from __future__ import annotations

import argparse
import dataclasses
import logging
import struct
import sys
import time
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ModuleNotFoundError:
    serial = None


TARGET_VID = 0x10C4
TARGET_PID = 0xEA60
BAUDRATE = 115200
FRAME_MAGIC = 0x55415254
FRAME_VERSION = 1
HEADER = struct.Struct("<IHHII")
DMA_ALIGNMENT = 8
MAX_PAYLOAD_SIZE = 6144


@dataclasses.dataclass(frozen=True)
class Profile:
    payload_size: int
    cycles: int


PROFILES = {
    "smoke": Profile(payload_size=256, cycles=1),
    "stress": Profile(payload_size=6144, cycles=10),
    "full": Profile(payload_size=6144, cycles=100),
}


class TestFailure(RuntimeError):
    pass


def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def make_payload(size: int, seed: int) -> bytes:
    state = seed & 0xFFFFFFFF
    if state == 0:
        state = 0x6D2B79F5
    result = bytearray(size)
    for index in range(size):
        state ^= (state << 13) & 0xFFFFFFFF
        state ^= state >> 17
        state ^= (state << 5) & 0xFFFFFFFF
        state &= 0xFFFFFFFF
        result[index] = state & 0xFF
    return bytes(result)


def make_frame(payload_size: int, sequence: int, seed: int) -> bytes:
    header = HEADER.pack(FRAME_MAGIC, FRAME_VERSION, sequence, payload_size, seed)
    body = header + make_payload(payload_size, seed)
    final_size = (len(body) + 2 + DMA_ALIGNMENT - 1) & ~(DMA_ALIGNMENT - 1)
    body += bytes(final_size - len(body) - 2)
    return body + struct.pack("<H", crc16_ccitt_false(body))


def find_serial_port() -> str | None:
    matches = [
        port.device
        for port in serial.tools.list_ports.comports()
        if port.vid == TARGET_VID and port.pid == TARGET_PID
    ]
    if len(matches) > 1:
        raise TestFailure(f"multiple CP210x ports found: {', '.join(matches)}; use --port")
    return matches[0] if matches else None


def read_line_until(ser: serial.Serial, marker: bytes, deadline: float) -> bytes:
    while time.monotonic() < deadline:
        line = ser.readline()
        if line:
            logging.debug("device: %r", line.rstrip())
            if marker in line:
                return line
    raise TestFailure(f"timeout waiting for {marker.decode('ascii', errors='replace')}")


def read_exactly(ser: serial.Serial, size: int, deadline: float) -> bytes:
    result = bytearray()
    while len(result) < size and time.monotonic() < deadline:
        chunk = ser.read(size - len(result))
        if chunk:
            result.extend(chunk)
    if len(result) != size:
        raise TestFailure(f"echo timeout: received {len(result)} of {size} bytes")
    return bytes(result)


def first_mismatch(expected: bytes, actual: bytes) -> str:
    for index, (left, right) in enumerate(zip(expected, actual)):
        if left != right:
            return f"offset={index} expected=0x{left:02X} actual=0x{right:02X}"
    if len(expected) != len(actual):
        return f"length expected={len(expected)} actual={len(actual)}"
    return "none"


def synchronize(ser: serial.Serial) -> None:
    ser.reset_input_buffer()
    ser.write(b"\x03\n")
    ser.flush()
    read_line_until(ser, b"uart> ", time.monotonic() + 2.0)


def run_cycle(ser: serial.Serial, payload_size: int, sequence: int, seed: int) -> float:
    frame = make_frame(payload_size, sequence, seed)
    command = f"auto dma {payload_size} {sequence} {seed}\n".encode("ascii")
    ser.write(command)
    ser.flush()

    ready = read_line_until(ser, b"@READY", time.monotonic() + 2.0)
    expected_ready = f"sequence={sequence}".encode("ascii")
    if expected_ready not in ready or f"frame={len(frame)}".encode("ascii") not in ready:
        raise TestFailure(f"invalid READY response: {ready!r}")

    time.sleep(0.05)  # временная диагностика гонки READY/RX DMA

    started = time.monotonic()
    ser.write(frame)
    ser.flush()
    wire_seconds = (len(frame) * 10.0) / BAUDRATE
    echo = read_exactly(ser, len(frame), time.monotonic() + max(2.0, wire_seconds * 4.0))
    elapsed = time.monotonic() - started

    if echo != frame:
        raise TestFailure(f"echo mismatch: {first_mismatch(frame, echo)}")
    echo_crc = struct.unpack_from("<H", echo, len(echo) - 2)[0]
    calculated_crc = crc16_ccitt_false(echo[:-2])
    if echo_crc != calculated_crc:
        raise TestFailure(
            f"echo CRC mismatch: received=0x{echo_crc:04X} calculated=0x{calculated_crc:04X}"
        )

    result = read_line_until(ser, b"@RESULT", time.monotonic() + 2.0)
    if b"status=PASS" not in result:
        raise TestFailure(f"device rejected frame: {result.decode('ascii', errors='replace').strip()}")
    return elapsed


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="serial device; CP210x is detected when omitted")
    parser.add_argument("--profile", choices=PROFILES, default="smoke")
    parser.add_argument("--size", type=int, help="override profile payload size")
    parser.add_argument("--cycles", type=int, help="override profile cycle count")
    parser.add_argument("--seed", type=lambda value: int(value, 0), default=0x12345678)
    parser.add_argument("--log", type=Path, help="write a detailed UTF-8 log file")
    parser.add_argument("--self-test", action="store_true", help="verify framing and CRC without UART hardware")
    parser.add_argument("-v", "--verbose", action="store_true")
    return parser.parse_args()


def configure_logging(args: argparse.Namespace) -> None:
    handlers: list[logging.Handler] = [logging.StreamHandler()]
    if args.log:
        handlers.append(logging.FileHandler(args.log, mode="w", encoding="utf-8"))
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)-7s %(message)s",
        handlers=handlers,
    )


def main() -> int:
    args = parse_args()
    configure_logging(args)
    profile = PROFILES[args.profile]
    payload_size = args.size if args.size is not None else profile.payload_size
    cycles = args.cycles if args.cycles is not None else profile.cycles
    if args.self_test:
        if crc16_ccitt_false(b"123456789") != 0x29B1:
            raise TestFailure("CRC-16/CCITT-FALSE check vector failed")
        frame = make_frame(6144, 7, 0x12345678)
        if len(frame) != 6168 or len(frame) % DMA_ALIGNMENT != 0:
            raise TestFailure(f"frame layout check failed: size={len(frame)}")
        if crc16_ccitt_false(frame[:-2]) != struct.unpack_from("<H", frame, len(frame) - 2)[0]:
            raise TestFailure("frame CRC check failed")
        logging.info("SELF-TEST status=PASS crc=0x29B1 frame_size=%d", len(frame))
        return 0
    if not 1 <= payload_size <= MAX_PAYLOAD_SIZE:
        raise TestFailure(f"payload size must be in range 1..{MAX_PAYLOAD_SIZE}")
    if not 1 <= cycles <= 0x10000:
        raise TestFailure("cycles must be in range 1..65536")

    if serial is None:
        raise TestFailure("pyserial is not installed; run: python3 -m pip install -r tools/requirements.txt")

    port = args.port or find_serial_port()
    if not port:
        raise TestFailure("CP210x port not found; specify it with --port")

    logging.info(
        "DMA profile=%s port=%s baud=%d payload=%d cycles=%d",
        args.profile, port, BAUDRATE, payload_size, cycles,
    )
    passed = 0
    total_time = 0.0
    with serial.Serial(port, BAUDRATE, timeout=0.05, write_timeout=2.0) as ser:
        synchronize(ser)
        for cycle in range(cycles):
            sequence = cycle & 0xFFFF
            seed = (args.seed + cycle * 0x9E3779B9) & 0xFFFFFFFF
            try:
                elapsed = run_cycle(ser, payload_size, sequence, seed)
            except (TestFailure, OSError) as error:
                logging.error("cycle=%d/%d FAIL: %s", cycle + 1, cycles, error)
                return 1
            passed += 1
            total_time += elapsed
            throughput = (2.0 * payload_size) / elapsed if elapsed else 0.0
            logging.info(
                "cycle=%d/%d PASS frame_payload=%d elapsed=%.3fs roundtrip=%.1f KiB/s",
                cycle + 1, cycles, payload_size, elapsed, throughput / 1024.0,
            )

    logging.info(
        "SUMMARY status=PASS passed=%d failed=0 elapsed=%.3fs",
        passed, total_time,
    )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (TestFailure, OSError) as error:
        logging.error("FATAL: %s", error)
        sys.exit(2)
