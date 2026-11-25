#pragma once

#include <string>
#include <vector>

#include "../Net/HttpClient.h"
#include "../Team/Team.h"

// A loaded league plus a short description of where it came from (shown in the playoff header).
struct League
{
	std::vector<Team> teams;
	std::string label;
	int season = 0; // set for downloaded leagues
};

// Loads a data file in the format described in docs/data-format.md. Throws DataError.
League loadLeagueFile(const std::string& path);

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
