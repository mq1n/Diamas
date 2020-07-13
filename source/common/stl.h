#pragma once
#include <vector>
#include <string>
#include <map>
#include <list>
#include <functional>
#include <stack>
#include <set>
#include <string>
#include <cstdint>
#include <algorithm>

#ifdef _WIN32
#pragma warning(push) 
#pragma warning(disable: 4242 4244)
#endif // _WIN32

inline void stl_lowers(std::string& rstRet)
{
	for (size_t i = 0; i < rstRet.length(); ++i)
		rstRet[i] = ::tolower(rstRet[i]);
}

#ifdef _WIN32
#pragma warning(push) 
#endif // _WIN32

static inline const char* whitespaces()
{
	return " \f\n\r\t\v";
}

static inline std::string& ltrim_in_place(std::string& str, const char* chars = whitespaces())
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

static inline std::string& rtrim_in_place(std::string& str, const char* chars = whitespaces())
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

static inline std::string ltrim(std::string str, const char* chars = whitespaces())
{
	return ltrim_in_place(str, chars);
}

static inline std::string rtrim(std::string str, const char* chars = whitespaces())
{
	return rtrim_in_place(str, chars);
}

static inline std::string& trim_in_place(std::string& str, const char* chars = whitespaces())
{
	return ltrim_in_place(rtrim_in_place(str, chars), chars);
}

static inline std::string trim(std::string str, const char* chars = whitespaces())
{
	return trim_in_place(str, chars);
}

inline std::vector <std::string> string_split(const std::string& str, char tok = ' ')
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

inline std::vector <std::string> string_split(const std::string& str, const std::string& tok = " ")
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

inline std::vector <std::string> string_split_trim(const std::string& str, const std::string& tok = " ")
{
	auto out = std::vector<std::string>();

	size_t prev = 0;
	auto cur = str.find(tok);
	while (cur != std::string::npos)
	{
		out.emplace_back(trim(str.substr(prev, cur - prev)));
		prev = cur + tok.size();
		cur = str.find(tok, prev);
	}

	out.emplace_back(trim(str.substr(prev, cur - prev)));
	return out;
}

inline std::vector <std::string> string_split_any(const std::string& str, const std::string& toks = " ")
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

inline void string_replace(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;

	std::size_t pos = 0;
	while ((pos = str.find(from, pos)) != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

inline bool CopyStringSafe(char* dst, const std::string& src, uint32_t size)
{
	if (size != 0) {
		auto cnt = std::min<uint32_t>(src.length(), size - 1);
		std::memcpy(dst, src.data(), cnt);
		dst[cnt] = '\0';
		return cnt != size - 1;
	}
	else {
		dst[0] = '\0';
		return true;
	}
}

template <std::size_t N>
inline bool CopyStringSafe(char(&dst)[N], const std::string& src)
{
	return CopyStringSafe(dst, src, N);
}

inline bool IsAlphaNumeric(const std::string& str)
{
	return std::all_of(str.begin(), str.end(), ::isalnum);
}

struct stringhash
{
	size_t operator () (const std::string& str) const
	{
		const uint8_t* s = reinterpret_cast<const uint8_t*>(str.c_str());
		const uint8_t* end = s + str.size();
		size_t h = 0;

		while (s < end)
		{
			h *= 16777619;
			h ^= *(s++);
		}

		return h;
	}
};

#if _HAS_CXX17
namespace std
{
	// FUNCTIONAL STUFF (from <functional>)
	// STRUCT TEMPLATE unary_function
	template <class _Arg, class _Result> struct unary_function
	{ // base class for unary functions
		typedef _Arg argument_type;
		typedef _Result result_type;
	};

	// STRUCT TEMPLATE binary_function
	template <class _Arg1, class _Arg2, class _Result> struct binary_function
	{ // base class for binary functions
		typedef _Arg1 first_argument_type;
		typedef _Arg2 second_argument_type;
		typedef _Result result_type;
	};
}
#endif /* _HAS_CXX17 */
