#pragma once
#include "entity.h"

enum ESectree
{
	SECTREE_SIZE	= 6400,
	SECTREE_HALF_SIZE	= 3200,
	CELL_SIZE		= 50
};

typedef struct sectree_coord
{
	uint32_t            x : 16;
	uint32_t            y : 16;
} SECTREE_COORD;

typedef union sectreeid
{
	uint32_t		package;
	SECTREE_COORD	coord;
} SECTREEID;

enum
{
	ATTR_BLOCK = (1 << 0),
	ATTR_WATER = (1 << 1),
	ATTR_BANPK = (1 << 2),
	ATTR_OBJECT = (1 << 7),
};

struct FCollectEntity
{
	void operator()(LPENTITY entity)
	{
		// Consider removing sanity check after debug pass
		/*
		if (entity->IsType(ENTITY_CHARACTER)) {
			LPCHARACTER character = (LPCHARACTER)entity;
			uint32_t vid = character->GetVID();
			LPCHARACTER found = CHARACTER_MANGAER::Instance().Find(vid);
			if (found == nullptr || vid != found->GetVID()) {
				sys_err("<Factor> Invalid character %p", get_pointer(character));
				return;
			}
		} else if (entity->IsType(ENTITY_ITEM)) {
			LPITEM item = (LPITEM)entity;
			uint32_t vid = item->GetVID();
			LPITEM found = ITEM_MANGAER::Instance().FindByVID(vid);
			if (found == nullptr || vid != found->GetVID()) {
				sys_err("<Factor> Invalid item %p", get_pointer(item));
				return;
			}
		} else if (entity->IsType(ENTITY_OBJECT)) {
			LPOBJECT object = (LPOBJECT)entity;
			uint32_t vid = object->GetVID();
			LPOBJECT found = CManager::Instance().FindObjectByVID(vid);
			if (found == nullptr || vid != found->GetVID()) {
				sys_err("<Factor> Invalid object %p", get_pointer(object));
				return;
			}
		} else {
			sys_err("<Factor> Invalid entity type %p", get_pointer(entity));
			return;
		}
		*/
		if (entity)
			result.push_back(entity);
	}
	template<typename F>
	void ForEach(F& f) 
	{
		for (auto it = result.begin(); it != result.end(); ++it)
		{
			LPENTITY entity = *it;
			f(entity);
		}
	}

	void GetCharacterEntities(ENTITY_VECTOR &entityList) 
	{
		for (auto it = result.begin(); it != result.end(); ++it)
		{
			LPENTITY ent = *it;
			if (ent && ent->IsType(ENTITY_CHARACTER)) 
			{
				entityList.push_back(ent);
			}
		}
	}

	typedef std::vector<LPENTITY> ListType;
	ListType result; // list collected
};

class CAttribute;

class SECTREE
{
	public:
		friend class SECTREE_MANAGER;
		friend class SECTREE_MAP;

	public:
		SECTREE();
		~SECTREE();

		void				Initialize();
		void				Destroy();

		SECTREEID			GetID() const;

		bool				InsertEntity(LPENTITY ent);
		void				RemoveEntity(LPENTITY ent);

		void				IncreasePC();
		void				DecreasePC();

		void				BindAttribute(CAttribute * pkAttribute);

		CAttribute *		GetAttributePtr() { return m_pkAttribute; }

		uint32_t			GetAttribute(int32_t x, int32_t y);
		bool				IsAttr(int32_t x, int32_t y, uint32_t dwFlag);

		void				CloneAttribute(LPSECTREE tree); // private map 처리시 사용

		int32_t				GetEventAttribute(int32_t x, int32_t y); // 20050313 현재는 사용하지 않음

		void				SetAttribute(uint32_t x, uint32_t y, uint32_t dwAttr);
		void				RemoveAttribute(uint32_t x, uint32_t y, uint32_t dwAttr);

		template <class _Func>
		void ForEachAround(_Func & func)
		{
			for (auto it_tree = m_neighbor_list.begin(); it_tree != m_neighbor_list.end(); ++it_tree) 
			{
				(*it_tree)->ForEachEntity(func);
			}
		}

//	private:
	public:
		template <class _Func> void ForEachEntity(_Func & func)
		{
			for (auto it = m_set_entity.begin(); it != m_set_entity.end(); ++it)
			{
				auto entity = *it;
				// <Factor> Sanity check
				if (entity->GetSectree() != this)
				{
					sys_err("<Factor> SECTREE-ENTITY relationship mismatch");

					m_set_entity.erase(it);
					continue;
				}

				func(entity);
			}
		}

		template <class _Func> void ForEachEntityOfType(_Func & func, int32_t type)
		{
			for (auto it = m_set_entity.begin(); it != m_set_entity.end(); ++it)
			{
				auto entity = *it;
				
				// <Factor> Sanity check
				if (entity->GetSectree() != this) 
				{
					sys_err("<Factor> SECTREE-ENTITY relationship mismatch");

					m_set_entity.erase(it);
					continue;
				}
				
				if (entity->IsType(type))
					func(entity);
			}
		}

		SECTREEID			m_id;
		ENTITY_SET			m_set_entity;
		LPSECTREE_LIST		m_neighbor_list;
		int32_t					m_iPCCount;
		bool				isClone;

		CAttribute *		m_pkAttribute;
};
