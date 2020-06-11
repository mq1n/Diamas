#pragma once

#include "AbstractCharacterManager.h"
#include "InstanceBase.h"
#include "../eterGameLib/PhysicsObject.h"

class CPythonCharacterManager : public CSingleton<CPythonCharacterManager>, public IAbstractCharacterManager, public IObjectManager
{
	public:
		// Character List
		typedef std::list<CInstanceBase *>			TCharacterInstanceList;
		typedef std::map<uint32_t, CInstanceBase *>	TCharacterInstanceMap;

		class CharacterIterator;

	public:
		CPythonCharacterManager();
		virtual ~CPythonCharacterManager();

		virtual void AdjustCollisionWithOtherObjects(CActorInstance* pInst ); 

		void EnableSortRendering(bool isEnable);

		bool IsRegisteredVID(uint32_t dwVID);
		bool IsAliveVID(uint32_t dwVID);
		bool IsDeadVID(uint32_t dwVID);
		bool IsCacheMode();

		bool OLD_GetPickedInstanceVID(uint32_t* pdwPickedActorID);
		CInstanceBase* OLD_GetPickedInstancePtr();
		D3DXVECTOR2& OLD_GetPickedInstPosReference();

		CInstanceBase* FindClickableInstancePtr();

		void InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
		void RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst);
		void ChangeGVG(uint32_t dwSrcGuildID, uint32_t dwDstGuildID);

		void GetInfo(std::string* pstInfo);

		void ClearMainInstance();
		bool SetMainInstance(uint32_t dwVID);
		CInstanceBase* GetMainInstancePtr();

		void								SCRIPT_SetAffect(uint32_t dwVID, uint32_t eAffect, BOOL isVisible);
		void								SetEmoticon(uint32_t dwVID, uint32_t eEmoticon);
		bool								IsPossibleEmoticon(uint32_t dwVID);
		void								ShowPointEffect(uint32_t dwVID, uint32_t ePoint);
		bool								RegisterPointEffect(uint32_t ePoint, const char* c_szFileName);

		// System
		void								Destroy();

		void								DeleteAllInstances();

		bool								CreateDeviceObjects();
		void								DestroyDeviceObjects();

		void								Update();
		void								Deform();
		void								Render();
		void								RenderShadowMainInstance();
		void								RenderShadowAllInstances();
		void								RenderCollision();

		// Create/Delete Instance
		CInstanceBase *						CreateInstance(const CInstanceBase::SCreateData& c_rkCreateData);
		CInstanceBase *						RegisterInstance(uint32_t VirtualID);

		void								DeleteInstance(uint32_t VirtualID);
		void								DeleteInstanceByFade(uint32_t VirtualID);
		void								DeleteVehicleInstance(uint32_t VirtualID);

		void 								DestroyAliveInstanceMap();
		void 								DestroyDeadInstanceList();

		inline CharacterIterator			CharacterInstanceBegin() { return CharacterIterator(m_kAliveInstMap.begin());}
		inline CharacterIterator			CharacterInstanceEnd() { return CharacterIterator(m_kAliveInstMap.end());}

		// Access Instance
		void								SelectInstance(uint32_t VirtualID);
		CInstanceBase *						GetSelectedInstancePtr();

		CInstanceBase *						GetInstancePtr(uint32_t VirtualID);
		CInstanceBase *						GetInstancePtrByName(const char *name);

		// Pick		
		int32_t									PickAll();
		CInstanceBase *						GetCloseInstance(CInstanceBase * pInstance);

		// Refresh TextTail
		void								RefreshAllPCTextTail();
		void								RefreshAllGuildMark();
		void								RefreshGuildSymbols(std::set<uint32_t> guildIDSet);

	protected:
		void								UpdateTransform();
		void								UpdateDeleting();

	protected:
		void __Initialize();

		void __DeleteBlendOutInstance(CInstanceBase* pkInstDel);

		void __OLD_Pick();
		void __NEW_Pick();

		void __UpdateSortPickedActorList();
		void __UpdatePickedActorList();
		void __SortPickedActorList();

		void __RenderSortedAliveActorList();
		void __RenderSortedDeadActorList();

	protected:
		CInstanceBase *						m_pkInstMain;
		CInstanceBase *						m_pkInstPick;
		CInstanceBase *						m_pkInstBind;
		D3DXVECTOR2							m_v2PickedInstProjPos;

		TCharacterInstanceMap				m_kAliveInstMap;
		TCharacterInstanceList				m_kDeadInstList;

		std::vector<CInstanceBase*>			m_kVct_pkInstPicked;

		uint32_t								m_adwPointEffect[POINT_MAX_NUM];

	public:
		class CharacterIterator
		{
		public:
			CharacterIterator(){}
			CharacterIterator(const TCharacterInstanceMap::iterator & it) : m_it(it) {}

			inline CInstanceBase * operator * () {	return m_it->second; }

			inline CharacterIterator & operator ++()
			{
				++m_it;
				return *this;
			}

			inline CharacterIterator operator ++(int32_t)
			{
				CharacterIterator new_it = *this;
				++(*this);
				return new_it;
			}

			inline CharacterIterator & operator = (const CharacterIterator & rhs)
			{
				m_it = rhs.m_it;
				return (*this);
			}

			inline bool operator == (const CharacterIterator & rhs) const
			{
				return m_it == rhs.m_it;
			}

			inline bool operator != (const CharacterIterator & rhs) const
			{
				return m_it != rhs.m_it;
			}

			private:
				TCharacterInstanceMap::iterator m_it;
		};
};
