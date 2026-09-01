#!/usr/bin/env python3
"""Shared UART automation protocol and test-suite implementation."""

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
BAUDRATES = (
    300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800,
    38400, 57600, 115200, 230400, 460800, 921600, 1843200,
    3686400,
)
BASELINE_BAUD_INDEX = 11
BAUD_MATRIX_ORDER = (11, 12, 13, 14, 15, 16, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
FRAME_MAGIC = 0x55415254
FRAME_VERSION = 1
HEADER = struct.Struct("<IHHII")
DMA_ALIGNMENT = 8
MAX_PAYLOAD_SIZE = 6144
CASES_PER_SUITE = 60
TRANSPORTS = ("polling", "isr", "dma")
QUICK_BAUDS = (14400, 115200, 921600)


@dataclasses.dataclass(frozen=True)
class Profile:
    payload_size: int
    cycles: int


PROFILES = {
    "smoke": Profile(payload_size=256, cycles=1),
    "stress": Profile(payload_size=6144, cycles=10),
    "full": Profile(payload_size=6144, cycles=100),
}

TAIL_SIZES = tuple(range(2, 17)) + tuple(range(6144, 6152))


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


def make_tx_packet(size: int, seed: int) -> bytes:
    if size < 2:
        raise TestFailure("TX packet must have room for CRC16")
    data = make_payload(size - 2, seed)
    return data + struct.pack("<H", crc16_ccitt_false(data))


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
    started = time.monotonic()
    while len(result) < size and time.monotonic() < deadline:
        chunk = ser.read(size - len(result))
        if chunk:
            result.extend(chunk)
            logging.debug(
                "echo chunk=%d total=%d/%d after=%.6fs",
                len(chunk), len(result), size, time.monotonic() - started,
            )
    if len(result) != size:
        preview = bytes(result[:128])
        raise TestFailure(
            f"echo timeout after {time.monotonic() - started:.3f}s: "
            f"received {len(result)} of {size} bytes; first bytes={preview.hex(' ')}"
        )
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


def run_cycle(ser: serial.Serial, transport: str, payload_size: int, sequence: int, seed: int,
              ready_delay: float, echo_timeout: float) -> float:
    frame = make_frame(payload_size, sequence, seed)
    command = f"auto loop {transport} {payload_size} {sequence} {seed}\n".encode("ascii")
    ser.write(command)
    ser.flush()

    ready = read_line_until(ser, b"@READY", time.monotonic() + max(2.0, echo_timeout))
    expected_ready = f"sequence={sequence}".encode("ascii")
    if (f"mode={transport}".encode("ascii") not in ready or
            expected_ready not in ready or
            f"frame={len(frame)}".encode("ascii") not in ready):
        raise TestFailure(f"invalid READY response: {ready!r}")

    logging.debug("READY accepted; delaying %.3fs before frame TX", ready_delay)
    time.sleep(ready_delay)

    started = time.monotonic()
    written = ser.write(frame)
    ser.flush()
    logging.debug(
        "frame TX submitted bytes=%d/%d write_and_flush=%.6fs crc=0x%04X",
        written, len(frame), time.monotonic() - started,
        struct.unpack_from("<H", frame, len(frame) - 2)[0],
    )
    echo = read_exactly(ser, len(frame), time.monotonic() + echo_timeout)
    elapsed = time.monotonic() - started

    if echo != frame:
        raise TestFailure(f"echo mismatch: {first_mismatch(frame, echo)}")
    echo_crc = struct.unpack_from("<H", echo, len(echo) - 2)[0]
    calculated_crc = crc16_ccitt_false(echo[:-2])
    if echo_crc != calculated_crc:
        raise TestFailure(
            f"echo CRC mismatch: received=0x{echo_crc:04X} calculated=0x{calculated_crc:04X}"
        )

    result = read_line_until(ser, b"@RESULT", time.monotonic() + max(2.0, echo_timeout))
    if b"status=PASS" not in result:
        raise TestFailure(f"device rejected frame: {result.decode('ascii', errors='replace').strip()}")
    return elapsed


def run_tx_tail_case(ser: serial.Serial, transport: str, size: int, sequence: int,
                     seed: int, echo_timeout: float) -> float:
    expected = make_tx_packet(size, seed)
    command = f"auto tx {transport} {size} {sequence} {seed}\n".encode("ascii")
    ser.write(command)
    ser.flush()

    ready = read_line_until(ser, b"@READY", time.monotonic() + max(2.0, echo_timeout))
    required = f"mode=tx".encode("ascii")
    if (required not in ready or f"sequence={sequence}".encode("ascii") not in ready or
            f"size={size}".encode("ascii") not in ready or
            (transport != "dma" and f"transport={transport}".encode("ascii") not in ready)):
        raise TestFailure(f"invalid TX READY response: {ready!r}")

    started = time.monotonic()
    received = read_exactly(ser, size, time.monotonic() + echo_timeout)
    elapsed = time.monotonic() - started
    if received != expected:
        raise TestFailure(f"TX data mismatch: {first_mismatch(expected, received)}")

    received_crc = struct.unpack_from("<H", received, size - 2)[0]
    calculated_crc = crc16_ccitt_false(received[:-2])
    if received_crc != calculated_crc:
        raise TestFailure(
            f"TX CRC mismatch: received=0x{received_crc:04X} calculated=0x{calculated_crc:04X}"
        )

    result = read_line_until(ser, b"@RESULT", time.monotonic() + max(2.0, echo_timeout))
    if b"status=PASS" not in result:
        raise TestFailure(f"device TX failure: {result.decode('ascii', errors='replace').strip()}")
    return elapsed


def run_rx_tail_case(ser: serial.Serial, transport: str, size: int, sequence: int,
                     seed: int, ready_delay: float, result_timeout: float) -> float:
    packet = make_tx_packet(size, seed)
    command = f"auto rx {transport} {size} {sequence} {seed}\n".encode("ascii")
    ser.write(command)
    ser.flush()

    ready = read_line_until(ser, b"@READY", time.monotonic() + max(2.0, result_timeout))
    required = f"mode=rx".encode("ascii")
    if (required not in ready or f"sequence={sequence}".encode("ascii") not in ready or
            f"size={size}".encode("ascii") not in ready or
            (transport != "dma" and f"transport={transport}".encode("ascii") not in ready)):
        raise TestFailure(f"invalid RX READY response: {ready!r}")

    logging.debug("RX READY accepted; delaying %.3fs before packet TX", ready_delay)
    time.sleep(ready_delay)
    started = time.monotonic()
    written = ser.write(packet)
    ser.flush()
    elapsed = time.monotonic() - started
    if written != size:
        raise TestFailure(f"short RX test write: submitted {written} of {size} bytes")

    received_crc = struct.unpack_from("<H", packet, size - 2)[0]
    logging.debug(
        "RX packet submitted bytes=%d write_and_flush=%.6fs crc=0x%04X",
        written, elapsed, received_crc,
    )
    result = read_line_until(ser, b"@RESULT", time.monotonic() + result_timeout)
    if (b"mode=rx" not in result or
            f"sequence={sequence}".encode("ascii") not in result or
            f"size={size}".encode("ascii") not in result or
            b"crc=PASS" not in result or
            b"data=PASS" not in result or
            b"status=PASS" not in result):
        raise TestFailure(
            f"device RX failure: {result.decode('ascii', errors='replace').strip()}"
        )
    return time.monotonic() - started


def run_timeout_recovery_case(ser: serial.Serial, transport: str,
                              sequence: int, seed: int,
                              ready_delay: float, timeout: float) -> float:
    payload_size = 256
    command = (
        f"auto recover {transport} {payload_size} {sequence} {seed}\n"
    ).encode("ascii")
    started = time.monotonic()
    ser.write(command)
    ser.flush()
    ready = read_line_until(ser, b"@READY", time.monotonic() + max(2.0, timeout))
    required = (
        f"mode=recover transport={transport} sequence={sequence} stage=timeout"
    ).encode("ascii")
    if required not in ready:
        raise TestFailure(f"invalid recovery READY response: {ready!r}")

    # Intentionally send no bytes. The target must time out and release every
    # resource belonging to the selected transport.
    result = read_line_until(ser, b"@RESULT", time.monotonic() + max(2.0, timeout))
    if (b"mode=recover" not in result or
            f"transport={transport}".encode("ascii") not in result or
            b"timeout=PASS" not in result or b"status=PASS" not in result):
        raise TestFailure(
            f"timeout/recovery probe failed: "
            f"{result.decode('ascii', errors='replace').strip()}"
        )

    # A normal CRC loopback after the forced timeout proves that UART, ISR and
    # EDMA resources were restored, rather than merely reporting a timeout.
    run_cycle(ser, transport, payload_size, sequence, seed,
              ready_delay, timeout)
    return time.monotonic() - started


def run_unaligned_case(ser: serial.Serial, transport: str,
                       sequence: int, seed: int,
                       ready_delay: float, timeout: float) -> float:
    payload_size = MAX_PAYLOAD_SIZE
    frame = make_frame(payload_size, sequence, seed)
    command = (
        f"auto offset {transport} {payload_size} {sequence} {seed}\n"
    ).encode("ascii")
    ser.write(command)
    ser.flush()
    ready = read_line_until(ser, b"@READY", time.monotonic() + max(2.0, timeout))
    required = f"mode=offset transport={transport} sequence={sequence}".encode("ascii")
    if (required not in ready or b"offset=1" not in ready or
            f"frame={len(frame)}".encode("ascii") not in ready):
        raise TestFailure(f"invalid unaligned READY response: {ready!r}")

    time.sleep(ready_delay)
    started = time.monotonic()
    written = ser.write(frame)
    ser.flush()
    if written != len(frame):
        raise TestFailure(f"short unaligned write: {written} of {len(frame)}")
    echo = read_exactly(ser, len(frame), time.monotonic() + timeout)
    if echo != frame:
        raise TestFailure(f"unaligned echo mismatch: {first_mismatch(frame, echo)}")
    result = read_line_until(ser, b"@RESULT", time.monotonic() + max(2.0, timeout))
    if (b"mode=offset" not in result or b"crc=PASS" not in result or
            b"data=PASS" not in result or b"guard=PASS" not in result or
            b"status=PASS" not in result):
        raise TestFailure(
            f"unaligned/cache test failed: "
            f"{result.decode('ascii', errors='replace').strip()}"
        )
    return time.monotonic() - started


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="serial device; CP210x is detected when omitted")
    parser.add_argument("--profile", choices=PROFILES, default="smoke")
    parser.add_argument(
        "--mode",
        choices=("loopback", "tx-tail", "rx-tail", "suite", "baud-matrix"),
        default="loopback", help="test mode (default: loopback)",
    )
    parser.add_argument(
        "--transports", default="dma",
        help="comma-separated polling,isr,dma (default: dma)",
    )
    parser.add_argument("--size", type=int, help="override profile payload size")
    parser.add_argument("--cycles", type=int, help="override profile cycle count")
    parser.add_argument("--seed", type=lambda value: int(value, 0), default=0x12345678)
    parser.add_argument("--ready-delay", type=float, default=0.05,
                        help="seconds between READY and frame TX (default: 0.05)")
    parser.add_argument("--echo-timeout", type=float, default=30.0,
                        help="seconds to wait for the complete DMA echo (default: 30)")
    parser.add_argument(
        "--bauds",
        help="comma-separated baud values or enum indexes for baud-matrix; default: all",
    )
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


