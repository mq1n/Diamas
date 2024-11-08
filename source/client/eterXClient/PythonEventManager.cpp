#include "StdAfx.h"
#include "PythonEventManager.h"
#include "PythonNetworkStream.h"
#include "PythonNonPlayer.h"
#include "AbstractApplication.h"
#include "../eterGameLib/ItemData.h"
#include "../eterGameLib/ItemManager.h"
#include <FileSystemIncl.hpp>
#include "PythonMiniMap.h"

const int32_t c_lNormal_Waiting_Time = 5;
const int32_t c_fLine_Temp = 16;

void ShowArgument(script::TArgList & rArgumentList)
{
	for (script::TArgList::iterator itor=rArgumentList.begin(); itor!=rArgumentList.end(); ++itor)
	{
		const std::string & rName = (*itor).strName;

		Tracen(rName.c_str());		
	}

}

const std::string& GetArgumentString(const char * c_szName, script::TArgList & rArgumentList)
{
	for (script::TArgList::iterator itor = rArgumentList.begin(); itor != rArgumentList.end(); ++itor)
	{
		const std::string & rName = (*itor).strName;

		if (0 == rName.compare(c_szName))
			return (*itor).strValue;
	}

	static std::string strEmptyValue = "";
	return strEmptyValue;
}

const char * GetArgument(const char * c_szName, script::TArgList & rArgumentList)
{
	return GetArgumentString(c_szName, rArgumentList).c_str();
}

void GetCameraSettingFromArgList(script::TArgList & rArgList, IAbstractApplication::SCameraSetting * pCameraSetting)
{
	int32_t ix = atoi(GetArgument("x", rArgList));
	int32_t iy = atoi(GetArgument("y", rArgList));
	int32_t iz = atoi(GetArgument("z", rArgList));
	int32_t iUpDir = atoi(GetArgument("up", rArgList));
	int32_t iViewDir = atoi(GetArgument("view", rArgList));
	int32_t iCrossDir = atoi(GetArgument("cross", rArgList));
	int32_t iDistance = atoi(GetArgument("distance", rArgList));
	int32_t iRot = atoi(GetArgument("rot", rArgList));
	int32_t iPitch = atoi(GetArgument("pitch", rArgList));

	ZeroMemory(pCameraSetting, sizeof(IAbstractApplication::SCameraSetting));
	pCameraSetting->v3CenterPosition.x = float(ix);
	pCameraSetting->v3CenterPosition.y = float(iy);
	pCameraSetting->v3CenterPosition.z = float(iz);
	pCameraSetting->kCmrPos.m_fUpDir = float(iUpDir);
	pCameraSetting->kCmrPos.m_fViewDir = float(iViewDir);
	pCameraSetting->kCmrPos.m_fCrossDir = float(iCrossDir);
	pCameraSetting->fZoom = float(iDistance);
	pCameraSetting->fRotation = float(iRot);
	pCameraSetting->fPitch = float(iPitch);
}

void CPythonEventManager::__InitEventSet(TEventSet& rEventSet)
{
	rEventSet.ix = 0;
	rEventSet.iy = 0;
	rEventSet.iWidth = 0;
	rEventSet.iyLocal = 0;

	rEventSet.isLock = false;
	rEventSet.lLastDelayTime = 0;
	rEventSet.iCurrentLetter = 0;
	rEventSet.CurrentColor = D3DXCOLOR(1, 1, 1, 1);
	rEventSet.strCurrentLine.clear();

	rEventSet.pCurrentTextLine = nullptr;
	rEventSet.ScriptTextLineList.clear();

	rEventSet.isConfirmWait = FALSE;
	rEventSet.pConfirmTimeTextLine = nullptr;
	rEventSet.iConfirmEndTime = 0;

	rEventSet.DiffuseColor = D3DXCOLOR(1, 1, 1, 1);
	rEventSet.lWaitingTime = c_lNormal_Waiting_Time;
	rEventSet.iRestrictedCharacterCount = 30;

	rEventSet.iVisibleStartLine = 0;
	rEventSet.iVisibleLineCount = BOX_VISIBLE_LINE_COUNT;

	rEventSet.iAdjustLine = 0;

	rEventSet.isTextCenterMode = false;
	rEventSet.isWaitFlag = false;

	rEventSet.isQuestInfo = false;

	__InsertLine(rEventSet);
}

int32_t CPythonEventManager::RegisterEventSet(const char * c_szFileName)
{
	CFile File;
	if (!FileSystemManager::Instance().OpenFile(c_szFileName, File))
		return -1;

	std::string strEventString;
	strEventString.resize(File.GetSize()+1);

	File.Read(&strEventString[0], File.GetSize());

	TEventSet * pEventSet = m_EventSetPool.Alloc();
	if (!pEventSet)
		return -1;

	int32_t iScriptGroup = pEventSet->ScriptGroup.Create(strEventString);
	if (iScriptGroup <= 0)
	{
		__ClearEventSetp(pEventSet);
		return -1;
	}

	pEventSet->iTotalLineCount = iScriptGroup;

	strncpy_s(pEventSet->szFileName, c_szFileName, 32);

	pEventSet->pCurrentTextLine = nullptr;
	pEventSet->poEventHandler = nullptr;

	__InitEventSet(*pEventSet);

	int32_t iEmptySlotIndex = GetEmptyEventSetSlot();
	m_EventSetVector[iEmptySlotIndex] = pEventSet;
	return iEmptySlotIndex;
}

