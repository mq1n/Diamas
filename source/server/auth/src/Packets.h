#pragma once

#define BEGIN_PACKET_DEF(name, type) typedef struct name {\
uint8_t header;\
static size_t size() { return sizeof(name); }\
name() { memset(this, 0, sizeof(name)); header = type; }
#define END_PACKET_DEF(name) } name;

enum {
    HWID_MAX_HASH_LEN = 120,
};

enum EPhase
{
	PHASE_CLOSE,
	PHASE_HANDSHAKE,
	PHASE_LOGIN,
	PHASE_SELECT,
	PHASE_LOADING,
	PHASE_GAME,
	PHASE_DEAD,

	PHASE_CLIENT_CONNECTING,
	PHASE_DBCLIENT,
	PHASE_P2P,
	PHASE_AUTH,
};


#pragma pack(push, 1)

enum EPacketHeaders
{
	HEADER_GD_AUTH_LOGIN = 100,
	HEADER_DG_AUTH_LOGIN = 122,
	
	HEADER_GC_LOGIN_FAILURE = 8,
	HEADER_GC_AUTH_SUCCESS = 112,
	
	HEADER_CG_LOGIN3 = 61,
	HEADER_CG_HACK = 57,

	HEADER_GC_PHASE = 253,
	HEADER_GD_SETUP = 255,
};

BEGIN_PACKET_DEF(SPacketGDSetup, HEADER_GD_SETUP)
	uint32_t handle;
	uint32_t packetSize;
	char publicIp[16];
	uint8_t channel;
	uint16_t mainPort;
	uint16_t p2pPort;
	int32_t maps[32];
	uint32_t loginCount;
	bool authServer;
	bool slaveAuth;
END_PACKET_DEF(TPacketGDSetup)

BEGIN_PACKET_DEF(SPacketGDAuthLogin, HEADER_GD_AUTH_LOGIN)
	uint32_t handle;
	uint32_t packetSize;
	int64_t id;
	uint32_t loginKey;
	char login[30 + 1];
	char socialId[18 + 1];
	uint8_t clientKey[16];
	char lang[3 + 1];
	int32_t premiumTimes[9];
#if defined(AUTH_USE_HWID)
	char hwid[HWID_MAX_HASH_LEN + 1];
#endif
END_PACKET_DEF(TPacketGDAuthLogin)

BEGIN_PACKET_DEF(SPacketDGAuthLogin, HEADER_DG_AUTH_LOGIN)
	uint32_t handle;
	uint32_t packetSize;
	uint8_t result;
END_PACKET_DEF(TPacketDGAuthLogin)

BEGIN_PACKET_DEF(SPacketGCPhase, HEADER_GC_PHASE)
	uint8_t phase;
END_PACKET_DEF(TPacketGCPhase)

BEGIN_PACKET_DEF(SPacketGCLoginFailure, HEADER_GC_LOGIN_FAILURE)
	char status[8 + 1];
END_PACKET_DEF(TPacketGCLoginFailure)

BEGIN_PACKET_DEF(SPacketGCAuthSuccess, HEADER_GC_AUTH_SUCCESS)
	uint32_t loginKey;
	uint8_t result;
END_PACKET_DEF(TPacketGCAuthSuccess)

BEGIN_PACKET_DEF(SPacketCGLogin3, HEADER_CG_LOGIN3)
	char login[30 + 1];
	char password[16 + 1];
	uint8_t clientKey[16];
	char timestamp[32 + 1];
#if defined(AUTH_USE_HWID)
	char hwid[64 + 1];
#endif
	char	lang[4 + 1];
END_PACKET_DEF(TPacketCGLogin3)

BEGIN_PACKET_DEF(SPacketCGHack, HEADER_CG_HACK)
	char	message[255 + 1];
	char	szInfo[255 + 1];
END_PACKET_DEF(TPacketCGHack)

#pragma pack(pop)