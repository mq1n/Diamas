#include "StdAfx.h"
#include "PythonRenderTargetManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/RenderTargetManager.h"
#include "../EterPythonLib/PythonGraphic.h"
#include "PythonApplication.h"

CPythonRenderTargetManager::CPythonRenderTargetManager() {
	Destroy();
}

CPythonRenderTargetManager::~CPythonRenderTargetManager() {
	Destroy();
}

void CPythonRenderTargetManager::Destroy() {
	for (const auto it : m_renderInstances)
		it.second->Destroy();

	m_renderInstances.clear();
}

void CPythonRenderTargetManager::InitializeRenderInstance(uint32_t index, const char* background_image) {
	// Find and delete if existing already
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end()) {
		it->second->Destroy();
		delete it->second;

		m_renderInstances.erase(index);
	}

	// Create a new instance
	const auto pRenderTarget = new CPythonRenderTarget;
	pRenderTarget->Initialize();
	pRenderTarget->BuildBackround(background_image);
	pRenderTarget->SetRenderIndex(index);
	pRenderTarget->Show(false);

	// Now store it
	m_renderInstances.insert(std::make_pair(index, pRenderTarget));
}

void CPythonRenderTargetManager::Show(uint32_t index, bool show) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->Show(show);
}

void CPythonRenderTargetManager::CreateModelInstance(uint32_t index, uint32_t race_vnum) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->CreateModelInstance(race_vnum);
}

void CPythonRenderTargetManager::SetRace(uint32_t index, uint32_t race_num) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetRace(race_num);
}

void CPythonRenderTargetManager::SetAlwaysRotate(uint32_t index, bool rotate) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetAlwaysRotate(rotate);
}

void CPythonRenderTargetManager::SetRotation(uint32_t index, float rotation) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetRotation(rotation);
}

void CPythonRenderTargetManager::SetArmor(uint32_t index, uint32_t armor_vnum) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetArmor(armor_vnum);
}

void CPythonRenderTargetManager::SetHair(uint32_t index, uint32_t hair_vnum) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetHair(hair_vnum);
}

void CPythonRenderTargetManager::SetWeapon(uint32_t index, uint32_t weapon_vnum) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetWeapon(weapon_vnum);
}

void CPythonRenderTargetManager::SetMotion(uint32_t index, uint32_t motion_index) {
	const auto it = m_renderInstances.find(index);
	if (it != m_renderInstances.end())
		it->second->SetMotion(motion_index);
}

void CPythonRenderTargetManager::RenderBackground() {
	for (const auto it : m_renderInstances)
		it.second->RenderBackground();
}

void CPythonRenderTargetManager::Deform() {
	for (const auto it : m_renderInstances)
		it.second->Deform();
}

void CPythonRenderTargetManager::RenderModel() {
	for (const auto it : m_renderInstances)
		it.second->RenderModel();
}

void CPythonRenderTargetManager::Update() {
	for (const auto it : m_renderInstances)
		it.second->Update();
}


CPythonRenderTargetManager::CPythonRenderTarget::CPythonRenderTarget()
	: 
	m_pModel(nullptr),
	m_pImageBG(nullptr),
	m_bAlwaysRotate(false),
	m_fRotation(0.0f),
	m_bShow(false),
	m_dwRenderIndex(0)
{
}

CPythonRenderTargetManager::CPythonRenderTarget::~CPythonRenderTarget() {
	Destroy();
}

void CPythonRenderTargetManager::CPythonRenderTarget::Initialize() {
	m_pModel = nullptr;
	m_pImageBG = nullptr;
	m_bAlwaysRotate = false;
	m_fRotation = 0.0f;
	m_bShow = false;
	m_dwRenderIndex = 0;
}

void CPythonRenderTargetManager::CPythonRenderTarget::Destroy() {
	m_bAlwaysRotate = false;
	m_fRotation = 0.0f;
	m_bShow = false;
	m_pModel = nullptr;
	m_dwRenderIndex = 0;

	if (m_pImageBG) {
		delete m_pImageBG;
		m_pImageBG = nullptr;
	}
}

bool CPythonRenderTargetManager::CPythonRenderTarget::CreateModelInstance(uint32_t index) {
	// Set basic data
	//	Note: Forcing TYPE_PC due to some annoying warnings when giving it armours and hairstyles
	CInstanceBase::SCreateData kCreateData;
	memset(&kCreateData, 0, sizeof (kCreateData));
	kCreateData.m_bType = (index >= 0 && index < 8) ? CActorInstance::TYPE_PC : CActorInstance::TYPE_NPC;
	kCreateData.m_dwRace = index;

	CInstanceBase* pModel = new CInstanceBase;
	if (pModel->Create(kCreateData)) {
		// Force position to 0, 0, 0
		pModel->NEW_SetPixelPosition({ 0.0f, 0.0f, 0.0f });
		m_pModel = pModel;
		return true;
	}

	delete pModel;
	return false;
}

void CPythonRenderTargetManager::CPythonRenderTarget::SetRace(uint32_t index) {
	if (!CreateModelInstance(index)) {
		m_pModel = nullptr;
		return;
	}

	if (!m_pModel)
		return;

	m_fRotation = 0.0f;

	//m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);
	CCameraManager::Instance().SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	pCam->SetTargetHeight(110.0);
	CCameraManager::Instance().ResetToPreviousCamera();
}

