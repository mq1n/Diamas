#include "StdAfx.h"
#include "../eterBase/Error.h"
#include "../eterLib/Camera.h"
#include "../eterLib/AttributeInstance.h"
#include "../eterGameLib/AreaTerrain.h"
#include "../eterGrnLib/Material.h"
#include "../eterWebBrowser/CWebBrowser.h"

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
	m_poMouseHandler(nullptr),
	m_fAveRenderTime(0.0f),
	m_fGlobalTime(0.0f),
	m_fGlobalElapsedTime(0.0f),
	m_dwUpdateFPS(0),
	m_dwRenderFPS(0),
	m_dwFaceCount(0),
	m_dwLButtonDownTime(0),
	m_bCursorVisible(TRUE),
	m_bLiarCursorOn(false),
	m_iCursorMode(CURSOR_MODE_HARDWARE),
	m_isWindowed(false),
	m_isFrameSkipDisable(false)
{
	CTimer::Instance().UseCustomTime();
	m_dwWidth = 800;
	m_dwHeight = 600;

	m_iGameStage = STAGE_NULL;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd=0.0f;

	m_dwFaceAccCount=0;
	m_dwFaceAccTime=0;

	m_dwFaceSpdSum=0;
	m_dwFaceSpdCount=0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable=false;
	else
		m_isFrameSkipDisable=true;
}

void CPythonApplication::GetInfo(uint32_t eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);			
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");
    PyThreadState * ts = PyThreadState_Get();
    PyFrameObject* frame = ts->frame;
    while (frame != 0)
    {
		char const* filename = PyString_AsString(frame->f_code->co_filename);
        char const* name = PyString_AsString(frame->f_code->co_name);
        TraceError("filename=%s, name=%s", filename, name);
        frame = frame->f_back;
    }

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}

void CPythonApplication::RenderGame()
{	
	{
		CPythonRenderTargetManager::Instance().RenderBackground();

		float fAspect=m_kWndMgr.GetAspect();
		float fFarClip=m_pyBackground.GetFarClip();

		m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);

		CCullingManager::Instance().Process();

		m_kChrMgr.Deform();
		m_kEftMgr.Update();
		CPythonRenderTargetManager::Instance().Deform();

		m_pyBackground.RenderCharacterShadowToTexture();

		m_pyGraphic.SetGameRenderState();
		m_pyGraphic.PushState();

		{
			int32_t lx, ly;
			m_kWndMgr.GetMousePosition(lx, ly);
			m_pyGraphic.SetCursorPosition(lx, ly);
		}

		m_pyBackground.RenderSky();

		m_pyBackground.RenderBeforeLensFlare();

		m_pyBackground.RenderCloud();

		m_pyBackground.BeginEnvironment();
		m_pyBackground.Render();

		m_pyBackground.SetCharacterDirLight();
		m_kChrMgr.Render();
		CPythonRenderTargetManager::Instance().RenderModel();

		m_pyBackground.SetBackgroundDirLight();
		m_pyBackground.RenderWater();
		m_pyBackground.RenderSnow();
		m_pyBackground.RenderEffect();

		m_pyBackground.EndEnvironment();

		m_kEftMgr.Render();
		m_pyItem.Render();
		m_FlyingManager.Render();

		m_pyBackground.BeginEnvironment();
		m_pyBackground.RenderPCBlocker();
		m_pyBackground.EndEnvironment();

		m_pyBackground.RenderAfterLensFlare();
	}
}

void CPythonApplication::UpdateGame()
{
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

	//!@# Alt+Tab 중 SetTransfor 에서 튕김 현상 해결을 위해 - [levites]
	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

		s.SetPerspective(30.0f,fAspect, 100.0f, fFarClip);
		s.BuildViewFrustum();
	}

	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	m_kChrMgr.Update();	

	m_kEftMgr.UpdateSound();

	m_FlyingManager.Update();

	m_pyItem.Update(ptMouse);

	m_pyPlayer.Update();

	CPythonRenderTargetManager::Instance().Update();

	// NOTE : Update 동안 위치 값이 바뀌므로 다시 얻어 옵니다 - [levites]
	//        이 부분 때문에 메인 케릭터의 Sound가 이전 위치에서 플레이 되는 현상이 있었음.
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

