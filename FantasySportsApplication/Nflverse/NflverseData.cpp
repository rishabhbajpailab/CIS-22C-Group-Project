#include "NflverseData.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <istream>
#include <sstream>

#include "../IO/Csv.h"
#include "../IO/DataIO.h"
#include "Franchises.h"

namespace
{
bool parseInt(const std::string& text, int& out)
{
	std::istringstream stream(text);
	stream >> out;
	return stream && stream.peek() == std::char_traits<char>::eof();
}

bool parseDouble(const std::string& text, double& out)
{
	std::istringstream stream(text);
	stream >> out;
	return stream && stream.peek() == std::char_traits<char>::eof();
}

const Franchise& requireFranchise(const std::string& code, const std::string& where)
{
	const Franchise* franchise = findFranchise(code);
	if (franchise == nullptr)
	{
		throw DataError(where + "unknown team code '" + code + "'");
	}
	return *franchise;
}

std::string rowLocation(const CsvTable& table, std::size_t rowIndex)
{
	// +2: one for the header row, one because line numbers start at 1.
	return table.source() + ":" + std::to_string(rowIndex + 2) + ": ";
}
} // namespace

std::string nflverseGamesUrl()
{
	return "https://raw.githubusercontent.com/nflverse/nfldata/master/data/games.csv";
}

std::string nflverseTeamStatsUrl(int season)
{
	return "https://github.com/nflverse/nflverse-data/releases/download/stats_team/stats_team_reg_"
		+ std::to_string(season) + ".csv";
}

double TeamRecord::winPercentage() const
{
	int games = gamesPlayed();
	return games == 0 ? 0.0 : (wins + 0.5 * ties) / games;
}

SeasonGames parseSeasonGames(std::istream& gamesCsv, int season, const std::string& sourceName)
{
	CsvTable table = CsvTable::parse(gamesCsv, sourceName);
	const std::size_t seasonCol = table.column("season");
	const std::size_t typeCol = table.column("game_type");
	const std::size_t weekCol = table.column("week");
	const std::size_t awayCol = table.column("away_team");
	const std::size_t awayScoreCol = table.column("away_score");
	const std::size_t homeCol = table.column("home_team");
	const std::size_t homeScoreCol = table.column("home_score");

	SeasonGames result;
	result.season = season;
	std::string seasonText = std::to_string(season);
	const auto& rows = table.rows();
	for (std::size_t i = 0; i < rows.size(); i++)
	{
		const auto& row = rows[i];
		if (table.field(row, typeCol) != "REG")
		{
			continue;
		}
		const std::string where = rowLocation(table, i);
		if (result.season == 0)
		{
			if (!parseInt(table.field(row, seasonCol), result.season))
			{
				throw DataError(where + "season must be a whole number");
			}
			seasonText = std::to_string(result.season);
		}
		if (table.field(row, seasonCol) != seasonText)
		{
			continue;
		}

		NflverseGame game;
		game.away = requireFranchise(table.field(row, awayCol), where).code;
		game.home = requireFranchise(table.field(row, homeCol), where).code;
		if (!parseInt(table.field(row, weekCol), game.week))
		{
			throw DataError(where + "week must be a whole number");
		}
		const std::string& awayText = table.field(row, awayScoreCol);
		const std::string& homeText = table.field(row, homeScoreCol);
		if (!awayText.empty() || !homeText.empty())
		{
			if (!parseInt(awayText, game.awayScore) || !parseInt(homeText, game.homeScore))
			{
				throw DataError(where + "scores must be whole numbers");
			}
			game.played = true;
		}
		result.games.push_back(game);
	}

	if (result.games.empty())
	{
		throw DataError(sourceName + ": no regular-season games found for "
			+ (season == 0 ? std::string("any season") : seasonText));
	}
	return result;
}

SeasonResults summarizeResults(const std::vector<NflverseGame>& games)
{
	SeasonResults results;
	for (const Franchise& franchise : allFranchises())
	{
		results.records[franchise.code] = TeamRecord{};
	}
	for (const NflverseGame& game : games)
	{
		results.gamesScheduled++;
		if (!game.played)
		{
			continue;
		}
		TeamRecord& away = results.records[game.away];
		TeamRecord& home = results.records[game.home];
		if (game.awayScore > game.homeScore)
		{
			away.wins++;
			home.losses++;
		}
		else if (game.homeScore > game.awayScore)
		{
			home.wins++;
			away.losses++;
		}
		else
		{
			away.ties++;
			home.ties++;
		}
		results.gamesPlayed++;
		results.lastWeekPlayed = std::max(results.lastWeekPlayed, game.week);
	}
	return results;
}

