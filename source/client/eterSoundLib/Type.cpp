#include "StdAfx.h"
#include "Type.h"
#include "../eterLib/TextFileLoader.h"

std::string NSound::strResult;

const char * NSound::GetResultString()
{
	return strResult.c_str();
}

void NSound::SetResultString(const char * c_pszStr)
{
	strResult.assign(c_pszStr);
}

bool NSound::LoadSoundInformationPiece(const char * c_szFileName, NSound::TSoundDataVector & rSoundDataVector, const char * c_szPathHeader, bool silent_failure)
{
	std::string strResult;
	strResult = c_szFileName;

	CTextFileLoader* pkTextFileLoader=CTextFileLoader::Cache(c_szFileName, silent_failure);
	if (!pkTextFileLoader)
		return false;

	CTextFileLoader& rkTextFileLoader=*pkTextFileLoader;
	if (rkTextFileLoader.IsEmpty())
	{
		SetResultString((strResult + " Can not open file for reading").c_str());
		return false;
	}

	rkTextFileLoader.SetTop();

	int32_t iCount;
	if (!rkTextFileLoader.GetTokenInteger("sounddatacount", &iCount))
	{
		SetResultString((strResult + " File format error, SoundDataCount Unable to find.").c_str());
		return false;
	}

	rSoundDataVector.clear();
	rSoundDataVector.resize(iCount);

	char szSoundDataHeader[32+1];
	for (uint32_t i = 0; i < rSoundDataVector.size(); ++i)
	{
		_snprintf_s(szSoundDataHeader, sizeof(szSoundDataHeader), "sounddata%02u", i);
		CTokenVector * pTokenVector;
		if (!rkTextFileLoader.GetTokenVector(szSoundDataHeader, &pTokenVector))
		{
			SetResultString((strResult + " File format error: " + szSoundDataHeader + " Unable to find").c_str());
			return false;
		}

		if (2 != pTokenVector->size())
		{
			SetResultString((strResult + " File format error: The size of the vector is not 2").c_str());
			return false;
		}

		rSoundDataVector[i].fTime = (float) atof(pTokenVector->at(0).c_str());
		if (c_szPathHeader)
		{
			rSoundDataVector[i].strSoundFileName = c_szPathHeader;
			rSoundDataVector[i].strSoundFileName += pTokenVector->at(1);
		}
		else
		{
			rSoundDataVector[i].strSoundFileName = pTokenVector->at(1);
		}
	}

	SetResultString((strResult + " Loaded").c_str());
	return true;
}

bool NSound::SaveSoundInformationPiece(const char * c_szFileName, NSound::TSoundDataVector & rSoundDataVector)
{
	if (rSoundDataVector.empty())	// 데이터가 없으면 성공으로 간주
	{
		if (IsFile(c_szFileName))	// 데이터는 비어있는데 파일이 있다면
		{
			_unlink(c_szFileName);		// 지운다.
		}
		return true;
	}

	std::string strResult;
	strResult = c_szFileName;

	msl::file_ptr fPtr(c_szFileName, "wt");

	if (!fPtr)
	{
		char szErrorText[256+1];
		_snprintf_s(szErrorText, sizeof(szErrorText), "Failed to save file (%s).\nPlease check if it is read-only or you have no space on the disk.\n", c_szFileName);
		LogBox(szErrorText, "Error");
		SetResultString((strResult + " Cannot open file for writing").c_str());
		return false;
	}

	fprintf(fPtr.get(), "ScriptType        CharacterSoundInformation\n");
	fprintf(fPtr.get(), "\n");

	fprintf(fPtr.get(), "SoundDataCount    %u\n", rSoundDataVector.size());

	for (uint32_t i = 0; i < rSoundDataVector.size(); ++i)
	{
		NSound::TSoundData & rSoundData = rSoundDataVector[i];
		fprintf(fPtr.get(), "SoundData%02u       %f \"%s\"\n", i, rSoundData.fTime, rSoundData.strSoundFileName.c_str());
	}
	return true;
}

void NSound::DataToInstance(const TSoundDataVector & c_rSoundDataVector, TSoundInstanceVector * pSoundInstanceVector)
{
	if (c_rSoundDataVector.empty())
		return;

	uint32_t dwFPS = 60;
	const float c_fFrameTime = 1.0f / float(dwFPS);

	pSoundInstanceVector->clear();
	pSoundInstanceVector->resize(c_rSoundDataVector.size());
	for (uint32_t i = 0; i < c_rSoundDataVector.size(); ++i)
	{
		const TSoundData & c_rSoundData = c_rSoundDataVector[i];
		TSoundInstance & rSoundInstance = pSoundInstanceVector->at(i);

		rSoundInstance.dwFrame = (uint32_t) (c_rSoundData.fTime / c_fFrameTime);
		rSoundInstance.strSoundFileName = c_rSoundData.strSoundFileName;
	}
}