void CPythonApplication::SkipRenderBuffering(uint32_t dwSleepMSec)
{
	m_dwBufSleepSkipTime=ELTimer_GetMSec()+dwSleepMSec;
}

bool CPythonApplication::Process()
{
	// POLL EVENTS...

	ELTimer_SetFrameMSec();

	// 	m_Profiler.Clear();
	uint32_t dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static uint32_t	s_dwUpdateFrameCount = 0;
	static uint32_t	s_dwRenderFrameCount = 0;
	static uint32_t	s_dwFaceCount = 0;
	static uint32_t		s_uiLoad = 0;
	static uint32_t	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / std::max<uint32_t>(1, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

	// Update Time
	static BOOL s_bFrameSkip = FALSE;
	static uint32_t s_uiNextFrameTime = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime1=ELTimer_GetMSec();
#endif
	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	uint32_t uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;	//17 - 1초당 60fps기준.

	uint32_t updatestart = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime2=ELTimer_GetMSec();
#endif
	// Network I/O	
	m_pyNetworkStream.Process();	

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

#ifdef __PERFORMANCE_CHECK__		
	uint32_t dwUpdateTime3=ELTimer_GetMSec();
#endif
	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime4=ELTimer_GetMSec();
#endif
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);		
	}
	//////////////////////
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime5=ELTimer_GetMSec();
#endif
	//!@# Alt+Tab 중 SetTransfor 에서 튕김 현상 해결을 위해 - [levites]
	//if (m_isActivateWnd)
	__UpdateCamera();
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime6=ELTimer_GetMSec();
#endif
	// Update Game Playing
	CResourceManager::Instance().Update();
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime7=ELTimer_GetMSec();
#endif
	OnCameraUpdate();
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime8=ELTimer_GetMSec();
#endif
	OnMouseUpdate();
#ifdef __PERFORMANCE_CHECK__
	uint32_t dwUpdateTime9=ELTimer_GetMSec();
#endif
	OnUIUpdate();

#ifdef __PERFORMANCE_CHECK__		
	uint32_t dwUpdateTime10=ELTimer_GetMSec();

	if (dwUpdateTime10-dwUpdateTime1>10)
	{			
		static FILE* fp=fopen("perf_app_update.txt", "w");

		fprintf(fp, "AU.Total %u (Time %u)\n", dwUpdateTime9-dwUpdateTime1, ELTimer_GetMSec());
		fprintf(fp, "AU.TU %u\n", dwUpdateTime2-dwUpdateTime1);
		fprintf(fp, "AU.NU %u\n", dwUpdateTime3-dwUpdateTime2);
		fprintf(fp, "AU.KU %u\n", dwUpdateTime4-dwUpdateTime3);
		fprintf(fp, "AU.MP %u\n", dwUpdateTime5-dwUpdateTime4);
		fprintf(fp, "AU.CP %u\n", dwUpdateTime6-dwUpdateTime5);
		fprintf(fp, "AU.RU %u\n", dwUpdateTime7-dwUpdateTime6);
		fprintf(fp, "AU.CU %u\n", dwUpdateTime8-dwUpdateTime7);
		fprintf(fp, "AU.MU %u\n", dwUpdateTime9-dwUpdateTime8);
		fprintf(fp, "AU.UU %u\n", dwUpdateTime10-dwUpdateTime9);
		fprintf(fp, "----------------------------------\n");
		fflush(fp);
	}		
