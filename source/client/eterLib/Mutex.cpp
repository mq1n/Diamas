#include "StdAfx.h"
#include "Mutex.h"

Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

void Mutex::Lock()
{
	lock.lock();
}

void Mutex::Unlock()
{
	lock.unlock();
}