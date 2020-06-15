#include "StdAfx.h"
#include "GrpDevice.h"
#include "../eterBase/Stl.h"
#include "../eterBase/Debug.h"
#include "../eterPythonLib/PythonGraphic.h"

bool GRAPHICS_CAPS_SOFTWARE_TILING = false;

D3DPRESENT_PARAMETERS g_kD3DPP;
bool g_isBrowserMode=false;
RECT g_rcBrowser;

CGraphicDevice::CGraphicDevice()
: m_uBackBufferCount(0)
{
	__Initialize();
}

CGraphicDevice::~CGraphicDevice()
{
	Destroy();
}

void CGraphicDevice::__Initialize()
{
	ms_iD3DAdapterInfo=D3DADAPTER_DEFAULT;
	ms_iD3DDevInfo=D3DADAPTER_DEFAULT;
	ms_iD3DModeInfo=D3DADAPTER_DEFAULT;

	ms_lpd3d			= nullptr;
	ms_lpd3dDevice		= nullptr;
	ms_lpd3dMatStack	= nullptr;

	ms_dwWavingEndTime = 0;
	ms_dwFlashingEndTime = 0;

	m_pStateManager		= nullptr;

	__InitializeDefaultIndexBufferList();
	__InitializePDTVertexBufferList();
}

void CGraphicDevice::RegisterWarningString(uint32_t uiMsg, const char * c_szString)
{
	m_kMap_strWarningMessage[uiMsg] = c_szString;
}

void CGraphicDevice::__WarningMessage(HWND hWnd, uint32_t uiMsg)
{
	if (m_kMap_strWarningMessage.end() == m_kMap_strWarningMessage.find(uiMsg))
		return;
	MessageBox(hWnd, m_kMap_strWarningMessage[uiMsg].c_str(), "Warning", MB_OK|MB_TOPMOST);
}

void CGraphicDevice::MoveWebBrowserRect(const RECT& c_rcWebPage)
{
	g_rcBrowser=c_rcWebPage;
}

void CGraphicDevice::EnableWebBrowserMode(const RECT& c_rcWebPage)
{
	if (!ms_lpd3dDevice)
		return;

	D3DPRESENT_PARAMETERS& rkD3DPP=ms_d3dPresentParameter;
	
	g_isBrowserMode=true;

	if (D3DSWAPEFFECT_COPY==rkD3DPP.SwapEffect)
		return;

	g_kD3DPP=rkD3DPP;
	g_rcBrowser=c_rcWebPage;
	
	//rkD3DPP.Windowed=TRUE;
	rkD3DPP.SwapEffect=D3DSWAPEFFECT_COPY;
	rkD3DPP.BackBufferCount = 1;
	rkD3DPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	
	IDirect3DDevice9& rkD3DDev=*ms_lpd3dDevice;
	HRESULT hr=rkD3DDev.Reset(&rkD3DPP);
	if (FAILED(hr))
		return;
	
	STATEMANAGER.SetDefaultState();	
}

void CGraphicDevice::DisableWebBrowserMode()
{
	if (!ms_lpd3dDevice)
		return;

	D3DPRESENT_PARAMETERS& rkD3DPP=ms_d3dPresentParameter;
	
	g_isBrowserMode=false;

	rkD3DPP=g_kD3DPP;

	IDirect3DDevice9& rkD3DDev=*ms_lpd3dDevice;
	HRESULT hr=rkD3DDev.Reset(&rkD3DPP);
	if (FAILED(hr))
		return;
	
	STATEMANAGER.SetDefaultState();	
}
		
bool CGraphicDevice::ResizeBackBuffer(uint32_t uWidth, uint32_t uHeight)
{
	if (!ms_lpd3dDevice)
		return false;

	D3DPRESENT_PARAMETERS& rkD3DPP=ms_d3dPresentParameter;
	if (rkD3DPP.Windowed)
	{
		if (rkD3DPP.BackBufferWidth!=uWidth || rkD3DPP.BackBufferHeight!=uHeight)
		{
			rkD3DPP.BackBufferWidth=uWidth;
			rkD3DPP.BackBufferHeight=uHeight;

			IDirect3DDevice9& rkD3DDev=*ms_lpd3dDevice;

			HRESULT hr=rkD3DDev.Reset(&rkD3DPP);
			if (FAILED(hr))
			{
				return false;
			}

			STATEMANAGER.SetDefaultState();
		}
	}

	return true;
}

