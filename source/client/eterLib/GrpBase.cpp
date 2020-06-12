#include "StdAfx.h"
#include "../eterBase/Utils.h"
#include "../eterBase/Timer.h"
#include "GrpBase.h"
#include "Camera.h"
#include "StateManager.h"

void PixelPositionToD3DXVECTOR3(const D3DXVECTOR3& c_rkPPosSrc, D3DXVECTOR3* pv3Dst)
{
	pv3Dst->x=+c_rkPPosSrc.x;
	pv3Dst->y=-c_rkPPosSrc.y;
	pv3Dst->z=+c_rkPPosSrc.z;
}

void D3DXVECTOR3ToPixelPosition(const D3DXVECTOR3& c_rv3Src, D3DXVECTOR3* pv3Dst)
{
	pv3Dst->x=+c_rv3Src.x;
	pv3Dst->y=-c_rv3Src.y;
	pv3Dst->z=+c_rv3Src.z;
}

uint32_t					CGraphicBase::ms_iD3DAdapterInfo=0;
uint32_t					CGraphicBase::ms_iD3DDevInfo=0;
uint32_t					CGraphicBase::ms_iD3DModeInfo=0;		
D3D_CDisplayModeAutoDetector				CGraphicBase::ms_kD3DDetector;

HWND CGraphicBase::ms_hWnd;
HDC CGraphicBase::ms_hDC;

LPDIRECT3D9				CGraphicBase::ms_lpd3d = nullptr;
LPDIRECT3DDEVICE9		CGraphicBase::ms_lpd3dDevice = nullptr;
ID3DXMatrixStack *		CGraphicBase::ms_lpd3dMatStack = nullptr;
D3DPRESENT_PARAMETERS	CGraphicBase::ms_d3dPresentParameter;
D3DVIEWPORT9			CGraphicBase::ms_Viewport;

HRESULT					CGraphicBase::ms_hLastResult = 0;

int32_t						CGraphicBase::ms_iWidth;
int32_t						CGraphicBase::ms_iHeight;

uint32_t					CGraphicBase::ms_faceCount = 0;

D3DCAPS9				CGraphicBase::ms_d3dCaps;

uint32_t					CGraphicBase::ms_dwD3DBehavior = 0;

LPDIRECT3DVERTEXDECLARATION9 CGraphicBase::ms_ptVS = 0;
LPDIRECT3DVERTEXDECLARATION9 CGraphicBase::ms_pntVS = 0;
LPDIRECT3DVERTEXDECLARATION9 CGraphicBase::ms_pnt2VS = 0;

D3DXMATRIX				CGraphicBase::ms_matIdentity;

D3DXMATRIX				CGraphicBase::ms_matView;
D3DXMATRIX				CGraphicBase::ms_matProj;
D3DXMATRIX				CGraphicBase::ms_matInverseView;
D3DXMATRIX				CGraphicBase::ms_matInverseViewYAxis;

D3DXMATRIX				CGraphicBase::ms_matWorld;
D3DXMATRIX				CGraphicBase::ms_matWorldView;

D3DXMATRIX				CGraphicBase::ms_matScreen0;
D3DXMATRIX				CGraphicBase::ms_matScreen1;
D3DXMATRIX				CGraphicBase::ms_matScreen2;

D3DXVECTOR3				CGraphicBase::ms_vtPickRayOrig;
D3DXVECTOR3				CGraphicBase::ms_vtPickRayDir;

float					CGraphicBase::ms_fFieldOfView;
float					CGraphicBase::ms_fNearY;
float					CGraphicBase::ms_fFarY;
float					CGraphicBase::ms_fAspect;

uint32_t					CGraphicBase::ms_dwWavingEndTime;
int32_t						CGraphicBase::ms_iWavingPower;
uint32_t					CGraphicBase::ms_dwFlashingEndTime;
D3DXCOLOR				CGraphicBase::ms_FlashingColor;

