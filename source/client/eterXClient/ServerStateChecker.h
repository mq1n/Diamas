#pragma once

#include "../eterLib/NetStream.h"

class CServerStateChecker : public CSingleton<CServerStateChecker>
{
	public:
		CServerStateChecker();
		virtual ~CServerStateChecker();

		void Create(PyObject* poWnd);
		void AddChannel(uint32_t uServerIndex, const char* c_szAddr, uint32_t uPort);
		void Request();
		void Update();

		void Initialize();
		
	private:
		typedef struct SChannel
		{
			uint32_t uServerIndex;
			const char* c_szAddr;
			uint32_t uPort;
		} TChannel;

		PyObject* m_poWnd;
		
		std::list<TChannel> m_lstChannel;

		CNetworkStream m_kStream;
};