int32_t CPythonEventManager::RegisterEventSetFromString(const std::string& strScript, bool bIsQuestInfo)
{
	TEventSet* pEventSet = m_EventSetPool.Alloc();
	if (!pEventSet)
		return -1;

	// SCRIPT_PARSING_FAILURE_CLEAR_BUG 스크립트 파싱 실패시 __ClearEventSetp 에서 에러 발생
	pEventSet->pCurrentTextLine = nullptr;
	// END_OF_SCRIPT_PARSING_FAILURE_CLEAR_BUG
	
	int32_t iScriptGroup = pEventSet->ScriptGroup.Create(strScript);
	if (iScriptGroup <= 0)
	{
		__ClearEventSetp(pEventSet);
		return -1;
	}

	pEventSet->iTotalLineCount = iScriptGroup;

	pEventSet->szFileName[0] = 0;
	pEventSet->poEventHandler = nullptr;
	__InitEventSet(*pEventSet);
	pEventSet->isQuestInfo = bIsQuestInfo;

	// NOTE : 만약 단순한 스크립트 이벤트 실행 커맨드라면 다시 만든다.
	script::TCmd ScriptCommand;
	int32_t pEventPosition;
	int32_t iEventType;
	if (pEventSet->ScriptGroup.ReadCmd(ScriptCommand))
	{
		if (GetScriptEventIndex(ScriptCommand.name.c_str(), &pEventPosition, &iEventType))
		{
			if (EVENT_TYPE_RUN_CINEMA == iEventType)
			{
				std::string strFileName = GetArgument("value", ScriptCommand.argList);

				__ClearEventSetp(pEventSet);

				return RegisterEventSet(strFileName.c_str());
			}
		}
	}

	int32_t iEmptySlotIndex = GetEmptyEventSetSlot();
	m_EventSetVector[iEmptySlotIndex] = pEventSet;
	return iEmptySlotIndex;
}

void CPythonEventManager::ClearEventSeti(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	__ClearEventSetp(m_EventSetVector[iIndex]);
	m_EventSetVector[iIndex] = nullptr;
}

void CPythonEventManager::__ClearEventSetp(TEventSet * pEventSet)
{
	if (!pEventSet)
		return;

	for (auto & rkLine : pEventSet->ScriptTextLineList)
	{
		rkLine.pInstance->Destroy();
		m_ScriptTextLinePool.Free(rkLine.pInstance);
	}
	pEventSet->ScriptTextLineList.clear();

	if (pEventSet->pCurrentTextLine)
	{
		pEventSet->pCurrentTextLine->Destroy();
		m_ScriptTextLinePool.Free(pEventSet->pCurrentTextLine);
	}
	pEventSet->pCurrentTextLine = nullptr;
	pEventSet->strCurrentLine.clear();
	pEventSet->iCurrentLetter = 0;

	m_EventSetPool.Free(pEventSet);
}

uint32_t CPythonEventManager::GetEmptyEventSetSlot()
{
	for (uint32_t i = 0; i < m_EventSetVector.size(); ++i)
	{
		if (nullptr == m_EventSetVector[i])
			return i;
	}

	m_EventSetVector.emplace_back(nullptr);
	return m_EventSetVector.size()-1;
}

void CPythonEventManager::SetRestrictedCount(int32_t iIndex, int32_t iCount)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
	{
		TraceError("CPythonEventManager::SetRestrictedCount m_EventSetVector[iIndex=%d]==nullptr", iIndex);
		return;
	}

	pEventSet->iRestrictedCharacterCount = iCount;
}

void CPythonEventManager::SetEventHandler(int32_t iIndex, PyObject * poEventHandler)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
	{
		TraceError("CPythonEventManager::SetEventHandler m_EventSetVector[iIndex=%d]==nullptr", iIndex);
		return;
	}

	pEventSet->poEventHandler = poEventHandler;
}

int32_t CPythonEventManager::GetEventSetLocalYPosition(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return 0;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return 0;

	return pEventSet->iyLocal;
}

void CPythonEventManager::AddEventSetLocalYPosition(int32_t iIndex, int32_t iAddValue)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	pEventSet->iyLocal += iAddValue;
}

void CPythonEventManager::InsertText(int32_t iIndex, const char * c_szText,int32_t iX_pos)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	pEventSet->strCurrentLine = c_szText;
	if (pEventSet->pCurrentTextLine)
	{
		pEventSet->pCurrentTextLine->SetValue(c_szText);
		if (iX_pos == 0)
		{
			pEventSet->pCurrentTextLine->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		}
	}
	
	pEventSet->pCurrentTextLine->Update();
	__InsertLine(*pEventSet, FALSE, iX_pos);
}


