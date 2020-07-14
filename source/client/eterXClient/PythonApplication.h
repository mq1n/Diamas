#pragma once
#include "../../common/defines.h"

#include "../eterLib/MSApplication.h"
#include "../eterLib/Input.h"
#include "../eterLib/GrpDevice.h"
#include "../eterLib/NetDevice.h"
#include "../eterLib/GrpLightManager.h"
#include "../eterLib/RenderTargetManager.h"
#include "../eterEffectLib/EffectManager.h"
#include "../eterGameLib/RaceManager.h"
#include "../eterGameLib/ItemManager.h"
#include "../eterGameLib/FlyingObjectManager.h"
#include "../eterGameLib/GameEventManager.h"
#include "../eterSoundLib/SoundManager.h"
#include "../eterSecurity/PythonStackCheck.h"
#include "../eterSecurity/AnticheatManager.h"

#include "PythonDynamicModuleNames.h"
#include "PythonEventManager.h"
#include "PythonPlayer.h"
#include "PythonNonPlayer.h"
#include "PythonMiniMap.h"
#include "PythonIME.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonChat.h"
#include "PythonTextTail.h"
#include "PythonSkill.h"
#include "PythonSystem.h"
#include "PythonNetworkStream.h"
#include "PythonCharacterManager.h"
#include "PythonQuest.h"
#include "PythonMessenger.h"
#include "PythonSafeBox.h"
#include "PythonGuild.h"
#include "PythonRenderTargetManager.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"

#include "AccountConnector.h"

#include "ServerStateChecker.h"
#include "AbstractApplication.h"
#ifdef ENABLE_ACCE_SYSTEM
#include "PythonAcce.h"
#endif

class CPythonApplication : public CMSApplication, public CInputKeyboard, public IAbstractApplication
{
	public:
		enum EDeviceState
		{
			DEVICE_STATE_FALSE,
			DEVICE_STATE_SKIP,
			DEVICE_STATE_OK
		};

		enum ECursorMode
		{
			CURSOR_MODE_HARDWARE,
			CURSOR_MODE_SOFTWARE
		};

		enum ECursorShape
		{
			CURSOR_SHAPE_NORMAL,
			CURSOR_SHAPE_ATTACK,
			CURSOR_SHAPE_TARGET,
			CURSOR_SHAPE_TALK,
			CURSOR_SHAPE_CANT_GO,
			CURSOR_SHAPE_PICK,

			CURSOR_SHAPE_DOOR,
			CURSOR_SHAPE_CHAIR,
			CURSOR_SHAPE_MAGIC,				// Magic
			CURSOR_SHAPE_BUY,				// Buy
			CURSOR_SHAPE_SELL,				// Sell

			CURSOR_SHAPE_CAMERA_ROTATE,		// Camera Rotate
			CURSOR_SHAPE_HSIZE,				// Horizontal Size
			CURSOR_SHAPE_VSIZE,				// Vertical Size
			CURSOR_SHAPE_HVSIZE,			// Horizontal & Vertical Size

			CURSOR_SHAPE_COUNT,

			// 안정적인 네이밍 변환을 위한 임시 enumerate
			NORMAL = CURSOR_SHAPE_NORMAL,
			ATTACK = CURSOR_SHAPE_ATTACK,
			TARGET = CURSOR_SHAPE_TARGET,
			CAMERA_ROTATE = CURSOR_SHAPE_CAMERA_ROTATE,
			CURSOR_COUNT = CURSOR_SHAPE_COUNT
		};

		enum EInfo
		{
			INFO_ACTOR,
			INFO_EFFECT,
			INFO_ITEM,
			INFO_TEXTTAIL
		};

		enum ECameraControlDirection
		{
			CAMERA_TO_POSITIVE = 1,
			CAMERA_TO_NEGITIVE = -1,
			CAMERA_STOP = 0
		};

		enum
		{
			CAMERA_MODE_NORMAL = 0,
			CAMERA_MODE_STAND = 1,
			CAMERA_MODE_BLEND = 2,
			CAMERA_MODE_FREE = 3,

			EVENT_CAMERA_NUMBER = 101
		};

