# CSV Data Format

This project loads team data from a comma-separated values (CSV) file. The parser is implemented in
`FantasySportsApplication/IO/DataIO.cpp` (`parseTeams`, `validateLeague`, `loadTeams`).

## Schema

Each data line contains six columns, in this order:

1. `wins` (integer)
2. `losses` (integer)
3. `yards` (integer)
4. `percentage` (decimal)
5. `division` (integer)
6. `team_name` (string, may include spaces and commas, since it is the last column)

The loader also:
- accepts an optional header row as the first line (detected because its first column is not a number);
- skips blank lines and lines starting with `#` (downloaded files use these for the source and date);
- trims spaces around fields and accepts Windows (CRLF) line endings.

### Validation

Loading fails with an error naming the file and line number if a row has fewer than six columns,
a non-numeric value, negative wins/losses/yards, a `percentage` outside 0 to 1, a `division` outside 1 to 8,
or an empty team name. The file as a whole must contain exactly 32 teams with 4 in each division,
which the playoff bracket logic requires.

## Files downloaded from nflverse

`ppp --download` (or `--season YEAR`) writes `data/nflverse-<season>.csv` in this same format, with a
comment line describing the data, e.g. `# 2026 season via nflverse (through week 11), downloaded 2026-11-25`.
Teams are listed by division and then team code. The columns are derived as follows
(see `FantasySportsApplication/Nflverse/NflverseData.h`):

| Column | Source |
| --- | --- |
| `wins`, `losses` | Regular-season games with a final score in nflverse `games.csv` (playoffs and unplayed games are ignored) |
| `yards` | Net offensive yards from `stats_team_reg_<season>.csv`: `passing_yards + rushing_yards` minus sack yards |
| `percentage` | Regular-season win percentage, ties counted as half a win, rounded to 3 places (informational) |
| `division` | Fixed 2002+ alignment in `FantasySportsApplication/Nflverse/Franchises.cpp` |
| `team_name` | Franchise name for that season (e.g. Oakland Raiders through 2019, Las Vegas Raiders after) |

The bundled `data/data.csv` is this file for 2014 (with a "bundled" comment line). It replaced a hand-entered
2014 file whose wins, losses and yards matched nflverse except for one typo (Tampa Bay 2-12, actually 2-14),
and whose `percentage` values matched no win percentage; that version is in the git history.

## Games files

The playoff seeding needs game results for its tiebreakers (head-to-head, division record, and so on).
A data file can have a games file next to it with the same name and a `.games.csv` extension:
`data/nflverse-2026.csv` goes with `data/nflverse-2026.games.csv`, and `data/data.csv` with
`data/data.games.csv`. Downloads write both. The games file uses the nflverse `games.csv` columns
(parsed by name, so extra columns are fine):

```
season,game_type,week,away_team,away_score,home_team,home_score
2014,REG,1,GB,16,SEA,36
2026,REG,13,GB,,CHI,
```

- Only `REG` rows of one season are used: the season in the data file's first comment
  (`# 2014 season via nflverse ...`), or else the first season in the games file.
- Team codes are nflverse abbreviations; historical ones (`OAK`, `SD`, `STL`) are accepted. Each team in the
  data file is matched to its franchise by name, using any name the franchise has had since 2002.
- Blank scores mean the game has not been played. Played games give each team its ties (the data file has
  no ties column) and every game-based tiebreaker.
- If the games file cannot be read or does not match the teams, the app warns and carries on without games.

Without a games file, the bracket uses the data file's wins and losses only, and orders teams with the same
record alphabetically.

## How the data is used

- **Record and win percentage** come from `wins`, `losses` and (from the games file) ties, with ties counted
  as half a win. The `percentage` column is validated but not used, so editing wins or losses changes the
  win percentage.
- **Edited records** (menu option 1) count for each team's overall record. Tiebreakers still use the games.
- **Yards** are shown and used for sorting and the hash table; they do not affect the playoff seeding.

### Division values

The `division` column maps to the following divisions:

- `1`: NFC North
- `2`: NFC East
- `3`: NFC South
- `4`: NFC West
- `5`: AFC North
- `6`: AFC East
- `7`: AFC South
- `8`: AFC West

## Example row

```
11,5,6506,0.667,7,Indianapolis Colts
```
