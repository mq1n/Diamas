#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "PythonPlayer.h"
#include "PythonSystem.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "../eterEffectLib/EffectManager.h"
#include "../eterEffectLib/ParticleSystemData.h"
#include "../eterLib/Camera.h"

float CInstanceBase::ms_fDustGap;
float CInstanceBase::ms_fHorseDustGap;
uint32_t CInstanceBase::ms_adwCRCAffectEffect[EFFECT_NUM];
std::string CInstanceBase::ms_astAffectEffectAttachBone[EFFECT_NUM];

#define BYTE_COLOR_TO_D3DX_COLOR(r, g, b) D3DXCOLOR(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, 1.0f)

/*
D3DXCOLOR CInstanceBase::ms_kD3DXClrPC(0xFFFFD84D);//1.0f, 0.8470f, 0.3f, 1.0f
D3DXCOLOR CInstanceBase::ms_kD3DXClrNPC(0xFF7BE85E);//0.4823f, 0.9098f, 0.3686f, 1.0f
D3DXCOLOR CInstanceBase::ms_kD3DXClrMOB(0xFFEC170a);//0.9254f, 0.0901f, 0.0392f, 1.0f
D3DXCOLOR CInstanceBase::ms_kD3DXClrPVP(0xFF8532D9);
D3DXCOLOR CInstanceBase::ms_kD3DXClrPVPSelf(0xFFEE36DF);
D3DXCOLOR CInstanceBase::ms_kD3DXClrKiller = BYTE_COLOR_TO_D3DX_COLOR(180, 100, 0);
D3DXCOLOR CInstanceBase::ms_kD3DXClrTitle[CInstanceBase::TITLE_MAX_NUM] =
{
	BYTE_COLOR_TO_D3DX_COLOR(  0, 204, 255),
	BYTE_COLOR_TO_D3DX_COLOR(  0, 144, 255),
	BYTE_COLOR_TO_D3DX_COLOR( 92, 110, 255),
	BYTE_COLOR_TO_D3DX_COLOR(155, 155, 255),
	0xFFFFFFFF, // None
	BYTE_COLOR_TO_D3DX_COLOR(207, 117,   0),
	BYTE_COLOR_TO_D3DX_COLOR(235,  83,   0),
	BYTE_COLOR_TO_D3DX_COLOR(227,   0,   0),
	BYTE_COLOR_TO_D3DX_COLOR(255,   0,   0),
};
*/

D3DXCOLOR g_akD3DXClrTitle[CInstanceBase::TITLE_NUM];
D3DXCOLOR g_akD3DXClrName[CInstanceBase::NAMECOLOR_NUM];

std::map<int32_t, std::string> g_TitleNameMap;
std::set<uint32_t> g_kSet_dwPVPReadyKey;
std::set<uint32_t> g_kSet_dwPVPKey;
std::set<uint32_t> g_kSet_dwGVGKey;
std::set<uint32_t> g_kSet_dwDUELKey;

bool g_isEmpireNameMode=false;

void  CInstanceBase::SetEmpireNameMode(bool isEnable)
{
	g_isEmpireNameMode=isEnable;

	if (isEnable)
	{
		g_akD3DXClrName[NAMECOLOR_MOB]=g_akD3DXClrName[NAMECOLOR_EMPIRE_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC]=g_akD3DXClrName[NAMECOLOR_EMPIRE_NPC];
		g_akD3DXClrName[NAMECOLOR_PC]=g_akD3DXClrName[NAMECOLOR_NORMAL_PC];

		for (uint32_t uEmpire=1; uEmpire<EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC+uEmpire]=g_akD3DXClrName[NAMECOLOR_EMPIRE_PC+uEmpire];
		
	}
	else
	{
		g_akD3DXClrName[NAMECOLOR_MOB]=g_akD3DXClrName[NAMECOLOR_NORMAL_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC]=g_akD3DXClrName[NAMECOLOR_NORMAL_NPC];

		for (uint32_t uEmpire=0; uEmpire<EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC+uEmpire]=g_akD3DXClrName[NAMECOLOR_NORMAL_PC];
	}
}

const D3DXCOLOR& CInstanceBase::GetIndexedNameColor(uint32_t eNameColor)
{
	if (eNameColor>=NAMECOLOR_NUM)
	{
		static D3DXCOLOR s_kD3DXClrNameDefault(0xffffffff);
		return s_kD3DXClrNameDefault;
	}

	return g_akD3DXClrName[eNameColor];
}

