#pragma once
typedef struct SVector
{
	float x;
	float y;
	float z;
} VECTOR;

extern void		Normalize(VECTOR * pV1, VECTOR * pV2);
extern float    GetDegreeFromPosition(float x, float y);
extern float    GetDegreeFromPositionXY(int32_t sx, int32_t sy, int32_t ex, int32_t ey);
extern void     GetDeltaByDegree(float fDegree, float fDistance, float *x, float *y);
extern float	GetDegreeDelta(float iDegree, float iDegree2);

void RotateRegion(int32_t& sx, int32_t& sy, int32_t& ex, int32_t& ey, float xRot, float yRot, float zRot);
