#pragma once
#include "../../../common/d3dtype.h"
#include "../../../common/pool.h"
#include "file_loader.h"

typedef std::map<std::string, TTokenVector>	TTokenVectorMap;

class CTextFileLoader
{
	public:
		typedef struct SGroupNode
		{
			std::string strGroupName;

			TTokenVectorMap LocalTokenVectorMap;

			SGroupNode * pParentNode;
			std::vector<SGroupNode*> ChildNodeVector;
		} TGroupNode;

		typedef std::vector<TGroupNode *> TGroupNodeVector;

	public:
		static void DestroySystem();

	public:
		CTextFileLoader();
		virtual ~CTextFileLoader();

		bool Load(const char * c_szFileName);
		const char * GetFileName();

		void SetTop();
		uint32_t GetChildNodeCount();
		bool SetChildNode(const char * c_szKey);
		bool SetChildNode(const std::string & c_rstrKeyHead, uint32_t dwIndex);
		bool SetChildNode(uint32_t dwIndex);
		bool SetParentNode();
		bool GetCurrentNodeName(std::string * pstrName);

		bool IsToken(const std::string & c_rstrKey);
		bool GetTokenVector(const std::string & c_rstrKey, TTokenVector ** ppTokenVector);
		bool GetTokenboolean(const std::string & c_rstrKey, bool * pData);
		bool GetTokenByte(const std::string & c_rstrKey, uint8_t * pData);
		bool GetTokenWord(const std::string & c_rstrKey, uint16_t * pData);
		bool GetTokenInteger(const std::string & c_rstrKey, int32_t * pData);
		bool GetTokenDoubleWord(const std::string & c_rstrKey, uint32_t * pData);
		bool GetTokenFloat(const std::string & c_rstrKey, float * pData);

		bool GetTokenVector2(const std::string & c_rstrKey, D3DXVECTOR2 * pVector2);
		bool GetTokenVector3(const std::string & c_rstrKey, D3DXVECTOR3 * pVector3);
		bool GetTokenVector4(const std::string & c_rstrKey, D3DXVECTOR4 * pVector4);

		bool GetTokenPosition(const std::string & c_rstrKey, D3DXVECTOR3 * pVector);
		bool GetTokenQuaternion(const std::string & c_rstrKey, D3DXQUATERNION * pQ);
		bool GetTokenDirection(const std::string & c_rstrKey, D3DVECTOR * pVector);
		bool GetTokenColor(const std::string & c_rstrKey, D3DXCOLOR * pColor);
		bool GetTokenColor(const std::string & c_rstrKey, D3DCOLORVALUE * pColor);
		bool GetTokenString(const std::string & c_rstrKey, std::string * pString);

	protected:
		bool LoadGroup(TGroupNode * pGroupNode);

	protected:
		std::string					m_strFileName;
		uint32_t						m_dwcurLineIndex;
		const void *					mc_pData;

		CMemoryTextFileLoader				m_fileLoader;

		TGroupNode					m_globalNode;
		TGroupNode *					m_pcurNode;

	private:
		static CDynamicPool<TGroupNode>			ms_groupNodePool;
};
