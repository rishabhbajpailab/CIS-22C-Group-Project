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
| `percentage` | Regular-season win percentage, ties counted as half a win, rounded to 3 places |
| `division` | Fixed 2002+ alignment in `FantasySportsApplication/Nflverse/Franchises.cpp` |
| `team_name` | Franchise name for that season (e.g. Oakland Raiders through 2019, Las Vegas Raiders after) |

For 2014, the derived `wins`, `losses` and `yards` match the hand-entered `data/data.csv` for every team
except one typo there (Tampa Bay 2-12, actually 2-14). The hand-entered `percentage` values do not match
overall, conference or division win percentage, so their meaning is unknown. Brackets from downloaded data
will therefore differ from brackets from `data/data.csv` for the same season.

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