		struct SCameraSpeed
		{
			float m_fUpDir;
			float m_fViewDir;
			float m_fCrossDir;

			SCameraSpeed() : m_fUpDir(0.0f), m_fViewDir(0.0f), m_fCrossDir(0.0f) {}
		};

	public:
		CPythonApplication();
		virtual ~CPythonApplication();

	public:
		void ShowWebPage(const char* c_szURL, const RECT& c_rcWebPage);
		void MoveWebPage(const RECT& c_rcWebPage);
		void HideWebPage();

		bool IsWebPageMode();

	public:
		void GetInfo(uint32_t eInfo, std::string* pstInfo);
		void GetMousePosition(POINT* ppt);
		
		static CPythonApplication& Instance()
		{
			assert(ms_pInstance != nullptr);
			return *ms_pInstance;
		}

		void Loop();
		void Destroy();
		void Clear();
		void Exit();
		void Abort();

		void SetMinFog(float fMinFog);
		void SetFrameSkip(bool isEnable);
		void SkipRenderBuffering(uint32_t dwSleepMSec);

		bool Create(PyObject* poSelf, const char* c_szName, int32_t width, int32_t height);
		bool CreateDevice(int32_t width, int32_t height, bool Windowed, int32_t bit = 32, int32_t frequency = 0);

		void UpdateGame();
		void RenderGame();

		bool Process();

		void UpdateClientRect();

		bool CreateCursors();
		void DestroyCursors();

		void SafeSetCapture();
		void SafeReleaseCapture();

		BOOL SetCursorNum(int32_t iCursorNum);
		void SetCursorVisible(BOOL bFlag, bool bLiarCursorOn = false);
		BOOL GetCursorVisible();
		bool GetLiarCursorOn();
		void SetCursorMode(int32_t iMode);
		int32_t GetCursorMode();
		int32_t GetCursorNum() { return m_iCursorNum; }

		void SetMouseHandler(PyObject * poMouseHandler);

		int32_t GetWidth();
		int32_t GetHeight();

		void SetGlobalCenterPosition(int32_t x, int32_t y);
		void SetCenterPosition(float fx, float fy, float fz);
		void GetCenterPosition(TPixelPosition * pPixelPosition);
		void SetCamera(float Distance, float Pitch, float Rotation, float fDestinationHeight);
		void GetCamera(float * Distance, float * Pitch, float * Rotation, float * DestinationHeight);
		void RotateCamera(int32_t iDirection);
		void PitchCamera(int32_t iDirection);
		void ZoomCamera(int32_t iDirection);
		void MovieRotateCamera(int32_t iDirection);
		void MoviePitchCamera(int32_t iDirection);
		void MovieZoomCamera(int32_t iDirection);
		void MovieResetCamera();
		void SetViewDirCameraSpeed(float fSpeed);
		void SetCrossDirCameraSpeed(float fSpeed);
		void SetUpDirCameraSpeed(float fSpeed);
		float GetRotation();
		float GetPitch();

		void SetFPS(int32_t iFPS);
		void SetServerTime(time_t tTime);
		time_t GetServerTime();
		time_t GetServerTimeStamp();
		float GetGlobalTime();
		float GetGlobalElapsedTime();

		float GetFaceSpeed()		{ return m_fFaceSpd; }
		float GetAveRenderTime()	{ return m_fAveRenderTime; }
		uint32_t GetCurRenderTime()	{ return m_dwCurRenderTime; }
		uint32_t GetCurUpdateTime()	{ return m_dwCurUpdateTime; }
		uint32_t GetUpdateFPS()		{ return m_dwUpdateFPS; }
		uint32_t GetRenderFPS()		{ return m_dwRenderFPS; }
		uint32_t GetLoad()			{ return m_dwLoad; }
		uint32_t GetFaceCount()	{ return m_dwFaceCount; }

		void SetConnectData(const char * c_szIP, int32_t iPort);
		void GetConnectData(std::string & rstIP, int32_t & riPort);

		void RunIMEUpdate();
		void RunIMETabEvent();
		void RunIMEReturnEvent();
		void RunPressExitKey();

