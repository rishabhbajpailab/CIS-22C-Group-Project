// Tests for turning nflverse CSVs into a league, using trimmed copies of the real 2014 files
// in tests/fixtures (no network access needed).

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "IO/DataIO.h"
#include "Nflverse/DataSource.h"
#include "Nflverse/Franchises.h"
#include "Nflverse/NflverseData.h"
#include "TestSupport.h"

namespace
{
std::string readFile(const std::string& path)
{
	std::ifstream in(path, std::ios::binary);
	std::ostringstream out;
	out << in.rdbuf();
	return out.str();
}

const std::string kGames = readFile("tests/fixtures/nflverse_games_2014.csv");
const std::string kStats2014 = readFile("tests/fixtures/nflverse_stats_team_reg_2014.csv");

const Team* findTeam(const std::vector<Team>& teams, const std::string& name)
{
	for (const Team& team : teams)
	{
		if (team.getName() == name)
		{
			return &team;
		}
	}
	return nullptr;
}

// Fake downloader: serves fixtures for known URLs and records what was requested.
struct FakeHttp
{
	std::map<std::string, std::string> responses;
	std::vector<std::string> requested;

	std::string operator()(const std::string& url)
	{
		requested.push_back(url);
		auto found = responses.find(url);
		if (found == responses.end())
		{
			throw NetworkError("404 for " + url, 404);
		}
		return found->second;
	}
};
} // namespace

