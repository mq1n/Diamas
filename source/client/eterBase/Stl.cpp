#include "StdAfx.h"
#include "Stl.h"

static std::list<std::string> s_stList;

char ascii_tolower(const char c)
{
	char ret = c;
	if (c >= 'A' && c <= 'Z')
		ret = c - 'A' + 'a';

	return ret;
}

std::string& stl_static_string(const char * c_sz)
{
	std::string str;
	str.assign(c_sz);

	s_stList.emplace_back(str);
	return s_stList.back();
}

int32_t split_string(const std::string& input, const std::string& delimiter, std::vector<std::string>& results, bool includeEmpties)
{
	int32_t iPos = 0;
	uint32_t sizeS2 = delimiter.size();
	uint32_t isize = input.size();

	if ((isize == 0) || (sizeS2 == 0))
		return 0;

	std::vector<int32_t> positions;

	int32_t newPos = input.find(delimiter, 0);

	if (newPos < 0)
		return 0;

	int32_t numFound = 0;

	while (newPos >= iPos)
	{
		numFound++;
		positions.emplace_back(newPos);
		iPos = newPos;
		newPos = input.find(delimiter, iPos+sizeS2);
	}

	if (numFound == 0)
		return 0;

	for (uint32_t i = 0; i <= positions.size(); ++i)
	{
		std::string s;
		if (i == 0)
			s = input.substr(i, positions[i]);
		else
		{
			uint32_t offset = positions[i - 1] + sizeS2;

			if (offset < isize)
			{
				if (i == positions.size())
					s = input.substr(offset);
				else if (i > 0)
					s = input.substr(positions[i - 1] + sizeS2, positions[i] - positions[i - 1] - sizeS2);
			}
		}
		if (includeEmpties || !s.empty())
			results.emplace_back(s);
	}
	return numFound;
}
