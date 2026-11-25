// Regression test: the bracket for the bundled data file must match the original implementation.

#include <string>
#include <vector>

#include "IO/DataIO.h"
#include "Playoff/PlayoffLogic.h"
#include "TestSupport.h"

int main()
{
	std::vector<Team> teams = loadTeams("data/data.csv");
	std::vector<Team> bracket = playoffBracket(teams);

	const std::vector<std::string> expected = {
		" From the NFC North The Green Bay Packers with the 1 seed",
		" From the NFC East The Philadelphia Eagles with the 2 seed",
		" From the NFC South The New Orleans Saints with the 3 seed",
		" From the NFC West The Arizona Cardinals with the 4 seed",
		" From the NFC West The Seattle Seahawks with the 5 seed",
		" From the NFC South The Atlanta Falcons with the 6 seed",
		" From the AFC North The Cincinnati Bengals with the 1 seed",
		" From the AFC East The New England Patriots with the 2 seed",
		" From the AFC South The Indianapolis Colts with the 3 seed",
		" From the AFC West The Denver Broncos with the 4 seed",
		" From the AFC East The Miami Dolphins with the 5 seed",
		" From the AFC West The Kansas City Chiefs with the 6 seed",
	};
	CHECK_EQ(bracket.size(), expected.size());
	for (std::size_t i = 0; i < expected.size() && i < bracket.size(); i++)
	{
		int seed = static_cast<int>(i % kSeedsPerConference) + 1;
		CHECK_EQ(bracket[i].playoffOutput(seed), expected[i]);
	}
	return finishTests("BracketRegressionTests");
}