LPDIRECT3DVERTEXDECLARATION9 CGraphicDevice::CreatePNTStreamVertexShader()
{
	assert(ms_lpd3dDevice != nullptr);

	LPDIRECT3DVERTEXDECLARATION9 dwShader = nullptr;

	D3DVERTEXELEMENT9 pShaderDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) != D3D_OK)
	{
		char szError[1024];
		sprintf(szError, "Failed to create CreatePNTStreamVertexShader");
		MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
	}

	return dwShader;
}

LPDIRECT3DVERTEXDECLARATION9 CGraphicDevice::CreatePNT2StreamVertexShader()
{
	assert(ms_lpd3dDevice != nullptr);

	LPDIRECT3DVERTEXDECLARATION9 dwShader = nullptr;

	D3DVERTEXELEMENT9 pShaderDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};

	if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) != D3D_OK)
	{
		char szError[1024];
		sprintf(szError, "Failed to create CreatePNT2StreamVertexShader");
		MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
	}

	return dwShader;
}

LPDIRECT3DVERTEXDECLARATION9 CGraphicDevice::CreatePTStreamVertexShader()
{
	assert(ms_lpd3dDevice != nullptr);

	LPDIRECT3DVERTEXDECLARATION9 dwShader = nullptr;

	D3DVERTEXELEMENT9 pShaderDecl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	if (ms_lpd3dDevice->CreateVertexDeclaration(pShaderDecl, &dwShader) != D3D_OK)
	{
		char szError[1024];
		sprintf(szError, "Failed to create CreatePTStreamVertexShader");
		MessageBox(nullptr, szError, "Vertex Shader Error", MB_ICONSTOP);
	}

	return dwShader;
}

CGraphicDevice::EDeviceState CGraphicDevice::GetDeviceState()
{
	if (!ms_lpd3dDevice)
		return DEVICESTATE_NULL;

	HRESULT hr;

	if (FAILED(hr = ms_lpd3dDevice->TestCooperativeLevel()))
	{
		if (D3DERR_DEVICELOST == hr)
			return DEVICESTATE_BROKEN;

		if (D3DERR_DEVICENOTRESET == hr)
			return DEVICESTATE_NEEDS_RESET;

		return DEVICESTATE_BROKEN;
	}
	
	return DEVICESTATE_OK;
}

LPDIRECT3D9 CGraphicDevice::GetDirectx9()
{
	return ms_lpd3d;
}

LPDIRECT3DDEVICE9 CGraphicDevice::GetDevice()
{
	return ms_lpd3dDevice;
}

bool CGraphicDevice::Reset()
{
	HRESULT hr;

	if (FAILED(hr = ms_lpd3dDevice->Reset(&ms_d3dPresentParameter)))
		return false;

	return true;
}

static LPDIRECT3DSURFACE9 s_lpStencil;
static uint32_t   s_MaxTextureWidth, s_MaxTextureHeight;

BOOL EL3D_ConfirmDevice(D3DCAPS9& rkD3DCaps, uint32_t uBehavior, D3DFORMAT /*eD3DFmt*/)
{
	// PUREDEVICE는 GetTransform / GetViewport 등이 되지 않는다.
	if (uBehavior & D3DCREATE_PUREDEVICE) 
        return FALSE;
	
	if (uBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) 
	{	
		// DirectionalLight
		if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS))
			return FALSE;
		
		// PositionalLight
		if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_POSITIONALLIGHTS))
			return FALSE;

		// Shadow/Terrain
		if (!(rkD3DCaps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN))
			return FALSE;
	}

	s_MaxTextureWidth = rkD3DCaps.MaxTextureWidth;
	s_MaxTextureHeight = rkD3DCaps.MaxTextureHeight;
	
	return TRUE;
}

uint32_t GetMaxTextureWidth()
{
	return s_MaxTextureWidth;
}

uint32_t GetMaxTextureHeight()
{
	return s_MaxTextureHeight;
}



