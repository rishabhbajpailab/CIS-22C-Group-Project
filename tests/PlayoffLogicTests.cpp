// Characterization test for the playoff algorithm on a synthetic league.
// Expected seeds were produced by the original (pre-refactor) implementation.

#include <string>
#include <vector>

#include "Playoff/PlayoffLogic.h"
#include "TestSupport.h"

namespace
{
std::vector<Team> buildTeams()
{
	std::vector<Team> teams;
	teams.reserve(32);
	int base = 1000;
	for (int div = 1; div <= 8; div++)
	{
		for (int idx = 0; idx < 4; idx++)
		{
			int yards = base + (div * 100) + (idx * 10);
			std::string name = "Div" + std::to_string(div) + "Team" + std::to_string(idx);
			teams.emplace_back(name, 10, 7, yards, 1.0, div);
		}
	}
	return teams;
}
} // namespace

int main()
{
	std::vector<Team> bracket = playoffBracket(buildTeams());
	CHECK_EQ(bracket.size(), static_cast<std::size_t>(kBracketSize));

	const std::vector<std::string> expectedOrder = {
		"Div1Team3", "Div2Team3", "Div3Team3", "Div4Team3", "Div2Team1", "Div2Team2",
		"Div5Team3", "Div6Team3", "Div7Team3", "Div8Team3", "Div6Team0", "Div6Team1",
	};
	for (std::size_t i = 0; i < expectedOrder.size() && i < bracket.size(); i++)
	{
		CHECK_EQ(bracket[i].getName(), expectedOrder[i]);
	}
	return finishTests("PlayoffLogicTests");
}
