# VS-22C-GP (Perfect Playoff Predictions)

## Project Pitch
An NFL playoff predictor with a what-if editor for team records and quick bracket generation from a CSV data set.

## Project Status
Legacy C++ coursework codebase, modernized to C++17 with a CMake build, automated tests and CI.
The playoff, hash table and binary search tree algorithms are unchanged from the original project;
regression tests pin their output to what the original code produced.

## Quickstart
Requires CMake 3.16+ and a C++17 compiler (GCC 8+, Clang 7+, or Visual Studio 2019+).

### Build and test (Linux/macOS)
```bash
./build.sh   # builds build/ppp
./test.sh    # builds and runs the unit tests
```

### Run (Linux/macOS)
```bash
./run.sh                    # asks whether to use the default data/data.csv
./run.sh data/data.csv      # loads a specific file without asking
./run.sh --no-clear         # keeps scrollback (no screen clearing between menus)
```
Run from the repository root (as `run.sh` does) so the default `data/data.csv` path resolves.

### Build (Windows)
See `build-windows.md` for Visual Studio and CMake steps.

### Minimal Example
- From the main menu, choose `4) Who Will Make The Playoffs?` to display the predicted playoff bracket.
- Choose `1) Edit Current Teams` to adjust a team's wins/losses/yards, then rerun the playoff bracket
  to see the effect (edits recompute the team's PPI).

## Current Capabilities
- Loads and validates team data from a CSV file (see `docs/data-format.md`): 32 teams, 4 per division.
- Supports interactive edits to team stats (wins/losses/yards).
- Displays teams unsorted, or sorted with a binary search tree (yards, wins, losses, PPI, name).
- Generates AFC/NFC playoff brackets with wildcard selection.
- Reports hash table statistics (load factor, bucket use).
- Optionally saves the current teams to `Output.txt` on exit.

## Project Layout
| Path | Contents |
| --- | --- |
| `FantasySportsApplication/Team/` | `Team` model and Playoff Performance Index (PPI) formulas |
| `FantasySportsApplication/Playoff/` | Playoff bracket algorithm |
| `FantasySportsApplication/HashTable/` | Separate-chaining hash table template |
| `FantasySportsApplication/BST/` | Binary search tree used for sorted output |
| `FantasySportsApplication/IO/` | CSV loading, validation and report output |
| `FantasySportsApplication/UI/` | Console input/output helpers |
| `FantasySportsApplication/App.*`, `main.cpp` | Menu-driven CLI |
| `tests/` | Unit and regression tests (run with `./test.sh` or `ctest`) |
| `data/data.csv` | Bundled team data (2014 regular season, i.e. the January 2015 playoffs) |

## Limitations
- CLI only.
- Menu option `2) Add or Remove a Team` is not implemented yet (it explains how to edit the data file instead).
- Team data must be updated manually in `data/data.csv` (no automated data feed yet).
- The hash table is keyed by total yards, so two teams with the same yards total share one entry.

## Roadmap
1. Fetch current-season data automatically instead of editing `data/data.csv` by hand.
2. Wire the add/remove team menu flow to actual logic.
3. Review the known quirks in the playoff algorithm (see "Known algorithm quirks" below) and decide
   which to correct, using the regression tests to measure the change.

## Known algorithm quirks
These are deliberate carry-overs from the original algorithm, kept so results do not change:
- `reorderVec` is not a full sort (its inner loop starts at index 1 instead of `i + 1`).
- `getWildCards` tries to remove the division leaders from the wild card pool with `==`, but the leaders
  carry their scaled PPI and the pool does not, so nothing is removed; wild cards come from the
  "skip the first three" offset instead, and a division leader could in principle also be seeded as a wild card.
- `scaledPPI` multiplies the PPI by itself, so scaled values are in the tens of millions.

## Change Log

Version 2.0.0
--
- Fixed the build on Linux/macOS (missing header case, invalid `return void;`, broken unused
  `LinkedList.h`) and on Visual Studio (precompiled header setting in Release, removed files).
- Moved to C++17 and CMake; added `test.sh` and GitHub Actions CI on Linux, macOS and Windows.
- Split header-only implementations into `.h`/`.cpp` pairs (fixes multiple-definition link errors).
- Replaced the recursive menu calls with a loop, and made all input line-based with validation.
- CSV loading now validates every row and the league shape, reports errors with line numbers,
  accepts Windows line endings and an optional header row.
- Team edits now recompute the PPI (previously edits had no effect on the bracket) and cannot go negative.
- Fixed: sorting by yards also printed the wins sort; exit confirmation ignored "no";
  `run.sh` started in the wrong directory, so the default data file was never found.
- Fixed memory handling in the hash table (`delete[]`, copy assignment) and binary search tree (nodes were never freed;
  deleting the root or duplicate keys corrupted the tree). The load factor is now computed correctly.
- Removed dead code: `LinkedList.h`, `BSTree.*`, the unused templated `binaryTree` and its demo `main`,
  `PlayoffPicture.cpp`, `BinarySearchTree.h`, `Menu options and input.h`, and `data/2016.csv` / `data/2017.csv`
  (both were byte-for-byte copies of `data/data.csv`).
- Corrected the expectations in `tests/PlayoffLogicTests.cpp`, which did not match the algorithm's real output.

Version 1.0.0
--
- Change Log created.