void CPythonEventManager::UpdateEventSet(int32_t iIndex, int32_t ix, int32_t iy)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	pEventSet->ix = ix;
	pEventSet->iy = iy;

	RefreshLinePosition(pEventSet);

	if (pEventSet->isConfirmWait)
	{
		int32_t iLeftTime = std::max<int32_t>(0, pEventSet->iConfirmEndTime - timeGetTime()/1000);
		pEventSet->pConfirmTimeTextLine->SetValue(_getf(m_strLeftTimeString.c_str(), iLeftTime));
	}

	if (pEventSet->isWaitFlag)
		return;

	///////////////////////////////////////////////////////////////////////////////////////////////

	// Process EventSet
	int32_t lElapsedTime = CTimer::Instance().GetElapsedMilliecond();

	pEventSet->lLastDelayTime = std::max<int32_t>(0, pEventSet->lLastDelayTime - lElapsedTime);

	while (lElapsedTime > 0)
	{
		pEventSet->lLastDelayTime -= lElapsedTime;

		if (pEventSet->lLastDelayTime <= 0)
		{
			lElapsedTime = -pEventSet->lLastDelayTime;
			if (lElapsedTime <= 0)
				break;

			ProcessEventSet(pEventSet);

			if (pEventSet->lLastDelayTime < 0)
			{
				pEventSet->lLastDelayTime = 0;
				break;
			}
		}
		else
			break;
	}
}

void CPythonEventManager::SetEventSetWidth(int32_t iIndex, int32_t iWidth)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	pEventSet->iWidth = iWidth;
}

