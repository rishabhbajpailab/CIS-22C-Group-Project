# Core Algorithm Revision: Analysis

Status: steps 1-3 of the plan (standings, seeding, tiebreakers) are implemented and have replaced the PPI
bracket; see section 8. Steps 4-8 are next. Sections 2-5 describe the old algorithm as it was before the change.

## 1. First principles: what the algorithm has to do

The app answers one question: **given the season so far, which 14 teams make the playoffs, and with which seeds?**

That answer is fixed by the NFL's rules, not by a team-quality score:

1. **Format.** Each conference seeds 7 teams (6 before 2020). Seeds 1-4 are the four division winners,
   ordered by record. Seeds 5-7 are the best remaining records in the conference (the wild cards).
   A wild card with a better record than a division winner is still seeded below it.
2. **Qualification depends only on final W-L-T**, plus the NFL tiebreakers when records are equal
   (head-to-head, division record, common games, conference record, strength of victory, strength of schedule, ...).
   Yards, points and other "quality" measures never decide who gets in.
3. **So a prediction has two parts:**
   - *Known:* the games already played. These are facts and should be used exactly.
   - *Unknown:* the games left to play. This is the only place a measure of team strength belongs.
     A team's strength matters only through the games it has left.

At the end of the regular season the prediction should match the real field exactly (except for rare
tiebreakers we don't model). The further from the end, the more it depends on forecasting the remaining games.

## 2. How the current algorithm works

`playoffBracket` (`FantasySportsApplication/Playoff/PlayoffLogic.cpp`):

1. **PPI** (`Team::setRank`): `PPI = int(yards * win_percentage)`, using offensive yards only.
2. **`reorderVec`** "sorts" the league by PPI. Its inner loop starts at 1, so only the first element ends up
   in place. The rest is scrambled: input `5 3 9 1 7 2 8 6 4 0` comes out as `0 9 3 7 2 8 6 4 5 1`.
3. **Split by conference** (divisions 1-4 NFC, 5-8 AFC).
4. **Division leaders** (`buildPlayoffBracket`): for each division, compute
   `divScalar = int(sum of the 4 PPIs / 4000)`, apply `scaledPPI` (≈ `PPI² · divScalar · pct`), and take the max.
5. **Wild cards** (`getWildCards`): try to remove the leaders from the conference list, re-"sort" it with
   `reorderVec`, skip the first 3 teams, and append the rest. The caller keeps the first 2.
6. **Output**: leaders in division order (North, East, South, West) labeled seeds 1-4, then the two wild cards as 5-6.

## 3. What doesn't work

| # | Problem | Effect |
|---|---|---|
| 1 | **Wild cards are close to random.** `reorderVec` scrambles the order, and "skip 3, take 2" picks two teams from the middle of that scrambled list. | Teams like the 4-13 2024 Patriots and the 5-12 2025 Commanders are seeded as wild cards. |
| 2 | **Division leaders aren't removed from the wild card pool.** The leaders carry their scaled PPI, so `==` never matches. | 18 of 24 seasons (2002-2025) seed the same team twice (2020 does it three times). |
| 3 | **`divScalar` does nothing useful.** It is the same for all four teams in a division, so it can't change who leads. It only adds integer-truncation noise. | When it truncates to 0, every scaled PPI is 0 and `max_element` returns the *first team listed* (alphabetical by code). At week 6 this happens in **98 of 192** divisions. |
| 4 | **Seeds aren't seeds.** Seeds 1-4 are just NFC/AFC North, East, South, West in that order. | The bracket's seed numbers are wrong almost every season. |
| 5 | **Only 6 seeds.** The format has had 7 seeds per conference since 2020. | The app can't list 2 of the 14 playoff teams in any season since 2020. |
| 6 | **The PPI ranks teams by quality, not by standings.** `yards × pct` lets a team's yardage outweigh games it has actually won. | Teams that are already eliminated, or have already clinched, can be ranked out of place. |
| 7 | **Offensive yards only.** Defense is ignored, and total yards grow with games played (bye weeks skew them). | Weak measure of strength, and teams with a different number of games played aren't compared fairly. |
| 8 | **No schedule.** The algorithm doesn't know who plays whom in the remaining games. | Can't account for head-to-head games, schedule difficulty, or elimination and clinching. |
| 9 | **No tiebreakers, and results depend on file order.** | Edits can reorder the bracket in ways that have nothing to do with the edit. |
| 10 | **Edits don't fit the model.** Editing "wins" doesn't change the win percentage, which comes from the file column. | What-if edits to wins and losses have almost no effect on PPI. |

## 4. What works and is worth keeping

- **The data pipeline.** nflverse `games.csv` already has the full schedule, including unplayed games
  with blank scores. That is exactly what a forecast needs, and it is already downloaded.
- **The idea of a single team-strength number.** Keep it, but use it only to forecast the remaining games.
- **The what-if editor**, aimed at better inputs: "who wins this game" rather than "+3 wins".
- **The Team, HashTable and BST structures** for display and sorting. They are separate from the
  prediction logic. (The hash table keyed by total yards, which makes teams collide, is a separate fix.)
- **The regression tests as a safety net.** They have to be replaced, not deleted, because they pin the
  quirky output on purpose.

## 5. Backtest: how each method does in real seasons

Method: for each season from 2002 to 2025, take the standings and stats through week W and predict the playoff field.
Then count how many of the real playoff teams it named (300 total). Every method except the current algorithm
uses the correct number of seeds. Scripts are in the session scratchpad, and porting them into the repo is listed in section 6.

| Method | Week 6 | Week 11 | Week 15 |
|---|---|---|---|
| **Current algorithm (PPI)** | 46.3% | 63.0% | 64.0% |
| Offensive yards × win% (the PPI idea without the quirks) | 73.3% | 78.7% | 85.7% |
| Yard differential per game | 57.7% | 65.3% | 68.3% |
| Point differential per game | 71.0% | 77.7% | 80.7% |
| Current standings (win%, then point diff) | 73.3% | **80.3%** | 87.0% |
| Simulate remaining games, rating = point diff | **74.0%** | 79.7% | 86.7% |
| Simulate remaining games, rating = yard diff | 70.7% | 77.7% | **87.3%** |

For reference, picking at random gets about 39%. With final-season data (W = 18), simple standings name
**284/300**, and the current algorithm names **202/300** (67%).

Probability quality (Brier score, lower is better; about 0.24 is "give everyone the base rate"):
the simulation scores 0.145 at week 6, 0.108 at week 11 and 0.066-0.071 at week 15.

### What the numbers say

- Most of the current algorithm's gap is the bugs. The same PPI idea without the quirks jumps from 63% to 79% at week 11.
- **Win-loss records do most of the work.** Ranking by current standings is as good as anything else at picking the field.
- Simulating the remaining games doesn't pick many more teams correctly. What it adds is **calibrated
  playoff probabilities**, a natural home for team strength, and a way to model the schedule.
  It also makes the what-if editor meaningful.
- Point differential beats yards as a strength measure early in the season. Yard differential catches up late.
  Offense-only yards (today's input) alone is the weakest. A blend is worth testing.
- The simulation here is simplified: random tiebreaks, a fixed 2-point home-field edge, and a rating that
  doesn't change during the simulation. Real tiebreakers and a tuned rating should improve it.

## 6. Proposed design

```
games.csv (played + unplayed) ──► Standings (W-L-T, division/conference records, head-to-head)
                                    │
team stats ──► Rating (points/game margin, shrunk early; optional yard blend)
                                    │
                    Season simulator (N runs of remaining games)
                                    │
                    Seeding + NFL tiebreakers (applied per run)
                                    │
          Per-team odds: make playoffs, win division, each seed  ──►  "Most likely bracket" + table
```

Components, roughly in build order:

1. **`Standings`**: W-L-T with ties, plus division, conference, head-to-head and common-games records, built from game results.
2. **`Seeding`**: the actual rules. Division winners ranked 1-4 by record, then wild cards, with 6 or 7 seeds
   depending on the season. Run on final-season data, this alone should match nearly every real field.
3. **`Tiebreakers`**: the NFL steps in order. Start with head-to-head, division record, conference record and
   common games, then strength of victory and schedule. Coin toss as a deterministic fallback.
4. **`Rating`**: team strength from point margin per game, shrunk toward average early in the season.
   Optional blend with yard differential. This replaces the PPI as the "quality" number.
5. **`Simulator`**: Monte Carlo over the remaining games with a fixed random seed, so tests are repeatable.
   Reports playoff, division and seed odds.
6. **Output**: the most likely bracket in real seed order, plus a playoff-odds table.
7. **What-if editor**: "set this game's winner" and "set this team's rating", then rerun.
   The existing W/L edits can stay as a shortcut.
8. **Backtest tool and tests**: port the backtest into the repo as a test or command, so every algorithm change
   is measured against 24 seasons. Replace the quirk-pinning regression tests with tests of the rules:
   known final fields, tiebreaker cases, and no duplicate seeds.

The first two steps alone fix problems #1-5 and #9 and bring the week-11 result from 63% to about 80%.

## 7. Decisions

There are no course constraints any more, so:

- The PPI is gone as the prediction engine. A team strength rating will be added in step 4.
- The old algorithm is removed rather than kept as a "legacy" option. It is in the git history, and sections
  2-5 document it.
- Offline mode works from a bundled games file (`data/data.games.csv`), so the tiebreakers work offline too.
- Every tiebreaker step is implemented that the data allows (see section 8).

## 8. Implementation (steps 1-3)

Code: `FantasySportsApplication/Season/`.

- `Standings` builds every record the rules need from game results: overall, division and conference records,
  points for and against, and each pair of teams' record and points against each other. The teams'
  own wins, losses and ties set the overall record, so edits made in the app count. Head-to-head and the other
  game-based steps still use the real games.
- `Seeding` ranks each division and seeds the division winners 1-4 by record. It then ranks every other
  team in wild card order, using 7 seeds per conference from 2020 and 6 before.
  - Ties are broken with the division procedure (within a division) or the wild card procedure
    (otherwise).
  - Before teams from different divisions are compared, only the best-ranked tied team from each division
    is kept.
  - Once a team is picked, the rest start the procedure over.
  - Each placement records the step that decided it, and the bracket shows it.

Tiebreaker steps, in order:

| Division | Wild card |
|---|---|
| Head-to-head | Head-to-head (two teams, if they played) / head-to-head sweep (three or more) |
| Division record | Conference record |
| Common games | Common games (at least 4 each) |
| Conference record | Strength of victory |
| Strength of victory | Strength of schedule |
| Strength of schedule | Combined ranking in points scored and allowed, conference |
| Combined ranking in points scored and allowed, conference | Same, league |
| Same, league | Net points in conference games |
| Net points in common games | Net points, all games |
| Net points, all games | Coin toss |
| Coin toss | |

Where this differs from the NFL's procedures, or interprets them:

- **Net touchdowns is skipped.** nflverse game data has scores but not touchdowns.
- **The coin toss is alphabetical by team name**, so results are repeatable.
- **Restarts:** whenever a step eliminates some teams but leaves two or more, the procedure restarts with the
  remaining teams, using the two-team list if two remain. The NFL's text on where a tie of three or more
  restarts has changed over the years. This matches the common reading, but it couldn't be checked against
  the current rulebook text from this environment.
- **Tied ranks** in the points rankings share the average of their places.
- **Mid-season**, a team with no games in a category (for example, it hasn't played its division rivals yet)
  counts as .500 in that category, so an unplayed game neither helps nor hurts it.

Validation, on final regular-season data for every season from 2002 to 2025:

- **Playoff teams:** 300/300 correct. The PPI bracket got 202/300.
- **First-round byes:** 84/84 correct.
- **Wild card round matchups (home and away):** 108/108 exactly right, so seeds 2-7 match in every season.
- `tests/SeedingTests.cpp` pins the real 2014 seeds and the tiebreakers that decided them. For example, Seattle
  and then Green Bay won the three-way 12-4 tie with Dallas on conference record. It also covers synthetic
  head-to-head, division record, three-team, wild card division-reduction and edited-record cases.
