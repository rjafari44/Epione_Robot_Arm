#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

# compile
arduino-cli compile \
  --fqbn arduino:avr:mega \
  --build-property "compiler.cpp.extra_flags=-Iinclude" .

# upload
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega

# monitor
arduino-cli monitor -p /dev/ttyACM0 --config baudrate=115200