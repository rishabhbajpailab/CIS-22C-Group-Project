#include "Franchises.h"

#include <unordered_map>

namespace
{
struct NameEra
{
	int lastSeason; // name used through this season (inclusive)
	const char* name;
};

const std::unordered_map<std::string, std::vector<NameEra>>& nameHistory()
{
	static const std::unordered_map<std::string, std::vector<NameEra>> history = {
		{"ARI", {{9999, "Arizona Cardinals"}}},
		{"ATL", {{9999, "Atlanta Falcons"}}},
		{"BAL", {{9999, "Baltimore Ravens"}}},
		{"BUF", {{9999, "Buffalo Bills"}}},
		{"CAR", {{9999, "Carolina Panthers"}}},
		{"CHI", {{9999, "Chicago Bears"}}},
		{"CIN", {{9999, "Cincinnati Bengals"}}},
		{"CLE", {{9999, "Cleveland Browns"}}},
		{"DAL", {{9999, "Dallas Cowboys"}}},
		{"DEN", {{9999, "Denver Broncos"}}},
		{"DET", {{9999, "Detroit Lions"}}},
		{"GB", {{9999, "Green Bay Packers"}}},
		{"HOU", {{9999, "Houston Texans"}}},
		{"IND", {{9999, "Indianapolis Colts"}}},
		{"JAX", {{9999, "Jacksonville Jaguars"}}},
		{"KC", {{9999, "Kansas City Chiefs"}}},
		{"LA", {{2015, "St. Louis Rams"}, {9999, "Los Angeles Rams"}}},
		{"LAC", {{2016, "San Diego Chargers"}, {9999, "Los Angeles Chargers"}}},
		{"LV", {{2019, "Oakland Raiders"}, {9999, "Las Vegas Raiders"}}},
		{"MIA", {{9999, "Miami Dolphins"}}},
		{"MIN", {{9999, "Minnesota Vikings"}}},
		{"NE", {{9999, "New England Patriots"}}},
		{"NO", {{9999, "New Orleans Saints"}}},
		{"NYG", {{9999, "New York Giants"}}},
		{"NYJ", {{9999, "New York Jets"}}},
		{"PHI", {{9999, "Philadelphia Eagles"}}},
		{"PIT", {{9999, "Pittsburgh Steelers"}}},
		{"SEA", {{9999, "Seattle Seahawks"}}},
		{"SF", {{9999, "San Francisco 49ers"}}},
		{"TB", {{9999, "Tampa Bay Buccaneers"}}},
		{"TEN", {{9999, "Tennessee Titans"}}},
		{"WAS", {{2019, "Washington Redskins"}, {2021, "Washington Football Team"}, {9999, "Washington Commanders"}}},
	};
	return history;
}
} // namespace

const std::vector<Franchise>& allFranchises()
{
	static const std::vector<Franchise> franchises = {
		{"CHI", 1}, {"DET", 1}, {"GB", 1}, {"MIN", 1},
		{"DAL", 2}, {"NYG", 2}, {"PHI", 2}, {"WAS", 2},
		{"ATL", 3}, {"CAR", 3}, {"NO", 3}, {"TB", 3},
		{"ARI", 4}, {"LA", 4}, {"SEA", 4}, {"SF", 4},
		{"BAL", 5}, {"CIN", 5}, {"CLE", 5}, {"PIT", 5},
		{"BUF", 6}, {"MIA", 6}, {"NE", 6}, {"NYJ", 6},
		{"HOU", 7}, {"IND", 7}, {"JAX", 7}, {"TEN", 7},
		{"DEN", 8}, {"KC", 8}, {"LAC", 8}, {"LV", 8},
	};
	return franchises;
}

const Franchise* findFranchise(const std::string& code)
{
	static const std::unordered_map<std::string, std::string> historical = {
		{"STL", "LA"}, {"LAR", "LA"}, {"SD", "LAC"}, {"OAK", "LV"},
	};
	auto alias = historical.find(code);
	const std::string& current = alias == historical.end() ? code : alias->second;
	for (const Franchise& franchise : allFranchises())
	{
		if (franchise.code == current)
		{
			return &franchise;
		}
	}
	return nullptr;
}

std::string franchiseName(const Franchise& franchise, int season)
{
	const std::vector<NameEra>& eras = nameHistory().at(franchise.code);
	for (const NameEra& era : eras)
	{
		if (season <= era.lastSeason)
		{
			return era.name;
		}
	}
	return eras.back().name;
}
