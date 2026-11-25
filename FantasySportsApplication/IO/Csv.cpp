#include "Csv.h"

#include <istream>

#include "DataIO.h"

std::vector<std::string> splitCsvLine(const std::string& line)
{
	std::vector<std::string> fields;
	std::string current;
	bool inQuotes = false;

	std::size_t end = line.size();
	if (end > 0 && line[end - 1] == '\r')
	{
		end--;
	}

	for (std::size_t i = 0; i < end; i++)
	{
		char c = line[i];
		if (inQuotes)
		{
			if (c == '"' && i + 1 < end && line[i + 1] == '"')
			{
				current += '"';
				i++;
			}
			else if (c == '"')
			{
				inQuotes = false;
			}
			else
			{
				current += c;
			}
		}
		else if (c == '"')
		{
			inQuotes = true;
		}
		else if (c == ',')
		{
			fields.push_back(current);
			current.clear();
		}
		else
		{
			current += c;
		}
	}
	fields.push_back(current);
	return fields;
}

CsvTable CsvTable::parse(std::istream& input, const std::string& sourceName)
{
	CsvTable table;
	table.sourceName = sourceName;

	std::string line;
	if (!std::getline(input, line))
	{
		throw DataError(sourceName + ": file is empty");
	}
	if (line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
	{
		line.erase(0, 3); // UTF-8 byte order mark
	}
	std::vector<std::string> header = splitCsvLine(line);
	for (std::size_t i = 0; i < header.size(); i++)
	{
		table.columns.emplace(header[i], i);
	}

	while (std::getline(input, line))
	{
		if (line.empty() || line == "\r")
		{
			continue;
		}
		table.records.push_back(splitCsvLine(line));
	}
	return table;
}

std::size_t CsvTable::column(const std::string& name) const
{
	auto found = columns.find(name);
	if (found == columns.end())
	{
		throw DataError(sourceName + ": missing column '" + name + "'");
	}
	return found->second;
}

const std::string& CsvTable::field(const std::vector<std::string>& row, std::size_t columnIndex) const
{
	static const std::string empty;
	return columnIndex < row.size() ? row[columnIndex] : empty;
}
