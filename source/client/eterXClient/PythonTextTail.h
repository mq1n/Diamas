#pragma once

#include "../eterBase/Singleton.h"

/*
 *	����ٴϴ� �ؽ�Ʈ ó��
 */
class CPythonTextTail : public CSingleton<CPythonTextTail>
{
	public:
		typedef struct STextTail
		{
			CGraphicTextInstance*			pTextInstance;
			CGraphicTextInstance*			pOwnerTextInstance;

			CGraphicMarkInstance*			pMarkInstance;
			CGraphicTextInstance*			pGuildNameTextInstance;

			CGraphicTextInstance*			pTitleTextInstance;
			CGraphicTextInstance*			pLevelTextInstance;


			// Todo : �̰� ���� VID�� �ٲٵ��� �Ѵ�.
			//        ���� ĳ���Ͱ� ������ ��� ƨ�� ���ɼ��� ����
			CGraphicObjectInstance *		pOwner;

			uint32_t							dwVirtualID;

			float							x, y, z;
			float							fDistanceFromPlayer;
			D3DXCOLOR						Color;
			BOOL							bNameFlag;		// �̸��� �Բ� �Ӱ������� �÷���

			float							xStart, yStart;
			float							xEnd, yEnd;

			uint32_t							LivingTime;

			float							fHeight;

			STextTail() {}
			virtual ~STextTail() {}
		} TTextTail;

		typedef std::map<uint32_t, TTextTail*>		TTextTailMap;
		typedef std::list<TTextTail*>			TTextTailList;
		typedef TTextTailMap					TChatTailMap;

	public:
		CPythonTextTail(void);
		virtual ~CPythonTextTail(void);

		void GetInfo(std::string* pstInfo);

		void Initialize();
		void Destroy();
		void Clear();

		void UpdateAllTextTail();
		void UpdateShowingTextTail();
		void Render();

		void ArrangeTextTail();
		void HideAllTextTail();
		void ShowAllTextTail();
		void ShowCharacterTextTail(uint32_t VirtualID);
		void ShowItemTextTail(uint32_t VirtualID);

		void RegisterCharacterTextTail(uint32_t dwGuildID, uint32_t dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight=10.0f);
		void RegisterItemTextTail(uint32_t VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner);
		void RegisterChatTail(uint32_t VirtualID, const char * c_szChat);
		void RegisterInfoTail(uint32_t VirtualID, const char * c_szChat);
		void SetCharacterTextTailColor(uint32_t VirtualID, const D3DXCOLOR & c_rColor);
		void SetItemTextTailOwner(uint32_t dwVID, const char * c_szName);
		void DeleteCharacterTextTail(uint32_t VirtualID);
		void DeleteItemTextTail(uint32_t VirtualID);

		int32_t Pick(int32_t ixMouse, int32_t iyMouse);
		void SelectItemName(uint32_t dwVirtualID);

		bool GetTextTailPosition(uint32_t dwVID, float* px, float* py, float* pz);
		bool IsChatTextTail(uint32_t dwVID);

		void EnablePKTitle(BOOL bFlag);
		void AttachTitle(uint32_t dwVID, const char * c_szName, const D3DXCOLOR& c_rColor);
		void DetachTitle(uint32_t dwVID);

		void AttachLevel(uint32_t dwVID, const char* c_szText, const D3DXCOLOR& c_rColor);
		void DetachLevel(uint32_t dwVID);
		

	protected:
		TTextTail * RegisterTextTail(uint32_t dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, float fHeight, const D3DXCOLOR & c_rColor);
		void DeleteTextTail(TTextTail * pTextTail);

		void UpdateTextTail(TTextTail * pTextTail);
		void RenderTextTailBox(TTextTail * pTextTail);
		void RenderTextTailName(TTextTail * pTextTail);
		void UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail);

		bool isIn(TTextTail * pSource, TTextTail * pTarget);

	protected:
		TTextTailMap				m_CharacterTextTailMap;
		TTextTailMap				m_ItemTextTailMap;
		TChatTailMap				m_ChatTailMap;

		TTextTailList				m_CharacterTextTailList;
		TTextTailList				m_ItemTextTailList;

	private:
		CDynamicPool<STextTail>		m_TextTailPool;
};