void CInstanceBase::AddDamageEffect(uint32_t damage,uint8_t flag,BOOL bSelf,BOOL bTarget)
{
	if(CPythonSystem::Instance().IsShowDamage())
	{		
		SEffectDamage sDamage;
		sDamage.bSelf = bSelf;
		sDamage.bTarget = bTarget;
		sDamage.damage = damage;
		sDamage.flag = flag;
		m_DamageQueue.emplace_back(sDamage);
	}
}

void CInstanceBase::ProcessDamage()
{
	if(m_DamageQueue.empty())
		return;

	SEffectDamage sDamage = m_DamageQueue.front();

	m_DamageQueue.pop_front();

	uint32_t damage = sDamage.damage;
	uint8_t flag = sDamage.flag;
	BOOL bSelf = sDamage.bSelf;
	BOOL bTarget = sDamage.bTarget;

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();	
	float cameraAngle = GetDegreeFromPosition2(pCamera->GetTarget().x,pCamera->GetTarget().y,pCamera->GetEye().x,pCamera->GetEye().y);

	uint32_t FONT_WIDTH = 30;
	
	CEffectManager& rkEftMgr=CEffectManager::Instance();

	D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();
	v3Pos.z += float(m_GraphicThingInstance.GetHeight());

	D3DXVECTOR3 v3Rot = D3DXVECTOR3(0.0f, 0.0f, cameraAngle);

	if ( (flag & DAMAGE_DODGE) || (flag & DAMAGE_BLOCK) )
	{
		if(bSelf)
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_MISS],v3Pos,v3Rot);
		else
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_TARGETMISS],v3Pos,v3Rot);
		//__AttachEffect(EFFECT_DAMAGE_MISS);
		return;
	}
//	else if (flag & DAMAGE_CRITICAL)
//	{
//		rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_CRITICAL],v3Pos,v3Rot);
//	}

	std::string strDamageType;
	uint32_t rdwCRCEft = 0;
	/*
	if ( (flag & DAMAGE_POISON) )
	{
		strDamageType = "poison_";
		rdwCRCEft = EFFECT_DAMAGE_POISON;
	}
	else
	*/
	{
		if(bSelf)
		{
			strDamageType = "damage_";
			if(m_bDamageEffectType==0)
				rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE;
			else
				rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE2;
			m_bDamageEffectType = !m_bDamageEffectType;
		}
		else if(bTarget == false)
			return;//현재 적용 안됨.
		else
		{
			strDamageType = "target_";
			rdwCRCEft = EFFECT_DAMAGE_TARGET;
		}
	}
	
	uint32_t index = 0;
	uint32_t num = 0;
	std::vector<std::string> textures;
	while(damage>0)
	{
		if(index > 7)
		{
			TraceError("ProcessDamage Possibility of endless loop");
			break;
		}
		num = damage%10;
		damage /= 10;
		char numBuf[MAX_PATH];
		sprintf_s(numBuf,"%u.dds",num);
		textures.emplace_back("d:/ymir work/effect/affect/damagevalue/" + strDamageType + numBuf);
		
		rkEftMgr.SetEffectTextures(ms_adwCRCAffectEffect[rdwCRCEft],textures);
		
		D3DXMATRIX matrix,matTrans;
		D3DXMatrixIdentity(&matrix);
		matrix._41 = v3Pos.x;
		matrix._42 = v3Pos.y;
		matrix._43 = v3Pos.z;
		D3DXMatrixTranslation(&matrix,v3Pos.x,v3Pos.y,v3Pos.z);
		D3DXMatrixMultiply(&matrix,&pCamera->GetInverseViewMatrix(),&matrix);
		D3DXMatrixTranslation(&matTrans,FONT_WIDTH*index,0,0);
		matTrans._41 = -matTrans._41;
		matrix = matTrans*matrix;
		D3DXMatrixMultiply(&matrix,&pCamera->GetViewMatrix(),&matrix);
		
		rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[rdwCRCEft], D3DXVECTOR3(matrix._41, matrix._42, matrix._43) ,v3Rot);

		
		textures.clear();

		index++;
	}	
}

void CInstanceBase::AttachSpecialEffect(uint32_t effect)
{
	uint32_t dwEffID = __AttachEffect(effect);
	m_specialEffects[effect].push_back(dwEffID);
}

void CInstanceBase::RemoveAttachedSpecialEffect(uint32_t dwEfType)
{
	const auto it = m_specialEffects.find(dwEfType);
	if (it == m_specialEffects.end())
		return;

	for (const uint32_t effID : it->second)
		__DetachEffect(effID);

	m_specialEffects.erase(dwEfType);
}