#endif

	//Update하는데 걸린시간.delta값
	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	uint32_t dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int32_t dt = dwCurrentTime - s_uiNextFrameTime;
		int32_t nAdjustTime = (static_cast<float>(dt) / static_cast<float>(uiFrameTime)) * uiFrameTime;

		if ( dt >= 500 )
		{
			s_uiNextFrameTime += nAdjustTime; 

			Tracenf("Process(): dt %d uiFrameTime %u nAdjustTime %d",
					dt, uiFrameTime, nAdjustTime);

			CTimer::Instance().Adjust(nAdjustTime);
		}

		if (!m_isFrameSkipDisable)
			s_bFrameSkip = true;

		bCurrentLateUpdate = TRUE;
	}

	//s_bFrameSkip = false;

	//if (dwCurrentTime > s_uiNextFrameTime)
	//{
	//	int32_t dt = dwCurrentTime - s_uiNextFrameTime;

	//	//너무 늦었을 경우 따라잡는다.
	//	//그리고 m_dwCurUpdateTime는 delta인데 delta랑 absolute time이랑 비교하면 어쩌자는겨?
	//	//if (dt >= 500 || m_dwCurUpdateTime > s_uiNextFrameTime)

	//	//기존코드대로 하면 0.5초 이하 차이난 상태로 update가 지속되면 계속 rendering frame skip발생
	//	if (dt >= 500 || m_dwCurUpdateTime > s_uiNextFrameTime)
	//	{
	//		s_uiNextFrameTime += dt / uiFrameTime * uiFrameTime; 
	//		printf("FrameSkip 보정 %d\n", dt / uiFrameTime * uiFrameTime);
	//		CTimer::Instance().Adjust((dt / uiFrameTime) * uiFrameTime);
	//		s_bFrameSkip = true;
	//	}
	//}

	if (m_isFrameSkipDisable)
		s_bFrameSkip = false;

	/*
	static bool s_isPrevFrameSkip=false;
	static uint32_t s_dwFrameSkipCount=0;
	static uint32_t s_dwFrameSkipEndTime=0;

	static uint32_t ERROR_FRAME_SKIP_COUNT = 60*5;
	static uint32_t ERROR_FRAME_SKIP_TIME = ERROR_FRAME_SKIP_COUNT*18;

	//static uint32_t MAX_FRAME_SKIP=0;

	if (IsActive())
	{
	uint32_t dwFrameSkipCurTime=ELTimer_GetMSec();

	if (s_bFrameSkip)
	{
	// 이전 프레임도 스킵이라면..
	if (s_isPrevFrameSkip)
	{
	if (s_dwFrameSkipEndTime==0)
	{
	s_dwFrameSkipCount=0; // 프레임 체크는 로딩 대비
	s_dwFrameSkipEndTime=dwFrameSkipCurTime+ERROR_FRAME_SKIP_TIME; // 시간 체크는 로딩후 프레임 스킵 체크

	//printf("FrameSkipCheck Start\n");
	}
	++s_dwFrameSkipCount;

	//if (MAX_FRAME_SKIP<s_dwFrameSkipCount)
	//	MAX_FRAME_SKIP=s_dwFrameSkipCount;

	//printf("u %d c %d/%d t %d\n", 
	//	dwUpdateTime9-dwUpdateTime1,
	//	s_dwFrameSkipCount, 
	//	MAX_FRAME_SKIP,
	//	s_dwFrameSkipEndTime);

	//#ifndef _DEBUG
	// 일정 시간동안 계속 프레임 스킵만 한다면...
	if (s_dwFrameSkipCount>ERROR_FRAME_SKIP_COUNT && s_dwFrameSkipEndTime<dwFrameSkipCurTime)
	{
	s_isPrevFrameSkip=false;
	s_dwFrameSkipEndTime=0;
	s_dwFrameSkipCount=0;

	//m_pyNetworkStream.AbsoluteExitGame();

	/*
	TraceError("무한 프레임 스킵으로 접속을 종료합니다");

	{
	FILE* fp=fopen("errorlog.txt", "w");
	if (fp)
	{
	fprintf(fp, "FRAMESKIP\n");
	fprintf(fp, "Total %d\n",		dwUpdateTime9-dwUpdateTime1);
	fprintf(fp, "Timer %d\n",		dwUpdateTime2-dwUpdateTime1);
	fprintf(fp, "Network %d\n",		dwUpdateTime3-dwUpdateTime2);
	fprintf(fp, "Keyboard %d\n",	dwUpdateTime4-dwUpdateTime3);
	fprintf(fp, "Controll %d\n",	dwUpdateTime5-dwUpdateTime4);
	fprintf(fp, "Resource %d\n",	dwUpdateTime6-dwUpdateTime5);
	fprintf(fp, "Camera %d\n",		dwUpdateTime7-dwUpdateTime6);
	fprintf(fp, "Mouse %d\n",		dwUpdateTime8-dwUpdateTime7);
	fprintf(fp, "UI %d\n",			dwUpdateTime9-dwUpdateTime8);
	fclose(fp);

	WinExec("errorlog.exe", SW_SHOW);
	}
	}
	}
	}

	s_isPrevFrameSkip=true;
	}
	else
	{
	s_isPrevFrameSkip=false;
	s_dwFrameSkipCount=0;
	s_dwFrameSkipEndTime=0;
	}
	}
	else
	{
	s_isPrevFrameSkip=false;
	s_dwFrameSkipCount=0;
	s_dwFrameSkipEndTime=0;
	}
	*/
	if (!s_bFrameSkip)
	{
		//		static double pos=0.0f;
		//		CGrannyMaterial::TranslateSpecularMatrix(fabs(sin(pos)*0.005), fabs(cos(pos)*0.005), 0.0f);
		//		pos+=0.01f;

		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		uint32_t dwRenderStartTime = ELTimer_GetMSec();		

		bool canRender = true;

		if (m_isMinimizedWnd)
		{
			canRender = false;
			Tracenf("Process(): Minimized, canRender %d", canRender);
		}
		else
		{
			if (m_pyGraphic.IsLostDevice())
			{
				CPythonBackground& rkBG = CPythonBackground::Instance();
				rkBG.ReleaseCharacterShadowTexture();

				CRenderTargetManager::Instance().ReleaseRenderTargetTextures();

				if (m_pyGraphic.RestoreDevice())
				{
					CRenderTargetManager::Instance().CreateRenderTargetTextures();
					rkBG.CreateCharacterShadowTexture();
				}
				else
				{
					canRender = false;
				}
				
				Tracenf("Process(): Lost device, canRender %d", canRender);
			}
		}

		if (!IsActive())
			SkipRenderBuffering(3000);

		// 리스토어 처리때를 고려해 일정 시간동안은 버퍼링을 하지 않는다
		if (!canRender)
			SkipRenderBuffering(3000);
		else
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin())
			{

				m_pyGraphic.ClearDepthBuffer();

#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();
				/////////////////////

				m_pyGraphic.End();

				//uint32_t t1 = ELTimer_GetMSec();
				m_pyGraphic.Show();
				//uint32_t t2 = ELTimer_GetMSec();

				uint32_t dwRenderEndTime = ELTimer_GetMSec();

				static uint32_t s_dwRenderCheckTime = dwRenderEndTime;
				static uint32_t s_dwRenderRangeTime = 0;
				static uint32_t s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;			
				s_dwRenderRangeTime += m_dwCurRenderTime;				
				++s_dwRenderRangeFrame;			

				if (dwRenderEndTime-s_dwRenderCheckTime>1000)
				{
					m_fAveRenderTime=float(double(s_dwRenderRangeTime)/double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime=ELTimer_GetMSec();
					s_dwRenderRangeTime=0;
					s_dwRenderRangeFrame=0;
				}										

				uint32_t dwCurFaceCount=m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					// 프레임 완충 처리
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{	
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						// 한계치를 정한다
						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						m_fAveRenderTime=s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd=(float(m_dwFaceAccCount)/m_dwFaceAccTime);

					// 거리 자동 조절
					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio=0.3f;
						s_fAveRenderTime=(s_fAveRenderTime*(100.0f-fRatio)+std::max<float>(16.0f, m_dwCurRenderTime)*fRatio)/100.0f;


						float fFar=25600.0f;
						float fNear=MIN_FOG;
						auto dbAvePow = double(1000.0f / s_fAveRenderTime);
						double dbMaxPow=60.0;
						float fDistance=std::max<float>(fNear+(fFar-fNear)*(dbAvePow)/dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
					// 거리 강제 설정시
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					// 10000 폴리곤 보다 적을때는 가장 멀리 보이게 한다
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	int32_t rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate )
	{
		s_uiLoad -= rest;	// 쉰 시간은 로드에서 뺀다..
		Sleep(rest);
	}	

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	//m_Profiler.ProfileByScreen();	
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{	
	m_poMouseHandler = poMouseHandler;
}

int32_t CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
		// 디바이스가 없으면 프로그램이 종료 되어야 한다.
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

		// DEVICESTATE_BROKEN일 때는 다음 루프에서 복구 될 수 있도록 리턴 한다.
		// 그냥 진행할 경우 DrawPrimitive 같은 것을 하면 프로그램이 터진다.
	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		if (!m_grpDevice.Reset())
			return DEVICE_STATE_SKIP;

		break;
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int32_t width, int32_t height, bool Windowed, int32_t bit /* = 32*/, int32_t frequency /* = 0*/)
{
	m_grpDevice.InitBackBufferCount(2);

	int32_t iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed, bit, frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		//PyErr_SetString(PyExc_RuntimeError, "DirectX 8.1 or greater required to run game");
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 9.0c or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		//PyErr_SetString(PyExc_RuntimeError, "GraphicDevice create failed");
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);
				TraceError("CreateDevice: Hardware Vertex Processing not available"); //LogBox("Your display Adapter doesnt support TNL", nullptr, GetWindowHandle());
			}
			return true;
		}

		//PyErr_SetString(PyExc_RuntimeError, "Unknown Error!");
		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{	
	while (true)
	{	
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;
		}
	}
}