void CPythonRenderTargetManager::CPythonRenderTarget::SetArmor(uint32_t armor_vnum) {
	if (m_pModel)
		m_pModel->ChangeArmor(armor_vnum);
}

void CPythonRenderTargetManager::CPythonRenderTarget::SetHair(uint32_t hair_vnum) {
	if (m_pModel)
		m_pModel->ChangeHair(hair_vnum);
}

void CPythonRenderTargetManager::CPythonRenderTarget::SetWeapon(uint32_t weapon_vnum) {
	if (m_pModel)
		m_pModel->ChangeWeapon(weapon_vnum);
}

void CPythonRenderTargetManager::CPythonRenderTarget::SetMotion(uint32_t motion_index) {
	if (m_pModel) {
		m_pModel->Refresh(motion_index, true);
		m_pModel->SetLoopMotion(motion_index);
	}
}

bool CPythonRenderTargetManager::CPythonRenderTarget::BuildBackround(const char* background_image) {
	if (m_pImageBG)
		return false;

	float fWidth = (float)CPythonApplication::Instance().GetWidth();
	float fHeight = (float)CPythonApplication::Instance().GetHeight();

	m_pImageBG = new CGraphicExpandedImageInstance;
	m_pImageBG->SetImagePointer(static_cast<CGraphicImage*>(CResourceManager::Instance().GetResourcePointer(background_image)));
	m_pImageBG->SetScale(fWidth / (float) m_pImageBG->GetWidth(), fHeight / (float) m_pImageBG->GetHeight());
	return true;
}

void CPythonRenderTargetManager::CPythonRenderTarget::RenderBackground() {
	if (!m_bShow || !m_pImageBG)
		return;

	RECT rectRender;
	if (!CRenderTargetManager::Instance().GetRenderTargetRect(m_dwRenderIndex, rectRender))
		return;

	if (!CRenderTargetManager::Instance().ChangeRenderTarget(m_dwRenderIndex))
		return;

	CRenderTargetManager::Instance().ClearRenderTarget();
	CPythonGraphic::Instance().SetInterfaceRenderState();

	float fx = rectRender.left;
	float fy = rectRender.top;
	float fWidth = (rectRender.right - fx);
	float fHeight = (rectRender.bottom - fy);

	CPythonGraphic::Instance().SetViewport(fx, fy, fWidth, fHeight);

	m_pImageBG->Render();

	CPythonGraphic::Instance().RestoreViewport();
	CRenderTargetManager::Instance().ResetRenderTarget();
}

void CPythonRenderTargetManager::CPythonRenderTarget::Update() {
	if (!m_bShow || !m_pModel)
		return;

	if (m_bAlwaysRotate) {
		if (m_fRotation < 360.0f)
			m_fRotation += 1.0f;
		else
			m_fRotation = 0.0f;
	}

	m_pModel->SetRotation(m_fRotation);
	m_pModel->Transform();
	m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CPythonRenderTargetManager::CPythonRenderTarget::Deform() {
	if (m_bShow && m_pModel)
		m_pModel->Deform();
}

void CPythonRenderTargetManager::CPythonRenderTarget::RenderModel() {
	if (!m_bShow)
		return;

	RECT rectRender;
	if (!CRenderTargetManager::Instance().GetRenderTargetRect(m_dwRenderIndex, rectRender))
		return;

	if (!CRenderTargetManager::Instance().ChangeRenderTarget(m_dwRenderIndex))
		return;

	if (!m_pModel) {
		CRenderTargetManager::Instance().ResetRenderTarget();
		return;
	}

	CPythonGraphic::Instance().ClearDepthBuffer();

	float fFov = CPythonGraphic::Instance().GetFOV();
	float fAspect = CPythonGraphic::Instance().GetAspect();
	float fNearY = CPythonGraphic::Instance().GetNear();
	float fFarY = CPythonGraphic::Instance().GetFar();

	float fx = rectRender.left;
	float fy = rectRender.top;
	float fWidth = (rectRender.right - fx);
	float fHeight = (rectRender.bottom - fy);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, false);
	
	CPythonGraphic::Instance().SetViewport(fx, fy, fWidth, fHeight);

	CCameraManager::Instance().SetCurrentCamera(CCameraManager::RENDER_TARGET_CAMERA);
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();

	CPythonGraphic::Instance().PushState();

	pCam->SetViewParams(
		D3DXVECTOR3(0.0f, -1500.0f, 600.0f),
		D3DXVECTOR3(0.0f, 0.0f, 95.0f),
		D3DXVECTOR3(0.0f, 0.0f, 1.0f)
	);

	UpdateViewMatrix();

	CPythonGraphic::Instance().SetPerspective(10.0f, fWidth / fHeight, 100.0f, 3000.0f);
	m_pModel->Render();

	CCameraManager::Instance().ResetToPreviousCamera();
	CPythonGraphic::Instance().RestoreViewport();
	CPythonGraphic::Instance().PopState();
	CPythonGraphic::Instance().SetPerspective(fFov, fAspect, fNearY, fFarY);
	CRenderTargetManager::Instance().ResetRenderTarget();
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, true);
}
