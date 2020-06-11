#pragma once

#include "GrpBase.h"
#include "GrpDetector.h"
#include "StateManager.h"

class CGraphicDevice : public CGraphicBase
{
public:
	enum EDeviceState
	{
		DEVICESTATE_OK,
		DEVICESTATE_BROKEN,
		DEVICESTATE_NEEDS_RESET,
		DEVICESTATE_NULL
	};

	enum ECreateReturnValues
	{
		CREATE_OK				= (1 << 0),
		CREATE_NO_DIRECTX		= (1 << 1),
		CREATE_GET_DEVICE_CAPS	= (1 << 2),
		CREATE_GET_DEVICE_CAPS2 = (1 << 3),
		CREATE_DEVICE			= (1 << 4),
		CREATE_REFRESHRATE		= (1 << 5),
		CREATE_ENUM				= (1 << 6), // 2003. 01. 09. myevan 모드 리스트 얻기 실패
		CREATE_DETECT			= (1 << 7), // 2003. 01. 09. myevan 모드 선택 실패
		CREATE_NO_TNL			= (1 << 8),
	};

	CGraphicDevice();
	virtual ~CGraphicDevice();

	void			InitBackBufferCount(uint32_t uBackBufferCount);

	void			Destroy();
	int32_t			Create(HWND hWnd, int32_t hres, int32_t vres, bool Windowed = true,
						   int32_t bit = 32, int32_t ReflashRate = 0, bool antialias = false);

	EDeviceState	GetDeviceState();
	LPDIRECT3D9		GetDirectx9();
	LPDIRECT3DDEVICE9 GetDevice();
	bool			Reset();

	void			EnableWebBrowserMode(const RECT& c_rcWebPage);		
	void			DisableWebBrowserMode();
	void			MoveWebBrowserRect(const RECT& c_rcWebPage);

	bool			ResizeBackBuffer(uint32_t uWidth, uint32_t uHeight);
	void			RegisterWarningString(uint32_t uiMsg, const char * c_szString);

protected:
	void __Initialize();
	void __WarningMessage(HWND hWnd, uint32_t uiMsg);

	void __InitializeDefaultIndexBufferList();
	void __DestroyDefaultIndexBufferList();	
	bool __CreateDefaultIndexBufferList();
	bool __CreateDefaultIndexBuffer(uint32_t eDefIB, uint32_t uIdxCount, const uint16_t* c_awIndices);

	void __InitializePDTVertexBufferList();
	void __DestroyPDTVertexBufferList();
	bool __CreatePDTVertexBufferList();

	LPDIRECT3DVERTEXDECLARATION9 CreatePTStreamVertexShader();
	LPDIRECT3DVERTEXDECLARATION9 CreatePNTStreamVertexShader();
	LPDIRECT3DVERTEXDECLARATION9 CreatePNT2StreamVertexShader();

protected:
	uint32_t						m_uBackBufferCount;
	std::map<uint32_t, std::string>	m_kMap_strWarningMessage;
	CStateManager*				m_pStateManager;
};
