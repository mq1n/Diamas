#include "stdafx.h"

static struct timeval null_time = { 0, 0 };

#define ishprint(x)     ((((x) & 0xE0) > 0x90) || isprint(x))

void printdata(const unsigned char* data, int bytes)
{
	int                 i, j, k;
	const unsigned char* p;

	fprintf(stderr, "------------------------------------------------------------------\n");
	j = bytes;
	while (1)
	{
		k = j >= 16 ? 16 : j;

		p = data;
		for (i = 0; i < 16; i++)
		{
			if (i >= k)
				fprintf(stderr, "   ");
			else
				fprintf(stderr, "%02x ", *p);
			p++;
		}

		fprintf(stderr, "| ");

		p = data;
		for (i = 0; i < k; i++)
		{
			if (i >= k)
				fprintf(stderr, " ");
			else
			{
				fprintf(stderr, "%c", ishprint(*p) && ishprint(*(p + 1)) ? *p : '.');
			}
			p++;
		}

		fprintf(stderr, "\n");

		j -= 16;
		data += 16;

		if (j <= 0)
			break;
	}

	fprintf(stderr, "------------------------------------------------------------------\n");
}


struct timeval* timediff(const struct timeval* a, const struct timeval* b)
{
	static struct timeval rslt;

	if (a->tv_sec < b->tv_sec)
		return &null_time;
	else if (a->tv_sec == b->tv_sec)
	{
		if (a->tv_usec < b->tv_usec)
			return &null_time;
		else
		{
			rslt.tv_sec = 0;
			rslt.tv_usec = a->tv_usec - b->tv_usec;
			return &rslt;
		}
	}
	else
	{                      /* a->tv_sec > b->tv_sec */
		rslt.tv_sec = a->tv_sec - b->tv_sec;

		if (a->tv_usec < b->tv_usec)
		{
			rslt.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
			rslt.tv_sec--;
		}
		else
			rslt.tv_usec = a->tv_usec - b->tv_usec;

		return &rslt;
	}
}

struct timeval* timeadd(struct timeval* a, struct timeval* b)
{
	static struct timeval rslt;

	rslt.tv_sec = a->tv_sec + b->tv_sec;
	rslt.tv_usec = a->tv_usec + b->tv_usec;

	while (rslt.tv_usec >= 1000000)
	{
		rslt.tv_usec -= 1000000;
		rslt.tv_sec++;
	}

	return &rslt;
}


char* time_str(time_t ct)
{
	auto sttime = std::to_string(ct);
	return const_cast<char*>(sttime.c_str());
	/*
	static char * time_s;

	time_s = asctime(localtime(&ct));

	time_s[strlen(time_s) - 6] = '\0';
	time_s += 4;

	return (time_s);
	*/
}

void trim_and_lower(const char* src, char* dest, size_t dest_size)
{
	const char* tmp = src;
	size_t len = 0;

	if (!dest || dest_size == 0)
		return;

	if (!src)
	{
		*dest = '\0';
		return;
	}

	// 앞에 빈칸 건너 뛰기
	while (*tmp)
	{
		if (!isnhspace(*tmp))
			break;

		tmp++;
	}

	// \0 확보
	--dest_size;

	while (*tmp && len < dest_size)
	{
		*(dest++) = LOWER(*tmp); // LOWER는 매크로라 ++ 쓰면 안됨
		++tmp;
		++len;
	}

	*dest = '\0';

	if (len > 0)
	{
		// 뒤에 빈칸 지우기
		--dest;

		while (*dest && isnhspace(*dest) && len--)
			*(dest--) = '\0';
	}
}

void lower_string(const char* src, char* dest, size_t dest_size)
{
	const char* tmp = src;
	size_t len = 0;

	if (!dest || dest_size == 0)
		return;

	if (!src)
	{
		*dest = '\0';
		return;
	}

	// \0 확보
	--dest_size;

	while (*tmp && len < dest_size)
	{
		*(dest++) = LOWER(*tmp); // LOWER는 매크로라 ++ 쓰면 안됨
		++tmp;
		++len;
	}

	*dest = '\0';
}

char* str_dup(const char* source)
{
	char* new_line;

	CREATE(new_line, char, strlen(source) + 1);
	return (strcpy(new_line, source));
}

/* arg1 이 arg2 의 앞단과 같을 때 1 을 리턴한다. */
int is_abbrev(char* arg1, char* arg2)
{
	if (!*arg1)
		return 0;

	for (; *arg1 && *arg2; arg1++, arg2++)
		if (LOWER(*arg1) != LOWER(*arg2))
			return 0;

	if (!*arg1)
		return 1;

	return 0;
}

/* "Name : 비엽" 과 같이 "항목 : 값" 으로 이루어진 문자열에서
   항목을 token 으로, 값을 value 로 복사하여 리턴한다. */
