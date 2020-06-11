#include "StdAfx.h"
#include <FileSystemIncl.hpp>
#include "FileLoaderThread.h"
#include "ResourceManager.h"
#include "../eterBase/Stl.h"

CFileLoaderThread::CFileLoaderThread() : m_bShutdowned(false), m_pArg(nullptr), m_hThread(nullptr), m_uThreadID(0)
{
}

CFileLoaderThread::~CFileLoaderThread()
{
	Destroy();
}

int32_t CFileLoaderThread::Create(void * arg)
{
	Arg(arg);
	m_hThread = (HANDLE) _beginthreadex(nullptr, 0, EntryPoint, this, 0, &m_uThreadID);

	if (!m_hThread)
		return false;

	SetThreadPriority(m_hThread, THREAD_PRIORITY_NORMAL);
	return true;
}

uint32_t CFileLoaderThread::Run(void * arg)
{
	if (!Setup())
		return 0;

	return (Execute(arg));
}

/* Static */
uint32_t CALLBACK CFileLoaderThread::EntryPoint(void * pThis)
{
	CFileLoaderThread * pThread = (CFileLoaderThread *) pThis;
	return pThread->Run(pThread->Arg());
}

//////////////////////////////////////////////////////////////////////////
void CFileLoaderThread::Destroy()
{
	if (m_hSemaphore)
	{
		CloseHandle(m_hSemaphore);
		m_hSemaphore = nullptr;
	}

	stl_wipe(m_pRequestDeque);
	stl_wipe(m_pCompleteDeque);
}

uint32_t CFileLoaderThread::Setup()
{
	m_hSemaphore = CreateSemaphore(nullptr,		// no security attributes
								   0,			// initial count
								   65535,		// maximum count
								   nullptr);		// unnamed semaphore
	if (!m_hSemaphore)
		return 0;

	return 1;
}

void CFileLoaderThread::Shutdown()
{
	if (!m_hSemaphore)
		return;

	BOOL bRet;
	
	m_bShutdowned = true;

	do
	{
		bRet = ReleaseSemaphore(m_hSemaphore, 1, nullptr);
	}
	while (!bRet);

	WaitForSingleObject(m_hThread, 10000);	// �����尡 ���� �Ǳ⸦ 10�� ��ٸ�
}

uint32_t CFileLoaderThread::Execute(void * /*pvArg*/)
{
	while (!m_bShutdowned)
	{
		uint32_t dwWaitResult; 

		dwWaitResult = WaitForSingleObject(m_hSemaphore, INFINITE);

		if (m_bShutdowned)
			break;

		switch (dwWaitResult) 
		{ 
			case WAIT_OBJECT_0:
				{
					Process();
				}
				break;

			case WAIT_TIMEOUT:
				TraceError("CFileLoaderThread::Execute: Timeout occured while time-out interval is INIFITE");
				break;
		}
	}

	Destroy();
	return 1;
}

void CFileLoaderThread::Request(std::string & c_rstFileName)	// called in main thread
{
	TData * pData = new TData;

	pData->dwSize = 0;
	pData->pvBuf = nullptr;
	pData->stFileName = c_rstFileName;

	m_RequestMutex.Lock();
	m_pRequestDeque.push_back(pData);
	m_RequestMutex.Unlock();

	++m_iRestSemCount;

	if (!ReleaseSemaphore(m_hSemaphore, m_iRestSemCount, nullptr))
		TraceError("CFileLoaderThread::Request: ReleaseSemaphore error");

	--m_iRestSemCount;
}

bool CFileLoaderThread::Fetch(TData ** ppData)	// called in main thread
{
	m_CompleteMutex.Lock();

	if (m_pCompleteDeque.empty())
	{
		m_CompleteMutex.Unlock();
		return false;
	}

	*ppData = m_pCompleteDeque.front();
	m_pCompleteDeque.pop_front();

	m_CompleteMutex.Unlock();
	return true;
}

void CFileLoaderThread::Process()	// called in loader thread
{
	m_RequestMutex.Lock();

	if (m_pRequestDeque.empty())
	{
		m_RequestMutex.Unlock();
		return;
	}

	TData * pData = m_pRequestDeque.front();
	m_pRequestDeque.pop_front();

	m_RequestMutex.Unlock();

	LPCVOID pvBuf;

	if (FileSystemManager::Instance().OpenFile(pData->stFileName, pData->File))
	{
		pData->dwSize = pData->File.GetSize();
		pData->pvBuf = new char[pData->dwSize];
		memcpy(pData->pvBuf, pData->File.GetData(), pData->dwSize);
	}

	m_CompleteMutex.Lock();
	m_pCompleteDeque.push_back(pData);
	m_CompleteMutex.Unlock();

	Sleep(g_iLoadingDelayTime);
}
