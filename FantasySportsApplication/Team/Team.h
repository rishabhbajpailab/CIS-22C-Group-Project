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
	Team(std::string name, int wins, int losses, int yards, double percentage, int division);

	// Replaces every stat and recomputes the Playoff Performance Index (PPI).
	void input(std::string name, int wins, int losses, int yards, double percentage, int division);

	// Recomputes the PPI from yards and percentage. Call after changing any stat.
	void setRank();

	// Applies the division strength scalar to the PPI (used when picking division leaders).
	void scaledPPI(int divScalar);

	// Applies an edit from the edit menu, keeping stats non-negative, then recomputes the PPI.
	void changeVals(TeamEdit edit, int amount);

	// Sort/hash key for the given key type: 'w' wins, 'l' losses, 'p' PPI, 'n' name, 'y' yards.
	// Any other key falls back to yards.
	int keyOutput(char keyVal = 'y') const;

	// Numeric key for sorting by name: the character code of the first letter of the last word.
	int nameToInt() const;

	// One line of the playoff bracket, e.g. " From the NFC North The Green Bay Packers with the 1 seed".
	std::string playoffOutput(int seed) const;

	const std::string& getName() const { return teamName; }
	int getWins() const { return wins; }
	int getLosses() const { return losses; }
	int getYards() const { return yards; }
	double getPercentage() const { return percentage; }
	int getPPI() const { return PPI; }
	int getDivVal() const { return division; }
	std::string getDiv() const { return divisionName(division); }

	// Ordering is by PPI only.
	bool operator<(const Team& rhs) const { return PPI < rhs.PPI; }
	bool operator>(const Team& rhs) const { return PPI > rhs.PPI; }
	bool operator<=(const Team& rhs) const { return PPI <= rhs.PPI; }
	bool operator>=(const Team& rhs) const { return PPI >= rhs.PPI; }

	// Equality compares PPI and every stat except division.
	bool operator==(const Team& rhs) const;
	bool operator!=(const Team& rhs) const { return !(*this == rhs); }

	friend std::ostream& operator<<(std::ostream& os, const Team& team);

private:
	std::string teamName;
	int wins = 0;
	int losses = 0;
	int yards = 0;
	int PPI = 0; // Playoff Performance Index
	int division = 0;
	double percentage = 0.0;
};
