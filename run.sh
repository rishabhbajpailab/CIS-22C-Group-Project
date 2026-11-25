#!/usr/bin/env bash
# Builds if needed, then runs the CLI from the repository root so data/data.csv resolves.
# Any arguments are passed through, e.g. ./run.sh data/data.csv
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ ! -x "$root_dir/build/ppp" ]]; then
  "$root_dir/build.sh"
fi

cd "$root_dir"
exec "$root_dir/build/ppp" "$@"