void CInstanceBase::LevelUp()
{
	__AttachEffect(EFFECT_LEVELUP);
}

void CInstanceBase::SkillUp()
{
	__AttachEffect(EFFECT_SKILLUP);
}

void CInstanceBase::CreateSpecialEffect(uint32_t iEffectIndex)
{
	const D3DXMATRIX & c_rmatGlobal = m_GraphicThingInstance.GetTransform();

	uint32_t dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	uint32_t dwEffectCRC = ms_adwCRCAffectEffect[iEffectIndex];
	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC);
	CEffectManager::Instance().SelectEffectInstance(dwEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(c_rmatGlobal);
}

void CInstanceBase::__EffectContainer_Continue()
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();

	SEffectContainer::Dict::iterator i;
	for (i = rkDctEftID.begin(); i != rkDctEftID.end(); ++i)
		__AttachEffect(i->second);
}

void CInstanceBase::__EffectContainer_Suspend()
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();

	SEffectContainer::Dict::iterator i;
	for (i = rkDctEftID.begin(); i != rkDctEftID.end(); ++i)
		__DetachEffect(i->second);
}

void CInstanceBase::__EffectContainer_Destroy()
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();

	for (auto & i : rkDctEftID)
		__DetachEffect(i.second);

	rkDctEftID.clear();
}

void CInstanceBase::__EffectContainer_Initialize()
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();
	rkDctEftID.clear();	
}

CInstanceBase::SEffectContainer::Dict& CInstanceBase::__EffectContainer_GetDict()
{
	return m_kEffectContainer.m_kDct_dwEftID;
}

// Return value 를 boolean 에서 ID 로 바꿉니다
uint32_t CInstanceBase::__EffectContainer_AttachEffect(uint32_t dwEftKey)
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();
	auto f = rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end()!=f)
		return 0;

	uint32_t dwEftID=__AttachEffect(dwEftKey);
	rkDctEftID.emplace(dwEftKey, dwEftID);
	return dwEftID;
}


void CInstanceBase::__EffectContainer_DetachEffect(uint32_t dwEftKey)
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();
	auto f = rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end()==f)
		return;

	__DetachEffect(f->second);

	rkDctEftID.erase(f);
}

void CInstanceBase::__AttachEmpireEffect(uint32_t eEmpire)
{
	if (!__IsExistMainInstance())
		return;	
	
	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	if (IsWarp())
		return;
	if (IsObject())
		return;
	if (IsFlag())
		return;
	if (IsResource())
		return;
	if (IsInvisibility()) //Assasins spawned guys etc
		return;

	if (!pkInstMain->IsGameMaster())
	{
		if (pkInstMain->IsSameEmpire(*this))
			return;

		if (IsAffect(AFFECT_EUNHYEONG))
			return;
	}

	if (IsGameMaster())
		return;


	if (CPythonBackground::Instance().IsPrimalMap())
		return;

	__EffectContainer_AttachEffect(EFFECT_EMPIRE+eEmpire);
}

void CInstanceBase::__AttachSelectEffect()
{
	__EffectContainer_AttachEffect(EFFECT_SELECT);
}

void CInstanceBase::__DetachSelectEffect()
{
	__EffectContainer_DetachEffect(EFFECT_SELECT);
}

void CInstanceBase::__AttachTargetEffect()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET);
}

void CInstanceBase::__DetachTargetEffect()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET);
}


void CInstanceBase::__StoneSmoke_Inialize()
{
	m_kStoneSmoke.m_dwEftID=0;
}

void CInstanceBase::__StoneSmoke_Destroy()
{
	if (!m_kStoneSmoke.m_dwEftID)
		return;

	__DetachEffect(m_kStoneSmoke.m_dwEftID);
	m_kStoneSmoke.m_dwEftID=0;
}

void CInstanceBase::__StoneSmoke_Create(uint32_t eSmoke)
{
	m_kStoneSmoke.m_dwEftID=m_GraphicThingInstance.AttachSmokeEffect(eSmoke);
}

void CInstanceBase::SetAlpha(float fAlpha)
{
	__SetBlendRenderingMode();
	__SetAlphaValue(fAlpha);
}

bool CInstanceBase::UpdateDeleting()
{
	Update();
	Transform();

	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();

	float fAlpha = __GetAlphaValue() - (rApp.GetGlobalElapsedTime() * 1.5f);
	__SetAlphaValue(fAlpha);

	if (fAlpha < 0.0f)
		return false;

	return true;
}

void CInstanceBase::DeleteBlendOut()
{
	__SetBlendRenderingMode();
	__SetAlphaValue(1.0f);
	DetachTextTail();

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyDeletingCharacterInstance(GetVirtualID());
}

