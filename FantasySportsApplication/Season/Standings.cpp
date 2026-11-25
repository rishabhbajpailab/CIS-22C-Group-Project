#include "Standings.h"

#include <stdexcept>

double Record::pct() const
{
	int total = games();
	return total == 0 ? 0.0 : (wins + 0.5 * ties) / total;
}

void Record::addResult(int pointsFor, int pointsAgainst)
{
	if (pointsFor > pointsAgainst)
	{
		wins++;
	}
	else if (pointsFor < pointsAgainst)
	{
		losses++;
	}
	else
	{
		ties++;
	}
}

Record& Record::operator+=(const Record& other)
{
	wins += other.wins;
	losses += other.losses;
	ties += other.ties;
	return *this;
}

int conferenceOf(int division)
{
	return division <= 4 ? kNfc : kAfc;
}

Standings::Standings(const std::vector<Team>& leagueTeams, const std::vector<Game>& games)
	: teams(leagueTeams),
	  overallRecords(leagueTeams.size()),
	  divisionRecords(leagueTeams.size()),
	  conferenceRecords(leagueTeams.size()),
	  totals(leagueTeams.size()),
	  pairs(leagueTeams.size() * leagueTeams.size())
{
	const int count = size();
	for (const Game& game : games)
	{
		if (game.home < 0 || game.home >= count || game.away < 0 || game.away >= count || game.home == game.away)
		{
			throw std::invalid_argument("game refers to a team outside the league");
		}
		if (!game.played)
		{
			continue;
		}
		addSide(game.home, game.away, game.homeScore, game.awayScore);
		addSide(game.away, game.home, game.awayScore, game.homeScore);
		gamesPlayed++;
	}

	// The teams' records win over the games: they include the user's edits, and they are the only
	// records available when there are no games.
	for (int i = 0; i < count; i++)
	{
		const Team& t = team(i);
		Record fromTeam{t.getWins(), t.getLosses(), t.getTies()};
		Record& record = overallRecords[index(i)];
		if (record.wins != fromTeam.wins || record.losses != fromTeam.losses || record.ties != fromTeam.ties)
		{
			edited = edited || hasGames();
			record = fromTeam;
		}
	}
}

void Standings::addSide(int team, int opponent, int pointsFor, int pointsAgainst)
{
	overallRecords[index(team)].addResult(pointsFor, pointsAgainst);
	Totals& total = totals[index(team)];
	total.pointsFor += pointsFor;
	total.pointsAgainst += pointsAgainst;
	if (conference(team) == conference(opponent))
	{
		conferenceRecords[index(team)].addResult(pointsFor, pointsAgainst);
		total.confPointsFor += pointsFor;
		total.confPointsAgainst += pointsAgainst;
	}
	if (division(team) == division(opponent))
	{
		divisionRecords[index(team)].addResult(pointsFor, pointsAgainst);
	}
	Pair& pair = pairs[pairIndex(team, opponent)];
	pair.record.addResult(pointsFor, pointsAgainst);
	pair.pointsFor += pointsFor;
	pair.pointsAgainst += pointsAgainst;
}
