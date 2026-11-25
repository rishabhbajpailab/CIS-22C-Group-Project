#pragma once

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

// Splits one CSV record into fields. Supports double-quoted fields with embedded commas and
// doubled quotes (""), and strips a trailing '\r'.
std::vector<std::string> splitCsvLine(const std::string& line);

// A CSV file with a header row, read fully into memory. Columns are looked up by header name.
class CsvTable
{
public:
	// Reads the header row and every non-empty record. Throws DataError if there is no header.
	static CsvTable parse(std::istream& input, const std::string& sourceName);

	// Index of the named column. Throws DataError naming the source if it is missing.
	std::size_t column(const std::string& name) const;

	const std::vector<std::vector<std::string>>& rows() const { return records; }
	const std::string& source() const { return sourceName; }

	// Field at (row, column), or an empty string if the row is short.
	const std::string& field(const std::vector<std::string>& row, std::size_t columnIndex) const;

private:
	std::string sourceName;
	std::unordered_map<std::string, std::size_t> columns;
	std::vector<std::vector<std::string>> records;
};