int32_t CGraphicDevice::Create(HWND hWnd, int32_t iHres, int32_t iVres, bool Windowed, int32_t iBit,
							   int32_t iRefreshRate, bool antialias)
{
	int32_t iRet = CREATE_OK;

	Destroy();

	ms_iWidth	= iHres;
	ms_iHeight	= iVres;

	ms_hWnd		= hWnd;
	ms_hDC		= GetDC(hWnd);
	ms_lpd3d	= Direct3DCreate9(D3D_SDK_VERSION);

	if (!ms_lpd3d)
		return CREATE_NO_DIRECTX;

	if (!ms_kD3DDetector.Build(*ms_lpd3d, EL3D_ConfirmDevice))
		return CREATE_ENUM;

	/*
	std::string stDevList;
	ms_kD3DDetector.GetString(&stDevList);

	Tracen(stDevList.c_str());
	Tracenf("adapter %d, device %d, mode %d", ms_iD3DAdapterInfo, ms_iD3DDevInfo, ms_iD3DModeInfo);
	*/

	D3D_CAdapterInfo * pkD3DAdapterInfo = ms_kD3DDetector.GetD3DAdapterInfop(ms_iD3DAdapterInfo);
	if (!pkD3DAdapterInfo)
	{
		Tracenf("adapter %d is EMPTY", ms_iD3DAdapterInfo);
		return CREATE_DETECT;
	}

	D3D_SModeInfo * pkD3DModeInfo = pkD3DAdapterInfo->GetD3DModeInfop(ms_iD3DDevInfo, ms_iD3DModeInfo);		
	if (!pkD3DModeInfo)
	{
		Tracenf("device %d, mode %d is EMPTY", ms_iD3DDevInfo, ms_iD3DModeInfo);
		return CREATE_DETECT;
	}

	if (pkD3DModeInfo->m_dwD3DBehavior==D3DCREATE_SOFTWARE_VERTEXPROCESSING) {
		iRet |= CREATE_NO_TNL;

		// DISABLE_NOTIFY_NOT_SUPPORT_TNL_MESSAGE
		//__WarningMessage(hWnd, CREATE_NO_TNL);
		// END_OF_DISABLE_NOTIFY_NOT_SUPPORT_TNL_MESSAGE
	}

	std::string stModeInfo;
	pkD3DModeInfo->GetString(&stModeInfo);

	//Tracen(stModeInfo.c_str());

	int32_t ErrorCorrection = 0;

RETRY:
	ZeroMemory(&ms_d3dPresentParameter, sizeof(ms_d3dPresentParameter));
	
	ms_d3dPresentParameter.Windowed							= Windowed;
	ms_d3dPresentParameter.BackBufferWidth					= iHres;
	ms_d3dPresentParameter.BackBufferHeight					= iVres;
	ms_d3dPresentParameter.hDeviceWindow					= hWnd;
	ms_d3dPresentParameter.BackBufferCount					= m_uBackBufferCount;
	ms_d3dPresentParameter.SwapEffect						= D3DSWAPEFFECT_DISCARD;

	if (Windowed)
	{
		ms_d3dPresentParameter.BackBufferFormat				= pkD3DAdapterInfo->GetDesktopD3DDisplayModer().Format;
	}
	else
	{
		ms_d3dPresentParameter.BackBufferFormat				= pkD3DModeInfo->m_eD3DFmtPixel;
		ms_d3dPresentParameter.FullScreen_RefreshRateInHz	= iRefreshRate;
	}

//	ms_d3dPresentParameter.Flags							= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	ms_d3dPresentParameter.EnableAutoDepthStencil			= TRUE;
	ms_d3dPresentParameter.AutoDepthStencilFormat			= pkD3DModeInfo->m_eD3DFmtDepthStencil;

	ms_dwD3DBehavior = pkD3DModeInfo->m_dwD3DBehavior;

	ms_bEnableGlobalAntialiasing = false;

	DWORD backBufferQualityLevels;     
	DWORD zStencilQualityLevels; 
	//acceptable MultiSapmpleTypes have got values from 0 to 16 
	for(int msType=0; msType<16; msType++){ 
		//check multisample type with BackBufferFormat 
		if(SUCCEEDED(ms_lpd3d->CheckDeviceMultiSampleType( 
							ms_iD3DAdapterInfo, 
							D3DDEVTYPE_HAL, 
							ms_d3dPresentParameter.BackBufferFormat, 
							Windowed, 
							*((D3DMULTISAMPLE_TYPE*)(&msType)), 
							&backBufferQualityLevels))){ 
		//check if there is at least one quality level 
		if(backBufferQualityLevels>0){ 
			//continue the check with the ZStencilFormat 
			if(SUCCEEDED(ms_lpd3d->CheckDeviceMultiSampleType( 
								ms_iD3DAdapterInfo, 
				D3DDEVTYPE_HAL, 
				pkD3DModeInfo->m_eD3DFmtDepthStencil, 
				Windowed, 
				*((D3DMULTISAMPLE_TYPE*)(&msType)), 
				&zStencilQualityLevels))){ 
			if(zStencilQualityLevels>0){ 
				Tracef("Type possible %d stencil %d backbuffer %d \n", *((D3DMULTISAMPLE_TYPE*)(&msType)), zStencilQualityLevels, backBufferQualityLevels);
					if(*((D3DMULTISAMPLE_TYPE*)(&msType)) == 8 && zStencilQualityLevels == 4)
					{
						ms_d3dPresentParameter.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
						ms_d3dPresentParameter.MultiSampleQuality = 3;
					}
			} 
			} 
		} 
		} 
	}

	if (antialias)
	{
		ms_d3dPresentParameter.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
		ms_d3dPresentParameter.MultiSampleQuality = 0;
	}

	if (FAILED(ms_hLastResult = ms_lpd3d->CreateDevice(
				ms_iD3DAdapterInfo,
				D3DDEVTYPE_HAL,
				hWnd,
				// 2004. 1. 9 myevan 버텍스 프로세싱 방식 자동 선택 추가
				pkD3DModeInfo->m_dwD3DBehavior,
				&ms_d3dPresentParameter,
				&ms_lpd3dDevice)))
	{
		switch (ms_hLastResult)
		{
			case D3DERR_INVALIDCALL:
				Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_INVALIDCALL\nThe method call is invalid. For example, a method's parameter may have an invalid value.");					
				break;
			case D3DERR_NOTAVAILABLE:
				Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_NOTAVAILABLE\nThis device does not support the queried technique. ");
				break;
			case D3DERR_OUTOFVIDEOMEMORY:
				Tracen("IDirect3DDevice.CreateDevice - ERROR D3DERR_OUTOFVIDEOMEMORY\nDirect3D does not have enough display memory to perform the operation");
				break;
			default:
				Tracenf("IDirect3DDevice.CreateDevice - ERROR %d", ms_hLastResult);
				break;
		}

		if (ErrorCorrection)
			return CREATE_DEVICE;
	
		// 2004. 1. 9 myevan 큰의미 없는 코드인듯.. 에러나면 표시하고 종료하자
		iRefreshRate = 0;
		++ErrorCorrection;
		iRet = CREATE_REFRESHRATE;
		goto RETRY;
	}

	// Check DXT Support Info
	if(ms_lpd3d->CheckDeviceFormat(
				ms_iD3DAdapterInfo, 
				D3DDEVTYPE_HAL,
				ms_d3dPresentParameter.BackBufferFormat,
				0,
				D3DRTYPE_TEXTURE,
				D3DFMT_DXT1) == D3DERR_NOTAVAILABLE)
	{
		ms_bSupportDXT = false;
	}

	if(ms_lpd3d->CheckDeviceFormat(
				ms_iD3DAdapterInfo, 
				D3DDEVTYPE_HAL,
				ms_d3dPresentParameter.BackBufferFormat,
				0,
				D3DRTYPE_TEXTURE,
				D3DFMT_DXT3) == D3DERR_NOTAVAILABLE)
	{
		ms_bSupportDXT = false;
	}

	if(ms_lpd3d->CheckDeviceFormat(
				ms_iD3DAdapterInfo, 
				D3DDEVTYPE_HAL,
				ms_d3dPresentParameter.BackBufferFormat,
				0,
				D3DRTYPE_TEXTURE,
				D3DFMT_DXT5) == D3DERR_NOTAVAILABLE)
	{
		ms_bSupportDXT = false;
	}	

	if (FAILED((ms_hLastResult = ms_lpd3dDevice->GetDeviceCaps(&ms_d3dCaps))))
	{
		Tracenf("IDirect3DDevice.GetDeviceCaps - ERROR %d", ms_hLastResult);
		return CREATE_GET_DEVICE_CAPS2;
	}

	DetectNonPow2TextureSupport();
	
	if (!Windowed)
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, iHres, iVres, SWP_SHOWWINDOW);

	//Tracef("vertex shader version : %X\n",(uint32_t)ms_d3dCaps.VertexShaderVersion);

	ms_lpd3dDevice->GetViewport(&ms_Viewport);

	m_pStateManager = new CStateManager(ms_lpd3dDevice);
	m_pStateManager->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	D3DXCreateMatrixStack(0, &ms_lpd3dMatStack);
	ms_lpd3dMatStack->LoadIdentity();

	ms_ptVS	= CreatePTStreamVertexShader();
	ms_pntVS = CreatePNTStreamVertexShader();
	ms_pnt2VS = CreatePNT2StreamVertexShader();

	D3DXMatrixIdentity(&ms_matIdentity);
	D3DXMatrixIdentity(&ms_matView);
	D3DXMatrixIdentity(&ms_matProj);
	D3DXMatrixIdentity(&ms_matInverseView);
	D3DXMatrixIdentity(&ms_matInverseViewYAxis);
	D3DXMatrixIdentity(&ms_matScreen0);
	D3DXMatrixIdentity(&ms_matScreen1);
	D3DXMatrixIdentity(&ms_matScreen2);

	ms_matScreen0._11 = 1;
	ms_matScreen0._22 = -1;	

	ms_matScreen1._41 = 1;
	ms_matScreen1._42 = 1;

	ms_matScreen2._11 = (float) iHres / 2;
	ms_matScreen2._22 = (float) iVres / 2;
	
	D3DXCreateSphere(ms_lpd3dDevice, 1.0f, 32, 32, &ms_lpSphereMesh, nullptr);
	D3DXCreateCylinder(ms_lpd3dDevice, 1.0f, 1.0f, 1.0f, 8, 8, &ms_lpCylinderMesh, nullptr);

	ms_lpd3dDevice->Clear(0L, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);

	if (!__CreateDefaultIndexBufferList())
		return 0;

	if (!__CreatePDTVertexBufferList())
		return 0;
	
	uint32_t dwTexMemSize = GetAvailableTextureMemory();

	if (dwTexMemSize < 64 * 1024 * 1024)
		ms_isLowTextureMemory = true;
	else
		ms_isLowTextureMemory = false;

	if (dwTexMemSize > 100 * 1024 * 1024)
		ms_isHighTextureMemory = true;
	else
		ms_isHighTextureMemory = false;

	return (iRet);
}

