// WorldEditor.h : main header file for the WORLDEDITOR application
//

#if !defined(AFX_WORLDEDITOR_H__23A6089B_4D11_4492_BA3F_A18389CD231D__INCLUDED_)
#define AFX_WORLDEDITOR_H__23A6089B_4D11_4492_BA3F_A18389CD231D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "../../Client/eterlib/GrpDevice.h"
#include "../../Client/eterPack/EterPackManager.h"
#include "../../Client/milesLib/SoundManager.h"
#include "../../Client/EffectLib/EffectManager.h"
#include "../../Client/gamelib/FlyingObjectManager.h"
#include "../../Client/EterLib/GrpLightManager.h"
#include "../../Client/gamelib/GameEventManager.h"
#include "../../Client/EterLib/CullingManager.h"

//------------------------//
// Temporary Code
#include "./DataCtrl/ObjectData.h"
#include "./DataCtrl/MapManagerAccessor.h"
//------------------------//

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp:
// See WorldEditor.cpp for the implementation of this class
//

class CWorldEditorApp : public CWinApp
{
public:
	CWorldEditorApp();
	~CWorldEditorApp();

public:
	CObjectData * GetObjectData();
	CEffectAccessor * GetEffectAccessor();
	CMapManagerAccessor * GetMapManagerAccessor();
	CSceneObject * GetSceneObject();
	CSceneEffect * GetSceneEffect();
	CSceneMap * GetSceneMap();
	CSceneFly * GetSceneFly();
	
	CGraphicDevice & GetGraphicDevice() { return m_GraphicDevice; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWorldEditorApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CTimer m_Timer;
	CPythonResource m_PythonResource;

	CEterPackManager m_EterPackManager;

	CGraphicDevice m_GraphicDevice;
	CSoundManager m_SoundManager;
	CEffectManager m_EffectManager;
	CFlyingManager m_FlyingManager;
	CLightManager m_LightManager;
	CGameEventManager m_GameEventManager;

	CCullingManager m_CullingManager;
	CLZO m_lzo;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDITOR_H__23A6089B_4D11_4492_BA3F_A18389CD231D__INCLUDED_)
