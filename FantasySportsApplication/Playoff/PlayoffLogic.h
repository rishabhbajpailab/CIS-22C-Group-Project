#pragma once

#include <vector>

#include "../Team/Team.h"

constexpr int kSeedsPerConference = 6;
constexpr int kBracketSize = 2 * kSeedsPerConference;

// Builds the 12-team playoff bracket from a full 32-team league:
// NFC seeds 1-6 followed by AFC seeds 1-6 (4 division leaders, then 2 wild cards each).
// Expects 4 teams in each of the 8 divisions (see validateLeague in IO/DataIO.h).
std::vector<Team> playoffBracket(const std::vector<Team>& teams);

// Division leaders for the conference whose divisions are firstDivision .. firstDivision + 3.
std::vector<Team> buildPlayoffBracket(const std::vector<Team>& currConf, int firstDivision);

// Teams in division inDiv from a 16-team conference. divScalar receives the division strength
// scalar (sum of the division's PPIs / 4000) used to weight each team's PPI.
std::vector<Team> getDivTeams(const std::vector<Team>& teams, int inDiv, int& divScalar);

// Appends the wild card teams from source to destination (which holds the division leaders).
void getWildCards(std::vector<Team> source, std::vector<Team>& destination);

// In-place ordering pass by PPI used before splitting the league into conferences.
void reorderVec(std::vector<Team>& teams);
