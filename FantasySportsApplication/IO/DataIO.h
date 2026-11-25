#pragma once

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

#include "../Team/Team.h"

constexpr int kLeagueSize = 32;
constexpr int kTeamsPerDivision = 4;

inline const std::string kDefaultDataFile = "data/data.csv";
inline const std::string kDefaultReportFile = "Output.txt";

// Thrown when a data file cannot be read or does not describe a valid league.
class DataError : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

// Parses team rows in the format documented in docs/data-format.md:
//   wins,losses,yards,percentage,division,team_name
// Blank lines, lines starting with '#', and a leading header row are skipped. Windows line endings
// are accepted. Throws DataError naming the line number for any malformed row.
std::vector<Team> parseTeams(std::istream& input, const std::string& sourceName = "input");

// Opens and parses a data file, then validates it with validateLeague. Throws DataError.
std::vector<Team> loadTeams(const std::string& path);

// Checks that the league has 32 teams with exactly 4 in each of the 8 divisions,
// which the playoff logic requires. Throws DataError describing the first problem found.
void validateLeague(const std::vector<Team>& teams);

// Writes every team to a text report. Throws DataError if the file cannot be written.
void writeReport(const std::vector<Team>& teams, const std::string& path = kDefaultReportFile);