bool CPythonEventManager::ProcessEventSet(TEventSet * pEventSet)
{
	if (pEventSet->isLock)
		return false;

	script::TCmd ScriptCommand;

	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();

	if (!pEventSet->ScriptGroup.GetCmd(ScriptCommand))
	{
		pEventSet->isLock = true;
		return false;
	}

	int32_t pEventPosition;
	int32_t iEventType;
	if (!GetScriptEventIndex(ScriptCommand.name.c_str(), &pEventPosition, &iEventType))
		return false;

	switch (iEventType)
	{
		case EVENT_TYPE_LETTER:
		{
			const std::string& c_rstValue = GetArgumentString("value", ScriptCommand.argList);
			pEventSet->strCurrentLine.append(c_rstValue);
			pEventSet->pCurrentTextLine->SetValueString(pEventSet->strCurrentLine);
			pEventSet->pCurrentTextLine->SetColor(pEventSet->CurrentColor.r,pEventSet->CurrentColor.g,pEventSet->CurrentColor.b);
			pEventSet->iCurrentLetter+=c_rstValue.length();

			if (pEventSet->iCurrentLetter >= pEventSet->iRestrictedCharacterCount)
				__InsertLine(*pEventSet);

			pEventSet->lLastDelayTime = pEventSet->lWaitingTime;
			break;
		}

		case EVENT_TYPE_DELAY:
		{
			if (EVENT_POSITION_START == pEventPosition)
				pEventSet->lWaitingTime = atoi(GetArgument("value", ScriptCommand.argList));
			else
				pEventSet->lWaitingTime = c_lNormal_Waiting_Time;
			break;
		}

		case EVENT_TYPE_COLOR:
		{
			if (EVENT_POSITION_START == pEventPosition)
			{
			pEventSet->CurrentColor.r = static_cast<float>(atof(GetArgument("r", ScriptCommand.argList)));
			pEventSet->CurrentColor.g = static_cast<float>(atof(GetArgument("g", ScriptCommand.argList)));
			pEventSet->CurrentColor.b = static_cast<float>(atof(GetArgument("b", ScriptCommand.argList)));
				pEventSet->CurrentColor.a = 1.0f;
			}
			else
			{
				pEventSet->CurrentColor.r = 1.0f;
				pEventSet->CurrentColor.g = 1.0f;
				pEventSet->CurrentColor.a = 1.0f;
				pEventSet->CurrentColor.b = 1.0f;
			}
			break;
		}

		case EVENT_TYPE_COLOR256:
		{
			if (EVENT_POSITION_START == pEventPosition)
			{
				pEventSet->CurrentColor.r = float(atof(GetArgument("r", ScriptCommand.argList)) / 255.0f);
				pEventSet->CurrentColor.g = float(atof(GetArgument("g", ScriptCommand.argList)) / 255.0f);
				pEventSet->CurrentColor.b = float(atof(GetArgument("b", ScriptCommand.argList)) / 255.0f);
				pEventSet->CurrentColor.a = 1.0f;
			}
			else
			{
				pEventSet->CurrentColor.r = 1.0f;
				pEventSet->CurrentColor.g = 1.0f;
				pEventSet->CurrentColor.a = 1.0f;
				pEventSet->CurrentColor.b = 1.0f;
			}
			break;
		}

		case EVENT_TYPE_ENTER:
		{
			__InsertLine(*pEventSet);
			break;
		}

		case EVENT_TYPE_WAIT:
		{
			//캐틱터 만들기 제국 설명등에서 설명 밑으로 쳐지는 버그 수정/EVENT_TYPE_WAIT 관련해서 테스트 필요.
			pEventSet->iyLocal = 0;
			pEventSet->isLock = true;
			break;
		}

		case EVENT_TYPE_NEXT:
		{
			MakeNextButton(pEventSet, BUTTON_TYPE_NEXT);
			pEventSet->iAdjustLine += 2;
			break;
		}

		case EVENT_TYPE_DONE:
		{
			MakeNextButton(pEventSet, BUTTON_TYPE_DONE);
			PyCallClassMemberFunc(pEventSet->poEventHandler, "DoneEvent", Py_BuildValue("()"));
			pEventSet->iAdjustLine += 2;
			break;
		}

		case EVENT_TYPE_CLEAR:
		{
			ClearLine(pEventSet);
			break;
		}

		case EVENT_TYPE_QUESTION:
		{
			MakeQuestion(pEventSet, ScriptCommand.argList);
			break;
		}

		case EVENT_TYPE_LEFT_IMAGE:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnLeftImage", Py_BuildValue("(s)", GetArgument("src", ScriptCommand.argList)));
			break;
		}

		case EVENT_TYPE_TOP_IMAGE:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnTopImage", Py_BuildValue("(s)", GetArgument("src", ScriptCommand.argList)));
			break;
		}

		case EVENT_TYPE_BACKGROUND_IMAGE:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnBackgroundImage", Py_BuildValue("(s)",GetArgument("src", ScriptCommand.argList)));
			break;
		}

		case EVENT_TYPE_IMAGE:
		{
			int32_t x = atoi(GetArgument("x", ScriptCommand.argList));
			int32_t y = atoi(GetArgument("y", ScriptCommand.argList));
			const char * src = GetArgument("src", ScriptCommand.argList);

			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnImage", Py_BuildValue("(iis)", x, y, src));
			break;
		}

		case EVENT_TYPE_INSERT_IMAGE:
		{
			const std::string & imageFile = GetArgumentString("image_name", ScriptCommand.argList);
			const char * title = GetArgument("title", ScriptCommand.argList);
			const char * desc = GetArgument("desc", ScriptCommand.argList);
			int32_t index = atoi(GetArgument("index", ScriptCommand.argList));
			int32_t total = atoi(GetArgument("total", ScriptCommand.argList));

			if (imageFile.empty())
			{
				const char * imageType = GetArgument("image_type", ScriptCommand.argList);
				int32_t iItemIndex = atoi(GetArgument("idx", ScriptCommand.argList));
				PyCallClassMemberFunc(pEventSet->poEventHandler, "OnInsertItemIcon", Py_BuildValue("(sissii)", imageType, iItemIndex, title, desc, index, total));
			}
			else
			{
				PyCallClassMemberFunc(pEventSet->poEventHandler, "OnInsertImage", Py_BuildValue("(ssssii)", imageFile.c_str(), title, title, desc, index, total));
			}
			pEventSet->iAdjustLine += 2;
			break;
		}

		case EVENT_TYPE_ADD_MAP_SIGNAL:
		{
			auto x = static_cast<float>(atof(GetArgument("x", ScriptCommand.argList)));
			auto y = static_cast<float>(atof(GetArgument("y", ScriptCommand.argList)));
			CPythonMiniMap::Instance().AddSignalPoint(x,y);
			CPythonMiniMap::Instance().OpenAtlasWindow();
			break;
		}

		case EVENT_TYPE_CLEAR_MAP_SIGNAL:
		{
			CPythonMiniMap::Instance().ClearAllSignalPoint();
			break;
		}

		// HIDE_QUEST_LETTER
		case EVENT_TYPE_QUEST_BUTTON_CLOSE:
		{
			PyCallClassMemberFunc(m_poInterface, "BINARY_ClearQuest", 
				Py_BuildValue("(i)", atoi(GetArgument("idx", ScriptCommand.argList))));
			break;
		}
		// END_OF_HIDE_QUEST_LETTER
		
		case EVENT_TYPE_QUEST_BUTTON:
		{
			const std::string& c_rstType = GetArgumentString("icon_type", ScriptCommand.argList);
			const std::string& c_rstFile = GetArgumentString("icon_name", ScriptCommand.argList);

			int32_t idx = atoi(GetArgument("idx", ScriptCommand.argList));
			const char * name = GetArgument("name", ScriptCommand.argList);
			
			// 퀘스트 UI 리뉴얼이 되면 해결 되므로 일단 용혼석만 땜빵 by chrislee
			if (!strcmp(name, "조각난 용혼석?....")) {
				PyCallClassMemberFunc(m_poInterface, "BINARY_RecvQuest", Py_BuildValue("(isss)", idx, name, "highlight", ""));
			}
			else {
				if (c_rstFile.empty())
				{
					PyCallClassMemberFunc(m_poInterface, "RecvQuest", Py_BuildValue("(is)", idx, name));
				}
				else
				{
					PyCallClassMemberFunc(m_poInterface, "BINARY_RecvQuest", 
						Py_BuildValue("(isss)", idx, name, c_rstType.c_str(), c_rstFile.c_str()));
				}
			}
			break;
		}
		case EVENT_TYPE_SET_MESSAGE_POSITION:
		{
			break;
		}
		case EVENT_TYPE_ADJUST_MESSAGE_POSITION:
		{
			break;
		}
		case EVENT_TYPE_SET_CENTER_MAP_POSITION:
		{
			CPythonMiniMap::Instance().SetAtlasCenterPosition(atoi(GetArgument("x", ScriptCommand.argList)),atoi(GetArgument("y", ScriptCommand.argList)));
			break;
		}
		case EVENT_TYPE_SLEEP:
			pEventSet->lLastDelayTime = atoi(GetArgument("value", ScriptCommand.argList));
			break;
		case EVENT_TYPE_SET_CAMERA:
		{
			IAbstractApplication::SCameraSetting CameraSetting;
			GetCameraSettingFromArgList(ScriptCommand.argList, &CameraSetting);
			rApp.SetEventCamera(CameraSetting);
			break;
		}
		case EVENT_TYPE_BLEND_CAMERA:
		{
			IAbstractApplication::SCameraSetting CameraSetting;
			GetCameraSettingFromArgList(ScriptCommand.argList, &CameraSetting);

			float fBlendTime = atoi(GetArgument("blendtime", ScriptCommand.argList));

			rApp.BlendEventCamera(CameraSetting, fBlendTime);
			break;
		}
		case EVENT_TYPE_RESTORE_CAMERA:
		{
			rApp.SetDefaultCamera();
			break;
		}
		case EVENT_TYPE_FADE_OUT:
		{
		auto fSpeed = static_cast<float>(atof(GetArgument("speed", ScriptCommand.argList)));
			PyCallClassMemberFunc(pEventSet->poEventHandler, "FadeOut", Py_BuildValue("(f)", fSpeed));
			pEventSet->isWaitFlag = true;
			break;
		}
		case EVENT_TYPE_FADE_IN:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "FadeIn", Py_BuildValue("(f)", atof(GetArgument("speed", ScriptCommand.argList))));
			pEventSet->isWaitFlag = true;
			break;
		}
		case EVENT_TYPE_WHITE_OUT:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "WhiteOut", Py_BuildValue("(f)", atof(GetArgument("speed", ScriptCommand.argList))));
			pEventSet->isWaitFlag = true;
			break;
		}
		case EVENT_TYPE_WHITE_IN:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "WhiteIn", Py_BuildValue("(f)", atof(GetArgument("speed", ScriptCommand.argList))));
			pEventSet->isWaitFlag = true;
			break;
		}
		case EVENT_TYPE_CLEAR_TEXT:
		{
			ClearLine(pEventSet);
			break;
		}
		case EVENT_TYPE_TEXT_HORIZONTAL_ALIGN_CENTER:
		{
			pEventSet->isTextCenterMode = true;
			if (pEventSet->pCurrentTextLine)
				pEventSet->pCurrentTextLine->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
			break;
		}
		case EVENT_TYPE_TITLE_IMAGE:
		{
			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnTitleImage", Py_BuildValue("(s)", GetArgument("src", ScriptCommand.argList)));
			break;
		}
		case EVENT_TYPE_DUNGEON_RESULT:
		{
			int32_t killstone_count = atoi(GetArgument("killstone_count", ScriptCommand.argList));
			int32_t killmob_count = atoi(GetArgument("killmob_count", ScriptCommand.argList));
			int32_t find_hidden = atoi(GetArgument("find_hidden", ScriptCommand.argList));
			int32_t hidden_total = atoi(GetArgument("hidden_total", ScriptCommand.argList));
			int32_t use_potion = atoi(GetArgument("use_potion", ScriptCommand.argList));
			int32_t is_revived = atoi(GetArgument("is_revived", ScriptCommand.argList));
			int32_t killallmob = atoi(GetArgument("killallmob", ScriptCommand.argList));
			int32_t total_time = atoi(GetArgument("total_time", ScriptCommand.argList));
			int32_t bonus_exp = atoi(GetArgument("bonus_exp", ScriptCommand.argList));

			PyCallClassMemberFunc(m_poInterface, "ShowDungeonResult", 
								  Py_BuildValue("(iiiiiiiii)",
												killstone_count,
												killmob_count,
												find_hidden,
												hidden_total,
												use_potion,
												is_revived,
												killallmob,
												total_time,
												bonus_exp));
			break;
		}
		case EVENT_TYPE_ITEM_NAME:
		{
			int32_t iIndex = atoi(GetArgument("value", ScriptCommand.argList));
			CItemData * pItemData;
			if (CItemManager::Instance().GetItemDataPointer(iIndex, &pItemData))
			{
				pEventSet->strCurrentLine.append(pItemData->GetName());
				pEventSet->pCurrentTextLine->SetValue(pEventSet->strCurrentLine.c_str());
				pEventSet->pCurrentTextLine->SetColor(1.0f, 0.2f, 0.2f);
				pEventSet->iCurrentLetter+= strlen(pItemData->GetName());

				if (pEventSet->iCurrentLetter >= pEventSet->iRestrictedCharacterCount)
					__InsertLine(*pEventSet);

				pEventSet->lLastDelayTime = pEventSet->lWaitingTime;
			}

			break;
		}
		case EVENT_TYPE_MONSTER_NAME:
		{
			int32_t iIndex = atoi(GetArgument("value", ScriptCommand.argList));
			const char * c_szName;

			CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
			if (rkNonPlayer.GetName(iIndex, &c_szName))
			{
				pEventSet->strCurrentLine.append(c_szName);
				pEventSet->pCurrentTextLine->SetValue(pEventSet->strCurrentLine.c_str());
				pEventSet->iCurrentLetter+= strlen(c_szName);

				if (pEventSet->iCurrentLetter >= pEventSet->iRestrictedCharacterCount)
					__InsertLine(*pEventSet);

				pEventSet->lLastDelayTime = pEventSet->lWaitingTime;
			}

			break;
		}
		case EVENT_TYPE_WINDOW_SIZE:
		{
			int32_t iWidth = atoi(GetArgument("width", ScriptCommand.argList));
			int32_t iHeight = atoi(GetArgument("height", ScriptCommand.argList));
			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnSize", Py_BuildValue("(ii)", iWidth, iHeight));
			break;
		}
		case EVENT_TYPE_INPUT:
		{
			__InsertLine(*pEventSet);
			PyCallClassMemberFunc(pEventSet->poEventHandler, "OnInput", Py_BuildValue("()"));
			break;
		}
		case EVENT_TYPE_CONFIRM_WAIT:
		{
			int32_t iTimeOut = atoi(GetArgument("timeout", ScriptCommand.argList));
			pEventSet->isConfirmWait = TRUE;
			pEventSet->pConfirmTimeTextLine = pEventSet->pCurrentTextLine;
			pEventSet->iConfirmEndTime = timeGetTime()/1000 + iTimeOut;
			__InsertLine(*pEventSet, TRUE);
			MakeNextButton(pEventSet, BUTTON_TYPE_CANCEL);
			break;
		}
		case EVENT_TYPE_END_CONFIRM_WAIT:
		{
		for (auto & i : m_EventSetVector)
		{
			if (nullptr == i)
				continue;

			TEventSet * pSet = i;
				if (!pSet->isConfirmWait)
					continue;

				pSet->isConfirmWait = FALSE;
				pSet->pConfirmTimeTextLine = nullptr;
				pSet->iConfirmEndTime = 0;

				PyCallClassMemberFunc(pSet->poEventHandler, "CloseSelf", Py_BuildValue("()"));
			}
			break;
		}
		case EVENT_TYPE_SELECT_ITEM:
		{
			PyCallClassMemberFunc(m_poInterface, "BINARY_OpenSelectItemWindow", Py_BuildValue("()"));
			break;
		}
	}
	return true;
}

