#include "Stdafx.h"
#include "SoundManager2D.h"

CSoundManager2D::CSoundManager2D()
{
}

CSoundManager2D::~CSoundManager2D()
{	
}

bool CSoundManager2D::Initialize()
{
	CSoundBase::Initialize();

	if (ms_DIGDriver)
		return true;

	ms_DIGDriver = AIL_open_digital_driver(44100, 16, 2, 0);
	
	for (int32_t i = 0; i < INSTANCE_MAX_COUNT; ++i)
		ms_Instances[i].Initialize();
/*	ms_DIGDriver = AIL_open_digital_driver(44100,
										   (DIG_F_STEREO_16 & DIG_F_16BITS_MASK) ? 16 : 8,
										   (DIG_F_STEREO_16 & DIG_F_16BITS_MASK) ? 2 : 1,
										   0);
*/
	return true;
}

void CSoundManager2D::Destroy()
{	
	for (int32_t i = 0; i < INSTANCE_MAX_COUNT; ++i)
		ms_Instances[i].Destroy();
	
	if (ms_DIGDriver != nullptr)
	{
		AIL_close_digital_driver(ms_DIGDriver);
		ms_DIGDriver = nullptr;
	}	

	CSoundBase::Destroy();
}

ISoundInstance * CSoundManager2D::GetInstance(const char * c_pszFileName)
{
	uint32_t dwFileCRC = GetFileCRC(c_pszFileName);
	TSoundDataMap::iterator itor = ms_dataMap.find(dwFileCRC);

	CSoundData * pkSoundData;

	if (itor == ms_dataMap.end())
		pkSoundData = AddFile(dwFileCRC, c_pszFileName); // CSoundBase::AddFile
	else
		pkSoundData = itor->second;

	assert(pkSoundData != nullptr);

	static uint32_t k = 0;

	uint32_t start = k++;
	uint32_t end = start + INSTANCE_MAX_COUNT;

	while (start < end)
	{
		CSoundInstance2D * pkInst = &ms_Instances[start % INSTANCE_MAX_COUNT];

		if (pkInst->IsDone())
		{
			if (!pkInst->SetSound(pkSoundData))
				TraceError("CSoundManager2D::GetInstance (filename: %s)", c_pszFileName);
			return (pkInst);
		}

		++start;
	}

	return nullptr;
}
