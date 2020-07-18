#pragma once
#include <cmath>
#include <sstream>
#include <iostream>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <random>
#include <cctype>

#define IS_SET(flag, bit)		((flag) & (bit))
#define SET_BIT(var, bit)		((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) = (var) ^ (bit))

inline bool SIIsNumber(const char* str)
{
	int32_t i;

	if (!str || !*str)
		return 0;

	for (i = 0; str[i]; ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}

	return true;
}

inline bool SIIsNumber(const char* str, int32_t Len)
{
	int32_t i;

	if (!str || !*str)
		return 0;

	for (i = 0; str[i] && i < Len; ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}

	return true;
}

inline float DISTANCE_SQRT(int64_t dx, int64_t dy)
{
	return sqrt(static_cast<float>(dx) * dx + static_cast<float>(dy) * dy);
}

inline float DISTANCE(int64_t x, int64_t y, int64_t dx, int64_t dy)
{
	return DISTANCE_SQRT(x - dx, y - dy);
}

inline int32_t DISTANCE_APPROX(int32_t dx, int32_t dy)
{
	int32_t min, max;

	if (dx < 0)
		dx = -dx;

	if (dy < 0)
		dy = -dy;

	if (dx < dy)
	{
		min = dx;
		max = dy;
	}
	else
	{
		min = dy;
		max = dx;
	}

    // coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
    return ((( max << 8 ) + ( max << 3 ) - ( max << 4 ) - ( max << 1 ) +
		( min << 7 ) - ( min << 5 ) + ( min << 3 ) - ( min << 1 )) >> 8 );
}

#ifndef _WIN32
inline uint16_t MAKEWORD(uint8_t a, uint8_t b)
{
	return static_cast<uint16_t>(a) | (static_cast<uint16_t>(b) << 8);
}
#endif

extern void set_global_time(time_t t);
extern time_t get_unix_time();

extern int32_t	dice(int32_t number, int32_t size);
extern size_t str_lower(const char * src, char * dest, size_t dest_size);

extern bool IsEqualStr(std::string rhs, std::string lhs);

extern void	skip_spaces(char **string);

extern const char *	one_argument(const char *argument, char *first_arg, size_t first_size);
extern const char *	two_arguments(const char *argument, char *first_arg, size_t first_size, char *second_arg, size_t second_size);
extern const char *	first_cmd(const char *argument, char *first_arg, size_t first_arg_size, size_t *first_arg_len_result);
extern void split_argument(const char *argument, std::vector<std::string> & vecArgs);

extern int32_t CalculateDuration(int32_t iSpd, int32_t iDur);

extern double gauss_random(double avg = 0, double sigma = 1);

extern int32_t parse_time_str(const char* str);

extern bool WildCaseCmp(const char *w, const char *s);

extern bool is_digits(const std::string &str);

template<class T>
inline std::string pretty_number(T value) 
{
	struct custom_numpunct : std::numpunct < char >
	{
	protected:
		virtual char do_thousands_sep() const { return '.'; }
		virtual std::string do_grouping() const { return "\03"; }
	};

	std::stringstream ss;
	ss.imbue(std::locale(std::cout.getloc(), new custom_numpunct));
	ss << std::fixed << value;
	return ss.str();
}

inline bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

namespace ext
{
	template<class InputIterator, class Predicate, class Function>
	void for_each_if(InputIterator begin, InputIterator end, Predicate pred, Function f) 
	{
		for (begin != end; ++begin; ) 
		{
			if (pred(*begin)) 
			{
				f(*begin);
			}
		}
	}
}