// Terrain picking�� Ray... CCamera �̿��ϴ� ����.. ������ Ray�� ���� �ʿ�...
CRay					CGraphicBase::ms_Ray;
bool					CGraphicBase::ms_bSupportDXT = true;
bool					CGraphicBase::ms_isLowTextureMemory = false;
bool					CGraphicBase::ms_isHighTextureMemory = false;
int32_t						CGraphicBase::ms_nonPow2Support = 0;
bool					CGraphicBase::ms_bEnableGlobalAntialiasing = false;
// 2004.11.18.myevan.DynamicVertexBuffer�� ��ü
/*
std::vector<TIndex>		CGraphicBase::ms_lineIdxVector;
std::vector<TIndex>		CGraphicBase::ms_lineTriIdxVector;
std::vector<TIndex>		CGraphicBase::ms_lineRectIdxVector;
std::vector<TIndex>		CGraphicBase::ms_lineCubeIdxVector;

std::vector<TIndex>		CGraphicBase::ms_fillTriIdxVector;
std::vector<TIndex>		CGraphicBase::ms_fillRectIdxVector;
std::vector<TIndex>		CGraphicBase::ms_fillCubeIdxVector;
*/

LPD3DXMESH				CGraphicBase::ms_lpSphereMesh = nullptr;
LPD3DXMESH				CGraphicBase::ms_lpCylinderMesh = nullptr;

LPDIRECT3DVERTEXBUFFER9	CGraphicBase::ms_alpd3dPDTVB[PDT_VERTEXBUFFER_NUM];

LPDIRECT3DINDEXBUFFER9	CGraphicBase::ms_alpd3dDefIB[DEFAULT_IB_NUM];

bool CGraphicBase::IsLowTextureMemory()
{
	return ms_isLowTextureMemory;
}

bool CGraphicBase::IsHighTextureMemory()
{
	return ms_isHighTextureMemory;
}

bool CGraphicBase::IsFastTNL()
{ 
	if (ms_dwD3DBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING ||
		ms_dwD3DBehavior & D3DCREATE_MIXED_VERTEXPROCESSING)
	{
		if (ms_d3dCaps.VertexShaderVersion>D3DVS_VERSION(1,0))
			return true;
	}
	return false;
}

bool CGraphicBase::IsTLVertexClipping()
{
	if (ms_d3dCaps.PrimitiveMiscCaps & D3DPMISCCAPS_CLIPTLVERTS)
		return true;

	return false;
}