void CInstanceBase::ClearPVPKeySystem()
{
	g_kSet_dwPVPReadyKey.clear();
	g_kSet_dwPVPKey.clear();
	g_kSet_dwGVGKey.clear();
	g_kSet_dwDUELKey.clear();
}

void CInstanceBase::InsertPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.emplace(dwPVPKey);
}

void CInstanceBase::InsertPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPReadyKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.emplace(dwPVPReadyKey);
}

void CInstanceBase::RemovePVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.erase(dwPVPKey);
}

void CInstanceBase::InsertGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID)
{
	uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.emplace(dwGVGKey);
}

void CInstanceBase::RemoveGVGKey(uint32_t dwSrcGuildVID, uint32_t dwDstGuildVID)
{
	uint32_t dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.erase(dwGVGKey);
}

void CInstanceBase::InsertDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwDUELKey.emplace(dwPVPKey);
}

uint32_t CInstanceBase::__GetPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	if (dwVIDSrc>dwVIDDst)
		std::swap(dwVIDSrc, dwVIDDst);

	uint32_t awSrc[2];
	awSrc[0]=dwVIDSrc;
	awSrc[1]=dwVIDDst;

	const auto * s = reinterpret_cast<const uint8_t *>(awSrc);
    const uint8_t * end = s + sizeof(awSrc);
    uint32_t h = 0;

    while (s < end)
    {
        h *= 16777619;
		h ^= *const_cast<uint8_t *>(s++);
    }

    return h;
}

bool CInstanceBase::__FindPVPKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPKey.end()==g_kSet_dwPVPKey.find(dwPVPKey))
		return false;

	return true;
}

bool CInstanceBase::__FindPVPReadyKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPReadyKey.end()==g_kSet_dwPVPReadyKey.find(dwPVPKey))
		return false;

	return true;
}
//길드전시 상대 길드인지 확인할때.
bool CInstanceBase::__FindGVGKey(uint32_t dwSrcGuildID, uint32_t dwDstGuildID)
{
	uint32_t dwGVGKey=__GetPVPKey(dwSrcGuildID, dwDstGuildID);

	if (g_kSet_dwGVGKey.end()==g_kSet_dwGVGKey.find(dwGVGKey))
		return false;

	return true;
}
//대련 모드에서는 대련 상대만 공격할 수 있다.
bool CInstanceBase::__FindDUELKey(uint32_t dwVIDSrc, uint32_t dwVIDDst)
{
	uint32_t dwDUELKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwDUELKey.end()==g_kSet_dwDUELKey.find(dwDUELKey))
		return false;

	return true;
}

bool CInstanceBase::IsPVPInstance(CInstanceBase& rkInstSel)
{
	uint32_t dwVIDSrc=GetVirtualID();
	uint32_t dwVIDDst=rkInstSel.GetVirtualID();

	uint32_t dwGuildIDSrc=GetGuildID();
	uint32_t dwGuildIDDst=rkInstSel.GetGuildID();

	if (GetDuelMode())	//대련 모드일때는 ~_~
		return true;	

	if (CPythonBackground::Instance().IsPrimalMap() && CPythonBackground::Instance().IsPrimalMapAttackable())
		return true;

	return __FindPVPKey(dwVIDSrc, dwVIDDst) || __FindGVGKey(dwGuildIDSrc, dwGuildIDDst);
											//__FindDUELKey(dwVIDSrc, dwVIDDst);
}

const D3DXCOLOR& CInstanceBase::GetNameColor()
{
	return GetIndexedNameColor(GetNameColorIndex());
}

