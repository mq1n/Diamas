#include "../include/random.h"

// DELETEME Switch to effolkronium random

std::mt19937& GetRand()
{
	static std::mt19937 rnd;
	return rnd;
}
