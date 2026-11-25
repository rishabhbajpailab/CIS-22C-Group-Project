#include "PlayoffLogic.h"

#include <algorithm>
#include <utility>

namespace
{
constexpr int kTeamsPerConference = 16;
constexpr int kDivisionsPerConference = 4;
constexpr int kNfcFirstDivision = 1;
constexpr int kAfcFirstDivision = 5;
constexpr int kDivScalarDivisor = 4000;

std::vector<Team> conferenceTeams(const std::vector<Team>& league, int firstDivision)
{
	std::vector<Team> conference;
	for (const Team& team : league)
	{
		int div = team.getDivVal();
		if (div >= firstDivision && div < firstDivision + kDivisionsPerConference)
		{
			conference.push_back(team);
		}
	}
	return conference;
}
} // namespace

void reorderVec(std::vector<Team>& teams)
{
	// Note: the inner loop starts at 1 rather than i + 1. That is how the original
	// algorithm ordered teams, and bracket results depend on it, so it is kept as-is.
	for (std::size_t i = 0; i < teams.size(); i++)
	{
		std::size_t pos = i;
		for (std::size_t j = 1; j < teams.size(); j++)
		{
			if (teams[j] < teams[pos])
			{
				pos = j;
			}
		}
		std::swap(teams[i], teams[pos]);
	}
}

std::vector<Team> playoffBracket(const std::vector<Team>& teams)
{
	std::vector<Team> ordered = teams;
	reorderVec(ordered);

	std::vector<Team> nfc = conferenceTeams(ordered, kNfcFirstDivision);
	std::vector<Team> bracketNFC = buildPlayoffBracket(nfc, kNfcFirstDivision);
	getWildCards(nfc, bracketNFC);

	std::vector<Team> afc = conferenceTeams(ordered, kAfcFirstDivision);
	std::vector<Team> bracketAFC = buildPlayoffBracket(afc, kAfcFirstDivision);
	getWildCards(afc, bracketAFC);

	std::vector<Team> bracket;
	bracket.reserve(kBracketSize);
	bracket.insert(bracket.end(), bracketNFC.begin(), bracketNFC.begin() + kSeedsPerConference);
	bracket.insert(bracket.end(), bracketAFC.begin(), bracketAFC.begin() + kSeedsPerConference);
	return bracket;
}

std::vector<Team> buildPlayoffBracket(const std::vector<Team>& currConf, int firstDivision)
{
	std::vector<Team> bracket;
	for (int i = 0; i < kDivisionsPerConference; i++)
	{
		int divScalar = 0;
		std::vector<Team> currDiv = getDivTeams(currConf, firstDivision + i, divScalar);
		for (Team& team : currDiv)
		{
			team.scaledPPI(divScalar);
		}
		bracket.push_back(*std::max_element(currDiv.begin(), currDiv.end()));
	}
	return bracket;
}

std::vector<Team> getDivTeams(const std::vector<Team>& teams, int inDiv, int& divScalar)
{
	std::vector<Team> retVal;
	for (int i = 0; i < kTeamsPerConference; i++)
	{
		if (teams[i].getDivVal() == inDiv)
		{
			retVal.push_back(teams[i]);
		}
	}

	// Division strength: the sum of the first four teams' PPIs, scaled down (integer division).
	int tempDiv = 0;
	for (std::size_t k = 0; k < retVal.size() && k < 4; k++)
	{
		tempDiv += retVal[k].keyOutput('p');
	}
	divScalar = tempDiv / kDivScalarDivisor;
	if (divScalar < 0)
	{
		divScalar *= -1;
	}
	return retVal;
}

void getWildCards(std::vector<Team> source, std::vector<Team>& destination)
{
	std::vector<Team> leaders = destination;
	reorderVec(leaders);
	reorderVec(source);

	for (const Team& leader : leaders)
	{
		auto position = std::find(source.begin(), source.end(), leader);
		if (position != source.end())
		{
			source.erase(position);
		}
	}

	// Skips the first three remaining teams and takes the rest; the caller keeps the first two.
	for (std::size_t i = 3; i < source.size(); i++)
	{
		destination.push_back(source[i]);
	}
}