uint32_t CInstanceBase::GetNameColorIndex()
{
	if (IsShop())
		return NAMECOLOR_SHOP;

	if (IsPC())
	{
		if (m_isKiller)
			return NAMECOLOR_PK;

		if (__IsExistMainInstance() && !__IsMainInstance())
		{			
			CInstanceBase* pkInstMain=__GetMainInstancePtr();
			if (!pkInstMain)
			{
				TraceError("CInstanceBase::GetNameColorIndex - MainInstance is nullptr");
				return NAMECOLOR_PC;
			}
			uint32_t dwVIDMain=pkInstMain->GetVirtualID();
			uint32_t dwVIDSelf=GetVirtualID();

			if (CPythonBackground::Instance().IsPrimalMap() && CPythonBackground::Instance().IsPrimalMapAttackable())
				return NAMECOLOR_PVP;

			if (pkInstMain->GetDuelMode())
			{
				switch(pkInstMain->GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return NAMECOLOR_PC + GetEmpireID();
				case DUEL_START:
				{
					if (__FindDUELKey(dwVIDMain, dwVIDSelf))
						return NAMECOLOR_PVP;
					return NAMECOLOR_PC + GetEmpireID();
				}
				}
			}

			if (pkInstMain->IsSameEmpire(*this))
			{
				if (__FindPVPKey(dwVIDMain, dwVIDSelf))
					return NAMECOLOR_PVP;

				uint32_t dwGuildIDMain = pkInstMain->GetGuildID();
				uint32_t dwGuildIDSelf = GetGuildID();
				if (__FindGVGKey(dwGuildIDMain, dwGuildIDSelf))
					return NAMECOLOR_PVP;
			}
			else
				return NAMECOLOR_PVP;
		}

		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		if (rPlayer.IsPartyMemberByVID(GetVirtualID()))
			return NAMECOLOR_PARTY;

		return NAMECOLOR_PC + GetEmpireID();
	}
	if (IsNPC())
		return NAMECOLOR_NPC;
	if (IsEnemy())
		return NAMECOLOR_MOB;
	if (IsPoly())
		return NAMECOLOR_MOB;


	return D3DXCOLOR(0xffffffff);
}

const D3DXCOLOR& CInstanceBase::GetTitleColor()
{
	uint32_t uGrade = GetAlignmentGrade();
	if ( uGrade >= TITLE_NUM)
	{
		static D3DXCOLOR s_kD3DXClrTitleDefault(0xffffffff);
		return s_kD3DXClrTitleDefault;
	}

	return g_akD3DXClrTitle[uGrade];
}

void CInstanceBase::AttachTextTail()
{
	if (m_isTextTail)
	{
		TraceError("CInstanceBase::AttachTextTail - VID [%d] ALREADY EXIST", GetVirtualID());
		return;
	}

	if(strcmp(GetNameString(), "<noname>") == 0)
	{
		// Don't 
		Tracenf("Not attaching text tail to %d, <noname> name.", GetVirtualID());
		return;
	}

	m_isTextTail=true;

	uint32_t dwVID=GetVirtualID();

	float fTextTailHeight=IsMountingHorse() ? 110.0f : 10.0f;

	static D3DXCOLOR s_kD3DXClrTextTail=D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, dwVID, s_kD3DXClrTextTail, fTextTailHeight);

	// CHARACTER_LEVEL
	if (m_dwLevel)
		UpdateTextTailLevel(m_dwLevel);
}

void CInstanceBase::DetachTextTail()
{
	if (!m_isTextTail)
		return;

	m_isTextTail=false;
	CPythonTextTail::Instance().DeleteCharacterTextTail(GetVirtualID());
}

void CInstanceBase::UpdateTextTailLevel(uint32_t level)
{
	//static D3DXCOLOR s_kLevelColor = D3DXCOLOR(119.0f/255.0f, 246.0f/255.0f, 168.0f/255.0f, 1.0f);
	static D3DXCOLOR s_kLevelColor = D3DXCOLOR(152.0f/255.0f, 255.0f/255.0f, 51.0f/255.0f, 1.0f);

	char szText[256];
	sprintf_s(szText, "Lv %u", level);
	m_dwLevel = level;
	CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kLevelColor);
}

void CInstanceBase::RefreshTextTail()
{
	CPythonTextTail::Instance().SetCharacterTextTailColor(GetVirtualID(), GetNameColor());

	int32_t iAlignmentGrade = GetAlignmentGrade();
	if (TITLE_NONE == iAlignmentGrade)
	{
		CPythonTextTail::Instance().DetachTitle(GetVirtualID());
	}
	else
	{
		auto itor = g_TitleNameMap.find(iAlignmentGrade);
		if (g_TitleNameMap.end() != itor)
		{
			const std::string & c_rstrTitleName = itor->second;
			CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
		}
	}
}

void CInstanceBase::RefreshTextTailTitle()
{
	RefreshTextTail();
}

/////////////////////////////////////////////////
void CInstanceBase::__ClearAffectFlagContainer()
{
	m_kAffectFlagContainer.Clear();
}

void CInstanceBase::__ClearAffects()
{
	if (IsStone())
	{
		__StoneSmoke_Destroy();
	}
	else
	{
		for (int32_t iAffect=0; iAffect<AFFECT_NUM; ++iAffect)
		{
			__DetachEffect(m_adwCRCAffectEffect[iAffect]);
			m_adwCRCAffectEffect[iAffect]=0;
		}

		__ClearAffectFlagContainer();
	}

	m_GraphicThingInstance.__OnClearAffects();
}

