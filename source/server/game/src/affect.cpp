#include "stdafx.h"
#include "pool.h"
#include "affect.h"

#ifndef DEBUG_ALLOC
ObjectPool<CAffect> affect_pool;
#endif

CAffect* CAffect::Acquire()
{
#ifndef DEBUG_ALLOC
	return affect_pool.Construct();
#else
	return M2_NEW CAffect;
#endif
}

void CAffect::Release(CAffect* p)
{
#ifndef DEBUG_ALLOC
	affect_pool.Destroy(p);
#else
	M2_DELETE(p);
#endif
}

