#pragma once
#include <string>
#include <vector>

static std::vector <std::string> string_split(const std::string& str, char tok = ' ')
{
	auto out = std::vector<std::string>();

	std::size_t prev = 0;
	auto cur = str.find(tok);
	while (cur != std::string::npos)
	{
		out.emplace_back(str.substr(prev, cur - prev));
		prev = cur + 1;
		cur = str.find(tok, prev);
	}
	out.emplace_back(str.substr(prev, cur - prev));
	return out;
}

static std::vector <std::string> string_split(const std::string& str, const std::string& tok = " ")
{
	auto out = std::vector<std::string>();

	std::size_t prev = 0;
	auto cur = str.find(tok);
	while (cur != std::string::npos)
	{
		out.emplace_back(str.substr(prev, cur - prev));
		prev = cur + tok.size();
		cur = str.find(tok, prev);
	}
	out.emplace_back(str.substr(prev, cur - prev));
	return out;
}

static std::vector <std::string> string_split_any(const std::string& str, const std::string& toks = " ")
{
	auto out = std::vector<std::string>();

	std::size_t prev = 0;
	auto cur = str.find_first_of(toks);
	while (cur != std::string::npos)
	{
		out.emplace_back(str.substr(prev, cur - prev));
		prev = cur + 1;
		cur = str.find_first_of(toks, prev);
	}
	out.emplace_back(str.substr(prev, cur - prev));
	return out;
}

static inline const char * whitespaces()
{
	return " \f\n\r\t\v";
}

static inline std::string & ltrim_in_place(std::string & str, const char * chars = whitespaces())
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

static inline std::string & rtrim_in_place(std::string & str, const char * chars = whitespaces())
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

static inline std::string ltrim(std::string str, const char * chars = whitespaces())
{
	return ltrim_in_place(str, chars);
}

static inline std::string rtrim(std::string str, const char * chars = whitespaces())
{
	return rtrim_in_place(str, chars);
}

static inline std::string & trim_in_place(std::string & str, const char * chars = whitespaces())
{
	return ltrim_in_place(rtrim_in_place(str, chars), chars);
}

static inline std::string trim(std::string str, const char * chars = whitespaces())
{
	return trim_in_place(str, chars);
}
