// Tests for the Team model: records, edits and sort keys.

#include <sstream>

#include "Team/Team.h"
#include "TestSupport.h"

int main()
{
	Team colts("Indianapolis Colts", 11, 5, 6506, 7);
	CHECK_EQ(colts.getPercentage(), 11.0 / 16.0);
	CHECK_EQ(colts.recordText(), std::string("11-5"));
	CHECK_EQ(colts.nameToInt(), static_cast<int>('C'));
	CHECK_EQ(colts.getDiv(), std::string("AFC South"));
	CHECK_EQ(colts.keyOutput('c'), 688);

	Team bengals("Cincinnati Bengals", 10, 5, 5936, 5, 1);
	CHECK_EQ(bengals.getPercentage(), 10.5 / 16.0); // ties count as half a win
	CHECK_EQ(bengals.recordText(), std::string("10-5-1"));
	CHECK_EQ(Team().getPercentage(), 0.0);

	Team edited("Test Team", 8, 8, 5000, 1);
	edited.changeVals(TeamEdit::AddYards, 1000);
	CHECK_EQ(edited.getYards(), 6000);
	edited.changeVals(TeamEdit::AddWins, 1);
	CHECK_EQ(edited.getPercentage(), 9.0 / 17.0); // edits change the win percentage
	edited.changeVals(TeamEdit::SubtractWins, 20);
	CHECK_EQ(edited.getWins(), 0); // stats never go negative
	edited.changeVals(TeamEdit::AddLosses, 2);
	CHECK_EQ(edited.keyOutput('l'), 10);
	CHECK_EQ(edited.keyOutput('y'), 6000);
	CHECK_EQ(edited.keyOutput('?'), 6000); // unknown keys fall back to yards

	Team a("A", 1, 1, 100, 1);
	CHECK(a == Team("A", 1, 1, 100, 1));
	CHECK(a != Team("A", 1, 1, 100, 2));
	CHECK(a != Team("A", 1, 1, 100, 1, 1));

	std::ostringstream text;
	text << bengals;
	CHECK_EQ(text.str(), std::string("Cincinnati Bengals (10-5-1), AFC North, 5936 yards\n"));
	return finishTests("TeamTests");
}