void parse_token(char* src, char* token, char* value)
{
	char* tmp;

	for (tmp = src; *tmp && *tmp != ':'; tmp++)
	{

		if (isspace(*tmp))
			continue;

		*(token++) = LOWER(*tmp);
	}

	*token = '\0';

	for (tmp += 2; *tmp; tmp++)
	{
		if (*tmp == '\n' || *tmp == '\r')
			continue;

		*(value++) = *tmp;
	}

	*value = '\0';
}


struct tm* tm_calc(const struct tm* curr_tm, int days)
{
	char                yoon = 0;
	static struct tm    new_tm;
	int                 monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (!curr_tm)
	{
		time_t time_s = time(nullptr);
		new_tm = *localtime(&time_s);
	}
	else
		memcpy(&new_tm, curr_tm, sizeof(struct tm));

	if (new_tm.tm_mon == 1)
	{
		if (!((new_tm.tm_year + 1900) % 4))
		{
			if (!((new_tm.tm_year + 1900) % 100))
			{
				if (!((new_tm.tm_year + 1900) % 400))
					yoon = 1;
			}
			else
				yoon = 1;
		}

		if (yoon)
			new_tm.tm_mday += 1;
	}

	if (yoon)
		monthdays[1] = 29;
	else
		monthdays[1] = 28;

	new_tm.tm_mday += days;

	if (new_tm.tm_mday <= 0)
	{
		new_tm.tm_mon--;

		if (new_tm.tm_mon < 0)
		{
			new_tm.tm_year--;
			new_tm.tm_mon = 11;
		}

		new_tm.tm_mday = monthdays[new_tm.tm_mon] + new_tm.tm_mday;
	}
	else if (new_tm.tm_mday > monthdays[new_tm.tm_mon])
	{
		new_tm.tm_mon++;

		if (new_tm.tm_mon > 11)
		{
			new_tm.tm_year++;
			new_tm.tm_mon = 0;
		}

		new_tm.tm_mday = monthdays[new_tm.tm_mon] - new_tm.tm_mday;
	}

	return (&new_tm);
}
int MIN(int a, int b)
{
	return a < b ? a : b;
}

int MAX(int a, int b)
{
	return a > b ? a : b;
}

int MINMAX(int min, int value, int max)
{
	int tv = (min > value ? min : value);
	return (max < tv) ? max : tv;
}

uint32_t thecore_random()
{
	return static_cast<uint32_t>(number(
		std::numeric_limits<int>::min(),
		std::numeric_limits<int>::max())
		);
}

int number_ex(int from, int to, const char* file, int line)
{
	// We only need (and want) to initialize the mersenne twister generator once
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_int_distribution<int> dist; //not too expensive to create, though

	if (from > to)
	{
		auto tmp = from;

		sys_err("number(): first argument is bigger than second argument %d -> %d, %s %d", from, to, file, line);

		from = to;
		to = tmp;
	}

	//Set the range we'd like our distribution to be on, and generate the number
	return dist(mt, std::uniform_int_distribution<int>::param_type(from, to));
}

long long number_ex_64(long long from, long long to, const char* file, int line)
{
	// We only need (and want) to initialize the mersenne twister generator once
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_int_distribution<long long> dist; //not too expensive to create, though

	if (from > to)
	{
		auto tmp = from;

		sys_err("number(): first argument is bigger than second argument %lld -> %lld, %s %d", from, to, file, line);

		from = to;
		to = tmp;
	}

	//Set the range we'd like our distribution to be on, and generate the number
	return dist(mt, std::uniform_int_distribution<long long>::param_type(from, to));
}

float fnumber(float from, float to)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(from, to);
	return dis(gen);
}

void thecore_sleep(struct timeval* timeout)
{
	std::this_thread::sleep_for(std::chrono::microseconds(timeout->tv_sec * 1000000 + timeout->tv_usec));
}

void thecore_msleep(uint32_t dwMillisecond)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(dwMillisecond));
}

void core_dump_ex(const char* who, long line, const char* function)
{
	sys_err("*** Dumping Core %s:%d (%s) ***", who, line, function);

	fflush(stdout);
	fflush(stderr);
	std::this_thread::sleep_for(std::chrono::seconds(10));

#ifndef _WIN32
	fork();
#endif

	* ((volatile int*)NULL) = 0;
	exit(1);
}

#ifdef _WIN32
void gettimeofday(struct timeval* t, struct timezone* dummy)
{
	uint32_t millisec = GetTickCount();

	t->tv_sec = (millisec / 1000);
	t->tv_usec = (millisec % 1000) * 1000;
}
#endif

float get_float_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tv.tv_sec -= 1057699978;
	return ((float)tv.tv_sec + ((float)tv.tv_usec / 1000000.0f));
}

uint32_t get_unix_ms_time()
{
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
