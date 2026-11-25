#pragma once

#include <string>
#include <vector>

#include "Standings.h"

// Playoff seeding by the NFL's rules: each conference's four division winners are seeds 1-4,
// ordered by record, and the best remaining records are the wild cards. Ties are broken with the
// NFL tie-breaking procedures (see docs/algorithm-revision.md for the steps and their limits).

// Playoff teams per conference: 7 from the 2020 season on, 6 for 2002-2019.
// A season of 0 (unknown, e.g. a hand-made data file) uses the current format.
int playoffSeedsPerConference(int season);

struct RankedTeam
{
	int team = -1;
	// The tiebreaker step that placed this team ahead of the teams with the same record behind it,
	// e.g. "head-to-head". Empty when its record alone decided its place.
	std::string tiebreak;
};

// A division's teams, best first.
std::vector<RankedTeam> rankDivision(const Standings& standings, int division);

struct Seed
{
	int team = -1;
	int seed = 0;               // 1 = best. Seeds above the conference's playoff seeds are out.
	bool divisionWinner = false;
	std::string divisionTiebreak; // how a division winner won its division, if by tiebreaker
	std::string seedTiebreak;     // how the team got this seed over teams with the same record
};

struct ConferenceSeeding
{
	int conference = kNfc;
	int playoffSeeds = 7;
	// Every team in the conference: the playoff seeds first, then the rest in wild card order.
	std::vector<Seed> teams;

	bool inPlayoffs(const Seed& seed) const { return seed.seed <= playoffSeeds; }
};

ConferenceSeeding seedConference(const Standings& standings, int conference, int playoffSeeds);

struct PlayoffField
{
	ConferenceSeeding nfc;
	ConferenceSeeding afc;
};

PlayoffField seedPlayoffs(const Standings& standings, int playoffSeeds);
