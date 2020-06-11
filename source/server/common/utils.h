#pragma once
#include <string>
#include <algorithm>

/*----- atoi function -----*/
inline bool str_is_number(const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	auto len = strlen(in);
	for (uint32_t i = 0; i < len; ++i)
	{
		if ((in[i] < '0' || in[i] > '9') && (i > 0 || in[i] != '-'))
			return false;
	}

	return true;
}

inline bool str_to_number(bool& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = (strtol(in, nullptr, 10) != 0);
	return true;
}

inline bool str_to_bool(bool& out, const std::string& in)
{
	out = in.at(0) == '1';
	return true;
}

inline bool str_to_number(char& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<char>(strtol(in, nullptr, 10));
	return true;
}

inline bool str_to_number(uint8_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint8_t>(strtoul(in, nullptr, 10));
	return true;
}

inline bool str_to_number(int16_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int16_t>(strtol(in, nullptr, 10));
	return true;
}

inline bool str_to_number(uint16_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint16_t>(strtoul(in, nullptr, 10));
	return true;
}

inline bool str_to_number(int32_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int32_t>(strtol(in, nullptr, 10));
	return true;
}

inline bool str_to_number(uint32_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint32_t>(strtoul(in, nullptr, 10));
	return true;
}

inline bool str_to_number(int64_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<int64_t>(strtoull(in, nullptr, 10));
	return true;
}

inline bool str_to_number(uint64_t& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<uint64_t>(strtoull(in, nullptr, 10));
	return true;
}

inline bool str_to_number(float& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<float>(std::stof(in));
	return true;
}

inline bool str_to_number(double& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<double>(strtod(in, nullptr));
	return true;
}

inline bool str_to_float(float& out, const char* in)
{
	char* end;
	double ret = (double)strtod(in, &end);
	bool parsed = (*end == '\0');

	out = parsed ? (float)ret : 0.0f;
	return parsed;
}

inline bool str_to_double(double& out, const char* in)
{
	char* end;
	double ret = (double)strtod(in, &end);
	bool parsed = (*end == '\0');

	out = parsed ? ret : 0;
	return parsed;
}

inline bool str_to_number(long double& out, const char* in)
{
	if (0 == in || 0 == in[0])
		return false;

	out = static_cast<long double>(strtold(in, nullptr));
	return true;
}

//@source: http://stackoverflow.com/a/2845275/2205532
inline bool is_positive_number(const std::string& str)
{
	if (str.empty() || ((!isdigit(str[0])) && (str[0] != '-') && (str[0] != '+')))
		return false;

	char* p;
	strtol(str.c_str(), &p, 10);

	//Finally, check that the referenced pointer points to the end of the string. If that happens, said string is a number.
	return (*p == 0);
}
/*----- atoi function -----*/

//Encode an url with safe chars
inline std::string uri_encode(const std::string& sSrc)
{
	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const uint8_t* pSrc = (const uint8_t*)sSrc.c_str();
	const int32_t SRC_LEN = sSrc.length();
	uint8_t* const pStart = new uint8_t[SRC_LEN * 3];
	uint8_t* pEnd = pStart;
	const uint8_t* const SRC_END = pSrc + SRC_LEN;

	for (; pSrc < SRC_END; ++pSrc)
	{
		char c = *pSrc;
		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
			*pEnd++ = *pSrc;
		else
		{
			// escape this char
			*pEnd++ = '%';
			*pEnd++ = DEC2HEX[*pSrc >> 4];
			*pEnd++ = DEC2HEX[*pSrc & 0x0F];
		}
	}

	std::string sResult((char*)pStart, (char*)pEnd);
	delete[] pStart;
	return sResult;
}

int32_t uniform_random(int32_t a, int32_t b);
double uniform_random(double a, double b);

//Deletes the second map entry and clears the map when its done.
template <typename M> void FreeClear(M& mM)
{
	for (typename M::iterator it = mM.begin(); it != mM.end(); ++it) {
		delete it->second;
	}
	mM.clear();
}

template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

template <class container>
void wipe(container& a) {
	typename container::iterator first, past;

	first = a.begin();
	past = a.end();

	while (first != past)
		delete* (first++);

	a.clear();
}

#if !defined(__FreeBSD__) && !defined(_WIN32)
/* strlcpy based on OpenBSDs strlcpy */
#include <stdio.h>
#include <sys/types.h>

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char* dst, const char* src, size_t siz)
{
	char* d = dst;
	const char* s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';  /* NUL-terminate dst */
		while (*s++);
	}

	return(s - src - 1);  /* count does not include NUL */
}

#endif