def suite_seed(base_seed: int, case_index: int) -> int:
    return (base_seed + case_index * 0x9E3779B9) & 0xFFFFFFFF


def run_loopback_group(ser: serial.Serial, transport: str, name: str, payload_size: int,
                       cycles: int, first_case: int, args: argparse.Namespace
                       ) -> tuple[int, float]:
    logging.info(
        "SECTION transport=%s name=%s status=BEGIN cases=%d payload=%d",
        transport, name, cycles, payload_size,
    )
    section_time = 0.0
    for cycle in range(cycles):
        case_index = first_case + cycle
        try:
            elapsed = run_cycle(
                ser, transport, payload_size, case_index & 0xFFFF,
                suite_seed(args.seed, case_index),
                args.ready_delay, args.echo_timeout,
            )
        except (TestFailure, OSError) as error:
            raise TestFailure(
                f"section={name} cycle={cycle + 1}/{cycles}: {error}"
            ) from error
        section_time += elapsed
        throughput = (2.0 * payload_size) / elapsed if elapsed else 0.0
        logging.info(
            "transport=%s section=%s cycle=%d/%d PASS frame_payload=%d "
            "elapsed=%.3fs roundtrip=%.1f KiB/s",
            transport, name, cycle + 1, cycles, payload_size,
            elapsed, throughput / 1024.0,
        )
    logging.info(
        "SECTION name=%s status=PASS passed=%d failed=0 elapsed=%.3fs",
        name, cycles, section_time,
    )
    return cycles, section_time