void CGraphicDevice::__InitializePDTVertexBufferList()
{
	for (uint32_t i=0; i<PDT_VERTEXBUFFER_NUM; ++i)
		ms_alpd3dPDTVB[i]=nullptr;	
}
		
void CGraphicDevice::__DestroyPDTVertexBufferList()
{
	for (uint32_t i=0; i<PDT_VERTEXBUFFER_NUM; ++i)
	{
		if (ms_alpd3dPDTVB[i])
		{
			ms_alpd3dPDTVB[i]->Release();
			ms_alpd3dPDTVB[i]=nullptr;
		}
	}
}

bool CGraphicDevice::__CreatePDTVertexBufferList()
{
	for (uint32_t i=0; i<PDT_VERTEXBUFFER_NUM; ++i)
	{
		if (FAILED(
			ms_lpd3dDevice->CreateVertexBuffer(
			sizeof(TPDTVertex)*PDT_VERTEX_NUM, 
			D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, 
			D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1, 
			D3DPOOL_SYSTEMMEM, 
			&ms_alpd3dPDTVB[i],
			nullptr)
		))
		return false;
	}
	return true;
}

void CGraphicDevice::__InitializeDefaultIndexBufferList()
{
	for (uint32_t i=0; i<DEFAULT_IB_NUM; ++i)
		ms_alpd3dDefIB[i]=nullptr;
}

