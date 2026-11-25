#include "Team.h"

#include <algorithm>
#include <ostream>
#include <sstream>
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

Team::Team(std::string name, int w, int l, int y, double p, int d)
{
	input(std::move(name), w, l, y, p, d);
}

void Team::input(std::string name, int w, int l, int y, double p, int d)
{
	teamName = std::move(name);
	wins = w;
	losses = l;
	yards = y;
	percentage = p;
	division = d;
	setRank();
}

void Team::setRank()
{
	double value = yards * percentage;
	PPI = static_cast<int>(value);
}

void Team::scaledPPI(int divScalar)
{
	// Original PPI formula, kept as-is. The int truncation of each step is part of the result.
	PPI = static_cast<int>(PPI * (PPI * divScalar * percentage));
	if (wins >= 8)
	{
		PPI = static_cast<int>(PPI + (wins * divScalar) * percentage);
	}
	else
	{
		PPI = static_cast<int>(PPI - (wins * divScalar) * percentage);
	}
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
	setRank();
}

int Team::keyOutput(char keyVal) const
{
	switch (keyVal)
	{
	case 'w': return wins;
	case 'l': return losses;
	case 'p': return PPI;
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

std::string Team::playoffOutput(int seed) const
{
	std::ostringstream output;
	output << " From the " << getDiv() << " The " << teamName << " with the " << seed << " seed";
	return output.str();
}

bool Team::operator==(const Team& rhs) const
{
	return PPI == rhs.PPI
		&& wins == rhs.wins
		&& losses == rhs.losses
		&& yards == rhs.yards
		&& teamName == rhs.teamName
		&& percentage == rhs.percentage;
}

std::ostream& operator<<(std::ostream& os, const Team& team)
{
	os << team.teamName << " " << team.wins << " Wins, " << team.losses << " Losses, " << '\n'
		<< "In the " << team.getDiv() << " with a PPI of " << team.PPI << '\n';
	return os;
}
