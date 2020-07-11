#pragma once
#include <cmath>
#include <cstdint>

struct POS
{
	int32_t x, y, z;
};

extern float GetDegreeFromPositionXY(int64_t sx, int64_t sy, int64_t ex, int64_t ey);
extern float GetRadianFromPositionXY(int64_t sx, int64_t sy, int64_t ex, int64_t ey);

struct GPOS : POS
{
	GPOS();
	GPOS(const POS& rhs);
	const GPOS& operator = (const GPOS& rhs);
	const GPOS& operator = (const POS& rhs);

	GPOS(const int32_t *);
	GPOS(int32_t x, int32_t y);
	GPOS(int32_t x, int32_t y, int32_t z);

	// casting
	operator int32_t* ();
	operator const int32_t* () const;

	// assignment operators
	GPOS& operator += (const GPOS&);
	GPOS& operator -= (const GPOS&);
	GPOS& operator *= (int32_t);
	GPOS& operator /= (int32_t);

	// unary operators
	GPOS operator + () const;
	GPOS operator - () const;

	// binary operators
	GPOS operator + (const GPOS&) const;
	GPOS operator - (const GPOS&) const;
	GPOS operator * (int32_t) const;
	GPOS operator / (int32_t) const;

	friend GPOS operator * (int32_t, const GPOS&);

	bool operator == (const GPOS&) const;
	bool operator != (const GPOS&) const;

	// ??? ???? ? : ?? ??? 0?
	float GetRadian(const GPOS& rPos) const
	{
		return GetRadianFromPositionXY(x, y, rPos.x, rPos.y);
	}

	float GetDegree(float nx, float ny) const
	{
		return GetDegreeFromPositionXY(int64_t(x), int64_t(y), int64_t(nx), int64_t(ny));
	}

	float GetDegree(const GPOS& rPos) const
	{
		return GetDegreeFromPositionXY(x, y, rPos.x, rPos.y);
	}

	void MovePosition(float radian, int32_t Distance)
	{
		x += static_cast<int32_t>(sinf(radian) * float(Distance));
		y += static_cast<int32_t>(cosf(radian) * float(Distance));
	}

	void Rotate(float fRadian)
	{
		auto fx = static_cast<float>(x);
		auto fy = static_cast<float>(y);
		x = static_cast<int32_t>(fx * cosf(fRadian) + fy*sinf(fRadian));
		y = static_cast<int32_t>(fy * cosf(fRadian) - fx*sinf(fRadian));
	}
};

inline GPOS::GPOS()
{
	x = y = z = 0;
}

inline GPOS::GPOS(const POS& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
}

inline const GPOS& GPOS::operator = (const GPOS& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

inline const GPOS& GPOS::operator = (const POS& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

inline GPOS::GPOS(const int32_t *pf)
{
	if (!pf)
		return;

	x = pf[0];
	y = pf[1];
	z = pf[2];
}

inline GPOS::GPOS(int32_t fx, int32_t fy)
{
	x = fx;
	y = fy;
	z = 0;
}

inline GPOS::GPOS(int32_t fx, int32_t fy, int32_t fz)
{
	x = fx;
	y = fy;
	z = fz;
}

// casting
inline GPOS::operator int32_t* ()
{
	return static_cast<int32_t *>(&x);
}

inline GPOS::operator const int32_t* () const
{
	return static_cast<const int32_t *>(&x);
}

// assignment operators
inline GPOS& GPOS::operator += (const GPOS& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

inline GPOS& GPOS::operator -= (const GPOS& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

inline GPOS& GPOS::operator *= (int32_t f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

inline GPOS& GPOS::operator /= (int32_t f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

// unary operators
inline GPOS GPOS::operator + () const
{
	return *this;
}

inline GPOS GPOS::operator - () const
{
	return GPOS(-x, -y, -z);
}

// binary operators
inline GPOS GPOS::operator + (const GPOS& v) const
{
	return GPOS(x + v.x, y + v.y, z + v.z);
}

inline GPOS GPOS::operator - (const GPOS& v) const
{
	return GPOS(x - v.x, y - v.y, z - v.z);
}

inline GPOS GPOS::operator * (int32_t f) const
{
	return GPOS(x * f, y * f, z * f);
}

inline GPOS GPOS::operator / (int32_t f) const
{
	int32_t fInv = f;
	return GPOS(x / fInv, y / fInv, z / fInv);
}

inline GPOS operator * (int32_t f, const GPOS& v)
{
	return GPOS(f * v.x, f * v.y, f * v.z);
}

inline bool GPOS::operator == (const GPOS& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

inline bool GPOS::operator != (const GPOS& v) const
{
	return x != v.x || y != v.y || z != v.z;
}