		void RunIMEChangeCodePage();
		void RunIMEOpenCandidateListEvent();
		void RunIMECloseCandidateListEvent();
		void RunIMEOpenReadingWndEvent();
		void RunIMECloseReadingWndEvent();

		void EnableSpecialCameraMode();
		void SetCameraSpeed(int32_t iPercentage);

		bool IsLockCurrentCamera();
		void SetEventCamera(const SCameraSetting & c_rCameraSetting);

		//Free camera [Think]
		void SetFreeCamera();
		void MoveFreeCamera(float fDirRot, bool isBackwards);
		void SetFreeCameraSpeed(int32_t pct);
		//End

		void BlendEventCamera(const SCameraSetting & c_rCameraSetting, float fBlendTime);
		void SetDefaultCamera();

		void SetCameraSetting(const SCameraSetting & c_rCameraSetting);
		void GetCameraSetting(SCameraSetting * pCameraSetting);
		void SaveCameraSetting(const char * c_szFileName);
		bool LoadCameraSetting(const char * c_szFileName);

		void SetForceSightRange(int32_t iRange);
	
		void UpdateEventCameraX(float fdx);
		void UpdateEventCameraY(float fdy);
		void UpdateEventCameraZ(float fdz);

	public:
		void SetTitle(const char* szTitle);
		
		void FlashApplication();

		int32_t GetGameStage() const;
		void SetGameStage(int32_t iNewStage);

	protected:
		LRESULT WindowProcedure(HWND hWnd, uint32_t uiMsg, WPARAM wParam, LPARAM lParam);

		void OnCameraUpdate();

		void OnUIUpdate();
		void OnUIRender();

		void OnMouseUpdate();
		void OnMouseRender();

		void OnMouseWheel(int32_t nLen);
		void OnMouseMove(int32_t x, int32_t y);
		void OnMouseMiddleButtonDown(int32_t x, int32_t y);
		void OnMouseMiddleButtonUp(int32_t x, int32_t y);
		void OnMouseLeftButtonDown(int32_t x, int32_t y);
		void OnMouseLeftButtonUp(int32_t x, int32_t y);
		void OnMouseLeftButtonDoubleClick(int32_t x, int32_t y);
		void OnMouseRightButtonDown(int32_t x, int32_t y);
		void OnMouseRightButtonUp(int32_t x, int32_t y);
#ifdef ENABLE_MOUSEWHEEL_EVENT
		bool OnMouseWheelScroll(int32_t x, int32_t y , int16_t wDelta);
#endif
		void OnSizeChange(int32_t width, int32_t height);
		void OnKeyDown(int32_t iIndex);
		void OnKeyUp(int32_t iIndex);
		void OnIMEKeyDown(int32_t iIndex);

		int32_t CheckDeviceState();

		BOOL __IsContinuousChangeTypeCursor(int32_t iCursorNum);

		void __UpdateCamera();

		void __SetFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight, uint32_t dwBPP);
		void __MinimizeFullScreenWindow(HWND hWnd, uint32_t dwWidth, uint32_t dwHeight);

	protected:
		CTimer m_timer;

		CLightManager				m_LightManager;
		CSoundManager				m_SoundManager;
		CFlyingManager				m_FlyingManager;
		CRaceManager				m_RaceManager;
		CGameEventManager			m_GameEventManager;
		CItemManager				m_kItemMgr;

		UI::CWindowManager			m_kWndMgr;
		CEffectManager				m_kEftMgr;
		CPythonCharacterManager		m_kChrMgr;

		CServerStateChecker			m_kServerStateChecker;
		CPythonGraphic				m_pyGraphic;
		CPythonNetworkStream		m_pyNetworkStream;
		CPythonPlayer				m_pyPlayer;
		CPythonIME					m_pyIme;
		CPythonItem					m_pyItem;
		CPythonShop					m_pyShop;
		CPythonExchange				m_pyExchange;
		CPythonChat					m_pyChat;
		CPythonTextTail				m_pyTextTail;
		CPythonNonPlayer			m_pyNonPlayer;
		CPythonMiniMap				m_pyMiniMap;
		CPythonEventManager			m_pyEventManager;
		CPythonBackground			m_pyBackground;
		CPythonSkill				m_pySkill;
		CPythonResource				m_pyRes;
		CPythonQuest				m_pyQuest;
		CPythonMessenger			m_pyManager;
