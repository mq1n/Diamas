#include "StdAfx.h"

bool PyTuple_GetTextInstance(PyObject* poArgs, int32_t pos, CGraphicTextInstance** ppTextInstance)
{
	int32_t handle;
	if (!PyTuple_GetInteger(poArgs, pos, &handle))
		return false;

	if (!handle)
		return false;

	*ppTextInstance = reinterpret_cast<CGraphicTextInstance *>(handle);

	return true;
}

PyObject* grpTextGenerate(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance * pTextInstance = CGraphicTextInstance::New();
	return Py_BuildValue("i", pTextInstance);
}

PyObject* grpTextDestroy(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	CGraphicTextInstance::Delete(pTextInstance);
	return Py_BuildNone();
}

PyObject* grpTextGetSize(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	int32_t width, height;
	pTextInstance->GetTextSize(&width, &height);
	return Py_BuildValue("(i, i)", width, height);
}

PyObject* grpTextSetPosition(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	int32_t ix;
	if (!PyTuple_GetInteger(poArgs, 1, &ix))
		return Py_BuildException();
	int32_t iy;
	if (!PyTuple_GetInteger(poArgs, 2, &iy))
		return Py_BuildException();

	pTextInstance->SetPosition(static_cast<float>(ix), static_cast<float>(iy));
	return Py_BuildNone();
}

PyObject* grpTextSetText(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	char* szText;
	if (!PyTuple_GetString(poArgs, 1, &szText))
		return Py_BuildException();

	pTextInstance->SetValue(szText);
	return Py_BuildNone();
}

PyObject* grpTextSetSecret(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	bool bValue;
	if (!PyTuple_GetBoolean(poArgs, 1, &bValue))
		return Py_BuildException();

	pTextInstance->SetSecret(bValue);
	return Py_BuildNone();
}

PyObject* grpTextOutline(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	bool bValue;
	if (!PyTuple_GetBoolean(poArgs, 1, &bValue))
		return Py_BuildException();

	pTextInstance->SetOutline(bValue);
	return Py_BuildNone();
}

PyObject* grpTextGetText(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	return Py_BuildValue("s", pTextInstance->GetValueStringReference().c_str());
}

PyObject* grpTextSetFontName(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	char* szFontName;
	if (!PyTuple_GetString(poArgs, 1, &szFontName))
		return Py_BuildException();

	std::string stFontName = szFontName;
	stFontName += ".fnt";

	CResource* pResource = CResourceManager::Instance().GetResourcePointer(stFontName.c_str());
	pTextInstance->SetTextPointer(static_cast<CGraphicText*>(pResource));
	return Py_BuildNone();
}

PyObject* grpTextSetFontColor(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	if (2 == PyTuple_Size(poArgs))
	{
		int32_t iColor;
		if (!PyTuple_GetInteger(poArgs, 1, &iColor))
			return Py_BuildException();
		pTextInstance->SetColor(uint32_t(iColor));
	}
	else if (4 == PyTuple_Size(poArgs))
	{
		float fRed;
		if (!PyTuple_GetFloat(poArgs, 1, &fRed))
			return Py_BuildException();
		float fGreen;
		if (!PyTuple_GetFloat(poArgs, 2, &fGreen))
			return Py_BuildException();
		float fBlue;
		if (!PyTuple_GetFloat(poArgs, 3, &fBlue))
			return Py_BuildException();
		pTextInstance->SetColor(fRed, fGreen, fBlue);
	}
	else
	{
		return Py_BuildException();
	}

	return Py_BuildNone();
}

PyObject* grpTextSetOutLineColor(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	float fRed;
	if (!PyTuple_GetFloat(poArgs, 1, &fRed))
		return Py_BuildException();
	float fGreen;
	if (!PyTuple_GetFloat(poArgs, 2, &fGreen))
		return Py_BuildException();
	float fBlue;
	if (!PyTuple_GetFloat(poArgs, 3, &fBlue))
		return Py_BuildException();

	float fAlpha;
	if (!PyTuple_GetFloat(poArgs, 4, &fAlpha))
		return Py_BuildException();

	pTextInstance->SetOutLineColor(fRed, fGreen, fBlue, fAlpha);
	return Py_BuildNone();
}


PyObject* grpTextRender(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	pTextInstance->Render();
	return Py_BuildNone();
}

PyObject* grpTextUpdate(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();
	
	pTextInstance->Update();
	return Py_BuildNone();
}

PyObject* grpTextShowCursor(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	pTextInstance->ShowCursor();
	return Py_BuildNone();
}

PyObject* grpTextHideCursor(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();
	
	pTextInstance->HideCursor();
	return Py_BuildNone();
}

PyObject* grpSetHorizontalAlign(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	int32_t iAlign;
	if (!PyTuple_GetInteger(poArgs, 1, &iAlign))
		return Py_BuildException();

	pTextInstance->SetHorizonalAlign(iAlign);
	return Py_BuildNone();
}

PyObject* grpSetVerticalAlign(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	int32_t iAlign;
	if (!PyTuple_GetInteger(poArgs, 1, &iAlign))
		return Py_BuildException();

	pTextInstance->SetVerticalAlign(iAlign);
	return Py_BuildNone();
}

PyObject* grpSetMax(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance* pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	int32_t iValue;
	if (!PyTuple_GetInteger(poArgs, 1, &iValue))
		return Py_BuildException();

	pTextInstance->SetMax(iValue);
	return Py_BuildNone();
}

