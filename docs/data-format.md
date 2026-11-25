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
- skips blank lines and lines starting with `#`;
- trims spaces around fields and accepts Windows (CRLF) line endings.

### Validation

Loading fails with an error naming the file and line number if a row has fewer than six columns,
a non-numeric value, negative wins/losses/yards, a `percentage` outside 0 to 1, a `division` outside 1 to 8,
or an empty team name. The file as a whole must contain exactly 32 teams with 4 in each division,
which the playoff bracket logic requires.

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
