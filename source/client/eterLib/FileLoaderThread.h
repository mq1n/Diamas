#ifndef __INC_YMIR_ETERLIB_FILELOADERTHREAD_H__
#define __INC_YMIR_ETERLIB_FILELOADERTHREAD_H__

#include <deque>
#include "Thread.h"
#include "Mutex.h"
#include <FileSystemIncl.hpp>

class CFileLoaderThread 
{
	public:
		typedef struct SData
		{
			std::string	stFileName;

			CFile	File;
			LPVOID		pvBuf;
			uint32_t		dwSize;
		} TData;

	public:
		CFileLoaderThread();
		~CFileLoaderThread();

		bool Create(void * arg);
	
	public:
		void	Request(std::string & c_rstFileName);
		bool	Fetch(TData ** ppData);
		void	Shutdown();

	protected:
		static uint32_t CALLBACK	EntryPoint(void * pThis);
		uint32_t					Run(void * arg);

		void *					Arg() const		{ return m_pArg; }
		void					Arg(void * arg) { m_pArg = arg; }
		
		HANDLE					m_hThread;

	private:
		void *					m_pArg;
		uint32_t				m_uThreadID;

	protected:
		uint32_t					Setup();
		uint32_t					Execute(void * pvArg);
		void					Destroy();
		void					Process();

	private:
		std::deque<TData *>		m_pRequestDeque;
		Mutex					m_RequestMutex;

		std::deque<TData *>		m_pCompleteDeque;
		Mutex					m_CompleteMutex;

		HANDLE					m_hSemaphore;
		int32_t						m_iRestSemCount;
		bool					m_bShutdowned;
};

#endif