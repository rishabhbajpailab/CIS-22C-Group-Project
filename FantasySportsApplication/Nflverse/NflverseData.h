#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

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

// Regular-season records for one season from games.csv. Unplayed games are counted as scheduled only.
// Throws DataError for unknown team codes or malformed scores.
SeasonResults parseSeasonResults(std::istream& gamesCsv, int season, const std::string& sourceName = "games.csv");

// Net offensive yards per franchise code from a stats_team_reg_<season>.csv file.
std::map<std::string, int> parseTeamYards(std::istream& statsCsv, int season, const std::string& sourceName = "stats_team.csv");

// Combines records and yards into 32 Teams ordered by division (see allFranchises()).
// Throws DataError if any franchise is missing yards.
std::vector<Team> buildLeague(int season, const SeasonResults& results, const std::map<std::string, int>& yards);

// The NFL season in progress (or most recently finished) on the given date:
// the calendar year from September on, otherwise the previous year.
int seasonForDate(int year, int month);
int currentSeason();
