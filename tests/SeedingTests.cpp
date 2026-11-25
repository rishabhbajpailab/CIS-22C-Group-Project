// Tests for standings, NFL tiebreakers and playoff seeding.

#include <string>
#include <vector>

#include "Nflverse/DataSource.h"
#include "Season/Seeding.h"
#include "Season/Standings.h"
#include "TestSupport.h"

namespace
{
// Team index for team 0-3 of division 1-8. Names sort by index: "D1-A" ... "D8-D".
int id(int division, int team)
{
	return (division - 1) * 4 + team;
}

std::string teamName(int index)
{
	return "D" + std::to_string(index / 4 + 1) + "-" + std::string(1, static_cast<char>('A' + index % 4));
}

// Builds a 32-team league whose records come from the games added to it.
struct LeagueBuilder
{
	std::vector<Game> games;
	int fillerOpponent = 0;

	void play(int home, int away, int homeScore, int awayScore)
	{
		games.push_back({1, home, away, true, homeScore, awayScore});
	}
	void win(int winner, int loser) { play(winner, loser, 20, 10); }

	// Adds wins and losses against AFC West teams, which take no part in the NFC scenarios.
	void pad(int team, int wins, int losses)
	{
		for (int i = 0; i < wins + losses; i++)
		{
			int opponent = id(8, fillerOpponent++ % 4);
			i < wins ? win(team, opponent) : win(opponent, team);
		}
	}

