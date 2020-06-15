#include "stdafx.h"
#include "Battleground.h"
#include "constants.h"
#include "config.h"
#include "quest_manager.h"
#include "start_position.h"
#include "packet.h"
#include "buffer_manager.h"
#include "log.h"
#include "char.h"
#include "char_manager.h"
#include "desc.h"
#include "sectree_manager.h"
#include "battle.h"
#include "item.h"

CBattlegroundQueue::CBattlegroundQueue()
{
	m_pkLstAttenders.clear();
}


void CBattlegroundQueue::Initialize()
{

}
void CBattlegroundQueue::Destroy()
{

}


bool CBattlegroundQueue::JoinToQueue(uint32_t dwPlayerID, uint8_t nGameMode, uint8_t nQueueType, uint8_t nMatchMakingType)
{
	return true;
}
bool CBattlegroundQueue::CanJoinToQueue(uint32_t dwPlayerID) const
{
	// IsBattlegroundCreateBlocked
	return true;
}

bool CBattlegroundQueue::InQueue(uint32_t dwPlayerID) const
{
	return false;
}

uint32_t CBattlegroundQueue::GetQueueSize() const
{
	return m_pkLstAttenders.size();
}

void CBattlegroundQueue::ProcessQueue()
{
	// oyun moduna göre gerekli miktarda oyuncu dilimini bir vectore aktar
	// aktif olan oyunculara onay mesajı gönder event oluşturup 30sn timeout ile(clientten progressbar) onay bekle
	// client onayı verdiğinde servere komut gönder event timeout olduğunda onay verilen client sayısı vektördeki client sayısına+
	// eşitse başlat eşit değilse onay verenleri yeni kuyruğa sokup sonraki işlemi beklet
}

