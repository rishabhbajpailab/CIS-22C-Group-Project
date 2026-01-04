#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../FantasySportsApplication/Playoff/PlayoffLogic.h"

namespace {
Team makeTeam(const std::string& name, int division, int yardOffset)
{
	Team team;
	team.input(name, 10, 7, yardOffset, 1.0, division);
	return team;
}

std::vector<Team> buildTeams()
{
	std::vector<Team> teams;
	teams.reserve(32);
	int base = 1000;
	for (int div = 1; div <= 8; div++)
	{
		for (int idx = 0; idx < 4; idx++)
		{
			int yardOffset = base + (div * 100) + (idx * 10);
			std::string name = "Div" + std::to_string(div) + "Team" + std::to_string(idx);
			teams.push_back(makeTeam(name, div, yardOffset));
		}
	}
	return teams;
}
}

int main()
{
	std::vector<Team> teams = buildTeams();
	std::vector<Team> bracket = playoffBracket(teams);

	assert(bracket.size() == 12);

	std::vector<std::string> expectedOrder = {
		"Div1Team3",
		"Div2Team3",
		"Div3Team3",
		"Div4Team3",
		"Div2Team0",
		"Div2Team1",
		"Div5Team3",
		"Div6Team3",
		"Div7Team3",
		"Div8Team3",
		"Div6Team0",
		"Div6Team1",
	};

	for (size_t i = 0; i < expectedOrder.size(); i++)
	{
		std::ostringstream out = bracket[i].playoffOutput(static_cast<int>(i));
		std::string output = out.str();
		assert(output.find(expectedOrder[i]) != std::string::npos);
	}

	std::cout << "PlayoffLogicTests passed." << std::endl;
	return 0;
}