void CGraphicBase::DetectNonPow2TextureSupport()
{
	if (ms_d3dCaps.TextureCaps & D3DPTEXTURECAPS_POW2)	{
		if (ms_d3dCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
			ms_nonPow2Support = NONPOW2_SUPPORT_LEVEL::PARTIAL_SUPPORT;
		else
			ms_nonPow2Support = NONPOW2_SUPPORT_LEVEL::NO_SUPPORT;
	}
	else
		ms_nonPow2Support = NONPOW2_SUPPORT_LEVEL::FULL_SUPPORT;
}

void CGraphicBase::GetBackBufferSize(uint32_t* puWidth, uint32_t* puHeight)
{
	*puWidth=ms_d3dPresentParameter.BackBufferWidth;
	*puHeight=ms_d3dPresentParameter.BackBufferHeight;
}

void CGraphicBase::SetDefaultIndexBuffer(uint32_t eDefIB)
{
	if (eDefIB>=DEFAULT_IB_NUM)
		return;

	STATEMANAGER.SetIndices(ms_alpd3dDefIB[eDefIB]);
}

bool CGraphicBase::SetPDTStream(SPDTVertex* pVertices, uint32_t uVtxCount)
{
	return SetPDTStream((SPDTVertexRaw*)pVertices, uVtxCount);
}

bool CGraphicBase::SetPDTStream(SPDTVertexRaw* pSrcVertices, uint32_t uVtxCount)
{
	if (!uVtxCount)
		return false;

	static uint32_t s_dwVBPos=0;

	if (s_dwVBPos>=PDT_VERTEXBUFFER_NUM)
		s_dwVBPos=0;

	IDirect3DVertexBuffer9* plpd3dFillRectVB=ms_alpd3dPDTVB[s_dwVBPos];
	++s_dwVBPos;

	assert(PDT_VERTEX_NUM>=uVtxCount);
	if (uVtxCount >= PDT_VERTEX_NUM)
		return false;

	TPDTVertex* pDstVertices;
	if (FAILED(
		plpd3dFillRectVB->Lock(0, sizeof(TPDTVertex)*uVtxCount, (void**)&pDstVertices, D3DLOCK_DISCARD)
	)) 
	{
		STATEMANAGER.SetStreamSource(0, nullptr, 0);
		return false;
	}
	
	
	memcpy(pDstVertices, pSrcVertices, sizeof(TPDTVertex)*uVtxCount);

	plpd3dFillRectVB->Unlock();

	STATEMANAGER.SetStreamSource(0, plpd3dFillRectVB, sizeof(TPDTVertex));	

	return true;
}

uint32_t CGraphicBase::GetAvailableTextureMemory()
{
	assert(ms_lpd3dDevice!=nullptr && "CGraphicBase::GetAvailableTextureMemory - D3DDevice is EMPTY");

	static uint32_t s_dwNextUpdateTime=0;
	static uint32_t s_dwTexMemSize=0;//ms_lpd3dDevice->GetAvailableTextureMem();

	uint32_t dwCurTime=ELTimer_GetMSec();
	if (s_dwNextUpdateTime<dwCurTime)
	{
		s_dwNextUpdateTime=dwCurTime+5000;
		s_dwTexMemSize=ms_lpd3dDevice->GetAvailableTextureMem();
	}

	return s_dwTexMemSize;
}

const D3DXMATRIX& CGraphicBase::GetViewMatrix()
{
	return ms_matView;
}

const D3DXMATRIX & CGraphicBase::GetIdentityMatrix()
{
	return ms_matIdentity;
}

void CGraphicBase::SetEyeCamera(float xEye, float yEye, float zEye,
								float xCenter, float yCenter, float zCenter,
								float xUp, float yUp, float zUp)
{
	D3DXVECTOR3 vectorEye(xEye, yEye, zEye);
	D3DXVECTOR3 vectorCenter(xCenter, yCenter, zCenter);
	D3DXVECTOR3 vectorUp(xUp, yUp, zUp);

//	CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
	CCameraManager::Instance().GetCurrentCamera()->SetViewParams(vectorEye, vectorCenter, vectorUp);
	UpdateViewMatrix();
}

void CGraphicBase::SetSimpleCamera(float x, float y, float z, float pitch, float roll)
{
	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 vectorEye(x, y, z);

	pCamera->SetViewParams(D3DXVECTOR3(0.0f, y, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f));
	pCamera->RotateEyeAroundTarget(pitch, roll);
	pCamera->Move(vectorEye);

	UpdateViewMatrix();

	// This is levites's virtual(?) code which you should not trust.
	ms_lpd3dDevice->GetTransform(D3DTS_WORLD, &ms_matWorld);
	D3DXMatrixMultiply(&ms_matWorldView, &ms_matWorld, &ms_matView);
}

void CGraphicBase::SetAroundCamera(float distance, float pitch, float roll, float lookAtZ)
{
	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	pCamera->SetViewParams(D3DXVECTOR3(0.0f, -distance, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f));
	pCamera->RotateEyeAroundTarget(pitch, roll);
	D3DXVECTOR3 v3Target = pCamera->GetTarget();
	v3Target.z = lookAtZ;
	pCamera->SetTarget(v3Target);
// 	pCamera->Move(v3Target);

	UpdateViewMatrix();

	// This is levites's virtual(?) code which you should not trust.
	ms_lpd3dDevice->GetTransform(D3DTS_WORLD, &ms_matWorld);
	D3DXMatrixMultiply(&ms_matWorldView, &ms_matWorld, &ms_matView);
}

void CGraphicBase::SetPositionCamera(float fx, float fy, float fz, float distance, float pitch, float roll)
{
	// I wanna downward this code to the game control level. - [levites]
	if (ms_dwWavingEndTime > CTimer::Instance().GetCurrentMillisecond())
	{
		if (ms_iWavingPower>0)
		{
			fx += float(rand() % ms_iWavingPower) / 10.0f;
			fy += float(rand() % ms_iWavingPower) / 10.0f;
			fz += float(rand() % ms_iWavingPower) / 10.0f;
		}
	}

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pCamera)
		return;

	pCamera->SetViewParams(D3DXVECTOR3(0.0f, -distance, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f));
	pitch = fMIN(80.0f, fMAX(-80.0f, pitch) );
//	Tracef("SetPosition Camera : %f, %f\n", pitch, roll);
	pCamera->RotateEyeAroundTarget(pitch, roll);
	pCamera->Move(D3DXVECTOR3(fx, fy, fz));

	UpdateViewMatrix();

	// This is levites's virtual(?) code which you should not trust.
	STATEMANAGER.GetTransform(D3DTS_WORLD, &ms_matWorld);
	D3DXMatrixMultiply(&ms_matWorldView, &ms_matWorld, &ms_matView);
}