void CPythonEventManager::RenderEventSet(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	int32_t iCount = 0;

	for (auto & rkLine : pEventSet->ScriptTextLineList)
	{
		if (iCount < pEventSet->iVisibleStartLine)
			continue;

		if (iCount >= pEventSet->iVisibleStartLine + pEventSet->iVisibleLineCount)
			continue;

		CGraphicTextInstance * pInstance = rkLine.pInstance;

		pInstance->Update();
		pInstance->Render();
		iCount++;
	}

	if (iCount >= pEventSet->iVisibleStartLine)
	if (iCount < pEventSet->iVisibleStartLine + pEventSet->iVisibleLineCount)
	{
		pEventSet->pCurrentTextLine->Update();
		pEventSet->pCurrentTextLine->Render();
	}
}

void CPythonEventManager::Skip(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];

	if (true == pEventSet->isLock)
	{
		pEventSet->lLastDelayTime = 0;
		pEventSet->isLock = false;
	}
	else
	{
		pEventSet->lLastDelayTime = -1000;
	}
}

bool CPythonEventManager::IsWait(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return false;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
	{
		return false;
	}

	return pEventSet->isLock;
}

void CPythonEventManager::EndEventProcess(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
	{
		return;
	}

	pEventSet->isWaitFlag = false;
}

