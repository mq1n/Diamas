#include "StdAfx.h"

#include <cassert>

static uint32_t randseed = 1;

void srandom(uint32_t seed)
{
	randseed = seed;
}

/*
 * Pseudo-random number generator for randomizing the profiling clock,
 * and whatever else we might use it for.  The result is uniform on
 * [0, 2^31 - 1].
 */
uint32_t random()
{       
	/*
	* Compute x[n + 1] = (7^5 * x[n]) mod (2^31 - 1).
	* From "Random number generators: good ones are hard to find",
	* Park and Miller, Communications of the ACM, vol. 31, no. 10,
	* October 1988, p. 1195.
	*/
	int32_t x = randseed;
	int32_t hi = x / 127773;
	int32_t lo = x % 127773;
	int32_t t = 16807 * lo - 2836 * hi;
	if (t <= 0)
		t += 0x7fffffff;
	randseed = t;
	return (t);
}


float frandom(float flLow, float flHigh)
{
	float fl = float(random()) / float(2147483648.0f); // float in [0,1)
	return (fl * (flHigh - flLow)) + flLow; // float in [low,high)
}

int32_t random_range(int32_t from, int32_t to)
{
	assert(from <= to);
	return ((random() % (to - from + 1)) + from);
}

uint32_t GetRandomInt(__in uint32_t uMin, __in uint32_t uMax)
{
	if (uMax < (uint32_t)0xFFFFFFFF)
		uMax++;

	return (rand() % (uMax - uMin)) + uMin;
}

LPSTR GetRandomStringA(__in uint32_t uMin)
{
	LPSTR pString = (LPSTR)malloc(uMin + 64);

	while (strlen(pString) < uMin)
	{
		CHAR pSubString[32] = { 0x0, 0x0 };

		_itoa_s(GetRandomInt(1, 0xffffff), pSubString, 6, 0x10);
		strcat_s(pString, uMin + 64, pSubString);
	}

	return pString;
}

