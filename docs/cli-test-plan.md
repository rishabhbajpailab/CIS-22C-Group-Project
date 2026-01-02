# CLI Integration Test Plan

This plan covers a quick, manual CLI smoke test for the Fantasy Sports application.

## Prerequisites
- Build the solution or compile the `FantasySportsApplication` target.
- Ensure `data.csv` is present in the working directory.

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
   - Return to `3) Display Teams` and verify the chosen stat changed for that team.

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