def run_tail_group(ser: serial.Serial, transport: str, mode: str, cycles: int,
                   first_case: int, args: argparse.Namespace
                   ) -> tuple[int, float]:
    total_cases = len(TAIL_SIZES) * cycles
    logging.info("SECTION transport=%s name=%s status=BEGIN cases=%d",
                 transport, mode, total_cases)
    section_time = 0.0
    local_case = 0
    for _cycle in range(cycles):
        for size in TAIL_SIZES:
            case_index = first_case + local_case
            local_case += 1
            try:
                if mode == "tx-tail":
                    elapsed = run_tx_tail_case(
                        ser, transport, size, case_index & 0xFFFF,
                        suite_seed(args.seed, case_index), args.echo_timeout,
                    )
                else:
                    elapsed = run_rx_tail_case(
                        ser, transport, size, case_index & 0xFFFF,
                        suite_seed(args.seed, case_index), args.ready_delay,
                        args.echo_timeout,
                    )
            except (TestFailure, OSError) as error:
                raise TestFailure(
                    f"section={mode} case={local_case}/{total_cases} "
                    f"size={size} tail={size % DMA_ALIGNMENT}: {error}"
                ) from error
            section_time += elapsed
            logging.info(
                "section=%s case=%d/%d PASS size=%d dma=%d tail=%d elapsed=%.3fs",
                mode, local_case, total_cases, size,
                size - size % DMA_ALIGNMENT, size % DMA_ALIGNMENT, elapsed,
            )
    logging.info(
        "SECTION name=%s status=PASS passed=%d failed=0 elapsed=%.3fs",
        mode, total_cases, section_time,
    )
    return total_cases, section_time