PyObject* grpGetSplitingTextLineCount(PyObject* poSelf, PyObject* poArgs)
{
	char * szText;
	if (!PyTuple_GetString(poArgs, 0, &szText))
		return Py_BuildException();

	int32_t iLineLimitation;
	if (!PyTuple_GetInteger(poArgs, 1, &iLineLimitation))
		return Py_BuildException();

	int32_t iPosition = 0;
	int32_t iLineCount = 0;

	for (size_t i = 0, textLen = strlen(szText); i < textLen;)
	{
		if ('|' == szText[i])
		{
			i += 1;
			
			// 자동 줄 바꿈되고 바로 | 가 있을 경우
			if (iPosition>0)
				++iLineCount;

			iPosition = 0;
			continue;
		}

		if (szText[i] & 0x80)
		{
			i += 2;
			iPosition += 2;
		}
		else
		{
			i += 1;
			iPosition += 1;
		}

		if (iPosition >= iLineLimitation)
		{
			++iLineCount;
			iPosition = 0;
		}
	}

	if (iPosition > 0)
		++iLineCount;

	return Py_BuildValue("i", iLineCount);
}

PyObject* grpGetSplitingTextLine(PyObject* poSelf, PyObject* poArgs)
{
	char * szText;
	if (!PyTuple_GetString(poArgs, 0, &szText))
		return Py_BuildException();

	int32_t iLineLimitation;
	if (!PyTuple_GetInteger(poArgs, 1, &iLineLimitation))
		return Py_BuildException();

	int32_t iGettingLine;
	if (!PyTuple_GetInteger(poArgs, 2, &iGettingLine))
		return Py_BuildException();

	std::string strLine = "";
	int32_t iPosition = 0;
	int32_t iLineCount = 0;

	// 1차 : 조금 더 깔끔하게 안될까 -_-a
	// 2차 : 오.. 좀 나아졌다 +_+
	for (size_t i = 0, textLen = strlen(szText); i < textLen;)
	{
		if ('|' == szText[i])
		{
			i += 1;

			// 자동 줄 바꿈되고 바로 | 가 있을 경우
			if (iPosition>0)
				++iLineCount;
			iPosition = 0;
			continue;
		}

		if (szText[i] & 0x80)
		{
			if (iLineCount == iGettingLine)
				strLine.append(szText+i, 2);

			i += 2;
			iPosition += 2;
		}
		else
		{
			if (iLineCount == iGettingLine)
				strLine += szText[i];

			i += 1;
			iPosition += 1;
		}

		if (iPosition >= iLineLimitation)
		{
			++iLineCount;
			iPosition = 0;
		}
	}

	return Py_BuildValue("s", strLine.c_str());
}

PyObject* grpTextPixelPositionToCharacterPosition(PyObject* poSelf, PyObject* poArgs)
{
	CGraphicTextInstance * pTextInstance;
	if (!PyTuple_GetTextInstance(poArgs, 0, &pTextInstance))
		return Py_BuildException();

	int32_t iPixelPosition;
	if (!PyTuple_GetInteger(poArgs, 1, &iPixelPosition))
		return Py_BuildException();

	int32_t iCharacterPosition = pTextInstance->PixelPositionToCharacterPosition(iPixelPosition);

	return Py_BuildValue("i", iCharacterPosition);
}

void initgrpText()
{
	static PyMethodDef s_methods[] =
	{
		{ "Update",								grpTextUpdate,								METH_VARARGS },
		{ "Render",								grpTextRender,								METH_VARARGS },
		{ "Generate",							grpTextGenerate,							METH_VARARGS },
		{ "Destroy",							grpTextDestroy,								METH_VARARGS },
		{ "SetFontName",						grpTextSetFontName,							METH_VARARGS },
		{ "SetFontColor",						grpTextSetFontColor,						METH_VARARGS },
		{ "SetOutLineColor",					grpTextSetOutLineColor,						METH_VARARGS },
		{ "SetSecret",							grpTextSetSecret,							METH_VARARGS },
		{ "SetOutline",							grpTextOutline,								METH_VARARGS },
		{ "SetText",							grpTextSetText,								METH_VARARGS },
		{ "GetText",							grpTextGetText,								METH_VARARGS },
		{ "GetSize",							grpTextGetSize,								METH_VARARGS },
		{ "SetPosition",						grpTextSetPosition,							METH_VARARGS },
		{ "ShowCursor",							grpTextShowCursor,							METH_VARARGS },
		{ "HideCursor",							grpTextHideCursor,							METH_VARARGS },
		{ "SetHorizontalAlign",					grpSetHorizontalAlign,						METH_VARARGS },
		{ "SetVerticalAlign",					grpSetVerticalAlign,						METH_VARARGS },
		{ "SetMax",								grpSetMax,									METH_VARARGS },
		{ "GetSplitingTextLineCount",			grpGetSplitingTextLineCount,				METH_VARARGS },
		{ "GetSplitingTextLine",				grpGetSplitingTextLine,						METH_VARARGS },
		{ "PixelPositionToCharacterPosition",	grpTextPixelPositionToCharacterPosition,	METH_VARARGS },
		{ nullptr, nullptr, 0 },
	};

	Py_InitModule("grpText", s_methods);
}
