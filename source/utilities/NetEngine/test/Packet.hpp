#pragma once
#include <NetPacket.hpp>
using namespace net_engine;

#if 0
enum ENetHeaders : uint8_t
{
    // CG
    HEADER_CG_LOGIN = 1,
    HEADER_CG_CHAT = 42,

    // GC
    HEADER_GC_CHAT = 69
};

struct SNetPacketCGLogin : SNetPacket
{
    char login[32]{};
    char password[32]{};

    NET_DECLARE_PACKET(HEADER_CG_LOGIN, SNetPacketCGLogin);
};

struct SNetPacketCGChat : SNetPacket
{
    char msg[256]{};

    NET_DECLARE_PACKET(HEADER_CG_CHAT, SNetPacketCGChat);
};

struct SNetPacketGCChat : SNetPacket
{
    char msg[256]{};

    NET_DECLARE_PACKET(HEADER_GC_CHAT, SNetPacketGCChat);
};
#endif