SeasonResults parseSeasonResults(std::istream& gamesCsv, int season, const std::string& sourceName)
{
	return summarizeResults(parseSeasonGames(gamesCsv, season, sourceName).games);
}

void writeSeasonGames(std::ostream& out, int season, const std::vector<NflverseGame>& games)
{
	out << "season,game_type,week,away_team,away_score,home_team,home_score\n";
	for (const NflverseGame& game : games)
	{
		out << season << ",REG," << game.week << ',' << game.away << ',';
		if (game.played)
		{
			out << game.awayScore;
		}
		out << ',' << game.home << ',';
		if (game.played)
		{
			out << game.homeScore;
		}
		out << '\n';
	}
}

std::vector<Game> leagueGames(const std::vector<Team>& teams, const std::vector<NflverseGame>& games)
{
	std::map<std::string, int> indexByCode;
	for (std::size_t i = 0; i < teams.size(); i++)
	{
		const Franchise* franchise = findFranchiseByName(teams[i].getName());
		if (franchise == nullptr)
		{
			throw DataError("'" + teams[i].getName() + "' is not an NFL franchise name, so its games cannot be matched");
		}
		if (!indexByCode.emplace(franchise->code, static_cast<int>(i)).second)
		{
			throw DataError("two teams are the same franchise (" + franchise->code + ")");
		}
	}

	std::vector<Game> result;
	result.reserve(games.size());
	for (const NflverseGame& game : games)
	{
		auto home = indexByCode.find(game.home);
		auto away = indexByCode.find(game.away);
		if (home == indexByCode.end() || away == indexByCode.end())
		{
			throw DataError("a game's team (" + game.away + " at " + game.home + ") is not in the league");
		}
		result.push_back({game.week, home->second, away->second, game.played, game.homeScore, game.awayScore});
	}
	return result;
}

std::map<std::string, int> parseTeamYards(std::istream& statsCsv, int season, const std::string& sourceName)
{
	CsvTable table = CsvTable::parse(statsCsv, sourceName);
	const std::size_t seasonCol = table.column("season");
	const std::size_t teamCol = table.column("team");
	const std::size_t passingCol = table.column("passing_yards");
	const std::size_t rushingCol = table.column("rushing_yards");
	const std::size_t sackCol = table.column("sack_yards_lost");

	std::map<std::string, int> yards;
	const std::string seasonText = std::to_string(season);
	const auto& rows = table.rows();
	for (std::size_t i = 0; i < rows.size(); i++)
	{
		const auto& row = rows[i];
		if (table.field(row, seasonCol) != seasonText)
		{
			continue;
		}
		const std::string where = rowLocation(table, i);
		const Franchise& franchise = requireFranchise(table.field(row, teamCol), where);

		double passing = 0.0, rushing = 0.0, sacks = 0.0;
		if (!parseDouble(table.field(row, passingCol), passing) || !parseDouble(table.field(row, rushingCol), rushing))
		{
			throw DataError(where + "passing_yards and rushing_yards must be numbers");
		}
		// Older seasons may leave sack yards blank; treat that as zero.
		const std::string& sackText = table.field(row, sackCol);
		if (!sackText.empty() && sackText != "NA" && !parseDouble(sackText, sacks))
		{
			throw DataError(where + "sack_yards_lost must be a number");
		}
		// nflverse reports sack yards as a negative number.
		yards[franchise.code] = static_cast<int>(std::lround(passing + rushing - std::fabs(sacks)));
	}
	return yards;
}

std::vector<Team> buildLeague(int season, const SeasonResults& results, const std::map<std::string, int>& yards)
{
	std::vector<Team> teams;
	teams.reserve(allFranchises().size());
	for (const Franchise& franchise : allFranchises())
	{
		auto teamYards = yards.find(franchise.code);
		if (teamYards == yards.end())
		{
			throw DataError("no team stats for " + franchiseName(franchise, season) + " (" + franchise.code
				+ ") in " + std::to_string(season));
		}
		const TeamRecord& record = results.records.at(franchise.code);
		teams.emplace_back(franchiseName(franchise, season), record.wins, record.losses, teamYards->second,
			franchise.division, record.ties);
	}
	validateLeague(teams);
	return teams;
}

int seasonForDate(int year, int month)
{
	return month >= 9 ? year : year - 1;
}

int currentSeason()
{
	std::time_t now = std::time(nullptr);
	std::tm local{};
#ifdef _WIN32
	localtime_s(&local, &now);
#else
	localtime_r(&now, &local);
#endif
	return seasonForDate(local.tm_year + 1900, local.tm_mon + 1);
}