def run_complete_suite(ser: serial.Serial, transport: str,
                       args: argparse.Namespace) -> tuple[int, float]:
    suite_cases = 14 + 2 * len(TAIL_SIZES)
    if suite_cases != CASES_PER_SUITE:
        raise TestFailure(f"internal suite size mismatch: {suite_cases}")
    logging.info("SUITE transport=%s status=BEGIN cases=%d full_profile=SKIPPED",
                 transport, suite_cases)
    passed = 0
    total_time = 0.0
    groups = (
        ("smoke", 256, 1),
        ("large-6144", 6144, 1),
        ("stress", 6144, 10),
    )
    for name, group_payload, group_cycles in groups:
        group_passed, group_time = run_loopback_group(
            ser, transport, name, group_payload, group_cycles, passed, args,
        )
        passed += group_passed
        total_time += group_time
    for tail_mode in ("tx-tail", "rx-tail"):
        group_passed, group_time = run_tail_group(
            ser, transport, tail_mode, 1, passed, args,
        )
        passed += group_passed
        total_time += group_time

    logging.info("SECTION transport=%s name=recovery status=BEGIN cases=1",
                 transport)
    try:
        recovery_elapsed = run_timeout_recovery_case(
            ser, transport, passed & 0xFFFF, suite_seed(args.seed, passed),
            args.ready_delay, args.echo_timeout,
        )
    except (TestFailure, OSError) as error:
        raise TestFailure(f"section=recovery: {error}") from error
    passed += 1
    total_time += recovery_elapsed
    logging.info(
        "SECTION transport=%s name=recovery status=PASS passed=1 failed=0 elapsed=%.3fs",
        transport, recovery_elapsed,
    )

    logging.info("SECTION transport=%s name=unaligned-cache status=BEGIN cases=1",
                 transport)
    try:
        unaligned_elapsed = run_unaligned_case(
            ser, transport, passed & 0xFFFF, suite_seed(args.seed, passed),
            args.ready_delay, args.echo_timeout,
        )
    except (TestFailure, OSError) as error:
        raise TestFailure(f"section=unaligned-cache: {error}") from error
    passed += 1
    total_time += unaligned_elapsed
    logging.info(
        "SECTION transport=%s name=unaligned-cache status=PASS passed=1 failed=0 elapsed=%.3fs",
        transport, unaligned_elapsed,
    )
    logging.info(
        "SUITE transport=%s status=PASS passed=%d failed=0 full_profile=SKIPPED",
        transport, passed,
    )
    return passed, total_time


