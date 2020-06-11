#include "StdAfx.h"
#include "AffectFlagContainer.h"

CAffectFlagContainer::CAffectFlagContainer()
{
	Clear();
}

CAffectFlagContainer::~CAffectFlagContainer()
{
}

void CAffectFlagContainer::Clear()
{
	memset(m_aElement, 0, sizeof(m_aElement));
}

void CAffectFlagContainer::CopyInstance(const CAffectFlagContainer& c_rkAffectContainer)
{
	memcpy(m_aElement, c_rkAffectContainer.m_aElement, sizeof(m_aElement));
}

void CAffectFlagContainer::CopyData(uint32_t uPos, uint32_t uByteSize, const void* c_pvData)
{
	const uint8_t* c_pbData=(const uint8_t*)c_pvData; 
	Element bMask=0x01;

	uint32_t uBitEnd=uPos+uByteSize*8;
	for (uint32_t i=uPos; i<uBitEnd; ++i)
	{
		Set(i, (*c_pbData & bMask) ? true : false);
		bMask<<=1;

		if (bMask==0)
		{
			++c_pbData;
			bMask=0x01;
		}
	}
}

void CAffectFlagContainer::ConvertToPosition(uint32_t* uRetX, uint32_t* uRetY) const
{
	uint32_t* pos = (uint32_t*)m_aElement;
	*uRetX = pos[0];
	*uRetY = pos[1];
}
/*
const void * CAffectFlagContainer::GetDataPtr(uint32_t uPos) const
{
	if (uPos/8>=BYTE_SIZE)
	{
		return nullptr;
	}

	return (const void *)&m_aElement[uPos];
}
*/

void CAffectFlagContainer::Set(uint32_t uPos, bool isSet)
{
	if (uPos/8>=BYTE_SIZE)
	{
		TraceError("CAffectFlagContainer::Set(uPos=%d>%d, isSet=%d", uPos, BYTE_SIZE*8, isSet);
		return;
	}

	uint8_t& rElement=(uint8_t)m_aElement[uPos/8];

	uint8_t bMask=uint8_t(1<<(uPos&7));
	if (isSet)
		rElement|=bMask;
	else
		rElement&=~bMask;
}

bool CAffectFlagContainer::IsSet(uint32_t uPos) const
{
	if (uPos/8>=BYTE_SIZE)
	{
		TraceError("CAffectFlagContainer::IsSet(uPos=%d>%d", uPos, BYTE_SIZE*8);
		return false;
	}

	const uint8_t& c_rElement=m_aElement[uPos/8];

	uint8_t bMask=uint8_t(1<<(uPos&7));
	if (c_rElement&bMask)
		return true;

	return false;
}
