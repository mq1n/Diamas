#include "../include/NetEngine.hpp"

namespace net_engine
{	
	bool NetPacketDispatcher::IsKnownPacketID(TNetOpcode header)
	{
		auto iter = m_handlers.find(header);
		if (iter != m_handlers.end())
		{
			return true;
		}
		return false;	
	}	
	bool NetPacketDispatcher::RegisterPacketHandler(const std::string& name, TNetOpcode header, THandlerFunc handler, const std::string& from_func)
	{ 
		if (!IsKnownPacketID(header))
		{
			if (handler)
				m_handlers.emplace(header, handler);
			return true;
		}
		return false;
	}
	bool NetPacketDispatcher::DeregisterPacketHandler(TNetOpcode header)
	{
		auto iter = m_handlers.find(header);
		if (iter != m_handlers.end())
		{
			m_handlers.erase(iter);
			return true;
		}
		return false;
	}
	NetPacketDispatcher::THandlerFunc NetPacketDispatcher::GetPacketHandler(TNetOpcode header)
	{
		auto iter = m_handlers.find(header);
		if (iter != m_handlers.end())
		{
			return iter->second;
		}
		return {};	
	}
	void NetPacketDispatcher::DispatchPacketHandler(TNetOpcode header, std::shared_ptr <Packet> packet)
	{
		auto iter = m_handlers.find(header);
		if (iter != m_handlers.end())
		{
			iter->second(packet);
		}
	}
};
