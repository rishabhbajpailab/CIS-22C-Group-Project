#pragma once

#include <string>
#include <vector>

#include "../Net/HttpClient.h"
#include "../Season/Standings.h"
#include "../Team/Team.h"

// A loaded league plus a short description of where it came from (shown in the playoff header).
struct League
{
	std::vector<Team> teams;
	std::vector<Game> games; // the season's games (played and unplayed), if known; indices into teams
	std::string label;
	int season = 0; // 0 if unknown
};

// Loads a data file in the format described in docs/data-format.md. If a games file sits next to it
// (see gamesPathFor), the season's games are loaded too; a games file that does not match the teams
// is reported on stderr and ignored. Throws DataError if the data file itself cannot be loaded.
League loadLeagueFile(const std::string& path);

// The games file that goes with a data file: data/nflverse-2026.csv -> data/nflverse-2026.games.csv.
std::string gamesPathFor(const std::string& dataPath);

// Path of the cached copy of a downloaded season, e.g. data/nflverse-2026.csv.
std::string cachePathForSeason(int season);

struct DownloadOptions
{
	// Season to download; 0 means the current season (falling back to the previous season
	// if nflverse has not published stats for the current one yet).
	int season = 0;
	// Directory for the cached data file.
	std::string cacheDirectory = "data";
	HttpGet httpGet = curlGet;
};

// Downloads a season from nflverse, saves it to the cache file, and returns it.
// Throws NetworkError or DataError.
League downloadLeague(const DownloadOptions& options);

// Like downloadLeague, but if the download fails it loads the cached file for that season, or
// failing that fallbackFile. Problems are reported on stderr. Throws DataError only if nothing loads.
League downloadLeagueOrFallback(const DownloadOptions& options, const std::string& fallbackFile);
