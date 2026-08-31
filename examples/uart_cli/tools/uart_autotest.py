#!/usr/bin/env python3
"""Fast UART acceptance test: 3 transports x 3 practical baud rates x 58 cases."""

from __future__ import annotations

import argparse
import sys

import uart_test_core


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="serial device; CP210x is detected when omitted")
    parser.add_argument("--log", help="write a detailed UTF-8 log file")
    parser.add_argument("--ready-delay", type=float, default=0.05)
    parser.add_argument("--echo-timeout", type=float, default=30.0)
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    forwarded = [
        sys.argv[0], "--mode", "baud-matrix",
        "--transports", ",".join(uart_test_core.TRANSPORTS),
        "--bauds", ",".join(str(value) for value in uart_test_core.QUICK_BAUDS),
        "--ready-delay", str(args.ready_delay),
        "--echo-timeout", str(args.echo_timeout),
    ]
    if args.port:
        forwarded += ["--port", args.port]
    if args.log:
        forwarded += ["--log", args.log]
    if args.self_test:
        forwarded.append("--self-test")
    if args.verbose:
        forwarded.append("--verbose")
    sys.argv = forwarded
    return uart_test_core.main()


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (uart_test_core.TestFailure, OSError) as error:
        uart_test_core.logging.error("FATAL: %s", error)
        sys.exit(2)
