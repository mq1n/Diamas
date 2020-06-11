
#pragma once
#include <SpeedTreeRT.h>
#include "SpeedTreeWrapper.h"

#include <vector>
#include <unordered_map>
#include <memory>
#define Forest_RenderBranches		(1 << 0)
#define Forest_RenderLeaves			(1 << 1)
#define Forest_RenderFronds			(1 << 2)
#define Forest_RenderBillboards		(1 << 3)
#define Forest_RenderAll			((1 << 4) - 1)
#define Forest_RenderToShadow		(1 << 5)
#define Forest_RenderToMiniMap		(1 << 6)
class CSpeedTreeForest
{
	public:
		typedef std::unordered_map<uint32_t, CSpeedTreeWrapper *> TTreeMap;

	public:
		CSpeedTreeForest();
		virtual ~CSpeedTreeForest();

		BOOL						GetMainTree(uint32_t dwCRC, CSpeedTreeWrapper ** ppMainTree, const char * c_pszFileName);
		CSpeedTreeWrapper *			GetMainTree(uint32_t dwCRC);

		CSpeedTreeWrapper *			CreateInstance(float x, float y, float z, uint32_t dwTreeCRC, const char * c_pszTreeName);
		void						DeleteInstance(CSpeedTreeWrapper * pTree);
		void						UpdateSystem(float fCurrentTime);

		void						Clear();

		void						SetLight(const float * afDirection, const float * afAmbient, const float * afDiffuse);
		void						SetFog(float fFogNear, float fFogFar);
		const float *				GetExtents(void) const						{ return m_afForestExtents; }
		float						GetWindStrength(void) const					{ return m_fWindStrength; }
		void						SetupWindMatrices(float fTimeInSecs);
		virtual	void				UploadWindMatrix(uint32_t uiLocation, const float* pMatrix) const = 0;
		virtual void				Render(uint32_t ulRenderBitVector) = 0;

	protected:
		TTreeMap					m_pMainTreeMap;

		float						m_afLighting[12];
		float						m_afFog[4];

	private:
		void						AdjustExtents(float x, float y, float z);

		float						m_afForestExtents[6];
		float						m_fWindStrength;

		float						m_fAccumTime;
};
