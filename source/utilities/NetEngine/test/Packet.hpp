#pragma once
#include <NetPacket.hpp>
using namespace net_engine;

enum ENetHeaders : uint8_t
{
    // CG
    HEADER_CG_LOGIN = 1,
    HEADER_CG_CHAT = 42,

    // GC
    HEADER_GC_CHAT = 69
};

enum ENetPhases : uint8_t
{
    PHASE_HANDSHAKE = 1,
    PHASE_LOGIN = 2,
    PHASE_SELECT = 3,
    PHASE_LOADING = 4,
    PHASE_GAME = 5,
    PHASE_AUTH = 10,
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