// SUPPORT_NEW_KOREA_SERVER
bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer&	rkNPCMgr	= CPythonNonPlayer::Instance();
	CItemManager&		rkItemMgr	= CItemManager::Instance();	
	CPythonSkill&		rkSkillMgr	= CPythonSkill::Instance();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];	
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
	_snprintf_s(szItemList,				sizeof(szItemList) ,			"%s/item_list.txt",	localePath);
	_snprintf_s(szItemProto,			sizeof(szItemProto),			"%s/item_proto",	localePath);
	_snprintf_s(szItemDesc,				sizeof(szItemDesc),				"%s/itemdesc.txt",	localePath);
	_snprintf_s(szMobProto,				sizeof(szMobProto),				"%s/mob_proto",		localePath);
	_snprintf_s(szSkillDescFileName,	sizeof(szSkillDescFileName),	"%s/SkillDesc.txt", localePath);
	_snprintf_s(szSkillTableFileName,	sizeof(szSkillTableFileName),	"%s/SkillTable.txt",localePath);
	_snprintf_s(szInsultList,			sizeof(szInsultList),			"%s/insult.txt",	localePath);

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();	
	rkSkillMgr.Destroy();

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}	

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);	
	}

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szMobProto);
		return false;
	}

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);				
	}

#ifdef ENABLE_ACCE_SYSTEM
	char szItemScale[256]{};
	snprintf(szItemScale, sizeof(szItemScale), "acce/item_scale.txt");

	if (!rkItemMgr.LoadItemScale(szItemScale))
	{
		Tracenf("LoadLocaleData: error while loading %s.", szItemScale);
		return false;
	}
