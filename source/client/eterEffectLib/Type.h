#pragma once

#define Clamp(x, min, max) x = ((x) < (min) ? (min) : (x) < (max) ? (x) : (max));
#define GRAVITY			D3DXVECTOR3(0.0f, 0.0f, -9.8f)

#define	MAX_FRAME		20
#define MAX_TEXTURE		20

typedef	struct	_FVF_POINT
{
	float	x, y, z;
} FVF_POINT;

#ifndef D3DFVF_POINT
#define	D3DFVF_POINT (D3DFVF_XYZ)
#endif


typedef	struct	_FVF_PT
{
	float	x, y, z;
	float	tu, tv;
} FVF_PT;

#ifndef D3DFVF_PT
#define	D3DFVF_PT (D3DFVF_XYZ|D3DFVF_TEX1)
#endif

typedef	struct	_FVF_PDT
{
	float	x, y, z;
	uint32_t	color;
	float	tu, tv;
} FVF_PDT;

#ifndef D3DFVF_PDT
#define	D3DFVF_PDT (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#endif

inline FVF_PDT _FVF_PDT(float x, float y, float z, uint32_t dif, float u, float v)
{
	FVF_PDT	result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.color = dif;
	result.tu = u;
	result.tv = v;
	return	result;
}

enum EEffectType
{
	EFFECT_TYPE_PARTICLE = 1,
	EFFECT_TYPE_ANIMATION_TEXTURE = 2,
	EFFECT_TYPE_MESH = 3,
	EFFECT_TYPE_SIMPLE_LIGHT = 4
};



enum EMeshBillBoardType
{
	MESH_BILLBOARD_TYPE_NONE,
		
	MESH_BILLBOARD_TYPE_ALL,
	MESH_BILLBOARD_TYPE_Y,

	MESH_BILLBOARD_TYPE_MOVE
};

enum EBillBoardType
{
	BILLBOARD_TYPE_NONE,

	BILLBOARD_TYPE_ALL,
	BILLBOARD_TYPE_Y,
	
	BILLBOARD_TYPE_LIE, // 바닥에 누은 형상

	BILLBOARD_TYPE_2FACE, //     / and \ 
	BILLBOARD_TYPE_3FACE, //     / and \ and -

	//BILLBOARD_TYPE_RAY, // 잔상

};

enum EMovingType
{
	MOVING_TYPE_DIRECT,
	MOVING_TYPE_BEZIER_CURVE
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct SEffectPosition
{
	float m_fTime;

	D3DXVECTOR3 m_vecPosition;

	// For Bezier Curve
	int32_t m_iMovingType;
	D3DXVECTOR3 m_vecControlPoint;

} TEffectPosition;

inline bool operator < (const SEffectPosition & lhs, const SEffectPosition & rhs)
{
	return lhs.m_fTime < rhs.m_fTime;
}
inline bool operator < (const float & lhs, const SEffectPosition & rhs)
{
	return lhs < rhs.m_fTime;
}
inline bool operator < (const SEffectPosition & lhs, const float & rhs)
{
	return lhs.m_fTime < rhs;
}

template<typename T>
class CTimeEvent
{
public:
	CTimeEvent(): m_fTime(0)
	{}
	~CTimeEvent() = default;

	float m_fTime;
	T m_Value;
};
#define AG_MASK 0xff00ff00
#define RB_MASK 0x00ff00ff

struct DWORDCOLOR
{
	uint32_t m_dwColor;

	DWORDCOLOR(): m_dwColor(0)
	{
	}
	DWORDCOLOR(const DWORDCOLOR& r)
		: m_dwColor(r.m_dwColor)
	{}

	DWORDCOLOR& operator = (const DWORDCOLOR& r)
	{
		m_dwColor = r.m_dwColor;
		return *this;
	}

