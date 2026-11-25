#pragma once

#include <string>
#include <vector>

// The 32 NFL franchises with their division ids (see Team.h) and season-dependent names.
// Valid for the 2002 season onward, when the league reached 32 teams in 8 divisions.
constexpr int kFirstSupportedSeason = 2002;

struct Franchise
{
	std::string code; // current nflverse abbreviation, e.g. "LV"
	int division;     // 1-8, as used by Team and the playoff logic
};

// All franchises, ordered by division and then by code. This order is used when building a
// league, so generated data files (and therefore brackets) are deterministic.
const std::vector<Franchise>& allFranchises();

// Finds a franchise by abbreviation. Accepts historical codes used in nflverse game data
// (STL, SD, OAK) as well as current ones. Returns nullptr if the code is unknown.
const Franchise* findFranchise(const std::string& code);

// The franchise's full name in the given season, e.g. "Oakland Raiders" for LV in 2019.
std::string franchiseName(const Franchise& franchise, int season);

// Finds a franchise by any name it has used since 2002, e.g. "Oakland Raiders" or "Las Vegas Raiders".
// Returns nullptr if the name is unknown.
const Franchise* findFranchiseByName(const std::string& name);