#ifdef ENABLE_ACCE_SYSTEM
		CPythonAcce	m_pyAcce;
#endif
		CPythonSafeBox				m_pySafeBox;
		CPythonGuild				m_pyGuild;

		CPythonDynamicModule m_pkPyDynamicModuleMgr;
		CPythonStackController		m_kPyStackController;
//		CAnticheatManager			m_kAnticheatManager;
		
		CGuildMarkManager			m_kGuildMarkManager;
		CGuildMarkDownloader		m_kGuildMarkDownloader;
		CGuildMarkUploader			m_kGuildMarkUploader;
		CAccountConnector			m_kAccountConnector;

		CGraphicDevice				m_grpDevice;
		CNetworkDevice				m_netDevice;

		CPythonSystem				m_pySystem;


		PyObject *					m_poMouseHandler;
		D3DXVECTOR3					m_v3CenterPosition;

		CRenderTargetManager		m_cRenderTargetManager;
		CPythonRenderTargetManager	m_pyRenderTargetManager;

		uint32_t				m_iFPS;
		float						m_fAveRenderTime;
		uint32_t						m_dwCurRenderTime;
		uint32_t						m_dwCurUpdateTime;
		uint32_t						m_dwLoad;
		uint32_t						m_dwWidth;
		uint32_t						m_dwHeight;

	protected:
		// Time
		uint32_t						m_dwStartLocalTime;
		time_t						m_tServerTime;
		time_t						m_tLocalStartTime;
		float						m_fGlobalTime;
		float						m_fGlobalElapsedTime;

		/////////////////////////////////////////////////////////////
		// Camera
		SCameraSetting				m_DefaultCameraSetting;
		SCameraSetting				m_kEventCameraSetting;

		int32_t							m_iCameraMode;
		float						m_fBlendCameraStartTime;
		float						m_fBlendCameraBlendTime;
		SCameraSetting				m_kEndBlendCameraSetting;

		float						m_fRotationSpeed;
		float						m_fPitchSpeed;
		float						m_fZoomSpeed;
		int32_t							m_freeCameraSpeedPct;
		float						m_fCameraRotateSpeed;
		float						m_fCameraPitchSpeed;
		float						m_fCameraZoomSpeed;

		SCameraPos					m_kCmrPos;
		SCameraSpeed				m_kCmrSpd;

		BOOL						m_isSpecialCameraMode;
		// Camera
		/////////////////////////////////////////////////////////////

		float						m_fFaceSpd;
		uint32_t						m_dwFaceSpdSum;
		uint32_t						m_dwFaceSpdCount;

		uint32_t						m_dwFaceAccCount;
		uint32_t						m_dwFaceAccTime;

		uint32_t						m_dwUpdateFPS;
		uint32_t						m_dwRenderFPS;
		uint32_t						m_dwFaceCount;

		uint32_t						m_dwLButtonDownTime;
		uint32_t						m_dwLButtonUpTime;

		typedef std::map<int32_t, HANDLE>		TCursorHandleMap;
		TCursorHandleMap			m_CursorHandleMap;
		HANDLE						m_hCurrentCursor;

		BOOL						m_bCursorVisible;
		bool						m_bLiarCursorOn;
		int32_t							m_iCursorMode;
		bool						m_isWindowed;
		bool						m_isFrameSkipDisable;

		// Connect Data
		std::string					m_strIP;
		int32_t							m_iPort;

	int32_t m_iGameStage;

		static CPythonApplication*	ms_pInstance;

		bool						m_isMinimizedWnd;
		bool						m_isActivateWnd;
		BOOL						m_isWindowFullScreenEnable;

		uint32_t						m_dwStickyKeysFlag;
		uint32_t						m_dwBufSleepSkipTime;
		int32_t							m_iForceSightRange;

	protected:
		int32_t m_iCursorNum;
		int32_t m_iContinuousCursorNum;
	public:
		LPDIRECT3D9		GetDirectx9();
		LPDIRECT3DDEVICE9 GetDevice();
};
