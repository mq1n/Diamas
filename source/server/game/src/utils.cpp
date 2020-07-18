#include "stdafx.h"

static time_t global_time_gap = 0;

time_t get_unix_time()
{
	return time(nullptr) + global_time_gap;
}

void set_global_time(time_t t)
{
	global_time_gap = t - time(nullptr);
}

int32_t dice(int32_t number, int32_t size)
{
	int32_t sum = 0, val;

	if (size <= 0 || number <= 0)
		return (0);

	while (number)
	{
		val = ((thecore_random() % size) + 1);
		sum += val;
		--number;
	}

	return (sum);
}

size_t str_lower(const char * src, char * dest, size_t dest_size)
{
	size_t len = 0;

	if (!dest || dest_size == 0)
		return len;

	if (!src)
	{
		*dest = '\0';
		return len;
	}

	// \0 자리 확보
	--dest_size;

	while (*src && len < dest_size)
	{
		*dest = LOWER(*src); // LOWER 매크로에서 ++나 --하면 안됨!!

		++src;
		++dest;
		++len;
	}

	*dest = '\0';
	return len;
}

bool IsEqualStr(std::string rhs, std::string lhs)
{
	std::transform(rhs.begin(), rhs.end(), rhs.begin(), ::tolower);
	std::transform(lhs.begin(), lhs.end(), lhs.begin(), ::tolower);
	return (lhs == rhs);
}

void skip_spaces(const char **string)
{   
	for (; **string != '\0' && isspace(**string); ++(*string));
}

const char *one_argument(const char *argument, char *first_arg, size_t first_size)
{
	char mark = 0;
	size_t first_len = 0;

	if (!argument || 0 == first_size)
	{
		sys_err("one_argument received a nullptr pointer!");               
		*first_arg = '\0';
		return nullptr;    
	} 

	// \0 자리 확보
	--first_size;

	skip_spaces(&argument);

	while (*argument && first_len < first_size)
	{ 
		if (*argument == '\"')
		{
			mark = !mark;
			++argument; 
			continue;   
		}

		if (!mark && isspace(*argument))
			break;

		*(first_arg++) = *argument;
		++argument;     
		++first_len;
	} 

	*first_arg = '\0';

	skip_spaces(&argument);
	return (argument);
}

const char *two_arguments(const char *argument, char *first_arg, size_t first_size, char *second_arg, size_t second_size)
{
	return (one_argument(one_argument(argument, first_arg, first_size), second_arg, second_size));
}

const char *first_cmd(const char *argument, char *first_arg, size_t first_arg_size, size_t *first_arg_len_result)
{           
	size_t cur_len = 0;
	skip_spaces(&argument);

	// \0 자리 확보
	first_arg_size -= 1;

	while (*argument && !isspace(*argument) && cur_len < first_arg_size)
	{
		*(first_arg++) = LOWER(*argument);
		++argument;
		++cur_len;
	}

	*first_arg_len_result = cur_len;
	*first_arg = '\0';
	return (argument);
}

void split_argument(const char *argument, std::vector<std::string> & vecArgs)
{
	std::string arg = argument;
	vecArgs = string_split(arg, " ");
}

int32_t CalculateDuration(int32_t speed, int32_t duration)
{
	assert(speed != 0 && "Divide by zero");
	return duration * 100 / speed;
}

double uniform_random(double a, double b)
{
	// Use random_device to generate a seed for Mersenne twister engine.
	std::random_device rd;

	// Use Mersenne twister engine to generate pseudo-random numbers.
	std::mt19937 engine(rd());

	// Establish the limit of [a, b) to the random generator
	std::uniform_real_distribution<double> dist(a, b);

	return dist(engine);
}

int32_t uniform_random(int32_t a, int32_t b)
{
	std::random_device rd;
	std::mt19937 engine(rd());

	// Set the random interval to [a, b]
	std::uniform_int_distribution<int32_t> dist(a, b);

	return dist(engine);
}
double gauss_random(double avg, double sigma)
{
	static bool haveNextGaussian = false;
	static double nextGaussian = 0.0f;

	if (haveNextGaussian) 
	{
		haveNextGaussian = false;
		return nextGaussian * sigma + avg;
	} 
	else 
	{
		double v1, v2, s;
		do { 
			v1 = uniform_random(-1.0, 1.0);
			v2 = uniform_random(-1.0, 1.0);
			s = v1 * v1 + v2 * v2;
		} while (s >= 1.f || fabs(s) < FLT_EPSILON);
		double multiplier = sqrt(-2 * log(s)/s);
		nextGaussian = v2 * multiplier;
		haveNextGaussian = true;
		return v1 * multiplier * sigma + avg;
	}
}

int32_t parse_time_str(const char* str)
{
	int32_t tmp = 0;
	int32_t secs = 0;

	while (*str != 0)
	{
		switch (*str)
		{
			case 'm':
			case 'M':
				secs += tmp * 60;
				tmp = 0;
				break;

			case 'h':
			case 'H':
				secs += tmp * 3600;
				tmp = 0;
				break;

			case 'd':
			case 'D':
				secs += tmp * 86400;
				tmp = 0;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				tmp *= 10;
				tmp += (*str) - '0';
				break;

			case 's':
			case 'S':
				secs += tmp;
				tmp = 0;
				break;
			default:
				return -1;
		}
		++str;
	}

	return secs + tmp;
}

bool WildCaseCmp(const char *w, const char *s)
{
	for (;;)
	{
		switch(*w)
		{
			case '*':
				if (!w[1])
					return true;
				{
					size_t sLength = strlen(s);
					for (size_t i = 0; i <= sLength; ++i)
					{
						if (true == WildCaseCmp(w + 1, s + i))
							return true;
					}
				}
				return false;

			case '?':
				if (!*s)
					return false;

				++w;
				++s;
				break;

			default:
				if (*w != *s)
				{
					if (tolower(*w) != tolower(*s))
						return false;
				}

				if (!*w)
					return true;

				++w;
				++s;
				break;
		}
	}

	return false;
}

bool is_digits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}
