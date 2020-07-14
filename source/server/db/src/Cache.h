#pragma once
#include "../../../common/cache.h"

class CItemCache : public cache<TPlayerItem>
{
    public:
	CItemCache();
	virtual ~CItemCache();

	void Delete();
	virtual void OnFlush();
};

class CPlayerTableCache : public cache<TPlayerTable>
{
    public:
	CPlayerTableCache();
	virtual ~CPlayerTableCache();

	virtual void OnFlush();

	uint32_t GetLastUpdateTime() { return static_cast<uint32_t>(m_lastUpdateTime); }
};

// MYSHOP_PRICE_LIST
/**
 * @class	CItemPriceListTableCache
 * @brief	개인상점의 아이템 가격정보 리스트에 대한 캐시 class
 * @version	05/06/10 Bang2ni - First release.
 */
class CItemPriceListTableCache : public cache< TItemPriceListTable >
{
    public:

	/// Constructor
	/**
	 * 캐시 만료 시간을 설정한다.
	 */
	CItemPriceListTableCache(void);
	virtual ~CItemPriceListTableCache();

	
	/**
	 * @param [in]	pUpdateList 갱신할 리스트
	 *
	 * 캐시된 가격정보를 갱신한다.
	 * 가격정보 리스트가 가득 찼을 경우 기존에 캐싱된 정보들을 뒤에서 부터 삭제한다.
	 */
	void	UpdateList(const TItemPriceListTable* pUpdateList);

	/// 가격정보를 DB 에 기록한다.
	virtual void	OnFlush(void);

    private:

	static const int32_t	s_nMinFlushSec;		///< Minimum cache expire time
};
// END_OF_MYSHOP_PRICE_LIST

class CActivityCache : public cache <TActivityTable>
{
public:
	CActivityCache();
	virtual ~CActivityCache();

	virtual void OnFlush();

	uint32_t GetLastUpdateTime() { return m_lastUpdateTime; }
};
