// ToolBarFile.cpp : implementation file
//

#include "stdafx.h"
#include "../WorldEditor.h"
#include "ToolBarFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBarFile

CToolBarFile::CToolBarFile()
{
}

CToolBarFile::~CToolBarFile()
{
}


BEGIN_MESSAGE_MAP(CToolBarFile, CToolBar256)
	//{{AFX_MSG_MAP(CToolBarFile)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolBarFile normal functions

void CToolBarFile::ShowButtonText(BOOL bShow)
{
	if (bShow)
	{
		SetSizes(CSize(43, 34), CSize(16, 15)); // �ѱ� 3����
		LoadBitmap(IDR_MAINFRAME);
		m_nBitmapID = IDR_MAINFRAME;

		SetButtonText(CommandToIndex(ID_FILE_NEW), "������");
		SetButtonText(CommandToIndex(ID_FILE_OPEN), "����");
		SetButtonText(CommandToIndex(ID_FILE_SAVE), "����");
		SetButtonText(CommandToIndex(ID_EDIT_UNDO), "���");

		SetButtonText(CommandToIndex(ID_VIEW_OBJECT), "������Ʈ");
		SetButtonText(CommandToIndex(ID_VIEW_ENVIRONMENT), "ȯ��");

		SetButtonText(CommandToIndex(ID_APP_ABOUT), "���۱�");
	}
	else
	{
		SetSizes(CSize(43, 34), CSize(36, 28)); // �ѱ� 3����
		LoadBitmap(IDB_MAINFRAME_LARGE);
		m_nBitmapID = IDB_MAINFRAME_LARGE;
	}
}

BOOL CToolBarFile::CN_UPDATE_HANDLER(UINT nID, CCmdUI *pCmdUI)
{
	switch (nID)
	{
//	case ID_FILE_BOX:
	case ID_EDIT_UNDO:
//	case ID_EDIT_DELETE:
		pCmdUI->Enable(TRUE); 
		return TRUE;
	}

	return FALSE;
}

BOOL CToolBarFile::CN_COMMAND_HANDLER(UINT nID, CCmdUI *pCmdUI)
{
		/*
	CUndoBuffer *pUndo = ((CMapEditApp*)AfxGetApp())->GetUndoBuffer();
	CViewInfo *pVI = ((CMapEditApp*)AfxGetApp())->GetViewInfo();

	switch (nID)
	{
	case ID_FILE_BOX:
		{
			CVDiskDlg filebox;
			filebox.DoModal();
		}
		return TRUE;
	case ID_EDIT_UNDO:
		pUndo->Undo(); 
		return TRUE;
	case ID_EDIT_DELETE:
		if (pVI->m_dwSubMode != SM_ANIMATION)
			pUndo->do_Delete(); 
		return TRUE;
	}
		*/

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CToolBarFile message handlers
