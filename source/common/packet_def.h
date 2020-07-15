#pragma once
#include <cstdint>
#include <memory>
#include <iostream>
#include <fmt/format.h>

using TPacketHeader = uint8_t;

#pragma pack(1)
struct SPacketHeader
{
	TPacketHeader header{ 0 };
	explicit SPacketHeader(TPacketHeader header_)
	{
		header = header_;
	}

	// REMOVEME
	void operator=(TPacketHeader header_)
	{
		header = header_;
	}
};

struct SNetPacket
{
	SPacketHeader header{ 0 };

	explicit SNetPacket(TPacketHeader header_)
	{
//		std::cout << fmt::format("Packet: %u(0x%x) has been declared", header_, header_) << std::endl;
		header = header_;
	}
	explicit SNetPacket(const char* buf)
	{
		std::memcpy(&header, buf, sizeof(SPacketHeader));
	}

	const char* data() const
	{
		return reinterpret_cast<const char*>(this);
	}

	const char* body() const
	{
		return reinterpret_cast<const char*>(this) + sizeof(SPacketHeader);
	}
};
#pragma pack()

constexpr auto packet_header_size = sizeof(TPacketHeader);

#define NET_DECLARE_PACKET(header, context)\
	context() : SNetPacket(header){}
