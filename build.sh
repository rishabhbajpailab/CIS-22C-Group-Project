#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

build_dir="$root_dir/build"
mkdir -p "$build_dir"

cxx="${CXX:-g++}"

sources=(
  "$root_dir/FantasySportsApplication/FantasySportsApplication.cpp"
  "$root_dir/FantasySportsApplication/PlayoffPicture.cpp"
  "$root_dir/FantasySportsApplication/BST/binaryTree.cpp"
  "$root_dir/FantasySportsApplication/BSTree.cpp"
  "$root_dir/FantasySportsApplication/HashTable/HashTableTest.cpp"
)

"$cxx" -std=c++14 -I"$root_dir/FantasySportsApplication" "${sources[@]}" -o "$build_dir/ppp"

echo "Build complete: $build_dir/ppp"
