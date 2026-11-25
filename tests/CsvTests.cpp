#include <sstream>
#include <string>
#include <vector>

#include "IO/Csv.h"
#include "IO/DataIO.h"
#include "TestSupport.h"

int main()
{
	using Fields = std::vector<std::string>;
	CHECK(splitCsvLine("a,b,c") == (Fields{"a", "b", "c"}));
	CHECK(splitCsvLine("a,,c,") == (Fields{"a", "", "c", ""}));
	CHECK(splitCsvLine("a,\"b,c\",d\r") == (Fields{"a", "b,c", "d"}));
	CHECK(splitCsvLine("\"say \"\"hi\"\"\",x") == (Fields{"say \"hi\"", "x"}));
	CHECK(splitCsvLine("\"\",x") == (Fields{"", "x"}));
	CHECK(splitCsvLine("") == (Fields{""}));

	std::istringstream input("\xEF\xBB\xBFname,score\r\nBears,24\r\n\r\n\"Lions, Detroit\",31\nshort\n");
	CsvTable table = CsvTable::parse(input, "scores.csv");
	CHECK_EQ(table.rows().size(), static_cast<std::size_t>(3));
	std::size_t name = table.column("name");
	std::size_t score = table.column("score");
	CHECK_EQ(table.field(table.rows()[1], name), std::string("Lions, Detroit"));
	CHECK_EQ(table.field(table.rows()[1], score), std::string("31"));
	CHECK_EQ(table.field(table.rows()[2], score), std::string("")); // short row
	CHECK_THROWS(table.column("missing"), DataError);

	std::istringstream empty("");
	CHECK_THROWS(CsvTable::parse(empty, "empty.csv"), DataError);
	return finishTests("CsvTests");
}