void CPythonEventManager::MakeNextButton(TEventSet * pEventSet, int32_t iButtonType)
{
	__AddSpace(*pEventSet, c_fLine_Temp+5);
	PyCallClassMemberFunc(pEventSet->poEventHandler, "MakeNextButton", Py_BuildValue("(i)", iButtonType));
}

void CPythonEventManager::MakeQuestion(TEventSet * pEventSet, script::TArgList & rArgumentList)
{
	if (rArgumentList.empty())
		return;

	PyCallClassMemberFunc(pEventSet->poEventHandler, "MakeQuestion", Py_BuildValue("(i)", rArgumentList.size()));
	pEventSet->nAnswer = rArgumentList.size();

	int32_t iIndex = 0;
	for (auto & rArgument : rArgumentList)
	{
		PyCallClassMemberFunc(pEventSet->poEventHandler, "AppendQuestion", Py_BuildValue("(si)", rArgument.strValue.c_str(), iIndex));
		++iIndex;
	}
}


void CPythonEventManager::SelectAnswer(int32_t iIndex, int32_t iAnswer)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	//TEventSet * pEventSet = m_EventSetVector[iIndex];

	CPythonNetworkStream::Instance().SendScriptAnswerPacket(iAnswer);
}

void CPythonEventManager::SetVisibleStartLine(int32_t iIndex, int32_t iStartLine)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];

	if (uint32_t(iStartLine) > pEventSet->ScriptTextLineList.size())
		return;

	pEventSet->iVisibleStartLine = iStartLine;
}

void CPythonEventManager::SetVisibleLineCount(int32_t iIndex, int32_t iLineCount)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];

	pEventSet->iVisibleLineCount = iLineCount;
}

int32_t CPythonEventManager::GetVisibleStartLine(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return 0;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	return pEventSet->iVisibleStartLine;
}

int32_t CPythonEventManager::GetLineCount(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return 0;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	return pEventSet->ScriptTextLineList.size()+pEventSet->iAdjustLine;
}

void CPythonEventManager::ClearLine(TEventSet * pEventSet)
{
	if (!pEventSet)
		return;

	for (auto & rkLine : pEventSet->ScriptTextLineList)
	{
		CGraphicTextInstance * pInstance = rkLine.pInstance;
		pInstance->Destroy();
		pInstance->Update();
	}

	pEventSet->pCurrentTextLine->Destroy();
	pEventSet->pCurrentTextLine->Update();

	// clear
	pEventSet->pCurrentTextLine = nullptr;
	pEventSet->ScriptTextLineList.clear();

	__InsertLine(*pEventSet);
}

