#include "StdAfx.h"
#include "Thread.h"

CThread::CThread() : m_pArg(nullptr), m_hThread(nullptr), m_uThreadID(0)
{
}

bool CThread::Create(void * arg)
{
	Arg(arg);
	m_hThread = (HANDLE) _beginthreadex(nullptr, 0, EntryPoint, this, 0, &m_uThreadID);

	if (!m_hThread)
		return false;

	SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);
	return true;
}

uint32_t CThread::Run(void * arg)
{
	if (!Setup())
		return 0;

	return (Execute(arg));
}

/* Static */
uint32_t CALLBACK CThread::EntryPoint(void * pThis)
{
	CThread * pThread = (CThread *) pThis;
	return pThread->Run(pThread->Arg());
}