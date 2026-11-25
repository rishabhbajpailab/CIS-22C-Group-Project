# CLI Integration Test Plan

This plan covers a quick, manual CLI smoke test for the Fantasy Sports application.

Automated coverage of the algorithms and data loading lives in `tests/` (`./test.sh`); this plan covers
the interactive menus.

## Prerequisites
- Build with `./build.sh` (or the Visual Studio solution).
- Run from the repository root (`./run.sh`) so `data/data.csv` is found.
- Tip: `./run.sh --no-clear` keeps the full session in scrollback, and the menus can be scripted by piping
  input, e.g. `printf '2\n4\n\n7\ny\nn\n' | ./build/ppp --no-clear`.

## Test Cases
1. **Launch and exit**
   - Start the application.
   - At the main menu, choose `7) Quit` and confirm exit with `Y`.
   - Expected: application exits cleanly without errors.

2. **Edit flow persists**
   - Start the application.
   - Select `1) Edit Current Teams`.
   - Choose any team.
   - Pick an edit operation (e.g., `1) Add Yards`) and enter a value.
   - Select `n` when asked to change another value.
   - Return to `3) Display Teams` and verify the chosen stat (and, for wins or losses, the team's record) changed.
   - Choose `4) Who Will Make The Playoffs?`: an edited record moves the team, and a note says edited records
     are in use.
   - Subtracting more than a team has leaves the stat at 0.

3. **Display and sort**
   - Start the application.
   - Select `3) Display Teams`.
   - Choose to order the output (`Y`).
   - Select a sort key (e.g., `1) Yards`).
   - Expected: teams render in sorted order for the selected key.

4. **Playoff bracket**
   - Start the application.
   - Select `4) Who Will Make The Playoffs?`.
   - Expected: a full NFC/AFC bracket prints without errors.

5. **Menu input validation**
   - At the main menu, enter a non-numeric value (e.g., `x`) then a valid choice.
   - Expected: prompt repeats until a valid number is entered.

6. **Bad data file**
   - Run `./build/ppp missing.csv`.
   - Expected: `Error: could not open data file 'missing.csv'` and exit code 1.
   - Run with a CSV that has a malformed row.
   - Expected: an error naming the file and line number.

7. **Save on exit**
   - Choose `7) Quit`, answer `y`, then `y` to save.
   - Expected: `Output.txt` in the working directory lists all 32 teams.

8. **Live download**
   - Start the application and press Enter at the data source prompt (download the latest season).
   - Expected: the main menu shows `Current data: <season> season via nflverse (through week N)`, and
     `data/nflverse-<season>.csv` exists.
   - Choose `6) Download Latest Data`, enter `2020`.
   - Expected: `Loaded 2020 season via nflverse (final regular season)`; the bracket header shows 2020.

9. **Offline fallback**
   - With no network (or `HTTPS_PROXY=http://127.0.0.1:9`), run `./build/ppp --download`.
   - Expected: a warning, then `Using the last downloaded copy: data/nflverse-<season>.csv`, or
     `Using data/data.csv instead.` if no copy exists.