void CPythonEventManager::__InsertLine(TEventSet& rEventSet, BOOL isCenter, int32_t iX_pos)
{
	if (rEventSet.pCurrentTextLine)
	{
		TTextLine kLine;
		if (CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER == rEventSet.pCurrentTextLine->GetHorizontalAlign())
		{
			kLine.ixLocal = rEventSet.iWidth/2;
			kLine.iyLocal = rEventSet.iyLocal;

		}
		else
		{
			int32_t textWidth;
			int32_t textHeight;
			rEventSet.pCurrentTextLine->GetTextSize(&textWidth,&textHeight);
			kLine.ixLocal = 0;
			if (iX_pos != 0)
			{
				kLine.ixLocal += (iX_pos - 20);
				kLine.ixLocal -= textWidth / 2;
			}

			kLine.iyLocal = rEventSet.iyLocal;
		}
		kLine.pInstance = rEventSet.pCurrentTextLine;
		rEventSet.ScriptTextLineList.emplace_back(kLine);
		__AddSpace(rEventSet, c_fLine_Temp);		
	}

	// DEFAULT_FONT
	//CGraphicText * pText = (CGraphicText*)CResourceManager::Instance().GetTypeResourcePointer(g_strDefaultFontName.c_str());
	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonEventManager::InsertLine - CANNOT_FIND_DEFAULT_FONT");
		return;
	}

	rEventSet.pCurrentTextLine = m_ScriptTextLinePool.Alloc();
	if (!rEventSet.pCurrentTextLine)
	{
		TraceError("CPythonEventManager::InsertLine - OUT_OF_TEXT_LINE");
		return;
	}

	rEventSet.pCurrentTextLine->SetTextPointer(pkDefaultFont);
	// END_OF_DEFAULT_FONT

	rEventSet.pCurrentTextLine->SetColor(1.0f, 1.0f, 1.0f);
	rEventSet.pCurrentTextLine->SetValue("");
	
	if (rEventSet.isTextCenterMode || isCenter)
	{
		rEventSet.pCurrentTextLine->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		rEventSet.pCurrentTextLine->SetPosition(rEventSet.ix+rEventSet.iWidth/2, rEventSet.iy + rEventSet.iyLocal);
	}
	else
	{
		rEventSet.pCurrentTextLine->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		rEventSet.pCurrentTextLine->SetPosition(rEventSet.ix, rEventSet.iy + rEventSet.iyLocal);	
	}

	rEventSet.iCurrentLetter = 0;
	rEventSet.strCurrentLine.clear();
}

void CPythonEventManager::RefreshLinePosition(TEventSet * pEventSet)
{
	//int32_t iCount = 0;
	for (auto & rkLine : pEventSet->ScriptTextLineList)
	{
		CGraphicTextInstance * pInstance = rkLine.pInstance;
		pInstance->SetPosition(pEventSet->ix + rkLine.ixLocal, pEventSet->iy + rkLine.iyLocal);
	}

	int32_t ixTextPos;
	if (CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER == pEventSet->pCurrentTextLine->GetHorizontalAlign())
	{
		ixTextPos = pEventSet->ix+pEventSet->iWidth/2;
	}
	else
	{
		ixTextPos = pEventSet->ix;
	}
	pEventSet->pCurrentTextLine->SetPosition(ixTextPos, pEventSet->iy + pEventSet->iyLocal);
}

void CPythonEventManager::__AddSpace(TEventSet& rEventSet, int32_t iSpace)
{
	rEventSet.iyLocal += iSpace;
}

bool CPythonEventManager::GetScriptEventIndex(const char * c_szName, int32_t * pEventPosition, int32_t * pEventType)
{
	const char * c_szEventName;

	if ('/' == c_szName[0])
	{
		*pEventPosition = EVENT_POSITION_END;
		c_szEventName = &c_szName[1];
	}
	else
	{
		*pEventPosition = EVENT_POSITION_START;
		c_szEventName = &c_szName[0];
	}
	
	auto it = EventTypeMap.find(c_szEventName);
	if (it == EventTypeMap.end())
	{
		Tracef(" !! PARSING ERROR - Strange Command : %s\n", c_szEventName);
		return false;
	}

	*pEventType = it->second;

	return true;
}

bool CPythonEventManager::CheckEventSetIndex(int32_t iIndex)
{
	if (iIndex < 0)
		return false;

	if (static_cast<uint32_t>(iIndex) >= m_EventSetVector.size())
		return false;

	return true;
}

void CPythonEventManager::Destroy()
{
	m_EventSetVector.clear();
	m_EventSetPool.Clear();
	m_ScriptTextLinePool.Clear();
}


int32_t CPythonEventManager::GetLineHeight(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return 0;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return 0;

	TTextLine & rkLine = pEventSet->ScriptTextLineList.front();
	CGraphicTextInstance * pInstance = rkLine.pInstance;
	return pInstance->GetLineHeight();
}

void CPythonEventManager::SetYPosition(int32_t iIndex, int32_t iY)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	pEventSet->iy = iY;
}

int32_t CPythonEventManager::GetProcessedLineCount(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return 0;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return 0;

	return pEventSet->ScriptTextLineList.size();
}

void CPythonEventManager::AllProcessEventSet(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	ProcessEventSet(pEventSet);
}

int32_t CPythonEventManager::GetTotalLineCount(int32_t iIndex)
{
	if (!CheckEventSetIndex(iIndex))
		return 0;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return 0;

	return pEventSet->iTotalLineCount;
}

void CPythonEventManager::SetInterfaceWindow(PyObject * poInterface)
{
	m_poInterface = poInterface;
}

