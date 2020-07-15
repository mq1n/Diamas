#pragma once
#include "../../../common/singleton.h"
#include <nameof.hpp>

namespace net_engine
{	
	class NetPacketDispatcher 
	{
		using THandlerFunc 	 = std::function <void(std::shared_ptr <Packet> packet)>;
		using TPacketHandler = std::unordered_map <TNetOpcode, THandlerFunc>;

		public:
			NetPacketDispatcher() = default;
			virtual ~NetPacketDispatcher() = default;

			bool IsKnownPacketID(TNetOpcode header);
			bool RegisterPacketHandler(const std::string& name, TNetOpcode header, THandlerFunc handler, const std::string& from_func);
			bool DeregisterPacketHandler(TNetOpcode header);
			THandlerFunc GetPacketHandler(TNetOpcode header);
			void DispatchPacketHandler(TNetOpcode header, std::shared_ptr <Packet> packet);

		private:
			TPacketHandler m_handlers;
	};
};

#ifndef REGISTER_PACKET_HANDLER
	#define REGISTER_PACKET_HANDLER(header, handler) m_dispatcher.RegisterPacketHandler(NAMEOF(header).c_str(), header, handler, __FUNCTION__)
#endif
