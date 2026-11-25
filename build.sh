#!/usr/bin/env bash
# Configures and builds the CLI and tests with CMake. Output: build/ppp
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$root_dir/build"

cmake -S "$root_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE="${BUILD_TYPE:-Release}"
cmake --build "$build_dir" --parallel

echo "Build complete: $build_dir/ppp"