#endif

	return true;
}
// END_OF_SUPPORT_NEW_KOREA_SERVER

uint32_t __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION |   WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject * poSelf, const char * c_szName, int32_t width, int32_t height)
{
	bool Windowed = CPythonSystem::Instance().IsWindowed();

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	uint32_t WindowMode = __GetWindowMode(Windowed);

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon( GetInstance(), MAKEINTRESOURCE( IDI_METIN2 ) ), IDC_CURSOR_NORMAL))
	{
		//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Create failed");
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
		CPythonIME::Instance().UseDefaultIME();

	// 풀스크린 모드이고
	// 디폴트 IME 를 사용하거나 유럽 버전이면
	// 윈도우 풀스크린 모드를 사용한다
	if (!m_pySystem.IsWindowed())
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;

			RECT workArea;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

			uint32_t workAreaWidth = (workArea.right - workArea.left);
			uint32_t workAreaHeight = (workArea.bottom - workArea.top);

			uint32_t windowWidth = m_pySystem.GetWidth() + GetSystemMetrics(SM_CXBORDER) * 2 + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CXFRAME) * 2;
			uint32_t windowHeight = m_pySystem.GetHeight() + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

			uint32_t x = workAreaWidth / 2 - windowWidth / 2;
			uint32_t y = workAreaHeight / 2 - windowHeight / 2;

			SetPosition(x, y);
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Cursor
		if (!CreateCursors())
		{
			//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Cursors Create Error");
			TraceError("CMSWindow::Cursors Create Error");
			SET_EXCEPTION("CREATE_CURSOR");
			return false;
		}

		if (!m_pySystem.IsNoSoundCard())
		{
			// Sound
			if (!m_SoundManager.Create())
			{
				// NOTE : 중국측의 요청으로 생략
				//		LogBox((IDS_WARN_NO_SOUND_DEVICE));
			}
		}

		// Device
		if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
			return false;

		GrannyCreateSharedDeformBuffer();

		SetVisibleMode(true);

		if (m_isWindowFullScreenEnable) //m_pySystem.IsUseDefaultIME() && !m_pySystem.IsWindowed())
			SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);

		if (!InitializeKeyboard(GetWindowHandle()))
			return false;

		m_pySystem.GetDisplaySettings();

		// Mouse
		if (m_pySystem.IsSoftwareCursor())
			SetCursorMode(CURSOR_MODE_SOFTWARE);
		else
			SetCursorMode(CURSOR_MODE_HARDWARE);

		// Network
		if (!m_netDevice.Create())
		{
			//PyErr_SetString(PyExc_RuntimeError, "NetDevice::Create failed");
			TraceError("NetDevice::Create failed");
			SET_EXCEPTION("CREATE_NETWORK");
			return false;
		}

		if (!m_grpDevice.IsFastTNL())
			CGrannyLODController::SetMinLODMode(true);

