#pragma once
#include <cstdint>
#include <unordered_map>
#include "NetConstants.h"

namespace net_engine
{
	static auto gs_packets = std::unordered_map <uint32_t, std::string>();
	
	static void AppendPacket(uint32_t header, const std::string& name)
	{
		gs_packets.emplace(header, name);
	}
	static std::string GetPacketName(uint32_t header)
	{
		auto it = gs_packets.find(header);
		if (it != gs_packets.end())
			return it->second;
		return "";
	}
	static std::string GetPacketName(const char* buf)
	{
		return GetPacketName(*(uint32_t*)buf);
	}

#pragma pack(1)
	struct SPacketHeader
	{
		uint32_t opcode;
		uint32_t version;
		uint32_t flags;
		uint32_t size;

		SPacketHeader() : 
			version(0), flags(0), opcode(0), size(0)
		{
			// default ctor
		}
		explicit SPacketHeader(uint32_t version_, uint32_t flags_, uint32_t opcode_, uint32_t size_)
		{
			version = version_;
			flags = flags_;
			opcode = opcode_;
			size = size_;
		}
		explicit SPacketHeader(uint32_t opcode_)
		{
			version = PACKET_VERSION;
			flags = PACKET_FLAG_RAW;
			size = 0;
			opcode = opcode_;
		}
		explicit SPacketHeader(const char* buf_)
		{
			version = PACKET_VERSION;
			flags = PACKET_FLAG_RAW;
			size = 0;
			opcode = *(uint32_t*)buf_;
		}
	};

	struct SNetPacket
	{
		SPacketHeader m_header;

		explicit SNetPacket(uint32_t header_) :
			m_header(header_)
		{
			// ctor
		}
		explicit SNetPacket(const char* buf)
		{
			// copy
			memcpy(&m_header, buf, sizeof(m_header));
		}

		const char* data() const
		{
			return reinterpret_cast<const char*>(this);
		}

		size_t size() const
		{
			return m_header.size;
		}
		void set_size(size_t len_)
		{
			m_header.size = len_;
		}

		const char* body() const
		{
			return reinterpret_cast<const char*>(this) + sizeof(SNetPacket);
		}
	};
#pragma pack()

	constexpr auto packet_header_min_size = sizeof(SPacketHeader);
}

#define NET_DECLARE_PACKET(header, context)\
	context() : SNetPacket(header)\
	{\
		set_size(sizeof(context));\
	}