void CGraphicDevice::__DestroyDefaultIndexBufferList()
{
	for (uint32_t i=0; i<DEFAULT_IB_NUM; ++i)
		if (ms_alpd3dDefIB[i])
		{
			ms_alpd3dDefIB[i]->Release();
			ms_alpd3dDefIB[i]=nullptr;
		}	
}

bool CGraphicDevice::__CreateDefaultIndexBuffer(uint32_t eDefIB, uint32_t uIdxCount, const uint16_t* c_awIndices)
{
	assert(ms_alpd3dDefIB[eDefIB]==nullptr);

	if (FAILED(
		ms_lpd3dDevice->CreateIndexBuffer(
			sizeof(uint16_t)*uIdxCount, 
			D3DUSAGE_WRITEONLY, 
			D3DFMT_INDEX16,
			D3DPOOL_MANAGED,
			&ms_alpd3dDefIB[eDefIB],
			nullptr)
	)) return false;
	
	uint16_t* dstIndices;
	if (FAILED(
		ms_alpd3dDefIB[eDefIB]->Lock(0, 0, (void**)&dstIndices, 0)
	)) return false;

	memcpy(dstIndices, c_awIndices, sizeof(uint16_t)*uIdxCount);

	ms_alpd3dDefIB[eDefIB]->Unlock();

	return true;
}

