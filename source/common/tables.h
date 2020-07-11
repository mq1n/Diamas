#pragma once
#include <cstdint>
#include "defines.h"
#include "constants.h"
#include "item_data.h"

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
    uint8_t Type;
    uint8_t Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
    uint8_t        bType;
    int16_t       sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
    uint32_t       vnum;
    uint8_t        count;
    uint32_t		flags;
    uint32_t		anti_flags;
    int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TItemData;

typedef struct packet_shop_item
{
    uint32_t       vnum;
    uint32_t       price;
    uint8_t        count;
    uint8_t		display_pos;
    int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TShopItemData;

typedef struct SChestDropInfoTable
{
    uint8_t	bPageIndex;
    uint8_t	bSlotIndex;
    uint32_t	dwItemVnum;
    uint8_t	bItemCount;
} TChestDropInfoTable;

typedef struct
{
    uint32_t       dwID;
    int32_t        x, y;
    int32_t        width, height;
    uint32_t       dwGuildID;
} TLandPacketElement;

typedef struct TPacketAffectElement
{
    uint32_t	dwType;
    uint8_t	bApplyOn;
    int32_t	lApplyValue;
    uint32_t	dwFlag;
    int32_t	lDuration;
    int32_t	lSPCost;
} TPacketAffectElement;

struct TNPCPosition
{
    uint8_t bType;
    char name[CHARACTER_NAME_MAX_LEN + 1];
    int32_t x;
    int32_t y;
};

struct TMaterial
{
    uint32_t vnum;
    uint32_t count;
};

typedef struct SRefineTable
{
    uint32_t id;
    uint32_t src_vnum;
    uint32_t result_vnum;
    uint8_t material_count;
    int32_t cost; // 소요 비용
    int32_t prob; // 확률
    TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SEquipmentItemSet
{
    uint32_t   vnum;
    uint8_t    count;
    int32_t    alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
} TEquipmentItemSet;

typedef struct SShopItemTable
{
    uint32_t		vnum;
    uint8_t		count;

    TItemPos	pos;			// PC 상점에만 이용
    uint32_t		price;			// PC 상점에만 이용
    uint8_t		display_pos;	//	PC 상점에만 이용, 보일 위치.
} TShopItemTable;

typedef struct SShopTable
{
    uint32_t		dwVnum;
    uint32_t		dwNPCVnum;

    uint8_t		byItemCount;
    TShopItemTable	items[SHOP_HOST_ITEM_MAX_NUM];
} TShopTable;

typedef struct SPlayerSkill
{
    uint8_t bMasterType;
    uint8_t bLevel;
    uint32_t tNextRead;
} TPlayerSkill;

typedef struct SRequestChargeCash
{
    uint32_t		dwAID;		// id(primary key) - Account Table
    uint32_t		dwAmount;
    ERequestChargeType	eChargeType;
} TRequestChargeCash;

typedef struct SSimplePlayerInformation
{
    uint32_t               dwID;
    char                szName[CHARACTER_NAME_MAX_LEN + 1];
    uint8_t                byJob;
    uint8_t                byLevel;
    uint32_t               dwPlayMinutes;
    uint8_t                byST, byHT, byDX, byIQ;
    //	uint16_t				wParts[CRaceData::PART_MAX_NUM];
    uint16_t                wMainPart;
    uint8_t                bChangeName;
    uint16_t				wHairPart;
#ifdef ENABLE_ACCE_SYSTEM
    uint32_t				dwAccePart;
#endif
    int32_t				x, y;
    int32_t				lAddr;
    uint16_t				wPort;
    uint8_t				bySkillGroup;
} TSimplePlayerInformation;

typedef struct sub_packet_shop_tab
{
    char name[SHOP_TAB_NAME_MAX];
    uint8_t coin_type;
    packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
} TSubPacketShopTab;

typedef struct SPacketAcce
{
    uint8_t	header;
    uint8_t	subheader;
    bool	bWindow;
    uint32_t	dwPrice;
    uint8_t	bPos;
    TItemPos	tPos;
    uint32_t	dwItemVnum;
    uint32_t	dwMinAbs;
    uint32_t	dwMaxAbs;
} TPacketAcce;

typedef struct SAcceMaterial
{
    uint8_t	bHere;
    uint16_t	wCell;
} TAcceMaterial;

typedef struct SAcceResult
{
    uint32_t	dwItemVnum;
    uint32_t	dwMinAbs;
    uint32_t	dwMaxAbs;
} TAcceResult;

typedef struct SChannelStatus
{
    uint16_t nPort;
    uint8_t bStatus;
} TChannelStatus;

#pragma pack(pop)
