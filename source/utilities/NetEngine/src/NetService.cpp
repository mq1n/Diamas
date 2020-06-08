#include "../include/NetEngine.h"

namespace net_engine
{
	size_t NetServiceBase::Run()
	{
		return m_service.run();
	}

	size_t NetServiceBase::Poll()
	{
		return m_service.poll();
	}

	void NetServiceBase::Stop()
	{
		m_service.stop();
	}

	bool NetServiceBase::HasStopped()
	{
		return m_service.stopped();
	}


	asio::io_context& NetServiceBase::operator()()
	{
		return m_service;
	}
}
