#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonApplication.h"

/**
* 0 is NONE
* 1 is ALL
* 2 is ITEM ONLY
* 3 is CHAR ONLY
*/
#define DEFAULT_VALUE_DISPLAY_NAME_FLAG		1

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
	m_poInterfaceHandler = poHandler;
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = nullptr;
}

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

	LPDIRECT3D9 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

	lpD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_WHQL_LEVEL, &d3dAdapterIdentifier);
	lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddmDesktop);

	// 이 어뎁터가 가지고 있는 디스플래이 모드갯수를 나열한다..
	uint32_t dwNumAdapterModes = lpD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, d3ddmDesktop.Format);

	for (uint32_t iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
		lpD3D->EnumAdapterModes(D3DADAPTER_DEFAULT, d3ddmDesktop.Format, iMode, &DisplayMode);
		uint32_t bpp = 0;

		// 800 600 이상만 걸러낸다.
		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		// 일단 16bbp 와 32bbp만 취급하자.
		// 16bbp만 처리하게끔 했음 - [levites]
		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		bool check_res = false;

		for (int32_t i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			bool check_fre = false;

			// 프리퀀시만 다르므로 프리퀀시만 셋팅해준다.
			for (int32_t j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			// 새로운 거니까 추가해주자.
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
}

int32_t	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int32_t CPythonSystem::GetFrequencyCount(int32_t index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int32_t index, OUT uint32_t *width, OUT uint32_t *height, OUT uint32_t *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int32_t index, int32_t freq_index, OUT uint32_t *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int32_t	CPythonSystem::GetResolutionIndex(uint32_t width, uint32_t height, uint32_t bit)
{
	uint32_t re_width, re_height, re_bit;
	int32_t i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int32_t	CPythonSystem::GetFrequencyIndex(int32_t res_index, uint32_t frequency)
{
	uint32_t re_frequency;
	int32_t i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

uint32_t CPythonSystem::GetWidth()
{
	return m_Config.width;
}

uint32_t CPythonSystem::GetHeight()
{
	return m_Config.height;
}
uint32_t CPythonSystem::GetBPP()
{
	return 32/*m_Config.bpp*/;
}
uint32_t CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

float CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolume(float fVolume)
{
	m_Config.voice_volume = fVolume;
}

int32_t CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

int32_t CPythonSystem::GetShadowLevel()
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(uint32_t level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::Instance().RefreshShadowLevel();
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(const TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	memset(&m_Config, 0, sizeof(m_Config));

	m_Config.width				= 1024;
	m_Config.height				= 768;
	m_Config.bpp				= 32;

	m_Config.bWindowed			= true;
	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
	m_Config.music_volume		= 1.0f;
	m_Config.voice_volume = 1.0f;

	m_Config.bDecompressDDS = false;
	m_Config.bUseDefaultIME = false;
	m_Config.iShadowLevel		= 3;
	m_Config.bViewChat			= true;
	m_Config.bShowNameFlag		= DEFAULT_VALUE_DISPLAY_NAME_FLAG;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	m_Config.bShowMobAIFlag		= true;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	m_Config.bShowMobLevel		= true;
#endif
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int32_t iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

uint8_t CPythonSystem::GetShowNameFlag()
{
	return m_Config.bShowNameFlag;
}

void CPythonSystem::SetShowNameFlag(int32_t iFlag)
{
	m_Config.bShowNameFlag = (uint8_t)iFlag;
}

bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int32_t iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int32_t iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}

void CPythonSystem::SetShowMobAIFlagFlag(int32_t iFlag)
{
	m_Config.bShowMobAIFlag = iFlag == 1 ? true : false;
}
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobLevelFlag(int32_t iFlag)
{
	m_Config.bShowMobLevel = iFlag == 1 ? true : false;
}
#endif


bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

bool CPythonSystem::LoadConfig()
{
	msl::file_ptr fPtr("config/settings.cfg", "rt");

	if (!fPtr)
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fPtr.get()))
	{
#pragma warning(disable:4996)
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
#pragma warning(default:4996)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width		= atoi(value);
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height	= atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp		= atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME"))
				m_Config.music_volume = atof(value);
		else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = atof(value);
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
		else if (!stricmp(command, "WINDOWED"))
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_NAME_FLAG"))
			m_Config.bShowNameFlag = atoi(value) ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
#endif
	}

	if (m_Config.bWindowed)
	{
		RECT workArea;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

		uint32_t workAreaWidth = (workArea.right - workArea.left);
		uint32_t workAreaHeight = (workArea.bottom - workArea.top);

		uint32_t maxWidth = workAreaWidth - GetSystemMetrics(SM_CXBORDER) * 2 - GetSystemMetrics(SM_CXDLGFRAME) * 2 - GetSystemMetrics(SM_CXFRAME) * 2;
		uint32_t maxHeight = workAreaHeight - GetSystemMetrics(SM_CYBORDER) * 2 - GetSystemMetrics(SM_CYDLGFRAME) * 2 - GetSystemMetrics(SM_CYFRAME) * 2 - GetSystemMetrics(SM_CYCAPTION);

		if (m_Config.width > maxWidth)
			m_Config.width = maxWidth;
		if (m_Config.height > maxHeight)
			m_Config.height = maxHeight;

		if (m_Config.width < 800)
			m_Config.width = 800;

		if (m_Config.height < 600)
			m_Config.height = 600;
	}
	
	m_OldConfig = m_Config;
	return true;
}

bool CPythonSystem::SaveConfig()
{
	msl::file_ptr fPtr("config/settings.cfg", "wt");

	if (!fPtr)
		return false;

	fprintf(fPtr.get(), "WIDTH	%d\n", m_Config.width);
	fprintf(fPtr.get(), "HEIGHT	%d\n", m_Config.height);
	fprintf(fPtr.get(), "BPP	%d\n", m_Config.bpp);
	fprintf(fPtr.get(), "FREQUENCY	%d\n", m_Config.frequency);
	fprintf(fPtr.get(), "SOFTWARE_CURSOR	%d\n", m_Config.is_software_cursor);
	fprintf(fPtr.get(), "OBJECT_CULLING	%d\n", m_Config.is_object_culling);
	fprintf(fPtr.get(), "VISIBILITY	%d\n", m_Config.iDistance);
	fprintf(fPtr.get(), "MUSIC_VOLUME	%.3f\n", m_Config.music_volume);
	fprintf(fPtr.get(), "VOICE_VOLUME	%.3f\n", m_Config.voice_volume);
	fprintf(fPtr.get(), "GAMMA	%d\n", m_Config.gamma);
	fprintf(fPtr.get(), "DECOMPRESSED_TEXTURE	%d\n", m_Config.bDecompressDDS);

	fprintf(fPtr.get(), "WINDOWED	%d\n", m_Config.bWindowed);
	fprintf(fPtr.get(), "VIEW_CHAT	%d\n", m_Config.bViewChat);
	if (m_Config.bShowNameFlag != DEFAULT_VALUE_DISPLAY_NAME_FLAG)
		fprintf(fPtr.get(), "VIEW_NAME_FLAG		%d\n", m_Config.bShowNameFlag);
	fprintf(fPtr.get(), "SHOW_DAMAGE	%d\n", m_Config.bShowDamage);
	fprintf(fPtr.get(), "SHOW_SALESTEXT	%d\n", m_Config.bShowSalesText);
	fprintf(fPtr.get(), "USE_DEFAULT_IME	%d\n", m_Config.bUseDefaultIME);
	fprintf(fPtr.get(), "SHADOW_LEVEL	%d\n", m_Config.iShadowLevel);
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	fprintf(fPtr.get(), "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAIFlag);
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	fprintf(fPtr.get(), "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
#endif


	fprintf(fPtr.get(), "\n");

	return true;
}

void CPythonSystem::ApplyConfig() // 이전 설정과 현재 설정을 비교해서 바뀐 설정을 적용 한다.
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;
		
		switch (m_Config.gamma)
		{
		case 0:
			val = 0.4f;
			break;
		case 1:
			val = 0.7f;
			break;
		case 2:
			val = 1.0f;
			break;
		case 3:
			val = 1.2f;
			break;
		case 4:
			val = 1.4f;
			break;
		}
		
		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}

	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem()
{
	CSoundManager & rkSndMgr = CSoundManager::Instance();
	rkSndMgr.SetMusicVolume(m_Config.music_volume);
	rkSndMgr.SetSoundVolume(m_Config.voice_volume);
}

void CPythonSystem::Clear()
{
	SetInterfaceHandler(nullptr);
}

CPythonSystem::CPythonSystem()
{
	memset(&m_Config, 0, sizeof(TConfig));

	m_poInterfaceHandler = nullptr;

	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler==nullptr && "CPythonSystem MUST CLEAR!");
}
