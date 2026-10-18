#!/usr/bin/env bash
set -euo pipefail

pio run
pio run -t compiledb
pio run -t upload
pio device monitor -b 115200