bool CGraphicDevice::__CreateDefaultIndexBufferList()
{
	static const uint16_t c_awLineIndices[2] = { 0, 1, };
	static const uint16_t c_awLineTriIndices[6] = { 0, 1, 0, 2, 1, 2, };
	static const uint16_t c_awLineRectIndices[8] = { 0, 1, 0, 2, 1, 3, 2, 3,};
	static const uint16_t c_awLineCubeIndices[24] = { 
		0, 1, 0, 2, 1, 3, 2, 3,
		0, 4, 1, 5, 2, 6, 3, 7,
		4, 5, 4, 6, 5, 7, 6, 7,
	};
	static const uint16_t c_awFillTriIndices[3]= { 0, 1, 2, };
	static const uint16_t c_awFillRectIndices[6] = { 0, 2, 1, 2, 3, 1, };
	static const uint16_t c_awFillCubeIndices[36] = { 
		0, 1, 2, 1, 3, 2,
		2, 0, 6, 0, 4, 6,
		0, 1, 4, 1, 5, 4,
		1, 3, 5, 3, 7, 5,
		3, 2, 7, 2, 6, 7,
		4, 5, 6, 5, 7, 6,
	};
	
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE, 2, c_awLineIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_TRI, 6, c_awLineTriIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_RECT, 8, c_awLineRectIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_LINE_CUBE, 24, c_awLineCubeIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_TRI, 3, c_awFillTriIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_RECT, 6, c_awFillRectIndices))
		return false;
	if (!__CreateDefaultIndexBuffer(DEFAULT_IB_FILL_CUBE, 36, c_awFillCubeIndices))
		return false;
	
	return true;
}

void CGraphicDevice::InitBackBufferCount(uint32_t uBackBufferCount)
{
	m_uBackBufferCount=uBackBufferCount;
}

void CGraphicDevice::Destroy()
{
	__DestroyPDTVertexBufferList();
	__DestroyDefaultIndexBufferList();

	if (ms_hDC)
	{
		ReleaseDC(ms_hWnd, ms_hDC);
		ms_hDC = nullptr;
	}

	if (ms_ptVS)
	{	
		ms_ptVS->Release();
		ms_ptVS = 0;
	}

	if (ms_pntVS)
	{	
		ms_pntVS->Release();
		ms_pntVS = 0;
	}

	if (ms_pnt2VS)
	{	
		ms_pnt2VS->Release();
		ms_pnt2VS = 0;
	}

	safe_release(ms_lpSphereMesh);
	safe_release(ms_lpCylinderMesh);

	safe_release(ms_lpd3dMatStack);
	safe_release(ms_lpd3dDevice);
	safe_release(ms_lpd3d);	

	if (m_pStateManager)
	{
		delete m_pStateManager;
		m_pStateManager = nullptr;
	}

	__Initialize();
}
