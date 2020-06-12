#pragma once


#include "../eterBase/FileLoader.h"

#include <map>
#include <vector>

#include <d3d9.h>
#include <d3dx9math.h>
template<typename T>
class CTransitor
{
	public:
		CTransitor(): m_dwID(0), m_bActivated(0), m_fStartTime(0), m_fEndTime(0)
		{}
		~CTransitor() {}

		void SetActive(BOOL bActive = TRUE)
		{
			m_bActivated = bActive;
		}

		BOOL isActive()
		{
			return m_bActivated;
		}

		BOOL isActiveTime(float fcurTime)
		{
			if (fcurTime >= m_fEndTime)
				return FALSE;

			return TRUE;
		}

		uint32_t GetID()
		{
			return m_dwID;
		}

		void SetID(uint32_t dwID)
		{
			m_dwID = dwID;
		}

		void SetSourceValue(const T & c_rSourceValue)
		{
			m_SourceValue = c_rSourceValue;
		}

		void SetTransition(const T & c_rSourceValue, const T & c_rTargetValue, float fStartTime, float fBlendTime)
		{
			m_SourceValue = c_rSourceValue;
			m_TargetValue = c_rTargetValue;
			m_fStartTime = fStartTime;
			m_fEndTime = fStartTime + fBlendTime;
		}

		BOOL GetValue(float fcurTime, T * pValue)
		{
			if (fcurTime <= m_fStartTime)
				return FALSE;

			float fPercentage = (fcurTime - m_fStartTime) / (m_fEndTime - m_fStartTime);
			*pValue = m_SourceValue + (m_TargetValue - m_SourceValue) * fPercentage;
			return TRUE;
		}

	protected:
		uint32_t	m_dwID;			// Public Transitor ID

		BOOL	m_bActivated;	// Have been started to blend?
		float	m_fStartTime;
		float	m_fEndTime;

		T		m_SourceValue;
		T		m_TargetValue;
};

typedef CTransitor<float>			TTransitorFloat;
typedef CTransitor<D3DXVECTOR3>		TTransitorVector3;
typedef CTransitor<D3DXCOLOR>		TTransitorColor;

///////////////////////////////////////////////////////////////////////////////////////////////////

void PrintfTabs(FILE * File, int32_t iTabCount, const char * c_szString, ...);


//typedef CTokenVector TTokenVector;

extern bool	LoadTextData(const char * c_szFileName, CTokenMap & rstTokenMap);
extern bool	LoadMultipleTextData(const char * c_szFileName, CTokenVectorMap & rstTokenVectorMap);

extern D3DXVECTOR3 TokenToVector(CTokenVector & rVector);
extern D3DXCOLOR TokenToColor(CTokenVector & rVector);

#define GOTO_CHILD_NODE(TextFileLoader, Index) CTextFileLoader::CGotoChild Child(TextFileLoader, Index);

///////////////////////////////////////////////////////////////////////////////////////////////////

extern void base64_decode(const char * str,char * resultStr);

extern uint32_t GetMaxTextureWidth();
extern uint32_t GetMaxTextureHeight();