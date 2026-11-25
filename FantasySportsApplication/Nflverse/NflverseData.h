#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "../Season/Standings.h"
#include "../Team/Team.h"

// Turns nflverse's public CSV releases into the 32-team league the playoff logic uses.
//
// Sources (no API key; updated by nflverse during the season):
//   games:      https://raw.githubusercontent.com/nflverse/nfldata/master/data/games.csv
//               one row per game since 1999 with scores (blank until played)
//   team stats: https://github.com/nflverse/nflverse-data/releases/download/stats_team/stats_team_reg_<season>.csv
//               regular-season team totals, used for net offensive yards
//
// How each Team column is derived:
//   wins/losses  regular-season games played so far (ties are counted separately)
//   percentage   regular-season win percentage, ties counted as half a win
//   yards        net offensive yards: passing_yards + rushing_yards + sack_yards_lost (negative)
//   division     from the fixed 2002+ alignment in Franchises.h

std::string nflverseGamesUrl();
std::string nflverseTeamStatsUrl(int season);

struct TeamRecord
{
	int wins = 0;
	int losses = 0;
	int ties = 0;
	int gamesPlayed() const { return wins + losses + ties; }
	double winPercentage() const;
};

struct SeasonResults
{
	std::map<std::string, TeamRecord> records; // keyed by current franchise code
	int gamesPlayed = 0;
	int gamesScheduled = 0;
	int lastWeekPlayed = 0;
};

// One regular-season game from games.csv, with current franchise codes (e.g. "LV" for "OAK").
struct NflverseGame
{
	int week = 0;
	std::string away;
	std::string home;
	bool played = false;
	int awayScore = 0;
	int homeScore = 0;
};

struct SeasonGames
{
	int season = 0;
	std::vector<NflverseGame> games; // played and unplayed, in file order
};

// Every regular-season game of one season from games.csv (or a games file written by
// writeSeasonGames). A season of 0 means the first season found in the file.
// Throws DataError for unknown team codes, malformed scores or weeks, or if the season has no games.
SeasonGames parseSeasonGames(std::istream& gamesCsv, int season, const std::string& sourceName = "games.csv");

// Records and progress (games played, last week played) from a season's games.
SeasonResults summarizeResults(const std::vector<NflverseGame>& games);

// Regular-season records for one season from games.csv. Unplayed games are counted as scheduled only.
// Throws DataError for unknown team codes or malformed scores.
SeasonResults parseSeasonResults(std::istream& gamesCsv, int season, const std::string& sourceName = "games.csv");

// Writes a season's games in the games.csv column format, readable by parseSeasonGames.
void writeSeasonGames(std::ostream& out, int season, const std::vector<NflverseGame>& games);

// Maps games onto a league's teams (by index), matching each team to its franchise by name.
// Throws DataError if a team's name is not a known franchise name, or a franchise appears twice.
std::vector<Game> leagueGames(const std::vector<Team>& teams, const std::vector<NflverseGame>& games);

// Net offensive yards per franchise code from a stats_team_reg_<season>.csv file.
std::map<std::string, int> parseTeamYards(std::istream& statsCsv, int season, const std::string& sourceName = "stats_team.csv");

// Combines records and yards into 32 Teams ordered by division (see allFranchises()).
// Throws DataError if any franchise is missing yards.
std::vector<Team> buildLeague(int season, const SeasonResults& results, const std::map<std::string, int>& yards);

// The NFL season in progress (or most recently finished) on the given date:
// the calendar year from September on, otherwise the previous year.
int seasonForDate(int year, int month);
int currentSeason();