/////////////////////////////////////////////////

void CInstanceBase::__SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	for (int32_t i=0; i<CAffectFlagContainer::BIT_SIZE; ++i)
	{
		bool isOldSet=m_kAffectFlagContainer.IsSet(i);
		bool isNewSet=c_rkAffectFlagContainer.IsSet(i);

		if (isOldSet != isNewSet)
		{
			__SetAffect(i, isNewSet);

			if (isNewSet)
				m_GraphicThingInstance.__OnSetAffect(i);
			else
				m_GraphicThingInstance.__OnResetAffect(i);
		}
	}

	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);
}

void CInstanceBase::__SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);

	uint32_t eSmoke;
	if (m_kAffectFlagContainer.IsSet(STONE_SMOKE8))
		eSmoke=3;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE5)|m_kAffectFlagContainer.IsSet(STONE_SMOKE6)|m_kAffectFlagContainer.IsSet(STONE_SMOKE7))
		eSmoke=2;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE2)|m_kAffectFlagContainer.IsSet(STONE_SMOKE3)|m_kAffectFlagContainer.IsSet(STONE_SMOKE4))
		eSmoke=1;
	else
		eSmoke=0;

	__StoneSmoke_Destroy();
	__StoneSmoke_Create(eSmoke);
}

void CInstanceBase::SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	if (IsBuilding())
		return;		

	uint32_t dwVnum = GetVirtualNumber();
	BOOL bIsStone = IsStone();

	switch (dwVnum) {
		// Meley's Dragon Statues
		case 6318:
		case 8077:
		case 8078:
		case 8079:
		case 8080:
			bIsStone = false;
			break;
	}

	if (bIsStone)
		__SetStoneSmokeFlagContainer(c_rkAffectFlagContainer);
	else
		__SetNormalAffectFlagContainer(c_rkAffectFlagContainer);
}


void CInstanceBase::SCRIPT_SetAffect(uint32_t eAffect, bool isVisible)
{
	__SetAffect(eAffect, isVisible);
}

void CInstanceBase::__SetReviveInvisibilityAffect(bool isVisible)
{
	if (isVisible) 
	{
		m_GraphicThingInstance.SetReviving(TRUE);
		// NOTE : Dress does not support alpha operations properly.
		if (IsWearingDress())
			return;

		m_GraphicThingInstance.BlendAlphaValue(0.5f, 0.1f);

		if (!__IsMainInstance())
			m_GraphicThingInstance.HideAllAttachingEffect();
	}
	else {
		m_GraphicThingInstance.SetReviving(FALSE);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.1f);
		m_GraphicThingInstance.ShowAllAttachingEffect();
	}
}

void CInstanceBase::__Assassin_SetEunhyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		// NOTE : Dress 를 입고 있으면 Alpha 를 넣지 않는다.
		if (IsWearingDress())
			return;

		if (__IsMainInstance() || __MainCanSeeHiddenThing())
		{
			m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
		}
		else
		{
			m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
			m_GraphicThingInstance.HideAllAttachingEffect();
		}
	}
	else
	{
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
		if (IsAffect(AFFECT_INVISIBILITY) && __MainCanSeeHiddenThing())
			return;
#endif
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);	
		m_GraphicThingInstance.ShowAllAttachingEffect();
	}
}

void CInstanceBase::__Shaman_SetParalysis(bool isParalysis)
{
	m_GraphicThingInstance.SetParalysis(isParalysis);
}



void CInstanceBase::__Warrior_SetGeomgyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (IsWearingDress())
			return;

		if (m_kWarrior.m_dwGeomgyeongEffect)
			__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);

		m_GraphicThingInstance.SetReachScale(1.5f);
		if (m_GraphicThingInstance.IsTwoHandMode())
			m_kWarrior.m_dwGeomgyeongEffect=__AttachEffect(EFFECT_WEAPON+WEAPON_TWOHAND);
		else
			m_kWarrior.m_dwGeomgyeongEffect=__AttachEffect(EFFECT_WEAPON+WEAPON_ONEHAND);
	}
	else
	{
		m_GraphicThingInstance.SetReachScale(1.0f);

		__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);
		m_kWarrior.m_dwGeomgyeongEffect=0;
	}
}

