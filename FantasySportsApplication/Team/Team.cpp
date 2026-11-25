#include "Team.h"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <utility>

std::string divisionName(int division)
{
	switch (division)
	{
	case 1: return "NFC North";
	case 2: return "NFC East";
	case 3: return "NFC South";
	case 4: return "NFC West";
	case 5: return "AFC North";
	case 6: return "AFC East";
	case 7: return "AFC South";
	case 8: return "AFC West";
	default: return "Unknown Division";
	}
}

bool isValidDivision(int division)
{
	return division >= kMinDivision && division <= kMaxDivision;
}

Team::Team(std::string name, int w, int l, int y, int d, int t)
	: teamName(std::move(name)), wins(w), losses(l), ties(t), yards(y), division(d)
{
}

double Team::getPercentage() const
{
	int games = wins + losses + ties;
	return games == 0 ? 0.0 : (wins + 0.5 * ties) / games;
}

std::string Team::recordText() const
{
	std::string text = std::to_string(wins) + "-" + std::to_string(losses);
	if (ties > 0)
	{
		text += "-" + std::to_string(ties);
	}
	return text;
}

void Team::changeVals(TeamEdit edit, int amount)
{
	switch (edit)
	{
	case TeamEdit::AddYards: yards += amount; break;
	case TeamEdit::SubtractYards: yards -= amount; break;
	case TeamEdit::AddWins: wins += amount; break;
	case TeamEdit::SubtractWins: wins -= amount; break;
	case TeamEdit::AddLosses: losses += amount; break;
	case TeamEdit::SubtractLosses: losses -= amount; break;
	}
	yards = std::max(yards, 0);
	wins = std::max(wins, 0);
	losses = std::max(losses, 0);
}

int Team::keyOutput(char keyVal) const
{
	switch (keyVal)
	{
	case 'w': return wins;
	case 'l': return losses;
	case 'c': return static_cast<int>(std::lround(getPercentage() * 1000.0));
	case 'n': return nameToInt();
	case 'y':
	default: return yards;
	}
}

int Team::nameToInt() const
{
	if (teamName.empty())
	{
		return 0;
	}
	int stringASCII = static_cast<unsigned char>(teamName.front());
	for (std::size_t i = 1; i + 1 < teamName.size(); i++)
	{
		if (teamName[i] == ' ')
		{
			stringASCII = static_cast<unsigned char>(teamName[i + 1]);
		}
	}
	return stringASCII;
}

bool Team::operator==(const Team& rhs) const
{
	return wins == rhs.wins
		&& losses == rhs.losses
		&& yards == rhs.yards
		&& teamName == rhs.teamName
		&& ties == rhs.ties
		&& division == rhs.division;
}

std::ostream& operator<<(std::ostream& os, const Team& team)
{
	os << team.teamName << " (" << team.recordText() << "), " << team.getDiv() << ", "
		<< team.yards << " yards" << '\n';
	return os;
}
