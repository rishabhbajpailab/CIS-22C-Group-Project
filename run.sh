#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ ! -x "$root_dir/build/ppp" ]]; then
  "$root_dir/build.sh"
fi

cd "$root_dir/FantasySportsApplication"
"$root_dir/build/ppp"