void CInstanceBase::__SetAffect(uint32_t eAffect, bool isVisible)
{
	switch (eAffect)
	{
		case AFFECT_YMIR:
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
			if (IsAffect(AFFECT_INVISIBILITY) && !__MainCanSeeHiddenThing())
#else
			if (IsAffect(AFFECT_INVISIBILITY))
#endif
				return;
				
			if (CPythonBackground::Instance().IsPrimalMap())
				return;
			break;
/*
		case AFFECT_GWIGEOM: // 전기 속성 공격으로 바뀔 예정
			if (isVisible)
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_ELECTRIC_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(ms_adwCRCAffectEffect[EFFECT_ELECTRIC_ATTACH]);
			}
			else
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(0);
			}
			return;
			break;
		case AFFECT_HWAYEOM: // 화염 속성 공격으로 바뀔 예정
			if (isVisible)
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_FLAME_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(ms_adwCRCAffectEffect[EFFECT_FLAME_ATTACH]);
			}
			else
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(0);
			}
			// 화염참은 공격할 때만 일시적으로 Visible 합니다.
			return;
			break;
*/
		case AFFECT_CHEONGEUN:
			m_GraphicThingInstance.SetResistFallen(isVisible);
			break;
		case AFFECT_GEOMGYEONG:
			__Warrior_SetGeomgyeongAffect(isVisible);
			return;
		case AFFECT_REVIVE_INVISIBILITY:
			__SetReviveInvisibilityAffect(isVisible);
			m_kAffectFlagContainer.Set(eAffect, isVisible);
			break;
		case AFFECT_EUNHYEONG:
			__Assassin_SetEunhyeongAffect(isVisible);
			//Set the affect right away so that other effects can be hidden/displayed accordingly
			m_kAffectFlagContainer.Set(eAffect, isVisible); 
			break;
		case AFFECT_GYEONGGONG:
		case AFFECT_KWAESOK:
			// 경공술, 쾌속은 뛸때만 Attaching 시킵니다. - [levites]
			if (isVisible)
				if (!IsWalking())
					return;
			break;
		case AFFECT_INVISIBILITY:
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
			if (__MainCanSeeHiddenThing())
			{
				if (isVisible)
					m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
				else
					m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				break;
			}
#endif
			// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
			if (isVisible)
			{
				m_GraphicThingInstance.HideAllAttachingEffect();
				// m_GraphicThingInstance.ClearAttachingEffect();
				__EffectContainer_Destroy();
				DetachTextTail();
			}
			else
			{
				m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				m_GraphicThingInstance.ShowAllAttachingEffect();
				AttachTextTail();
				RefreshTextTail();
			}
			return;
//		case AFFECT_FAINT:
//			m_GraphicThingInstance.SetFaint(isVisible);
//			break;
//		case AFFECT_SLEEP:
//			m_GraphicThingInstance.SetSleep(isVisible);
//			break;
		case AFFECT_STUN:
			m_GraphicThingInstance.SetSleep(isVisible);
			break;
	}

	if (eAffect>=AFFECT_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetAffect(eAffect:%d<AFFECT_NUM:%d, isVisible=%d)", GetVirtualID(), eAffect, isVisible);
		return;
	}

	if (isVisible)
	{
		if (!m_adwCRCAffectEffect[eAffect])
		{
			m_adwCRCAffectEffect[eAffect]=__AttachEffect(EFFECT_AFFECT+eAffect);
		}
	}
	else
	{
		if (m_adwCRCAffectEffect[eAffect])
		{
			__DetachEffect(m_adwCRCAffectEffect[eAffect]);
			m_adwCRCAffectEffect[eAffect]=0;
		}
	}
}

bool CInstanceBase::IsPossibleEmoticon()
{
	CEffectManager& rkEftMgr=CEffectManager::Instance();
	for(uint32_t eEmoticon = 0; eEmoticon < EMOTICON_NUM; eEmoticon++)
	{
		uint32_t effectID = ms_adwCRCAffectEffect[EFFECT_EMOTICON+eEmoticon];
		if( effectID &&	rkEftMgr.IsAliveEffect(effectID) )
			return false;
	}

	if(ELTimer_GetMSec() - m_dwEmoticonTime < 1000)
	{
		return false;
	}

	return true;
}

void CInstanceBase::SetFishEmoticon()
{
	SetEmoticon(EMOTICON_FISH);
}

void CInstanceBase::SetEmoticon(uint32_t eEmoticon)
{
	if (eEmoticon == 99) { //Fake emoti
		m_dwEmoticonTime = ELTimer_GetMSec();
		return;
	}

	if (eEmoticon>=EMOTICON_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetEmoticon(eEmoticon:%d<EMOTICON_NUM:%d, isVisible=%d)",
			GetVirtualID(), eEmoticon);
		return;
	}
	if (IsPossibleEmoticon())
	{
		D3DXVECTOR3 v3Pos = D3DXVECTOR3(0, 0, float(m_GraphicThingInstance.GetHeight()));
		m_GraphicThingInstance.AttachEffectByID(0, nullptr, ms_adwCRCAffectEffect[EFFECT_EMOTICON+eEmoticon],&v3Pos);
		m_dwEmoticonTime = ELTimer_GetMSec();
	}
}

