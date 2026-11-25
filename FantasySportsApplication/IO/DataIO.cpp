#include "DataIO.h"

#include <array>
#include <cctype>
#include <fstream>
#include <sstream>

namespace
{
std::string trim(const std::string& text)
{
	std::size_t begin = 0;
	std::size_t end = text.size();
	while (begin < end && std::isspace(static_cast<unsigned char>(text[begin])))
	{
		begin++;
	}
	while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1])))
	{
		end--;
	}
	return text.substr(begin, end - begin);
}

// Splits on the first five commas; the team name (last field) may itself contain commas.
bool splitRow(const std::string& line, std::array<std::string, 6>& fields)
{
	std::size_t start = 0;
	for (std::size_t i = 0; i < 5; i++)
	{
		std::size_t comma = line.find(',', start);
		if (comma == std::string::npos)
		{
			return false;
		}
		fields[i] = trim(line.substr(start, comma - start));
		start = comma + 1;
	}
	fields[5] = trim(line.substr(start));
	return true;
}

template <class Number>
bool parseNumber(const std::string& text, Number& out)
{
	std::istringstream stream(text);
	stream >> out;
	return stream && stream.peek() == std::char_traits<char>::eof();
}

std::string where(const std::string& sourceName, int lineNumber)
{
	return sourceName + ":" + std::to_string(lineNumber) + ": ";
}
} // namespace

std::vector<Team> parseTeams(std::istream& input, const std::string& sourceName)
{
	std::vector<Team> teams;
	std::string line;
	int lineNumber = 0;
	bool skippedHeader = false;

	while (std::getline(input, line))
	{
		lineNumber++;
		std::string row = trim(line);
		if (row.empty() || row[0] == '#')
		{
			continue;
		}

		std::array<std::string, 6> f;
		if (!splitRow(row, f))
		{
			throw DataError(where(sourceName, lineNumber) + "expected 6 comma-separated columns "
				"(wins,losses,yards,percentage,division,team_name)");
		}

		int wins = 0, losses = 0, yards = 0, division = 0;
		double percentage = 0.0;
		bool numeric = parseNumber(f[0], wins) && parseNumber(f[1], losses) && parseNumber(f[2], yards)
			&& parseNumber(f[3], percentage) && parseNumber(f[4], division);
		if (!numeric)
		{
			// Allow a single header row (first column not a number) at the top of the file.
			int ignored = 0;
			if (teams.empty() && !skippedHeader && !parseNumber(f[0], ignored))
			{
				skippedHeader = true;
				continue;
			}
			throw DataError(where(sourceName, lineNumber) + "wins, losses, yards, percentage and division must be numbers");
		}

		if (wins < 0 || losses < 0 || yards < 0)
		{
			throw DataError(where(sourceName, lineNumber) + "wins, losses and yards cannot be negative");
		}
		if (percentage < 0.0 || percentage > 1.0)
		{
			throw DataError(where(sourceName, lineNumber) + "percentage must be between 0 and 1");
		}
		if (!isValidDivision(division))
		{
			throw DataError(where(sourceName, lineNumber) + "division must be between "
				+ std::to_string(kMinDivision) + " and " + std::to_string(kMaxDivision));
		}
		if (f[5].empty())
		{
			throw DataError(where(sourceName, lineNumber) + "team name is missing");
		}

		teams.emplace_back(f[5], wins, losses, yards, division);
	}
	return teams;
}

void validateLeague(const std::vector<Team>& teams)
{
	if (static_cast<int>(teams.size()) != kLeagueSize)
	{
		throw DataError("expected " + std::to_string(kLeagueSize) + " teams but found " + std::to_string(teams.size()));
	}
	std::array<int, kMaxDivision + 1> perDivision{};
	for (const Team& team : teams)
	{
		perDivision[team.getDivVal()]++;
	}
	for (int div = kMinDivision; div <= kMaxDivision; div++)
	{
		if (perDivision[div] != kTeamsPerDivision)
		{
			throw DataError("the " + divisionName(div) + " has " + std::to_string(perDivision[div])
				+ " teams; every division needs exactly " + std::to_string(kTeamsPerDivision));
		}
	}
}

std::vector<Team> loadTeams(const std::string& path)
{
	std::ifstream file(path);
	if (!file)
	{
		throw DataError("could not open data file '" + path + "'");
	}
	std::vector<Team> teams = parseTeams(file, path);
	try
	{
		validateLeague(teams);
	}
	catch (const DataError& error)
	{
		throw DataError(path + ": " + error.what());
	}
	return teams;
}

void writeReport(const std::vector<Team>& teams, const std::string& path)
{
	std::ofstream file(path);
	if (!file)
	{
		throw DataError("could not open '" + path + "' for writing");
	}
	for (const Team& team : teams)
	{
		file << team << '\n';
	}
	if (!file)
	{
		throw DataError("failed while writing '" + path + "'");
	}
}
