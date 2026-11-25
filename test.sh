#!/usr/bin/env bash
# Builds and runs the unit tests.
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"$root_dir/build.sh"
ctest --test-dir "$root_dir/build" --output-on-failure