	DWORDCOLOR& operator *= (float f)
	{
		const auto idx = static_cast<uint32_t>(f * 256);
		m_dwColor = ((static_cast<uint32_t>(((m_dwColor & AG_MASK) >> 8) * idx)) & AG_MASK) +
			(static_cast<uint32_t>(((m_dwColor & RB_MASK) * idx) >> 8) & RB_MASK);
		return *this;
	}
	DWORDCOLOR& operator += (const DWORDCOLOR& r)
	{
		m_dwColor += r.m_dwColor;
		return *this;
	}
	operator uint32_t() const
	{
		return m_dwColor;
	}
};
#undef AG_MASK
#undef RB_MASK

inline DWORDCOLOR operator * (DWORDCOLOR dc, float f)
{
	DWORDCOLOR tmp(dc);
	tmp *= f;
	return tmp;
}

inline DWORDCOLOR operator * (float f, DWORDCOLOR dc)
{
	DWORDCOLOR tmp(dc);
	tmp *= f;
	return tmp;
}

template <typename T>
__forceinline bool operator < (const CTimeEvent<T> & lhs, const CTimeEvent<T> & rhs)
{
	return lhs.m_fTime < rhs.m_fTime;
}

template <typename T>
__forceinline bool operator < (const CTimeEvent<T> & lhs, const float & rhs)
{
	return lhs.m_fTime < rhs;
}

template <typename T>
__forceinline bool operator < (const float & lhs, const CTimeEvent<T> & rhs)
{
	return lhs < rhs.m_fTime;
}

using TTimeEventTypeCharacter = CTimeEvent<char>;
using TTimeEventTypeShort = CTimeEvent<int16_t>;
using TTimeEventTypeFloat = CTimeEvent<float>;
using TTimeEventTypeWord = CTimeEvent<uint16_t>;
using TTimeEventTypeDoubleWord = CTimeEvent<uint32_t>;
using TTimeEventTypeColor = CTimeEvent<DWORDCOLOR>;
using TTimeEventTypeVector2 = CTimeEvent<D3DXVECTOR2>;
using TTimeEventTypeVector3 = CTimeEvent<D3DXVECTOR3>;

using TTimeEventTable = std::vector<float>;
using TTimeEventTablePosition = std::vector<TEffectPosition>;
using TTimeEventTableCharacter = std::vector<TTimeEventTypeCharacter>;
using TTimeEventTableShort = std::vector<TTimeEventTypeShort>;
using TTimeEventTableFloat = std::vector<TTimeEventTypeFloat>;
using TTimeEventTableWord = std::vector<TTimeEventTypeWord>;
using TTimeEventTableDoubleWord = std::vector<TTimeEventTypeDoubleWord>;
using TTimeEventTableColor = std::vector<TTimeEventTypeColor>;
using TTimeEventTableVector2 = std::vector<TTimeEventTypeVector2>;
using TTimeEventTableVector3 = std::vector<TTimeEventTypeVector3>;


// NOTE : TimeEventValue 함수들은 값을 넘겨 받지 말아야 하는 때도 있으므로
//        값의 직접 리턴이 아닌 포인터 리턴으로 작성 했습니다. - [levites]

template <typename T>
__forceinline void GetTimeEventBlendValue(float fElapsedTime, std::vector<CTimeEvent<T> >& rVector, T * pReturnValue)
{
	if (rVector.empty())
	{
		*pReturnValue = T();
		return;
	}

	if(rVector.begin()+1==rVector.end())
	{
		*pReturnValue = rVector.front().m_Value;
		return;
	}
	
	if (fElapsedTime < rVector.front().m_fTime)
	{
		*pReturnValue = rVector.front().m_Value;
		return;
	}

	if (fElapsedTime > rVector.back().m_fTime)
	{
		*pReturnValue = rVector.back().m_Value;
		return;
	}

	typedef typename std::vector<CTimeEvent<T> >::iterator iterator;

	std::pair<iterator, iterator> result = std::equal_range(rVector.begin(), rVector.end(), fElapsedTime);

	if (result.first != result.second)
		*pReturnValue = result.first->m_Value;
	else
	{
		--result.first;
		float Head = (result.second->m_fTime - fElapsedTime) / (result.second->m_fTime - result.first->m_fTime);
		*pReturnValue = T((result.first->m_Value-result.second->m_Value)*Head+(result.second->m_Value));
	}

}

extern BOOL GetTokenTimeEventFloat(CTextFileLoader & rTextFileLoader, const char * c_szKey, TTimeEventTableFloat * pTimeEventTableFloat);
//extern void InsertItemTimeEventFloat(TTimeEventTableFloat * pTable, float fTime, float fValue);

template <typename T>
void InsertItemTimeEvent(std::vector<CTimeEvent<T> > * pTable, float fTime, T fValue)
{
	auto itor = std::lower_bound(pTable->begin(), pTable->end(), fTime);

	CTimeEvent<T> TimeEvent;
	TimeEvent.m_fTime = fTime;
	TimeEvent.m_Value = fValue;

	pTable->emplace(itor, TimeEvent);
}