void CGraphicBase::SetOrtho2D(float hres, float vres, float zres)
{
	//CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_ORTHO_CAMERA);
	D3DXMatrixOrthoOffCenterRH(&ms_matProj, 0, hres, vres, 0, 0, zres);
	//UpdatePipeLineMatrix();
	UpdateProjMatrix();
}

void CGraphicBase::SetOrtho3D(float hres, float vres, float zmin, float zmax)
{
	//CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
	D3DXMatrixOrthoRH(&ms_matProj, hres, vres, zmin, zmax);
	//UpdatePipeLineMatrix();
	UpdateProjMatrix();
}

void CGraphicBase::SetPerspective(float fov, float aspect, float nearz, float farz)
{
	ms_fFieldOfView = fov;


	//if (ms_d3dPresentParameter.BackBufferWidth>0 && ms_d3dPresentParameter.BackBufferHeight>0)
	//	ms_fAspect = float(ms_d3dPresentParameter.BackBufferWidth)/float(ms_d3dPresentParameter.BackBufferHeight);
	//else
	ms_fAspect = aspect;

	ms_fNearY = nearz;
	ms_fFarY = farz;

	//CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_PERSPECTIVE_CAMERA);
	D3DXMatrixPerspectiveFovRH(&ms_matProj, D3DXToRadian(fov), ms_fAspect, nearz, farz);		
	//UpdatePipeLineMatrix();
	UpdateProjMatrix();
}

void CGraphicBase::UpdateProjMatrix()
{
	STATEMANAGER.SetTransform(D3DTS_PROJECTION, &ms_matProj);
}

void CGraphicBase::UpdateViewMatrix()
{
	CCamera* pkCamera=CCameraManager::Instance().GetCurrentCamera();
	if (!pkCamera)
		return;

	ms_matView = pkCamera->GetViewMatrix();
	STATEMANAGER.SetTransform(D3DTS_VIEW, &ms_matView);

	D3DXMatrixInverse(&ms_matInverseView, nullptr, &ms_matView);
	ms_matInverseViewYAxis._11 = ms_matInverseView._11;
	ms_matInverseViewYAxis._12 = ms_matInverseView._12;
	ms_matInverseViewYAxis._21 = ms_matInverseView._21;
	ms_matInverseViewYAxis._22 = ms_matInverseView._22;
}

void CGraphicBase::UpdatePipeLineMatrix()
{
	UpdateProjMatrix();
	UpdateViewMatrix();
}

void CGraphicBase::SetViewport(uint32_t dwX, uint32_t dwY, uint32_t dwWidth, uint32_t dwHeight, float fMinZ, float fMaxZ)
{
	ms_Viewport.X = dwX;
	ms_Viewport.Y = dwY;
	ms_Viewport.Width = dwWidth;
	ms_Viewport.Height = dwHeight;
	ms_Viewport.MinZ = fMinZ;
	ms_Viewport.MaxZ = fMaxZ;
}

void CGraphicBase::GetTargetPosition(float * px, float * py, float * pz)
{
	const D3DXVECTOR3& camTarget = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
	*px = camTarget.x;
	*py = camTarget.y;
	*pz = camTarget.z;
}

void CGraphicBase::GetCameraPosition(float * px, float * py, float * pz)
{
	const D3DXVECTOR3& camEye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
	*px = camEye.x;
	*py = camEye.y;
	*pz = camEye.z;
}

void CGraphicBase::GetMatrix(D3DXMATRIX* pRetMatrix) const
{
	assert(ms_lpd3dMatStack != nullptr);
	*pRetMatrix = *ms_lpd3dMatStack->GetTop();
}

const D3DXMATRIX* CGraphicBase::GetMatrixPointer() const
{
	assert(ms_lpd3dMatStack!=nullptr);
	return ms_lpd3dMatStack->GetTop();
}

