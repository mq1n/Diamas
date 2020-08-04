#include "stdafx.h"
#include "vector.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

#define _PI    ((float) 3.141592654f)
#define _1BYPI ((float) 0.318309886f)

#define DegreeToRadian( degree ) ((degree) * (_PI / 180.0f))
#define RadianToDegree( radian ) ((radian) * (180.0f / _PI))

void Normalize(VECTOR * pV1, VECTOR * pV2)
{
	float l = (float)sqrt(pV1->x * pV1->x + pV1->y * pV1->y + pV1->z * pV1->z + 1.0e-12);

	pV2->x = pV1->x / l;
	pV2->y = pV1->y / l;
	pV2->z = pV1->z / l;
}

float DotProduct(VECTOR * pV1, VECTOR * pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

float GetDegreeFromPosition(float x, float y)
{   
	VECTOR	vtDir;
	VECTOR	vtStan;
	float	ret;

	vtDir.x = x;
	vtDir.y = y; 
	vtDir.z = 0.0f;

	Normalize(&vtDir, &vtDir); 

	vtStan.x = 0.0f;
	vtStan.y = 1.0f;
	vtStan.z = 0.0f;

	ret = RadianToDegree(acosf(DotProduct(&vtDir, &vtStan)));

	if (vtDir.x < 0.0f)
		ret = 360.0f - ret;

	return (ret);
}   

float GetDegreeFromPositionXY(int32_t sx, int32_t sy, int32_t ex, int32_t ey)
{
	return GetDegreeFromPosition((float)ex - sx, (float)ey - sy);
}

void GetDeltaByDegree(float fDegree, float fDistance, float *x, float *y)
{
	float fRadian = DegreeToRadian(fDegree);

	*x = fDistance * sin(fRadian);
	*y = fDistance * cos(fRadian);
}

float GetDegreeDelta(float iDegree, float iDegree2)
{
	if (iDegree > 180.0f)
		iDegree = iDegree - 360.0f;

	if (iDegree2 > 180.0f)
		iDegree2 = iDegree2 - 360.0f;

	return fabs(iDegree - iDegree2);
}

inline float RadiansFromDegrees(float value)
{
	return value * M_PI / 180.0f;
}

void RotateRegion(int32_t& sx, int32_t& sy, int32_t& ex, int32_t& ey, float xRot, float yRot, float zRot)
{
	xRot = RadiansFromDegrees(xRot);
	yRot = RadiansFromDegrees(yRot);
	zRot = RadiansFromDegrees(zRot);
	Eigen::Vector3f center(
		static_cast<const float>(sx + (ex - sx) / 2),
		static_cast<const float>(sy + (ey - sy) / 2),
		0.0f
	);
	Eigen::Matrix3f m;
	m = Eigen::AngleAxisf(xRot, Eigen::Vector3f::UnitX()) *
		Eigen::AngleAxisf(yRot, Eigen::Vector3f::UnitY()) *
		Eigen::AngleAxisf(zRot, Eigen::Vector3f::UnitZ());
	Eigen::Vector3f start = center + m * (Eigen::Vector3f(static_cast<const float>(sx), static_cast<const float>(sy), 0.0f) - center);
	Eigen::Vector3f end = center + m * (Eigen::Vector3f(static_cast<const float>(ex), static_cast<const float>(ey), 0.0f) - center);
	sx = static_cast<int32_t>(start.x());
	sy = static_cast<int32_t>(start.y());
	ex = static_cast<int32_t>(end.x());
	ey = static_cast<int32_t>(end.y());
	if (sx > ex)
		std::swap(sx, ex);
	if (sy > ey)
		std::swap(sy, ey);
}
