#pragma once
#include "gposition.h"
#include "typedef.h"
#include <unordered_map>
#include <functional>

class SECTREE;

class CEntity
{
	public:
		typedef std::unordered_map<LPENTITY, int32_t> MAP_VIEW;
		typedef std::function<bool(LPCHARACTER)> ENTITY_REQ_FUNC;

	public:
		CEntity();
		virtual	~CEntity();

		virtual void	EncodeInsertPacket(LPENTITY entity) = 0;
		virtual	void	EncodeRemovePacket(LPENTITY entity) = 0;

	protected:
		void			Initialize(int32_t type = -1);
		virtual void	Destroy();


	public:
		void			SetType(int32_t type);
		int32_t			GetType() const;
		bool			IsType(int32_t type) const;

		void			ViewCleanup();
		void			ViewInsert(LPENTITY entity, bool recursive = true);
		void			ViewRemove(LPENTITY entity, bool recursive = true);
		void			ViewReencode();	// 주위 Entity에 패킷을 다시 보낸다.

		int32_t			GetViewAge() const	{ return m_viewAge;	}

		int32_t			GetX() const		{ return m_position.x; }
		int32_t			GetY() const		{ return m_position.y; }
		int32_t			GetZ() const		{ return m_position.z; }
		const GPOS &	GetXYZ() const		{ return m_position; }

		void			SetXYZ(int32_t x, int32_t y, int32_t z)		{ m_position.x = x, m_position.y = y, m_position.z = z; }
		void			SetXYZ(const GPOS & pos)	{ m_position = pos; }

		LPSECTREE		GetSectree() const			{ return m_sectree;	}
		void			SetSectree(LPSECTREE tree)	{ m_sectree = tree;	}

		void			UpdateSectree();
		void			PacketAround(const void * data, int32_t bytes, LPENTITY except = nullptr);
		void			PacketView(const void * data, int32_t bytes, LPENTITY except = nullptr);
		void			PacketMap(int32_t nMapIndex, const void * data, int32_t bytes);

		template <typename Function>
		void ForEachSeen(Function& f);
		void			BindDesc(LPDESC _d)     { m_desc = _d; }
		LPDESC			GetDesc() const			{ return m_desc; }

		void			SetMapIndex(int32_t l)	{ m_mapIndex = l; }
		int32_t			GetMapIndex() const	{ return m_mapIndex; }

		void			SetObserverMode(bool bFlag);
		bool			IsObserverMode() const	{ return m_isObserver; }

		void			SetBattlegroundEntity();
		bool			IsBattlegroundEntity() { return m_isBattlegroundEntity; };

		ENTITY_REQ_FUNC GetRequirementFunction() const { return m_fReq; };
		void			SetRequirementFunction(ENTITY_REQ_FUNC f) { m_fReq = f; }
	protected:
		bool			m_isObserver;
		bool			m_isObserverModeUpdate;
		MAP_VIEW		m_mapView;
		int32_t			m_mapIndex;
		bool			m_isBattlegroundEntity;

	private:
		LPDESC			m_desc;

		int32_t			m_objectType;
		bool			m_isDestroyed;

		GPOS			m_position;

		int32_t			m_viewAge;

		LPSECTREE		m_sectree;
		bool			m_isShow;
		ENTITY_REQ_FUNC m_fReq;
};

class CharacterSnapshotGuard;

template <typename Function>
void CEntity::ForEachSeen(Function& f)
{
	std::vector<CEntity*> seen;
	seen.reserve(m_mapView.size());

	auto collector = [&seen](const MAP_VIEW::value_type& p)
	{
		seen.push_back(p.first);
	};

	// TODO: Some functions expect the old behaviour of including this in
	// their around view.
	// figure those out and manually call f on this
	seen.push_back(this);

	std::for_each(m_mapView.begin(), m_mapView.end(), collector);

	// This is very fragile. All of ForEachSeen()'s callers are only interested
	// in characters. One of these characters might however be destroyed while
	// we are iterating over our snapshot.
	CharacterSnapshotGuard guard;

	for (const auto& e : seen)
		f(e);
}