void CInstanceBase::SetDustGap(float fDustGap)
{
	ms_fDustGap=fDustGap;
}

void CInstanceBase::SetHorseDustGap(float fDustGap)
{
	ms_fHorseDustGap=fDustGap;
}

void CInstanceBase::__DetachEffect(uint32_t dwEID)
{
	m_GraphicThingInstance.DettachEffect(dwEID);
}

uint32_t CInstanceBase::__AttachEffect(uint32_t eEftType)
{
	// 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
#ifdef ENABLE_CANSEEHIDDENTHING_FOR_GM
	if (IsInvisibility() && !__MainCanSeeHiddenThing())
#else
	if (IsInvisibility() && !__IsMainInstance())
#endif
		return 0;

	if (eEftType>=EFFECT_NUM)
		return 0;

	if (ms_astAffectEffectAttachBone[eEftType].empty())
		return m_GraphicThingInstance.AttachEffectByID(0, nullptr, ms_adwCRCAffectEffect[eEftType]);
	std::string & rstrBoneName = ms_astAffectEffectAttachBone[eEftType];
	const char * c_szBoneName;
	// 양손에 붙일 때 사용한다.
		// 이런 식의 예외 처리를 해놓은 것은 캐릭터 마다 Equip 의 Bone Name 이 다르기 때문.
	if ("PART_WEAPON" == rstrBoneName)
	{
		if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szBoneName))
			return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]);
	}
	else if ("PART_WEAPON_LEFT" == rstrBoneName)
	{
		if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szBoneName))
			return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]);
	}
	else
		return m_GraphicThingInstance.AttachEffectByID(0, rstrBoneName.c_str(), ms_adwCRCAffectEffect[eEftType]);

	return 0;
}

void CInstanceBase::__ComboProcess()
{
	/*
	uint32_t dwcurComboIndex = m_GraphicThingInstance.GetComboIndex();

	if (0 != dwcurComboIndex)
	{
		if (m_dwLastComboIndex != m_GraphicThingInstance.GetComboIndex())
		{
			if (!m_GraphicThingInstance.IsHandMode() & IsAffect(AFFECT_HWAYEOM))
			{
				__AttachEffect(EFFECT_FLAME_ATTACK);
			}
		}
	}

	m_dwLastComboIndex = dwcurComboIndex;
	*/
}

bool CInstanceBase::RegisterEffect(uint32_t eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache)
{
	if (eEftType>=EFFECT_NUM)
		return false;

	ms_astAffectEffectAttachBone[eEftType]=c_szEftAttachBone;

	uint32_t& rdwCRCEft=ms_adwCRCAffectEffect[eEftType];
	if (!CEffectManager::Instance().RegisterEffect2(c_szEftName, &rdwCRCEft, isCache))
	{
		TraceError("CInstanceBase::RegisterEffect(eEftType=%d, c_szEftAttachBone=%s, c_szEftName=%s, isCache=%d) - Error", eEftType, c_szEftAttachBone, c_szEftName, isCache);
		rdwCRCEft=0;
		return false;
	}

	return true;
}

void CInstanceBase::RegisterTitleName(int32_t iIndex, const char * c_szTitleName)
{
	g_TitleNameMap.emplace(iIndex, c_szTitleName);
}

D3DXCOLOR __RGBToD3DXColoru(uint32_t r, uint32_t g, uint32_t b)
{
	uint32_t dwColor = 0xff;
	dwColor <<= 8;
	dwColor |= r;
	dwColor <<= 8;
	dwColor |= g;
	dwColor <<= 8;
	dwColor|=b;

	return D3DXCOLOR(dwColor);
}

bool CInstanceBase::RegisterNameColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b)
{
	if (uIndex>=NAMECOLOR_NUM)
		return false;

	g_akD3DXClrName[uIndex]=__RGBToD3DXColoru(r, g, b);
	return true;
}

bool CInstanceBase::RegisterTitleColor(uint32_t uIndex, uint32_t r, uint32_t g, uint32_t b)
{
	if (uIndex>=TITLE_NUM)
		return false;

	g_akD3DXClrTitle[uIndex]=__RGBToD3DXColoru(r, g, b);
	return true;	
}
