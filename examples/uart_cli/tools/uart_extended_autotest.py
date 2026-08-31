#!/usr/bin/env python3
"""Extended configurable UART test runner."""

from __future__ import annotations

import sys

import uart_test_core


if __name__ == "__main__":
    try:
        sys.exit(uart_test_core.main())
    except (uart_test_core.TestFailure, OSError) as error:
        uart_test_core.logging.error("FATAL: %s", error)
        sys.exit(2)