int main()
{
	CHECK(!kGames.empty());
	CHECK(!kStats2014.empty());

	// Franchise table.
	CHECK_EQ(allFranchises().size(), static_cast<std::size_t>(32));
	CHECK(findFranchise("OAK") == findFranchise("LV"));
	CHECK(findFranchise("STL") == findFranchise("LA"));
	CHECK(findFranchise("SD") == findFranchise("LAC"));
	CHECK(findFranchise("XYZ") == nullptr);
	CHECK_EQ(franchiseName(*findFranchise("LV"), 2019), std::string("Oakland Raiders"));
	CHECK_EQ(franchiseName(*findFranchise("LV"), 2020), std::string("Las Vegas Raiders"));
	CHECK_EQ(franchiseName(*findFranchise("WAS"), 2021), std::string("Washington Football Team"));
	CHECK_EQ(franchiseName(*findFranchise("WAS"), 2022), std::string("Washington Commanders"));
	CHECK_EQ(franchiseName(*findFranchise("LA"), 2015), std::string("St. Louis Rams"));
	CHECK_EQ(findFranchise("LAC")->division, 8);

	// Season detection.
	CHECK_EQ(seasonForDate(2026, 11), 2026);
	CHECK_EQ(seasonForDate(2027, 2), 2026);
	CHECK_EQ(seasonForDate(2027, 9), 2027);

	// Game results: regular season only, 2014 only (fixture also has 2013 and playoff games).
	std::istringstream games(kGames);
	SeasonResults results = parseSeasonResults(games, 2014);
	CHECK_EQ(results.gamesScheduled, 256);
	CHECK_EQ(results.gamesPlayed, 256);
	CHECK_EQ(results.lastWeekPlayed, 17);
	const TeamRecord& cin = results.records.at("CIN");
	CHECK_EQ(cin.wins, 10);
	CHECK_EQ(cin.losses, 5);
	CHECK_EQ(cin.ties, 1);
	CHECK_EQ(cin.winPercentage(), 10.5 / 16.0);
	CHECK_EQ(results.records.at("LA").wins, 6); // St. Louis (STL in games.csv)

	// Unplayed games count as scheduled only.
	std::istringstream partial("season,game_type,week,away_team,away_score,home_team,home_score\n"
		"2026,REG,1,CHI,20,GB,17\n2026,REG,2,GB,,CHI,\n2026,POST,20,GB,1,CHI,0\n");
	SeasonResults partialResults = parseSeasonResults(partial, 2026);
	CHECK_EQ(partialResults.gamesScheduled, 2);
	CHECK_EQ(partialResults.gamesPlayed, 1);
	CHECK_EQ(partialResults.lastWeekPlayed, 1);
	CHECK_EQ(partialResults.records.at("CHI").wins, 1);
	CHECK_EQ(partialResults.records.at("GB").losses, 1);
	CHECK_EQ(partialResults.records.at("NE").gamesPlayed(), 0);
	CHECK_EQ(partialResults.records.at("NE").winPercentage(), 0.0);

	std::istringstream badTeam("season,game_type,week,away_team,away_score,home_team,home_score\n2026,REG,1,ZZZ,1,GB,0\n");
	CHECK_THROWS(parseSeasonResults(badTeam, 2026), DataError);
	std::istringstream badScore("season,game_type,week,away_team,away_score,home_team,home_score\n2026,REG,1,CHI,x,GB,0\n");
	CHECK_THROWS(parseSeasonResults(badScore, 2026), DataError);
	std::istringstream noSeason(kGames);
	CHECK_THROWS(parseSeasonResults(noSeason, 2030), DataError);
	std::istringstream missingColumn("season,game_type\n2026,REG\n");
	CHECK_THROWS(parseSeasonResults(missingColumn, 2026), DataError);

	// Yards: net offensive yards, which match the hand-entered 2014 values in data/data.csv.
	std::istringstream stats(kStats2014);
	std::map<std::string, int> yards = parseTeamYards(stats, 2014);
	CHECK_EQ(yards.size(), static_cast<std::size_t>(32));
	CHECK_EQ(yards.at("IND"), 6506);
	CHECK_EQ(yards.at("NE"), 5848);
	CHECK_EQ(yards.at("LV"), 4515);

	// Full league matches data/data.csv on wins, losses and yards for every team
	// (except data.csv's Tampa Bay typo: 2-12 instead of 2-14).
	std::vector<Team> league = buildLeague(2014, results, yards);
	validateLeague(league);
	std::vector<Team> handEntered = loadTeams("data/data.csv");
	for (const Team& expected : handEntered)
	{
		std::string name = expected.getName() == "St Louis Rams" ? "St. Louis Rams" : expected.getName();
		const Team* actual = findTeam(league, name);
		CHECK(actual != nullptr);
		if (actual == nullptr)
		{
			continue;
		}
		CHECK_EQ(actual->getYards(), expected.getYards());
		CHECK_EQ(actual->getWins(), expected.getWins());
		CHECK_EQ(actual->getDivVal(), expected.getDivVal());
		if (name != "Tampa Bay Buccaneers")
		{
			CHECK_EQ(actual->getLosses(), expected.getLosses());
		}
	}
	const Team* colts = findTeam(league, "Indianapolis Colts");
	CHECK(colts != nullptr && colts->getPercentage() == 0.688); // 11/16 rounded to 3 places

	std::map<std::string, int> missingTeam = yards;
	missingTeam.erase("NE");
	CHECK_THROWS(buildLeague(2014, results, missingTeam), DataError);

	// Download flow with a fake downloader, caching to a temp directory.
	const std::string cacheDir = "build/nflverse-test-cache";
	std::filesystem::remove_all(cacheDir);

	FakeHttp http;
	http.responses[nflverseGamesUrl()] = kGames;
	http.responses[nflverseTeamStatsUrl(2014)] = kStats2014;

	DownloadOptions options;
	options.season = 2014;
	options.cacheDirectory = cacheDir;
	options.httpGet = [&http](const std::string& url) { return http(url); };

	League downloaded = downloadLeague(options);
	CHECK_EQ(downloaded.season, 2014);
	CHECK_EQ(downloaded.label, std::string("2014 season via nflverse (final regular season)"));
	CHECK_EQ(downloaded.teams.size(), static_cast<std::size_t>(32));

	// The cache file is a valid data file that reproduces the same league.
	const std::string cacheFile = cacheDir + "/nflverse-2014.csv";
	CHECK(std::filesystem::exists(cacheFile));
	League cached = loadLeagueFile(cacheFile);
	CHECK_EQ(cached.teams.size(), static_cast<std::size_t>(32));
	CHECK(cached.label.find("2014 season via nflverse") == 0);
	CHECK(cached.label.find("(cached)") != std::string::npos);
	for (std::size_t i = 0; i < cached.teams.size() && i < downloaded.teams.size(); i++)
	{
		CHECK(cached.teams[i] == downloaded.teams[i]);
	}

	// Seasons before 2002 are rejected.
	DownloadOptions tooEarly = options;
	tooEarly.season = 2001;
	CHECK_THROWS(downloadLeague(tooEarly), DataError);

	// Offline: falls back to the cached season, then to the given file.
	DownloadOptions offline = options;
	offline.httpGet = [](const std::string& url) -> std::string { throw NetworkError("offline: " + url); };
	League fromCache = downloadLeagueOrFallback(offline, "data/data.csv");
	CHECK(fromCache.label.find("(cached)") != std::string::npos);
	offline.season = 2013;
	League fromFile = downloadLeagueOrFallback(offline, "data/data.csv");
	CHECK_EQ(fromFile.label, std::string("data/data.csv"));

	// Errors other than 404 are not mistaken for "season not published".
	DownloadOptions serverError = options;
	serverError.httpGet = [](const std::string&) -> std::string { throw NetworkError("server error", 500); };
	CHECK_THROWS(downloadLeague(serverError), NetworkError);

	// Latest season (season = 0): if the current season's stats are not published yet (404),
	// the previous season is used. Relabel the 2014 fixtures as last season to simulate this.
	const int lastSeason = currentSeason() - 1;
	auto relabel = [lastSeason](std::string text) {
		const std::string from = "\n2014,";
		const std::string to = "\n" + std::to_string(lastSeason) + ",";
		for (std::size_t pos = text.find(from); pos != std::string::npos; pos = text.find(from, pos + to.size()))
		{
			text.replace(pos, from.size(), to);
		}
		return text;
	};
	std::string gamesNoIds;
	{
		// Drop the game_id column so the season column is first and easy to relabel.
		std::istringstream in(kGames);
		std::string line;
		while (std::getline(in, line))
		{
			gamesNoIds += line.substr(line.find(',') + 1) + "\n";
		}
	}
	FakeHttp latestHttp;
	latestHttp.responses[nflverseGamesUrl()] = relabel("\n" + gamesNoIds).substr(1);
	latestHttp.responses[nflverseTeamStatsUrl(lastSeason)] = relabel(kStats2014);
	DownloadOptions latest = options;
	latest.season = 0;
	latest.httpGet = [&latestHttp](const std::string& url) { return latestHttp(url); };
	League previous = downloadLeague(latest);
	CHECK_EQ(previous.season, lastSeason);
	CHECK_EQ(previous.teams.size(), static_cast<std::size_t>(32));
	CHECK(latestHttp.requested.size() >= 2 && latestHttp.requested[0] == nflverseTeamStatsUrl(lastSeason + 1));

	std::filesystem::remove_all(cacheDir);
	return finishTests("NflverseTests");
}
