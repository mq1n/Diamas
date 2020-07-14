#pragma once

#define BEGIN_PACKET_DEF(name, type) typedef struct name {\
uint8_t header;\
static size_t size() { return sizeof(name); }\
name() { memset(this, 0, sizeof(name)); header = type; }
#define END_PACKET_DEF(name) } name;

enum {
    HWID_MAX_HASH_LEN = 120,
};


#pragma pack(push, 1)
/*
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
	char hwid[HWID_MAX_HASH_LEN + 1];
END_PACKET_DEF(TPacketGDAuthLogin)

*/
BEGIN_PACKET_DEF(SPacketDGAuthLogin, HEADER_DG_AUTH_LOGIN)
	uint32_t handle;
	uint32_t packetSize;
	uint8_t result;
END_PACKET_DEF(TPacketDGAuthLogin)
#pragma pack(pop)
