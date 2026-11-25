#include "Seeding.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>

namespace
{
constexpr double kEpsilon = 1e-9;
constexpr int kWildCardMinCommonGames = 4;

using TeamSet = std::vector<int>;
// A tiebreaker step: one value per tied team (higher is better), or an empty vector if the step
// does not apply to these teams (for example, they have not played each other).
using Metric = std::function<std::vector<double>(const Standings&, const TeamSet&)>;

struct Step
{
	const char* name;
	Metric metric;
};

struct Decision
{
	int team = -1;
	std::string step;
};

bool contains(const TeamSet& set, int team)
{
	return std::find(set.begin(), set.end(), team) != set.end();
}

// Win percentage for tiebreakers. A team with no games in the category counts as .500, so that an
// unplayed game (mid-season) neither helps nor hurts it.
double neutralPct(const Record& record)
{
	return record.games() == 0 ? 0.5 : record.pct();
}

// Opponents (outside the tied teams) that every tied team has played.
TeamSet commonOpponents(const Standings& s, const TeamSet& tied)
{
	TeamSet common;
	for (int opponent = 0; opponent < s.size(); opponent++)
	{
		if (contains(tied, opponent))
		{
			continue;
		}
		bool all = std::all_of(tied.begin(), tied.end(), [&](int team) { return s.gamesVersus(team, opponent) > 0; });
		if (all)
		{
			common.push_back(opponent);
		}
	}
	return common;
}

// --- Tiebreaker metrics --------------------------------------------------------------------------

// Head-to-head: record in games among the tied teams. Applies only if they have played each other.
std::vector<double> headToHead(const Standings& s, const TeamSet& tied)
{
	std::vector<double> values;
	int gamesAmong = 0;
	for (int team : tied)
	{
		Record record;
		for (int other : tied)
		{
			if (other != team)
			{
				record += s.versus(team, other);
			}
		}
		gamesAmong += record.games();
		values.push_back(neutralPct(record));
	}
	return gamesAmong == 0 ? std::vector<double>{} : values;
}

// Wild card head-to-head for three or more teams: applies only if one team beat each of the others
// in every meeting (it advances) or lost every meeting to each of the others (it is eliminated).
std::vector<double> headToHeadSweep(const Standings& s, const TeamSet& tied)
{
	std::vector<double> values(tied.size(), 0.0);
	bool applies = false;
	for (std::size_t i = 0; i < tied.size(); i++)
	{
		bool sweptAll = true;
		bool lostAll = true;
		for (int other : tied)
		{
			if (other == tied[i])
			{
				continue;
			}
			const Record& record = s.versus(tied[i], other);
			sweptAll = sweptAll && record.games() > 0 && record.wins == record.games();
			lostAll = lostAll && record.games() > 0 && record.losses == record.games();
		}
		if (sweptAll)
		{
			values[i] = 1.0;
			applies = true;
		}
		else if (lostAll)
		{
			values[i] = -1.0;
			applies = true;
		}
	}
	return applies ? values : std::vector<double>{};
}

std::vector<double> divisionRecord(const Standings& s, const TeamSet& tied)
{
	if (!s.hasGames())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		values.push_back(neutralPct(s.divisionRecord(team)));
	}
	return values;
}

std::vector<double> conferenceRecord(const Standings& s, const TeamSet& tied)
{
	if (!s.hasGames())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		values.push_back(neutralPct(s.conferenceRecord(team)));
	}
	return values;
}

// Record in games against common opponents. Wild card ties need at least minGames such games per team.
Metric commonGames(int minGames)
{
	return [minGames](const Standings& s, const TeamSet& tied) -> std::vector<double> {
		TeamSet common = commonOpponents(s, tied);
		if (common.empty())
		{
			return {};
		}
		std::vector<double> values;
		for (int team : tied)
		{
			Record record;
			for (int opponent : common)
			{
				record += s.versus(team, opponent);
			}
			if (record.games() < minGames)
			{
				return {};
			}
			values.push_back(neutralPct(record));
		}
		return values;
	};
}

// Strength of victory: combined record of the teams beaten (a team beaten twice counts twice).
std::vector<double> strengthOfVictory(const Standings& s, const TeamSet& tied)
{
	if (!s.hasGames())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		Record combined;
		for (int opponent = 0; opponent < s.size(); opponent++)
		{
			for (int win = 0; win < s.versus(team, opponent).wins; win++)
			{
				combined += s.overall(opponent);
			}
		}
		values.push_back(combined.pct());
	}
	return values;
}

// Strength of schedule: combined record of every opponent played (once per game).
std::vector<double> strengthOfSchedule(const Standings& s, const TeamSet& tied)
{
	if (!s.hasGames())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		Record combined;
		for (int opponent = 0; opponent < s.size(); opponent++)
		{
			for (int game = 0; game < s.gamesVersus(team, opponent); game++)
			{
				combined += s.overall(opponent);
			}
		}
		values.push_back(combined.pct());
	}
	return values;
}