def switch_baudrate(ser: serial.Serial, old_index: int, new_index: int,
                    timeout: float) -> None:
    old_baud = BAUDRATES[old_index]
    new_baud = BAUDRATES[new_index]
    command = f"auto baud {new_index}\n".encode("ascii")
    ser.write(command)
    ser.flush()
    ready = read_line_until(ser, b"@BAUD READY", time.monotonic() + timeout)
    required = (
        f"old_index={old_index} old={old_baud} "
        f"new_index={new_index} new={new_baud}"
    ).encode("ascii")
    if required not in ready:
        raise TestFailure(f"invalid BAUD READY response: {ready!r}")

    try:
        ser.baudrate = new_baud
    except (ValueError, OSError) as error:
        raise TestFailure(f"adapter rejected baud={new_baud}: {error}") from error
    ser.reset_input_buffer()
    sync = bytes((0x55, 0xAA, new_index, new_index ^ 0xFF))
    ser.write(sync)
    ser.flush()
    active = read_line_until(ser, b"@BAUD ACTIVE", time.monotonic() + timeout)
    expected = f"index={new_index} baud={new_baud}".encode("ascii")
    if expected not in active:
        raise TestFailure(f"invalid BAUD ACTIVE response: {active!r}")
    logging.info(
        "BAUD_SWITCH status=PASS old_index=%d old=%d new_index=%d new=%d",
        old_index, old_baud, new_index, new_baud,
    )


def matrix_timeout(baud: int, minimum: float) -> float:
    largest_frame = len(make_frame(MAX_PAYLOAD_SIZE, 0, 0x12345678))
    roundtrip_wire_seconds = 2.0 * largest_frame * 10.0 / baud
    return max(minimum, roundtrip_wire_seconds * 1.5 + 5.0)


def parse_baud_selection(value: str | None) -> tuple[int, ...]:
    if value is None:
        return BAUD_MATRIX_ORDER
    result = []
    for item in value.split(","):
        token = item.strip()
        if not token:
            raise TestFailure("--bauds contains an empty item")
        try:
            numeric = int(token, 0)
        except ValueError as error:
            raise TestFailure(f"invalid baud/index in --bauds: {token}") from error
        if numeric in BAUDRATES:
            index = BAUDRATES.index(numeric)
        elif 0 <= numeric < len(BAUDRATES):
            index = numeric
        else:
            raise TestFailure(f"unknown baud/index in --bauds: {token}")
        if index not in result:
            result.append(index)
    if not result:
        raise TestFailure("--bauds must select at least one baud")
    return tuple(result)


