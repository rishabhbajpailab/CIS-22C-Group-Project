// Port of the original HashTableTest.cpp checks, as automated assertions.

#include <string>
#include <vector>

#include "HashTable/HashTable.h"
#include "TestSupport.h"

using StringTable = HashTable<int, std::string>;

int main()
{
	HashTable<int, std::string> table;
	table.add(0, "a");
	table.add(1, "b");
	table.add(2, "c");
	CHECK_EQ(table.size(), 3);
	CHECK(!table.isEmpty());

	HashTable<int, std::string> copy(table);
	CHECK_EQ(copy.size(), 3);
	CHECK_EQ(copy.get(1), std::string("b"));
	copy.clear();
	CHECK(copy.isEmpty());
	CHECK_EQ(table.size(), 3); // copy is independent

	HashTable<int, std::string> assigned;
	assigned = table;
	CHECK_EQ(assigned.get(2), std::string("c"));

	table.add(2, "d"); // duplicate key updates in place
	CHECK_EQ(table.size(), 3);
	CHECK_EQ(table.get(2), std::string("d"));
	CHECK_EQ(assigned.get(2), std::string("c"));

	CHECK(table.get(0) == "a" && table.get(1) == "b");
	CHECK_THROWS(table.get(5), StringTable::NotFoundException);
	CHECK(table.contains(0));
	CHECK(!table.contains(10));

	CHECK(table.remove(2));
	CHECK(!table.remove(2));
	CHECK_EQ(table.size(), 2);
	CHECK_THROWS(table.get(2), StringTable::NotFoundException);

	std::vector<std::string> seen;
	table.traverse([&](const std::string& item) { seen.push_back(item); });
	CHECK_EQ(seen.size(), static_cast<std::size_t>(2));

	HashTable<int, int> small(4);
	for (int i = 0; i < 8; i++)
	{
		small.add(i, i * i); // forces chaining
	}
	CHECK_EQ(small.size(), 8);
	CHECK_EQ(small.get(7), 49);
	CHECK_EQ(small.bucketCount(), 4);
	CHECK_EQ(small.loadFactor(), 2.0);
	CHECK_EQ(small.occupiedBuckets(), 4);

	table.clear();
	CHECK_EQ(table.size(), 0);
	CHECK(table.isEmpty());
	CHECK(!table.contains(1));
	return finishTests("HashTableTests");
}
