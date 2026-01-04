# CSV Data Format

This project loads team data from a comma-separated values (CSV) file. The parser is implemented in
`FantasySportsApplication/fileIO.h` and assigns values through `Team::input` in
`FantasySportsApplication/Team/Team.h`.

## Schema

The file has **no header row**. Each line contains exactly six columns, in this order:

1. `wins` (integer)
2. `losses` (integer)
3. `yards` (integer)
4. `percentage` (decimal)
5. `division` (integer)
6. `team_name` (string, may include spaces)

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