void CGraphicBase::GetSphereMatrix(D3DXMATRIX * pMatrix, float fValue)
{
	D3DXMatrixIdentity(pMatrix);
	pMatrix->_11 = fValue * ms_matWorldView._11;
	pMatrix->_21 = fValue * ms_matWorldView._21;
	pMatrix->_31 = fValue * ms_matWorldView._31;
	pMatrix->_41 = fValue;
	pMatrix->_12 = -fValue * ms_matWorldView._12;
	pMatrix->_22 = -fValue * ms_matWorldView._22;
	pMatrix->_32 = -fValue * ms_matWorldView._32;
	pMatrix->_42 = -fValue;
}

float CGraphicBase::GetFOV()
{
	return ms_fFieldOfView;
}

float CGraphicBase::GetAspect()
{
	return ms_fAspect;
}

float CGraphicBase::GetNear()
{
	return ms_fNearY;
}

float CGraphicBase::GetFar()
{
	return ms_fFarY;
}

void CGraphicBase::PushMatrix()
{
	ms_lpd3dMatStack->Push();
}

void CGraphicBase::Scale(float x, float y, float z)
{
	ms_lpd3dMatStack->Scale(x, y, z);
}

void CGraphicBase::Rotate(float degree, float x, float y, float z)
{
	D3DXVECTOR3 vec(x, y, z);
	ms_lpd3dMatStack->RotateAxis(&vec, D3DXToRadian(degree));
}

void CGraphicBase::RotateLocal(float degree, float x, float y, float z)
{
	D3DXVECTOR3 vec(x, y, z);
	ms_lpd3dMatStack->RotateAxisLocal(&vec, D3DXToRadian(degree));
}

void CGraphicBase::MultMatrix( const D3DXMATRIX* pMat)
{
	ms_lpd3dMatStack->MultMatrix(pMat);
}

void CGraphicBase::MultMatrixLocal( const D3DXMATRIX* pMat)
{
	ms_lpd3dMatStack->MultMatrixLocal(pMat);
}

void CGraphicBase::RotateYawPitchRollLocal(float fYaw, float fPitch, float fRoll)
{
	ms_lpd3dMatStack->RotateYawPitchRollLocal(D3DXToRadian(fYaw), D3DXToRadian(fPitch), D3DXToRadian(fRoll));
}

void CGraphicBase::Translate(float x, float y, float z)
{
	ms_lpd3dMatStack->Translate(x, y, z);
}

void CGraphicBase::LoadMatrix(const D3DXMATRIX& c_rSrcMatrix)
{
	ms_lpd3dMatStack->LoadMatrix(&c_rSrcMatrix);
}

void CGraphicBase::PopMatrix()
{
	ms_lpd3dMatStack->Pop();
}

uint32_t CGraphicBase::GetColor(float r, float g, float b, float a)
{
	uint8_t argb[4] =
	{
		(uint8_t) (255.0f * b),
		(uint8_t) (255.0f * g),
		(uint8_t) (255.0f * r),
		(uint8_t) (255.0f * a)
	};

	return *((uint32_t *) argb);
}

void CGraphicBase::InitScreenEffect()
{
	ms_dwWavingEndTime = 0;
	ms_dwFlashingEndTime = 0;
	ms_iWavingPower = 0;
	ms_FlashingColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
}

void CGraphicBase::SetScreenEffectWaving(float fDuringTime, int32_t iPower)
{
	ms_dwWavingEndTime = CTimer::Instance().GetCurrentMillisecond() + int32_t(fDuringTime * 1000.0f);
	ms_iWavingPower = iPower;
}

void CGraphicBase::SetScreenEffectFlashing(float fDuringTime, const D3DXCOLOR & c_rColor)
{
	ms_dwFlashingEndTime = CTimer::Instance().GetCurrentMillisecond() + int32_t(fDuringTime * 1000.0f);
	ms_FlashingColor = c_rColor;
}

uint32_t CGraphicBase::GetFaceCount()
{
	return ms_faceCount;
}

void CGraphicBase::ResetFaceCount()
{
	ms_faceCount = 0;
}

HRESULT CGraphicBase::GetLastResult()
{
	return ms_hLastResult;
}

CGraphicBase::CGraphicBase()
{
}

CGraphicBase::~CGraphicBase()
{
}
