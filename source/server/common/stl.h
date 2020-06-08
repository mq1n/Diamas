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

inline std::vector <std::string> string_split(const std::string& str, const std::string& tok = " ")
{
	std::vector <std::string> vec;

	std::size_t prev = 0;
	auto cur = str.find(tok);
	while (cur != std::string::npos)
	{
		vec.emplace_back(str.substr(prev, cur - prev));
		prev = cur + tok.size();
		cur = str.find(tok, prev);
	}

	vec.emplace_back(str.substr(prev, cur - prev));
	return vec;
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

struct stringhash
{
	size_t operator () (const std::string & str) const
	{
		const uint8_t * s = (const uint8_t*) str.c_str();
		const uint8_t * end = s + str.size();
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
