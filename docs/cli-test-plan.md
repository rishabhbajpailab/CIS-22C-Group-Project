# CLI Integration Test Plan

This plan covers a quick, manual CLI smoke test for the Fantasy Sports application.

Automated coverage of the algorithms and data loading lives in `tests/` (`./test.sh`); this plan covers
the interactive menus.

## Prerequisites
- Build with `./build.sh` (or the Visual Studio solution).
- Run from the repository root (`./run.sh`) so `data/data.csv` is found.
- Tip: `./run.sh --no-clear` keeps the full session in scrollback, and the menus can be scripted by piping
  input, e.g. `printf 'n\n4\n\n6\ny\nn\n' | ./build/ppp --no-clear`.

## Test Cases
1. **Launch and exit**
   - Start the application.
   - At the main menu, choose `6) Quit` and confirm exit with `Y`.
   - Expected: application exits cleanly without errors.

2. **Edit flow persists**
   - Start the application.
   - Select `1) Edit Current Teams`.
   - Choose any team.
   - Pick an edit operation (e.g., `1) Add Yards`) and enter a value.
   - Select `n` when asked to change another value.
   - Return to `3) Display Teams` and verify the chosen stat and the team's PPI changed.
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
   - Choose `6) Quit`, answer `y`, then `y` to save.
   - Expected: `Output.txt` in the working directory lists all 32 teams.
