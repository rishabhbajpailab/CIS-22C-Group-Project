#include "PlayoffLogic.h"

#include <algorithm>

void reorderVec(std::vector<Team>& teams)
{
	Team temp = teams[0];
	int pos = 0;

	for (int i = 0; i < teams.size(); i++)
	{
		pos = i;
		for (int j = 1; j < teams.size(); j++)
		{
			if (teams[j] < teams[pos])
				pos = j;
		}
		temp = teams[i];
		teams[i] = teams[pos];
		teams[pos] = temp;
	}
}

std::vector<Team> playoffBracket(std::vector<Team> teams)
{
	std::vector<Team> bracket;
	std::vector<Team> bracketAFC;
	std::vector<Team> bracketNFC;
	std::vector<Team> currDiv;
	std::vector<Team> currConf;
	std::vector<Team> temp = teams;

	reorderVec(temp);
	//GET NFC
	for (int i = 0; i <= 31; i++)
	{
		if (temp[i].getDivVal() == 1 || temp[i].getDivVal() == 2 || temp[i].getDivVal() == 3 || temp[i].getDivVal() == 4)
		{
			currConf.push_back(temp[i]);
		}
	}
	bracketNFC = buildPlayoffBracket(currConf);
	//get NFC wild cards
	getWildCards(currConf, bracketNFC);
	//reorderVec(bracketNFC);
	currConf.clear();
	currDiv.clear();
	//GET AFC
	for (int i = 0; i <= 31; i++)
	{
		if (temp[i].getDivVal() == 5 || temp[i].getDivVal() == 6 || temp[i].getDivVal() == 7 || temp[i].getDivVal() == 8)
		{
			currConf.push_back(temp[i]);
		}
	}
	//get AFC Div Leaders
	bracketAFC = buildPlayoffBracket(currConf, 1);
	//Get AFC Wildcards
	getWildCards(currConf, bracketAFC);

	for (int j = 0; j <= 5; j++)
	{
		bracket.push_back(bracketNFC[j]);
	}
	for (int j = 0; j <= 5; j++)
	{
		bracket.push_back(bracketAFC[j]);
	}
	return bracket;
}

std::vector<Team> buildPlayoffBracket(std::vector<Team> currConf)
{
	std::vector<Team> currDiv, bracket;
	Team test;
	int divScalar;
	for (int i = 1; i <= 4; i++)
	{
		if (currDiv.empty() != true)
		{
			currDiv.clear();
		}
		currDiv = getDivTeams(currConf, i, divScalar);
		for (int j = 0; j < currDiv.size(); j++)
		{
			currDiv[j].scaledPPI(divScalar);
		}
		test = *std::max_element(currDiv.begin(), currDiv.end());
		bracket.push_back(test);
	}
	return bracket;
}

std::vector<Team> buildPlayoffBracket(std::vector<Team> currConf, int AFC)
{
	std::vector<Team> currDiv, bracket;
	Team test;
	int divScalar;
	for (int i = 1; i <= 4; i++)
	{
		if (currDiv.empty() != true)
		{
			currDiv.clear();
		}
		currDiv = getDivTeams(currConf, i + 4, divScalar);
		for (int j = 0; j < currDiv.size(); j++)
		{
			currDiv[j].scaledPPI(divScalar);
		}
		test = *std::max_element(currDiv.begin(), currDiv.end());
		bracket.push_back(test);
	}
	return bracket;
}

std::vector<Team> getDivTeams(std::vector<Team>& teams, int inDiv, int& divScalar)
{
	std::vector<Team> retVal;
	std::vector<int> temp;
	Team dummy;
	int tempInt;
	int pos;
	int tempDiv = 0;

	retVal = { dummy, dummy, dummy, dummy };
	temp.resize(4);

	for (int i = 0; i <= 15; i++)
	{
		if (teams[i].getDivVal() == inDiv)
		{
			retVal.push_back(teams[i]);
		}
	}

	int it = 0;

	do
	{
		if (retVal[0].getDivVal() == 0)
		{
			retVal.erase(retVal.begin());
		}
		it += 1;

	} while (it <= 5);
	retVal.shrink_to_fit();
	pos = 0;
	for (int k = 0; k <= 3; k++)
	{
		if (k != pos)
		{
			k = pos;
		}
		else
		{
			tempInt = retVal[k].keyOutput('p');
			temp.push_back(tempInt);
		}
		pos += 1;
	}
	for (int i = 0; i < temp.size(); i++)
	{
		tempDiv += temp[i];
	}
	divScalar = tempDiv;
	divScalar /= 4000;
	if (divScalar < 0)
		divScalar *= -1;

	return retVal;
}

void getWildCards(std::vector<Team> source, std::vector<Team>& destination)
{
	std::vector<Team> temp = destination;
	reorderVec(temp);
	reorderVec(source);

	for (int i = 0; i < temp.size(); i++)
	{
		std::vector<Team>::iterator position = std::find(source.begin(), source.end(), temp[i]);
		if (position != source.end())
		{
			source.erase(position);
		}
	}

	for (int i = 3; i < source.size(); i++)
	{
		destination.push_back(source[i]);
	}
}