// Rank (1 = best) of value among values, with tied values sharing the average of their ranks.
double averageRank(double value, const std::vector<double>& all, bool higherIsBetter)
{
	int better = 0;
	int equal = 0;
	for (double other : all)
	{
		if (std::abs(other - value) < kEpsilon)
		{
			equal++;
		}
		else if (higherIsBetter ? other > value : other < value)
		{
			better++;
		}
	}
	return 1.0 + better + (equal - 1) / 2.0;
}

// Combined ranking in points scored and points allowed (all games), among the conference's teams
// or among the whole league. A lower combined rank is better.
Metric combinedPointsRanking(bool conferenceOnly)
{
	return [conferenceOnly](const Standings& s, const TeamSet& tied) -> std::vector<double> {
		if (!s.hasGames())
		{
			return {};
		}
		std::vector<double> values;
		for (int team : tied)
		{
			std::vector<double> scored;
			std::vector<double> allowed;
			for (int other = 0; other < s.size(); other++)
			{
				if (!conferenceOnly || s.conference(other) == s.conference(team))
				{
					scored.push_back(s.pointsFor(other));
					allowed.push_back(s.pointsAgainst(other));
				}
			}
			double rank = averageRank(s.pointsFor(team), scored, true) + averageRank(s.pointsAgainst(team), allowed, false);
			values.push_back(-rank);
		}
		return values;
	};
}

std::vector<double> netPointsCommonGames(const Standings& s, const TeamSet& tied)
{
	TeamSet common = commonOpponents(s, tied);
	if (common.empty())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		int net = 0;
		for (int opponent : common)
		{
			net += s.pointsForVersus(team, opponent) - s.pointsAgainstVersus(team, opponent);
		}
		values.push_back(net);
	}
	return values;
}

std::vector<double> netPointsConference(const Standings& s, const TeamSet& tied)
{
	if (!s.hasGames())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		values.push_back(s.conferencePointsFor(team) - s.conferencePointsAgainst(team));
	}
	return values;
}

std::vector<double> netPointsAll(const Standings& s, const TeamSet& tied)
{
	if (!s.hasGames())
	{
		return {};
	}
	std::vector<double> values;
	for (int team : tied)
	{
		values.push_back(s.pointsFor(team) - s.pointsAgainst(team));
	}
	return values;
}

// --- Procedures ----------------------------------------------------------------------------------
// The NFL's net touchdowns step is left out: games.csv has scores but not touchdowns.

const std::vector<Step>& divisionSteps()
{
	static const std::vector<Step> steps = {
		{"head-to-head", headToHead},
		{"division record", divisionRecord},
		{"common games", commonGames(0)},
		{"conference record", conferenceRecord},
		{"strength of victory", strengthOfVictory},
		{"strength of schedule", strengthOfSchedule},
		{"conference points ranking", combinedPointsRanking(true)},
		{"league points ranking", combinedPointsRanking(false)},
		{"net points in common games", netPointsCommonGames},
		{"net points", netPointsAll},
	};
	return steps;
}

const std::vector<Step>& wildCardSteps(std::size_t tiedTeams)
{
	static const std::vector<Step> twoTeams = {
		{"head-to-head", headToHead},
		{"conference record", conferenceRecord},
		{"common games", commonGames(kWildCardMinCommonGames)},
		{"strength of victory", strengthOfVictory},
		{"strength of schedule", strengthOfSchedule},
		{"conference points ranking", combinedPointsRanking(true)},
		{"league points ranking", combinedPointsRanking(false)},
		{"net points in conference games", netPointsConference},
		{"net points", netPointsAll},
	};
	static std::vector<Step> moreTeams = [] {
		std::vector<Step> steps = twoTeams;
		steps[0] = {"head-to-head sweep", headToHeadSweep};
		return steps;
	}();
	return tiedTeams == 2 ? twoTeams : moreTeams;
}

enum class Format
{
	Division,
	WildCard,
};

// Stand-in for the coin toss that ends every NFL procedure: deterministic, so results are repeatable.
Decision coinToss(const Standings& s, const TeamSet& tied)
{
	int winner = *std::min_element(tied.begin(), tied.end(),
		[&](int a, int b) { return s.team(a).getName() < s.team(b).getName(); });
	return {winner, "coin toss (alphabetical)"};
}

// Picks the one team that comes out on top of a tie (teams with the same record).
// Whenever a step eliminates some teams but leaves more than one, the procedure starts over with
// the teams that are left (using the two-team procedure if two remain).
Decision breakTie(const Standings& s, const TeamSet& tied, Format format)
{
	if (tied.size() == 1)
	{
		return {tied.front(), ""};
	}
	const std::vector<Step>& steps = format == Format::Division ? divisionSteps() : wildCardSteps(tied.size());
	for (const Step& step : steps)
	{
		std::vector<double> values = step.metric(s, tied);
		if (values.empty())
		{
			continue;
		}
		double best = *std::max_element(values.begin(), values.end());
		TeamSet survivors;
		for (std::size_t i = 0; i < tied.size(); i++)
		{
			if (values[i] > best - kEpsilon)
			{
				survivors.push_back(tied[i]);
			}
		}
		if (survivors.size() == 1)
		{
			return {survivors.front(), step.name};
		}
		if (survivors.size() < tied.size())
		{
			return breakTie(s, survivors, format);
		}
	}
	return coinToss(s, tied);
}