def parse_transport_selection(value: str) -> tuple[str, ...]:
    result = []
    for item in value.split(","):
        transport = item.strip().lower()
        if transport not in TRANSPORTS:
            raise TestFailure(f"unknown transport: {transport or '<empty>'}")
        if transport not in result:
            result.append(transport)
    if not result:
        raise TestFailure("at least one transport must be selected")
    return tuple(result)


def main() -> int:
    args = parse_args()
    configure_logging(args)
    profile = PROFILES[args.profile]
    transports = parse_transport_selection(args.transports)
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
        for size in TAIL_SIZES:
            packet = make_tx_packet(size, 0x12345678)
            if len(packet) != size or crc16_ccitt_false(packet[:-2]) != struct.unpack_from("<H", packet, size - 2)[0]:
                raise TestFailure(f"TX packet self-test failed for size={size}")
        if len(BAUDRATES) != 17 or BAUDRATES[BASELINE_BAUD_INDEX] != BAUDRATE:
            raise TestFailure("baud table self-test failed")
        if parse_baud_selection("11,230400,13") != (11, 12, 13):
            raise TestFailure("baud selection self-test failed")
        if parse_transport_selection("polling,isr,dma") != TRANSPORTS:
            raise TestFailure("transport selection self-test failed")
        if QUICK_BAUDS != (14400, 115200, 921600):
            raise TestFailure("quick baud selection self-test failed")
        if 14 + 2 * len(TAIL_SIZES) != CASES_PER_SUITE:
            raise TestFailure("suite case-count self-test failed")
        logging.info("SELF-TEST status=PASS crc=0x29B1 frame_size=%d", len(frame))
        return 0
    if not 1 <= payload_size <= MAX_PAYLOAD_SIZE:
        raise TestFailure(f"payload size must be in range 1..{MAX_PAYLOAD_SIZE}")
    if not 1 <= cycles <= 0x10000:
        raise TestFailure("cycles must be in range 1..65536")
    if args.ready_delay < 0.0 or args.echo_timeout <= 0.0:
        raise TestFailure("--ready-delay must be >= 0 and --echo-timeout must be > 0")

    if serial is None:
        raise TestFailure("pyserial is not installed; run: python3 -m pip install -r tools/requirements.txt")

    port = args.port or find_serial_port()
    if not port:
        raise TestFailure("CP210x port not found; specify it with --port")

    logging.info(
        "UART mode=%s transports=%s profile=%s port=%s baud=%d payload=%d cycles=%d",
        args.mode, ",".join(transports), args.profile, port, BAUDRATE,
        payload_size, cycles,
    )
    passed = 0
    total_time = 0.0
    wall_started = time.monotonic()
    with serial.Serial(port, BAUDRATE, timeout=0.05, write_timeout=2.0) as ser:
        synchronize(ser)
        try:
            if args.mode == "loopback":
                if len(transports) != 1:
                    raise TestFailure("loopback mode requires exactly one transport")
                passed, total_time = run_loopback_group(
                    ser, transports[0], args.profile, payload_size, cycles, 0, args,
                )
            elif args.mode in ("tx-tail", "rx-tail"):
                if len(transports) != 1:
                    raise TestFailure("tail modes require exactly one transport")
                passed, total_time = run_tail_group(
                    ser, transports[0], args.mode, cycles, 0, args)
            elif args.mode == "suite":
                for transport in transports:
                    count, elapsed = run_complete_suite(ser, transport, args)
                    passed += count
                    total_time += elapsed
            else:
                selected_bauds = parse_baud_selection(args.bauds)
                rates_passed = 0
                rates_unsupported = 0
                rates_failed = 0
                logging.info(
                    "BAUD_MATRIX status=BEGIN rates=%d transports=%d cases_per_rate=%d order=%s",
                    len(selected_bauds), len(transports),
                    CASES_PER_SUITE * len(transports),
                    ",".join(str(BAUDRATES[index]) for index in selected_bauds),
                )
                for rate_number, baud_index in enumerate(selected_bauds, 1):
                    baud = BAUDRATES[baud_index]
                    rate_timeout = matrix_timeout(baud, args.echo_timeout)
                    original_timeout = args.echo_timeout
                    switched = False
                    suite_started = False
                    logging.info(
                        "BAUD_RATE status=BEGIN rate=%d/%d index=%d baud=%d timeout=%.1fs",
                        rate_number, len(selected_bauds), baud_index, baud, rate_timeout,
                    )
                    try:
                        switch_baudrate(
                            ser, BASELINE_BAUD_INDEX, baud_index,
                            max(30.0, 500.0 * 10.0 / min(BAUDRATE, baud)),
                        )
                        switched = True
                        ser.write_timeout = max(2.0, rate_timeout)
                        args.echo_timeout = rate_timeout
                        suite_started = True
                        rate_passed = 0
                        rate_time = 0.0
                        for transport in transports:
                            count, elapsed = run_complete_suite(ser, transport, args)
                            rate_passed += count
                            rate_time += elapsed
                        passed += rate_passed
                        total_time += rate_time
                        rates_passed += 1
                        logging.info(
                            "BAUD_RATE status=PASS rate=%d/%d index=%d baud=%d "
                            "passed=%d failed=0 elapsed=%.3fs",
                            rate_number, len(selected_bauds), baud_index, baud,
                            rate_passed, rate_time,
                        )
                    except (TestFailure, OSError) as error:
                        if suite_started:
                            rates_failed += 1
                            logging.error(
                                "BAUD_RATE status=FAIL rate=%d/%d index=%d baud=%d error=%s",
                                rate_number, len(selected_bauds), baud_index, baud, error,
                            )
                        else:
                            rates_unsupported += 1
                            logging.warning(
                                "BAUD_RATE status=UNSUPPORTED rate=%d/%d index=%d "
                                "baud=%d adapter_or_link=%s",
                                rate_number, len(selected_bauds), baud_index, baud, error,
                            )
                    finally:
                        args.echo_timeout = original_timeout
                        ser.write_timeout = max(2.0, original_timeout)
                        try:
                            if switched:
                                switch_baudrate(
                                    ser, baud_index, BASELINE_BAUD_INDEX,
                                    max(30.0, 500.0 * 10.0 / min(BAUDRATE, baud)),
                                )
                            else:
                                ser.baudrate = BAUDRATE
                                time.sleep(1.0)
                                synchronize(ser)
                        except (TestFailure, OSError, ValueError) as recovery_error:
                            raise TestFailure(
                                f"cannot recover baseline baud after {baud}: {recovery_error}"
                            ) from recovery_error
                logging.info(
                    "BAUD_MATRIX status=%s rates_passed=%d rates_unsupported=%d "
                    "rates_failed=%d hardware_cases_passed=%d",
                    "PASS" if rates_failed == 0 else "FAIL",
                    rates_passed, rates_unsupported, rates_failed, passed,
                )
                if rates_failed != 0:
                    return 1
        except (TestFailure, OSError) as error:
            logging.error(
                "SUMMARY status=FAIL passed=%d failed=1 wall_elapsed=%.3fs error=%s",
                passed, time.monotonic() - wall_started, error,
            )
            return 1

    logging.info(
        "SUMMARY status=PASS passed=%d failed=0 elapsed=%.3fs wall_elapsed=%.3fs",
        passed, total_time, time.monotonic() - wall_started,
    )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (TestFailure, OSError) as error:
        logging.error("FATAL: %s", error)
        sys.exit(2)
