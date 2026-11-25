# VS-22C-GP (Perfect Playoff Predictions)

## Project Pitch
An NFL playoff predictor with a what-if editor for team records. It downloads current team records and
yards from [nflverse](https://github.com/nflverse) (free, no API key), or loads them from a CSV file.

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
./run.sh                        # asks where to get data (default: download the latest season)
./run.sh --download             # download the latest season without asking
./run.sh --season 2020          # download a past season (2002 or later)
./run.sh data/data.csv          # load a data file instead
./run.sh --download-only        # just save data/nflverse-<season>.csv and exit (for scripts)
./run.sh --no-clear             # keep scrollback (no screen clearing between menus)
```
Run from the repository root (as `run.sh` does) so the `data/` paths resolve. Downloading needs the
`curl` command-line tool, which ships with Windows 10+, macOS and most Linux distributions.

### Live data
Team data comes from two public nflverse files, fetched over HTTPS with no API key:
- [`games.csv`](https://github.com/nflverse/nfldata/blob/master/data/games.csv): every game's score since 1999,
  used for wins, losses and win percentage. Unplayed games are ignored, so mid-season data is "standings so far".
- [`stats_team_reg_<season>.csv`](https://github.com/nflverse/nflverse-data/releases/tag/stats_team): regular-season
  team totals, used for net offensive yards (passing + rushing - sack yards).

Each download is saved to `data/nflverse-<season>.csv`. If a later download fails (for example, offline), the
app uses that saved copy, or `data/data.csv` if there isn't one. Use menu option `6) Download Latest Data` to
refresh or switch seasons while the app is running. See `docs/data-format.md` for how each column is derived.

### Build (Windows)
See `build-windows.md` for Visual Studio and CMake steps.

### Minimal Example
- From the main menu, choose `4) Who Will Make The Playoffs?` to display the predicted playoff bracket.
- Choose `1) Edit Current Teams` to adjust a team's wins/losses/yards, then rerun the playoff bracket
  to see the effect (edits recompute the team's PPI).

## Current Capabilities
- Downloads any season from 2002 on from nflverse, with an offline fallback to the last saved copy.
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
| `FantasySportsApplication/IO/` | CSV reading, data file loading/validation and report output |
| `FantasySportsApplication/Nflverse/` | nflverse download, franchise table (codes, names, divisions), caching |
| `FantasySportsApplication/Net/` | HTTP download via the `curl` command-line tool |
| `FantasySportsApplication/UI/` | Console input/output helpers |
| `FantasySportsApplication/App.*`, `main.cpp` | Menu-driven CLI |
| `tests/` | Unit and regression tests (run with `./test.sh` or `ctest`) |
| `data/data.csv` | Bundled offline data (2014 regular season, hand-entered) |
| `tests/fixtures/` | Trimmed copies of the 2014 nflverse files used by the tests |

## Limitations
- CLI only.
- Menu option `2) Add or Remove a Team` is not implemented yet (it explains how to edit the data file instead).
- Downloads support the 2002 season onward (the current 32-team, 8-division alignment).
- Ties are counted in the win percentage but not shown, since data files have no ties column.
- The hash table is keyed by total yards, so two teams with the same yards total share one entry.

## Roadmap
1. Fix the wild card selection so a division winner cannot also be seeded as a wild card
   (see "Known algorithm quirks"); with live data this now happens in most seasons.
2. Wire the add/remove team menu flow to actual logic.
3. Use more nflverse stats (points, turnovers, ...) in the PPI, and measure accuracy against past seasons.

## Known algorithm quirks
These are deliberate carry-overs from the original algorithm, kept so results do not change:
- `reorderVec` is not a full sort (its inner loop starts at index 1 instead of `i + 1`).
- `getWildCards` tries to remove the division leaders from the wild card pool with `==`, but the leaders
  carry their scaled PPI and the pool does not, so nothing is removed; wild cards come from the
  "skip the first three" offset instead. A division winner can therefore also be seeded as a wild card:
  the 2026 (through week 11) bracket lists the Buffalo Bills as both the AFC 2 and 6 seed, and 2020 repeats
  three teams.
- Results depend on the order of teams in the data file, because of the `reorderVec` quirk. Downloaded data
  is always ordered by division and then team code, so results are repeatable.
- `scaledPPI` multiplies the PPI by itself, so scaled values are in the tens of millions.

## Change Log

Version 2.1.0
--
- Live data from nflverse: `--download`, `--season YEAR`, `--download-only`, a startup prompt, and main menu
  option `6) Download Latest Data` (Quit is now option 7).
- Downloads are cached in `data/nflverse-<season>.csv`, with an offline fallback to the cache or `data/data.csv`.
- Data files may now include `#` comment lines; the playoff header shows the data source and week.
- New tests (`CsvTests`, `NflverseTests`) run offline against trimmed real 2014 nflverse files. They check that
  the derived yards, wins and divisions match the hand-entered `data/data.csv` for every team.

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
