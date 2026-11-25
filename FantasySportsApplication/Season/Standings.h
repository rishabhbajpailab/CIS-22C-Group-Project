#pragma once

#include <vector>

#include "../Team/Team.h"

// One regular-season game between two teams of a league, by index into the league's team list.
struct Game
{
	int week = 0;
	int home = -1;
	int away = -1;
	bool played = false;
	int homeScore = 0;
	int awayScore = 0;
};

// Won-lost-tied record. Ties count as half a win and half a loss, as in the NFL standings.
struct Record
{
	int wins = 0;
	int losses = 0;
	int ties = 0;

	int games() const { return wins + losses + ties; }
	// Win percentage; 0 when no games have been played.
	double pct() const;
	// Adds one game's result from this team's side.
	void addResult(int pointsFor, int pointsAgainst);
	Record& operator+=(const Record& other);
};

// 0 for the NFC (divisions 1-4), 1 for the AFC (divisions 5-8).
int conferenceOf(int division);
constexpr int kNfc = 0;
constexpr int kAfc = 1;

// Everything the playoff seeding and tiebreakers need, computed from a league and its games.
//
// Games are optional. Without them (a data file with season totals only), the overall records come
// from the teams and every game-based tiebreaker treats the teams as equal.
//
// The teams' own wins/losses/ties are authoritative for the overall record: if they differ from the
// games (because the user edited a team), the team's numbers are used for the overall record while
// head-to-head, division and other game-based tiebreakers still use the actual games.
class Standings
{
public:
	Standings(const std::vector<Team>& teams, const std::vector<Game>& games);

	int size() const { return static_cast<int>(teams.size()); }
	const Team& team(int i) const { return teams[static_cast<std::size_t>(i)]; }
	int division(int i) const { return team(i).getDivVal(); }
	int conference(int i) const { return conferenceOf(division(i)); }

	bool hasGames() const { return gamesPlayed > 0; }
	// True if any team's overall record was edited away from what its games say.
	bool hasEditedRecords() const { return edited; }

	const Record& overall(int i) const { return overallRecords[index(i)]; }
	const Record& divisionRecord(int i) const { return divisionRecords[index(i)]; }
	const Record& conferenceRecord(int i) const { return conferenceRecords[index(i)]; }

	int pointsFor(int i) const { return totals[index(i)].pointsFor; }
	int pointsAgainst(int i) const { return totals[index(i)].pointsAgainst; }
	int conferencePointsFor(int i) const { return totals[index(i)].confPointsFor; }
	int conferencePointsAgainst(int i) const { return totals[index(i)].confPointsAgainst; }

	// Team i's record and points in played games against team j.
	const Record& versus(int i, int j) const { return pairs[pairIndex(i, j)].record; }
	int pointsForVersus(int i, int j) const { return pairs[pairIndex(i, j)].pointsFor; }
	int pointsAgainstVersus(int i, int j) const { return pairs[pairIndex(i, j)].pointsAgainst; }
	int gamesVersus(int i, int j) const { return versus(i, j).games(); }

private:
	struct Totals
	{
		int pointsFor = 0;
		int pointsAgainst = 0;
		int confPointsFor = 0;
		int confPointsAgainst = 0;
	};
	struct Pair
	{
		Record record;
		int pointsFor = 0;
		int pointsAgainst = 0;
	};

	std::size_t index(int i) const { return static_cast<std::size_t>(i); }
	std::size_t pairIndex(int i, int j) const { return index(i) * teams.size() + index(j); }
	void addSide(int team, int opponent, int pointsFor, int pointsAgainst);

	std::vector<Team> teams;
	std::vector<Record> overallRecords;
	std::vector<Record> divisionRecords;
	std::vector<Record> conferenceRecords;
	std::vector<Totals> totals;
	std::vector<Pair> pairs;
	int gamesPlayed = 0;
	bool edited = false;
};
