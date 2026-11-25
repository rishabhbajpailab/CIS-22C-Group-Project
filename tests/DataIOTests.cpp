#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "IO/DataIO.h"
#include "TestSupport.h"

namespace
{
std::vector<Team> parse(const std::string& text)
{
	std::istringstream in(text);
	return parseTeams(in, "test.csv");
}

std::string league()
{
	std::string text;
	for (int div = 1; div <= 8; div++)
	{
		for (int i = 0; i < 4; i++)
		{
			text += "8,8,5000,0.5," + std::to_string(div) + ",Team " + std::to_string(div) + "-" + std::to_string(i) + "\n";
		}
	}
	return text;
}
} // namespace

int main()
{
	// Basic row, Windows line ending, spaces around fields.
	std::vector<Team> teams = parse("11,5,6506,0.667,7,Indianapolis Colts\r\n 12 , 4 , 5848 , 0.75 , 6 , New England Patriots \n");
	CHECK_EQ(teams.size(), static_cast<std::size_t>(2));
	CHECK_EQ(teams[0].getName(), std::string("Indianapolis Colts"));
	CHECK_EQ(teams[0].getYards(), 6506);
	CHECK_EQ(teams[1].getName(), std::string("New England Patriots"));
	CHECK_EQ(teams[1].getDivVal(), 6);

	// Header row, comments and blank lines are skipped.
	teams = parse("wins,losses,yards,percentage,division,team_name\n\n# comment\n1,2,3,0.5,1,A\n");
	CHECK_EQ(teams.size(), static_cast<std::size_t>(1));

	// Malformed rows are rejected.
	CHECK_THROWS(parse("1,2,3,0.5,1\n"), DataError);           // too few columns
	CHECK_THROWS(parse("1,2,3,0.5,9,Team\n"), DataError);      // bad division
	CHECK_THROWS(parse("1,2,x,0.5,1,Team\n1,2,3,0.5,1,B\n"), DataError); // a numeric first column is data, not a header
	CHECK_THROWS(parse("1,2,3,0.5,1,A\n1,2,x,0.5,1,B\n"), DataError); // non-numeric after data
	CHECK_THROWS(parse("1,2,3,1.5,1,Team\n"), DataError);      // percentage out of range
	CHECK_THROWS(parse("-1,2,3,0.5,1,Team\n"), DataError);     // negative
	CHECK_THROWS(parse("1,2,3,0.5,1,\n"), DataError);          // missing name

	// League validation.
	CHECK_EQ(parse(league()).size(), static_cast<std::size_t>(32));
	validateLeague(parse(league()));
	CHECK_THROWS(validateLeague(parse("1,2,3,0.5,1,A\n")), DataError);
	std::string unbalanced = league();
	unbalanced.replace(unbalanced.find(",1,Team 1-0"), 3, ",2,");
	CHECK_THROWS(validateLeague(parse(unbalanced)), DataError);

	// The bundled data file loads.
	CHECK_EQ(loadTeams("data/data.csv").size(), static_cast<std::size_t>(32));
	CHECK_THROWS(loadTeams("data/does-not-exist.csv"), DataError);

	// Report round trip.
	const std::string reportPath = "build/DataIOTests-report.txt";
	writeReport(parse(league()), reportPath);
	std::ifstream report(reportPath);
	std::string firstLine;
	std::getline(report, firstLine);
	CHECK_EQ(firstLine, std::string("Team 1-0 (8-8), NFC North, 5000 yards"));
	report.close();
	std::remove(reportPath.c_str());

	return finishTests("DataIOTests");
}
