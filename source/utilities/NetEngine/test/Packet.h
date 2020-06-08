#pragma once
#include <NetPacket.h>
using namespace net_engine;

enum ENetHeaders
{
    HEADER_GC_LOGIN = 1,
    HEADER_GC_CHAT = 2,
};

struct SNetPacketGCLogin : SNetPacket
{
    char login[32];
    char password[32];

    NET_DECLARE_PACKET(HEADER_GC_LOGIN, SNetPacketGCLogin);
};

struct SNetPacketGCChat : SNetPacket
{
    char msg[256];

    NET_DECLARE_PACKET(HEADER_GC_CHAT, SNetPacketGCChat);
};