#ifdef ENABLE_ANTICHEAT
		CAnticheatManager::Instance().InitializeAnticheatRoutines(GetInstance(), GetWindowHandle());
#endif

		m_pyItem.Create();

		// Other Modules
		DefaultFont_Startup();

		CPythonIME::Instance().Create(GetWindowHandle());
		CPythonIME::Instance().SetText("", 0);
		CPythonTextTail::Instance().Initialize();

		// Light Manager
		m_LightManager.Initialize();

		CGraphicImageInstance::CreateSystem(32);

		// 백업
		STICKYKEYS sStickKeys;
		memset(&sStickKeys, 0, sizeof(sStickKeys));
		sStickKeys.cbSize = sizeof(sStickKeys);
		SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
		m_dwStickyKeysFlag = sStickKeys.dwFlags;

		// 설정
		sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );

		// SphereMap
		CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
		CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");

		// Render Target
		for (int32_t i = 0; i < CPythonRenderTargetManager::RENDER_TYPE_MAX_NUM; i++) {
			if (!CRenderTargetManager::Instance().CreateRenderTargetWithIndex(m_dwWidth, m_dwHeight, i))
				TraceError("WARNING - Could not create render target (%d, %d, %d).", m_dwWidth, m_dwHeight, i);
		}

		return true;
}

void CPythonApplication::SetGlobalCenterPosition(int32_t x, int32_t y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}


void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

// 2005.03.28 - MALL 아이템에 들어있는 시간의 단위가 서버에서 time(0) 으로 만들어지는
//              값이기 때문에 단위를 맞추기 위해 시간 관련 처리를 별도로 추가
time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int32_t iFPS)
{
	m_iFPS = iFPS;
}

int32_t CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int32_t CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int32_t iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int32_t & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int32_t iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int32_t iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::SetTitle(const char* szTitle)
{
	CMSWindow::SetText(szTitle);
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

void CPythonApplication::FlashApplication()
{
   HWND hWnd = GetWindowHandle();
   FLASHWINFO fi;
   fi.cbSize = sizeof(FLASHWINFO);
   fi.hwnd = hWnd;
   fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
   fi.uCount = 0;
   fi.dwTimeout = 0;
   FlashWindowEx(&fi);
}

int32_t CPythonApplication::GetGameStage() const
{
	return m_iGameStage;
}

void CPythonApplication::SetGameStage(int32_t iNewStage)
{
	m_iGameStage = iNewStage;
}

void CPythonApplication::Destroy()
{
	WebBrowser_Destroy();

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

	DestroyCollisionInstanceSystem();

	CRenderTargetManager::Instance().Destroy();
	CPythonRenderTargetManager::Instance().Destroy();

	m_pyEventManager.Destroy();	
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();	
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();


	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	// FIXME : 만들어져 있지 않음 - [levites]
	//CSpeedTreeForestDirectX9::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}

LPDIRECT3D9 CPythonApplication::GetDirectx9()
{
	return m_grpDevice.GetDirectx9();
}

LPDIRECT3DDEVICE9 CPythonApplication::GetDevice()
{
	return m_grpDevice.GetDevice();
}