// Orders teams by overall record. Each group of teams with the same record is ordered by repeatedly
// taking the tiebreaker winner and starting over with the rest, as the NFL procedures require.
std::vector<RankedTeam> rankByRecord(const Standings& s, TeamSet teams,
	const std::function<Decision(const TeamSet&)>& pickBest)
{
	std::stable_sort(teams.begin(), teams.end(),
		[&](int a, int b) { return s.overall(a).pct() > s.overall(b).pct() + kEpsilon; });

	std::vector<RankedTeam> ranked;
	std::size_t start = 0;
	while (start < teams.size())
	{
		std::size_t end = start + 1;
		while (end < teams.size() && std::abs(s.overall(teams[end]).pct() - s.overall(teams[start]).pct()) < kEpsilon)
		{
			end++;
		}
		TeamSet group(teams.begin() + static_cast<std::ptrdiff_t>(start), teams.begin() + static_cast<std::ptrdiff_t>(end));
		while (!group.empty())
		{
			Decision decision = pickBest(group);
			ranked.push_back({decision.team, group.size() > 1 ? decision.step : ""});
			group.erase(std::find(group.begin(), group.end(), decision.team));
		}
		start = end;
	}
	return ranked;
}

TeamSet teamsInDivision(const Standings& s, int division)
{
	TeamSet teams;
	for (int i = 0; i < s.size(); i++)
	{
		if (s.division(i) == division)
		{
			teams.push_back(i);
		}
	}
	return teams;
}
} // namespace

int playoffSeedsPerConference(int season)
{
	return season != 0 && season < 2020 ? 6 : 7;
}

std::vector<RankedTeam> rankDivision(const Standings& standings, int division)
{
	return rankByRecord(standings, teamsInDivision(standings, division),
		[&](const TeamSet& tied) { return breakTie(standings, tied, Format::Division); });
}

ConferenceSeeding seedConference(const Standings& standings, int conference, int playoffSeeds)
{
	ConferenceSeeding seeding;
	seeding.conference = conference;
	seeding.playoffSeeds = playoffSeeds;

	const int firstDivision = conference == kNfc ? 1 : 5;
	std::map<int, int> divisionPlace; // team -> 0-based place in its division
	std::map<int, std::string> divisionTiebreak;
	TeamSet winners;
	TeamSet others;
	for (int division = firstDivision; division < firstDivision + 4; division++)
	{
		std::vector<RankedTeam> ranked = rankDivision(standings, division);
		for (std::size_t place = 0; place < ranked.size(); place++)
		{
			divisionPlace[ranked[place].team] = static_cast<int>(place);
			(place == 0 ? winners : others).push_back(ranked[place].team);
		}
		if (!ranked.empty())
		{
			divisionTiebreak[ranked.front().team] = ranked.front().tiebreak;
		}
	}

	// Division winners: seeds 1-4 by record, ties broken with the wild card procedure.
	auto wildCardTie = [&](const TeamSet& tied) { return breakTie(standings, tied, Format::WildCard); };
	for (const RankedTeam& ranked : rankByRecord(standings, winners, wildCardTie))
	{
		Seed seed;
		seed.team = ranked.team;
		seed.seed = static_cast<int>(seeding.teams.size()) + 1;
		seed.divisionWinner = true;
		seed.divisionTiebreak = divisionTiebreak[ranked.team];
		seed.seedTiebreak = ranked.tiebreak;
		seeding.teams.push_back(seed);
	}

	// Everyone else in wild card order. Before comparing teams from different divisions, all but the
	// highest-ranked tied team from each division are set aside (they keep their division order).
	auto wildCardPick = [&](const TeamSet& tied) {
		std::map<int, int> bestInDivision; // division -> team
		for (int team : tied)
		{
			auto found = bestInDivision.find(standings.division(team));
			if (found == bestInDivision.end() || divisionPlace[team] < divisionPlace[found->second])
			{
				bestInDivision[standings.division(team)] = team;
			}
		}
		TeamSet candidates;
		for (const auto& entry : bestInDivision)
		{
			candidates.push_back(entry.second);
		}
		if (candidates.size() == 1 && tied.size() > 1)
		{
			return Decision{candidates.front(), "division rank"};
		}
		return breakTie(standings, candidates, Format::WildCard);
	};
	for (const RankedTeam& ranked : rankByRecord(standings, others, wildCardPick))
	{
		Seed seed;
		seed.team = ranked.team;
		seed.seed = static_cast<int>(seeding.teams.size()) + 1;
		seed.seedTiebreak = ranked.tiebreak;
		seeding.teams.push_back(seed);
	}
	return seeding;
}

PlayoffField seedPlayoffs(const Standings& standings, int playoffSeeds)
{
	return {seedConference(standings, kNfc, playoffSeeds), seedConference(standings, kAfc, playoffSeeds)};
}