void CPythonEventManager::SetLeftTimeString(const char * c_szString)
{
	m_strLeftTimeString = c_szString;
}

void CPythonEventManager::SetFontColor(int32_t iIndex, float r, float g, float b)
{
	if (!CheckEventSetIndex(iIndex))
		return;

	TEventSet * pEventSet = m_EventSetVector[iIndex];
	if (!pEventSet)
		return;

	if (pEventSet->pCurrentTextLine)
	{
		pEventSet->CurrentColor.r = r;
		pEventSet->CurrentColor.g = g;
		pEventSet->CurrentColor.b = b;
		pEventSet->pCurrentTextLine->SetColor(pEventSet->CurrentColor.r, pEventSet->CurrentColor.g, pEventSet->CurrentColor.b);
	}
}

CPythonEventManager::CPythonEventManager()
	: m_isQuestConfirmWait(0), m_poInterface(0), m_strLeftTimeString("남은 시간 : %d초")
{
	EventTypeMap["LETTER"]=EVENT_TYPE_LETTER;
	EventTypeMap["COLOR"]=EVENT_TYPE_COLOR;
	EventTypeMap["DELAY"]=EVENT_TYPE_DELAY;
	EventTypeMap["ENTER"]=EVENT_TYPE_ENTER;
	EventTypeMap["WAIT"]=EVENT_TYPE_WAIT;
	EventTypeMap["CLEAR"]=EVENT_TYPE_CLEAR;
	EventTypeMap["QUESTION"]=EVENT_TYPE_QUESTION;
	EventTypeMap["NEXT"]=EVENT_TYPE_NEXT;
	EventTypeMap["DONE"]=EVENT_TYPE_DONE;

	EventTypeMap["LEFTIMAGE"]=EVENT_TYPE_LEFT_IMAGE;
	EventTypeMap["TOPIMAGE"]=EVENT_TYPE_TOP_IMAGE;
	EventTypeMap["BGIMAGE"]=EVENT_TYPE_BACKGROUND_IMAGE;
	EventTypeMap["IMAGE"]=EVENT_TYPE_IMAGE;

	EventTypeMap["ADDMAPSIGNAL"]=EVENT_TYPE_ADD_MAP_SIGNAL;
	EventTypeMap["CLEARMAPSIGNAL"]=EVENT_TYPE_CLEAR_MAP_SIGNAL;

	EventTypeMap["SETMSGPOS"]=EVENT_TYPE_SET_MESSAGE_POSITION;
	EventTypeMap["ADJMSGPOS"]=EVENT_TYPE_ADJUST_MESSAGE_POSITION;
	EventTypeMap["SETCMAPPOS"]=EVENT_TYPE_SET_CENTER_MAP_POSITION;

	EventTypeMap["QUESTBUTTON"]=EVENT_TYPE_QUEST_BUTTON;

	// HIDE_QUEST_LETTER
	EventTypeMap["QUESTBUTTON_CLOSE"]=EVENT_TYPE_QUEST_BUTTON_CLOSE;
	// END_OF_HIDE_QUEST_LETTER

	EventTypeMap["SLEEP"]=EVENT_TYPE_SLEEP;
	EventTypeMap["SET_CAMERA"]=EVENT_TYPE_SET_CAMERA;
	EventTypeMap["BLEND_CAMERA"]=EVENT_TYPE_BLEND_CAMERA;
	EventTypeMap["RESTORE_CAMERA"]=EVENT_TYPE_RESTORE_CAMERA;
	EventTypeMap["FADE_OUT"]=EVENT_TYPE_FADE_OUT;
	EventTypeMap["FADE_IN"]=EVENT_TYPE_FADE_IN;
	EventTypeMap["WHITE_OUT"]=EVENT_TYPE_WHITE_OUT;
	EventTypeMap["WHITE_IN"]=EVENT_TYPE_WHITE_IN;
	EventTypeMap["CLEAR_TEXT"]=EVENT_TYPE_CLEAR_TEXT;
	EventTypeMap["TEXT_HORIZONTAL_ALIGN_CENTER"]=EVENT_TYPE_TEXT_HORIZONTAL_ALIGN_CENTER;
	EventTypeMap["TITLE_IMAGE"]=EVENT_TYPE_TITLE_IMAGE;

	EventTypeMap["RUN_CINEMA"]=EVENT_TYPE_RUN_CINEMA;
	EventTypeMap["DUNGEON_RESULT"]=EVENT_TYPE_DUNGEON_RESULT;

	EventTypeMap["ITEM"]=EVENT_TYPE_ITEM_NAME;
	EventTypeMap["MOB"]=EVENT_TYPE_MONSTER_NAME;

	EventTypeMap["COLOR256"]=EVENT_TYPE_COLOR256;
	EventTypeMap["WINDOW_SIZE"]=EVENT_TYPE_WINDOW_SIZE;

	EventTypeMap["INPUT"]=EVENT_TYPE_INPUT;
	EventTypeMap["CONFIRM_WAIT"]=EVENT_TYPE_CONFIRM_WAIT;
	EventTypeMap["END_CONFIRM_WAIT"]=EVENT_TYPE_END_CONFIRM_WAIT;

	EventTypeMap["INSERT_IMAGE"]=EVENT_TYPE_INSERT_IMAGE;

	EventTypeMap["SELECT_ITEM"]=EVENT_TYPE_SELECT_ITEM;
}

CPythonEventManager::~CPythonEventManager()
{
}
