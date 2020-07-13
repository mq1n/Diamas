#pragma once
#include "../eterBase/FileLoader.h"
#include "../eterLib/Util.h"
#include "../eterLib/Pool.h"

class CTextFileLoader
{
	public:
		typedef struct SGroupNode
		{
			static uint32_t GenNameKey(const char* c_szGroupName, uint32_t uGroupNameLen);

			void SetGroupName(const std::string& c_rstGroupName);
			bool IsGroupNameKey(uint32_t dwGroupNameKey);

			const std::string& GetGroupName();

			CTokenVector* GetTokenVector(const std::string& c_rstGroupName);
			bool IsExistTokenVector(const std::string& c_rstGroupName);
			void InsertTokenVector(const std::string& c_rstGroupName, const CTokenVector& c_rkVct_stToken);

			uint32_t m_dwGroupNameKey;
			std::string m_strGroupName;

			std::map<uint32_t, CTokenVector> m_kMap_dwKey_kVct_stToken;

			SGroupNode * pParentNode;
			std::vector<SGroupNode*> ChildNodeVector;

			static SGroupNode* New();
			static void Delete(SGroupNode* pkNode);

			static void DestroySystem();
			static CDynamicPool<SGroupNode>	ms_kPool;
		} TGroupNode;

		typedef std::vector<TGroupNode*> TGroupNodeVector;

		class CGotoChild
		{
		public:
			CGotoChild(CTextFileLoader * pOwner, const char * c_szKey) : m_pOwner(pOwner)
			{
				m_pOwner->SetChildNode(c_szKey);
			}
			CGotoChild(CTextFileLoader * pOwner, uint32_t dwIndex) : m_pOwner(pOwner)
			{
				m_pOwner->SetChildNode(dwIndex);
			}
			~CGotoChild()
			{
				m_pOwner->SetParentNode();
			}

			CTextFileLoader * m_pOwner;
		};

	public:
		static void DestroySystem();

		static void SetCacheMode();

		static CTextFileLoader* Cache(const char* c_szFileName, bool silentFailure = false);

	public:
		CTextFileLoader();
		virtual ~CTextFileLoader();

		void Destroy();

		bool Load(const char * c_szFileName, bool silentFailure = false);
		const char * GetFileName();

		bool IsEmpty();

		void SetTop();
		uint32_t GetChildNodeCount();
		BOOL SetChildNode(const char * c_szKey);
		BOOL SetChildNode(const std::string & c_rstrKeyHead, uint32_t dwIndex);
		BOOL SetChildNode(uint32_t dwIndex);
		BOOL SetParentNode();
		BOOL GetCurrentNodeName(std::string * pstrName);

		BOOL IsToken(const std::string & c_rstrKey);
		BOOL GetTokenVector(const std::string & c_rstrKey, CTokenVector ** ppTokenVector);
		BOOL GetTokenBoolean(const std::string & c_rstrKey, BOOL * pData);
		BOOL GetTokenByte(const std::string & c_rstrKey, uint8_t * pData);
		BOOL GetTokenWord(const std::string & c_rstrKey, uint16_t * pData);
		BOOL GetTokenInteger(const std::string & c_rstrKey, int32_t * pData);
		BOOL GetTokenDoubleWord(const std::string & c_rstrKey, uint32_t * pData);
		BOOL GetTokenFloat(const std::string & c_rstrKey, float * pData);
		BOOL GetTokenVector2(const std::string & c_rstrKey, D3DXVECTOR2 * pVector2);
		BOOL GetTokenVector3(const std::string & c_rstrKey, D3DXVECTOR3 * pVector3);
		BOOL GetTokenVector4(const std::string & c_rstrKey, D3DXVECTOR4 * pVector4);

		BOOL GetTokenPosition(const std::string & c_rstrKey, D3DXVECTOR3 * pVector);
		BOOL GetTokenQuaternion(const std::string & c_rstrKey, D3DXQUATERNION * pQ);
		BOOL GetTokenDirection(const std::string & c_rstrKey, D3DVECTOR * pVector);
		BOOL GetTokenColor(const std::string & c_rstrKey, D3DXCOLOR * pColor);
		BOOL GetTokenColor(const std::string & c_rstrKey, D3DCOLORVALUE * pColor);
		BOOL GetTokenString(const std::string & c_rstrKey, std::string * pString);

	protected:
		void __DestroyGroupNodeVector();

		bool LoadGroup(TGroupNode * pGroupNode);

	protected:
		std::string					m_strFileName;

		char*						m_acBufData;
		uint32_t						m_dwBufSize;
		uint32_t						m_dwBufCapacity;

		uint32_t						m_dwcurLineIndex;

		CMemoryTextFileLoader		m_textFileLoader;

		TGroupNode					m_GlobalNode;
		TGroupNode *				m_pcurNode;

		std::vector<SGroupNode*>	m_kVct_pkNode;

	protected:
		static std::map<uint32_t, CTextFileLoader*> ms_kMap_dwNameKey_pkTextFileLoader;
		static bool ms_isCacheMode;
};
