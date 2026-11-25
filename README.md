# VS-22C-GP (Perfect Playoff Predictions)

## Project Pitch
An NFL playoff predictor with a what-if editor for team records. It downloads the current season's games,
records and yards from [nflverse](https://github.com/nflverse) (free, no API key), or loads them from a CSV file,
and seeds the playoffs with the NFL's own rules and tiebreakers.

## Project Status
Legacy C++ coursework codebase, modernized to C++17 with a CMake build and automated tests.
The original Playoff Performance Index (PPI) bracket was replaced in version 3.0 by standings, seeding and
NFL tiebreakers computed from game results. On final regular-season data this reproduces every real playoff
field and every wild card matchup from 2002 to 2025 (see `docs/algorithm-revision.md`).

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
  to see the effect. Edited wins and losses change the team's record; tiebreakers still use the real games.

## Current Capabilities
- Downloads any season from 2002 on from nflverse, with an offline fallback to the last saved copy.
- Loads and validates team data from a CSV file (see `docs/data-format.md`): 32 teams, 4 per division.
- Supports interactive edits to team stats (wins/losses/yards).
- Displays teams unsorted, or sorted with a binary search tree (yards, wins, losses, win %, name).
- Seeds the AFC and NFC playoffs by the NFL's rules: division winners are seeds 1-4 by record, then the wild
  cards (7 per conference from 2020, 6 before). Ties are broken with the NFL tiebreakers (head-to-head,
  division and conference records, common games, strength of victory and schedule, points rankings, net
  points), and the bracket says which tiebreaker decided each place. Mid-season it shows the bracket
  "if the season ended today" plus the next three teams in line.
- Reports hash table statistics (load factor, bucket use).
- Optionally saves the current teams to `Output.txt` on exit.

## Project Layout
| Path | Contents |
| --- | --- |
| `FantasySportsApplication/Team/` | `Team` model (record, yards, division) |
| `FantasySportsApplication/Season/` | Standings from game results, NFL tiebreakers and playoff seeding |
| `FantasySportsApplication/HashTable/` | Separate-chaining hash table template |
| `FantasySportsApplication/BST/` | Binary search tree used for sorted output |
| `FantasySportsApplication/IO/` | CSV reading, data file loading/validation and report output |
| `FantasySportsApplication/Nflverse/` | nflverse download, franchise table (codes, names, divisions), caching |
| `FantasySportsApplication/Net/` | HTTP download via the `curl` command-line tool |
| `FantasySportsApplication/UI/` | Console input/output helpers |
| `FantasySportsApplication/App.*`, `main.cpp` | Menu-driven CLI |
| `tests/` | Unit tests (run with `./test.sh` or `ctest`) |
| `data/data.csv`, `data/data.games.csv` | Bundled offline data: the 2014 regular season from nflverse, with its games |
| `tests/fixtures/` | Trimmed copies of the 2014 nflverse files used by the tests |

## Limitations
- CLI only.
- Menu option `2) Add or Remove a Team` is not implemented yet (it explains how to edit the data file instead).
- Downloads support the 2002 season onward (the current 32-team, 8-division alignment).
- Data files have no ties column, so ties come from the games file next to the data file (downloads and the
  bundled data have one). A data file without games gets no game-based tiebreakers: teams with the same
  record are ordered alphabetically.
- The NFL's "net touchdowns" tiebreaker is skipped (nflverse game data has scores but not touchdowns), and
  the final coin toss is replaced by alphabetical order so results are repeatable.
- The bracket is "if the season ended today": it does not yet forecast the remaining games.
- The hash table is keyed by total yards, so two teams with the same yards total share one entry.

## Roadmap
See `docs/algorithm-revision.md` for the full plan. Next:
1. A team strength rating (point margin per game, shrunk early in the season) to replace the PPI as the
   "quality" number.
2. Simulate the remaining games to give each team's odds of making the playoffs, winning its division and
   each seed.
3. A what-if editor at the game level ("set who wins this game"), built on the simulator.
4. Bring the backtest into the repository so every algorithm change is measured against past seasons.
5. Wire the add/remove team menu flow to actual logic.

## Change Log

Unreleased (3.0.0)
--
- New playoff algorithm: standings from game results, seeding by the NFL's rules (division winners 1-4,
  then wild cards; 7 seeds per conference from 2020, 6 before) and the NFL tiebreakers. It replaces the PPI
  bracket, which picked about two-thirds of real playoff teams and often seeded a team twice.
- The bracket lists real seeds, shows which tiebreaker decided each place, marks mid-season brackets
  "If the season ended today", and lists the next three teams in each conference.
- Downloads also save the season's games (`data/nflverse-<season>.games.csv`), so tiebreakers work offline.
- `data/data.csv` is now the 2014 season as derived from nflverse (fixing the hand-entered Tampa Bay record
  and the unexplained `percentage` values), with its games in `data/data.games.csv`.
- The win percentage is computed from wins, losses and ties (ties count half), so edits change it; the
  file's `percentage` column is still validated but no longer used.
- Removed the PPI (`Playoff/PlayoffLogic.*`, `Team::scaledPPI`); the "PPI" sort option is now "Win %".
- GitHub Actions CI no longer runs automatically on push or pull request (manual "Run workflow" only)
  while builds are validated locally during the proof-of-concept stage.

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
