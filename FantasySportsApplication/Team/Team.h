#pragma once

#include <iosfwd>
#include <string>

// Division ids used throughout the data files and playoff logic.
//   1-4: NFC North, East, South, West
//   5-8: AFC North, East, South, West
constexpr int kMinDivision = 1;
constexpr int kMaxDivision = 8;

// Returns the human-readable name for a division id, or "Unknown Division".
std::string divisionName(int division);
bool isValidDivision(int division);

// Operations offered by the "Edit Current Teams" menu (values match the menu numbers).
enum class TeamEdit
{
	AddYards = 1,
	SubtractYards = 2,
	AddWins = 3,
	SubtractWins = 4,
	AddLosses = 5,
	SubtractLosses = 6,
};

class Team
{
public:
	Team() = default;
	Team(std::string name, int wins, int losses, int yards, int division, int ties = 0);

	// Applies an edit from the edit menu, keeping stats non-negative.
	void changeVals(TeamEdit edit, int amount);

	// Sort/hash key for the given key type: 'w' wins, 'l' losses, 'c' win percentage (x1000),
	// 'n' name, 'y' yards. Any other key falls back to yards.
	int keyOutput(char keyVal = 'y') const;

	// Numeric key for sorting by name: the character code of the first letter of the last word.
	int nameToInt() const;

	// Record as "W-L" or "W-L-T" when the team has ties, e.g. "10-5-1".
	std::string recordText() const;

	const std::string& getName() const { return teamName; }
	int getWins() const { return wins; }
	int getLosses() const { return losses; }
	int getTies() const { return ties; }
	int getYards() const { return yards; }
	// Win percentage with ties counted as half a win; 0 before any games.
	double getPercentage() const;
	int getDivVal() const { return division; }
	std::string getDiv() const { return divisionName(division); }

	void setTies(int value) { ties = value < 0 ? 0 : value; }

	bool operator==(const Team& rhs) const;
	bool operator!=(const Team& rhs) const { return !(*this == rhs); }

	friend std::ostream& operator<<(std::ostream& os, const Team& team);

private:
	std::string teamName;
	int wins = 0;
	int losses = 0;
	int ties = 0;
	int yards = 0;
	int division = 0;
};
