# VS-22C-GP (Perfect Playoff Predictions)

## Project Pitch
An NFL playoff predictor with a what-if editor for team records and quick bracket generation from a CSV data set.

## Project Status
Legacy C++ coursework codebase, actively being documented and modernized.

## Quickstart
### Build (Windows / Visual Studio)
1. Open `FantasySportsApplication.sln` in Visual Studio.
2. Build the `FantasySportsApplication` project (Debug or Release).
3. Ensure `data.csv` is present in the working directory (the app defaults to it).

### Run
1. Run the built executable from Visual Studio or the build output directory.
2. When prompted, accept the default `data.csv` or enter a compatible CSV file.

### Minimal Example
- From the main menu, choose `4) Who Will Make The Playoffs?` to display the predicted playoff bracket.
- Choose `1) Edit Current Teams` to adjust a team's wins/losses/yards and rerun the playoff bracket.

## Current Capabilities
- Loads team data from a CSV file and builds an in-memory roster.
- Supports interactive edits to team stats (wins/losses/yards).
- Displays teams unsorted or sorted via a binary tree (yards, wins, losses, PPI, name).
- Generates AFC/NFC playoff brackets with wildcard selection.

## Limitations
- CLI-only UX (Windows-only `CLS`/`PAUSE` usage).
- Menu option `2) Add or Remove a Team` is not yet wired to functional add/remove logic.
- Team data must be updated manually in `data.csv` (no automated data feed).
- No persisted output beyond optional `Output.txt` export.

## Roadmap
1. Wire the add/remove team menu flow to actual logic (UX `addMenu`, Implementation `addMenuImp`, and `mainMenueImp` case 2).
2. Expand CSV validation and error messages in `inputFilePrompt` and `readDataInput`.
3. Introduce a configurable data refresh step (e.g., external API) that replaces manual `data.csv` updates.
4. Add automated tests for playoff bracket building and wildcard selection.
5. Replace Windows-specific console commands to improve cross-platform support.

## Change Log

Version 1.0.0
--
- Change Log created.
- Will adjust version numbering after documentation backlog is fixed.