	std::vector<Team> teams() const
	{
		std::vector<Record> records(32);
		for (const Game& game : games)
		{
			if (!game.played)
			{
				continue;
			}
			records[static_cast<std::size_t>(game.home)].addResult(game.homeScore, game.awayScore);
			records[static_cast<std::size_t>(game.away)].addResult(game.awayScore, game.homeScore);
		}
		std::vector<Team> result;
		for (int i = 0; i < 32; i++)
		{
			const Record& r = records[static_cast<std::size_t>(i)];
			result.emplace_back(teamName(i), r.wins, r.losses, 5000, i / 4 + 1, r.ties);
		}
		return result;
	}
};

int placeInDivision(const std::vector<RankedTeam>& ranked, int team)
{
	for (std::size_t i = 0; i < ranked.size(); i++)
	{
		if (ranked[i].team == team)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

const Seed* findSeed(const ConferenceSeeding& seeding, int team)
{
	for (const Seed& seed : seeding.teams)
	{
		if (seed.team == team)
		{
			return &seed;
		}
	}
	return nullptr;
}

void testRecords()
{
	Record record{10, 5, 1};
	CHECK_EQ(record.games(), 16);
	CHECK_EQ(record.pct(), 10.5 / 16.0);
	CHECK_EQ(Record{}.pct(), 0.0);

	LeagueBuilder league;
	league.play(id(1, 0), id(1, 1), 24, 24);  // division tie game
	league.play(id(1, 0), id(5, 0), 30, 10);  // interconference win
	league.games.push_back({2, id(1, 0), id(2, 0), false, 0, 0}); // unplayed: ignored
	Standings standings(league.teams(), league.games);
	CHECK(standings.hasGames());
	CHECK(!standings.hasEditedRecords());
	CHECK_EQ(standings.overall(id(1, 0)).wins, 1);
	CHECK_EQ(standings.overall(id(1, 0)).ties, 1);
	CHECK_EQ(standings.divisionRecord(id(1, 0)).ties, 1);
	CHECK_EQ(standings.conferenceRecord(id(1, 0)).games(), 1);
	CHECK_EQ(standings.pointsFor(id(1, 0)), 54);
	CHECK_EQ(standings.pointsAgainst(id(1, 0)), 34);
	CHECK_EQ(standings.gamesVersus(id(1, 0), id(2, 0)), 0);
	CHECK_EQ(standings.versus(id(1, 1), id(1, 0)).ties, 1);
}

void testFormatAndTotalsOnly()
{
	CHECK_EQ(playoffSeedsPerConference(2019), 6);
	CHECK_EQ(playoffSeedsPerConference(2020), 7);
	CHECK_EQ(playoffSeedsPerConference(0), 7);

	// Season totals only (no games): every division is 10-7, 9-8, 8-9, 7-10, except the NFC North,
	// whose top two teams are 12-5 and 11-6.
	std::vector<Team> teams;
	const int wins[] = {10, 9, 8, 7};
	for (int i = 0; i < 32; i++)
	{
		int w = wins[i % 4] + (i / 4 == 0 && i % 4 < 2 ? 2 : 0);
		teams.emplace_back(teamName(i), w, 17 - w, 5000, i / 4 + 1);
	}
	Standings standings(teams, {});
	CHECK(!standings.hasGames());
	ConferenceSeeding nfc = seedConference(standings, kNfc, 7);
	CHECK_EQ(nfc.teams.size(), static_cast<std::size_t>(16));

	const int expected[] = {id(1, 0), id(2, 0), id(3, 0), id(4, 0), id(1, 1), id(2, 1), id(3, 1)};
	for (int seed = 1; seed <= 7; seed++)
	{
		CHECK_EQ(nfc.teams[static_cast<std::size_t>(seed - 1)].team, expected[seed - 1]);
		CHECK_EQ(nfc.teams[static_cast<std::size_t>(seed - 1)].seed, seed);
		CHECK_EQ(nfc.teams[static_cast<std::size_t>(seed - 1)].divisionWinner, seed <= 4);
	}
	// The 11-6 wild card is seeded below the 10-7 division winners.
	CHECK(teams[static_cast<std::size_t>(id(1, 1))].getWins() > teams[static_cast<std::size_t>(id(4, 0))].getWins());
	// Same records with no games: the coin toss stand-in decides, and says so.
	CHECK_EQ(nfc.teams[1].seedTiebreak, std::string("coin toss (alphabetical)"));
	CHECK(nfc.teams[0].seedTiebreak.empty()); // 12-5 is best on record alone
	CHECK(nfc.inPlayoffs(nfc.teams[6]));
	CHECK(!nfc.inPlayoffs(nfc.teams[7]));

	// Each team appears once.
	for (std::size_t i = 0; i < nfc.teams.size(); i++)
	{
		for (std::size_t j = i + 1; j < nfc.teams.size(); j++)
		{
			CHECK(nfc.teams[i].team != nfc.teams[j].team);
		}
	}
	CHECK_EQ(seedConference(standings, kAfc, 6).playoffSeeds, 6);
}

void testDivisionTiebreakers()
{
	const int a = id(1, 0), b = id(1, 1), c = id(1, 2), d = id(1, 3);

	// Two teams at 1-1: head-to-head.
	{
		LeagueBuilder league;
		league.win(b, a); // B beat A
		league.win(a, id(2, 0));
		league.win(id(2, 1), b);
		Standings standings(league.teams(), league.games);
		std::vector<RankedTeam> ranked = rankDivision(standings, 1);
		CHECK_EQ(ranked[0].team, b);
		CHECK_EQ(ranked[0].tiebreak, std::string("head-to-head"));
		CHECK_EQ(ranked[1].team, a);
	}

	// Two teams at 1-1 that have not met: division record.
	{
		LeagueBuilder league;
		league.win(a, c);        // A 1-0 in the division
		league.win(id(2, 0), a); // A 1-1
		league.win(b, id(2, 1)); // B 1-0 outside the division
		league.win(d, b);        // B 0-1 in the division, 1-1 overall
		Standings standings(league.teams(), league.games);
		std::vector<RankedTeam> ranked = rankDivision(standings, 1);
		CHECK_EQ(ranked[0].team, d); // 1-0
		CHECK_EQ(ranked[1].team, a);
		CHECK_EQ(ranked[1].tiebreak, std::string("division record"));
		CHECK_EQ(ranked[2].team, b);
		CHECK_EQ(ranked[3].team, c);
	}

	// Three teams at 3-2: A is best head-to-head among the three (2-1), then B beats C head-to-head.
	{
		LeagueBuilder league;
		league.win(a, b);
		league.win(a, b);
		league.win(c, a);
		league.win(b, c);
		league.pad(a, 1, 1);
		league.pad(b, 2, 0);
		league.pad(c, 2, 1);
		Standings standings(league.teams(), league.games);
		CHECK_EQ(standings.overall(a).wins, 3);
		CHECK_EQ(standings.overall(b).wins, 3);
		CHECK_EQ(standings.overall(c).wins, 3);
		std::vector<RankedTeam> ranked = rankDivision(standings, 1);
		CHECK_EQ(ranked[0].team, a);
		CHECK_EQ(ranked[0].tiebreak, std::string("head-to-head"));
		CHECK_EQ(ranked[1].team, b);
		CHECK_EQ(ranked[1].tiebreak, std::string("head-to-head"));
		CHECK_EQ(ranked[2].team, c);
		CHECK_EQ(ranked[3].team, d);
	}
}

void testWildCards()
{
	// NFC division leaders go 10-0. B1 and C1 (NFC North) and W2 (NFC East) finish 3-3.
	// B1 beat C1, C1 beat W2, W2 beat B1. Only the best of B1/C1 (B1, on head-to-head) may be compared
	// with W2 at first, and W2 beat B1, so W2 is seed 5, then B1 (division order), then C1.
	const int b1 = id(1, 1), c1 = id(1, 2), w2 = id(2, 0);
	LeagueBuilder league;
	league.win(b1, c1);
	league.win(c1, w2);
	league.win(w2, b1);
	for (int team : {b1, c1, w2})
	{
		league.pad(team, 2, 2);
	}
	for (int leader : {id(1, 0), id(2, 1), id(3, 0), id(4, 0)})
	{
		league.pad(leader, 10, 0);
	}
	Standings standings(league.teams(), league.games);
	ConferenceSeeding nfc = seedConference(standings, kNfc, 7);

	const Seed* seedW2 = findSeed(nfc, w2);
	const Seed* seedB1 = findSeed(nfc, b1);
	const Seed* seedC1 = findSeed(nfc, c1);
	CHECK(seedW2 != nullptr && seedB1 != nullptr && seedC1 != nullptr);
	if (seedW2 && seedB1 && seedC1)
	{
		CHECK_EQ(seedW2->seed, 5);
		CHECK_EQ(seedW2->seedTiebreak, std::string("head-to-head"));
		CHECK_EQ(seedB1->seed, 6);
		CHECK_EQ(seedB1->seedTiebreak, std::string("division rank"));
		CHECK_EQ(seedC1->seed, 7);
		CHECK(!seedC1->divisionWinner);
	}
	CHECK_EQ(placeInDivision(rankDivision(standings, 1), b1), 1);
	CHECK_EQ(placeInDivision(rankDivision(standings, 1), c1), 2);
}

void testEditedRecords()
{
	const int a = id(1, 0), b = id(1, 1);
	LeagueBuilder league;
	league.win(b, a);
	league.win(a, id(2, 0));
	league.win(id(2, 1), b);
	std::vector<Team> teams = league.teams();
	teams[static_cast<std::size_t>(a)].changeVals(TeamEdit::AddWins, 1); // A edited to 2-1

	Standings standings(teams, league.games);
	CHECK(standings.hasEditedRecords());
	CHECK_EQ(standings.overall(a).wins, 2);
	CHECK_EQ(standings.versus(b, a).wins, 1); // games are unchanged
	std::vector<RankedTeam> ranked = rankDivision(standings, 1);
	CHECK_EQ(ranked[0].team, a);
	CHECK(ranked[0].tiebreak.empty());
}

// The bundled 2014 season matches the real playoff seeds.
void testBundled2014()
{
	League league = loadLeagueFile("data/data.csv");
	CHECK_EQ(league.season, 2014);
	CHECK_EQ(league.games.size(), static_cast<std::size_t>(256));
	Standings standings(league.teams, league.games);
	CHECK(!standings.hasEditedRecords());
	PlayoffField field = seedPlayoffs(standings, playoffSeedsPerConference(league.season));

	const std::vector<std::string> nfc = {"Seattle Seahawks", "Green Bay Packers", "Dallas Cowboys",
		"Carolina Panthers", "Arizona Cardinals", "Detroit Lions"};
	const std::vector<std::string> afc = {"New England Patriots", "Denver Broncos", "Pittsburgh Steelers",
		"Indianapolis Colts", "Cincinnati Bengals", "Baltimore Ravens"};
	CHECK_EQ(field.nfc.playoffSeeds, 6);
	for (std::size_t i = 0; i < 6; i++)
	{
		CHECK_EQ(standings.team(field.nfc.teams[i].team).getName(), nfc[i]);
		CHECK_EQ(standings.team(field.afc.teams[i].team).getName(), afc[i]);
	}
	// Seattle, Green Bay and Dallas were all 12-4: Seattle and then Green Bay on conference record.
	CHECK_EQ(field.nfc.teams[0].seedTiebreak, std::string("conference record"));
	CHECK_EQ(field.nfc.teams[1].seedTiebreak, std::string("conference record"));
	// New England over Denver and Pittsburgh over Indianapolis on head-to-head.
	CHECK_EQ(field.afc.teams[0].seedTiebreak, std::string("head-to-head"));
	CHECK_EQ(field.afc.teams[2].seedTiebreak, std::string("head-to-head"));
	// Carolina (7-8-1) won its division and is seeded above 11-5 Arizona.
	CHECK_EQ(standings.team(field.nfc.teams[3].team).recordText(), std::string("7-8-1"));
}
} // namespace

int main()
{
	testRecords();
	testFormatAndTotalsOnly();
	testDivisionTiebreakers();
	testWildCards();
	testEditedRecords();
	testBundled2014();
	return finishTests("SeedingTests");
}
