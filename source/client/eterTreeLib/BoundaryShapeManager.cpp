
#include "StdAfx.h"

#include <Windows.h>
#include "../EterBase/Random.h"
#include "../eterBase/file_ptr.h"
#include "BoundaryShapeManager.h"

using namespace std;
CBoundaryShapeManager::CBoundaryShapeManager() = default;

CBoundaryShapeManager::~CBoundaryShapeManager() = default;

bool CBoundaryShapeManager::LoadBsfFile(const char* pszFilename)
{
	bool bSuccess = true;
	try
	{
		msl::file_ptr fPtr(pszFilename, "rb");
		if (fPtr)
		{

			uint32_t nNumBoundaries;
			if (fread(&nNumBoundaries, sizeof(uint32_t), 1, fPtr.get()) == 1)
			{
				for (uint32_t i = 0; i < nNumBoundaries && bSuccess; ++i)
				{
					SBoundaryShape sShape;
					uint32_t nNumContours;
					if (fread(&nNumContours, sizeof(uint32_t), 1, fPtr.get()) == 1)
					{
						for (uint32_t j = 0; j < nNumContours && bSuccess; ++j)
						{

							vector<SPoint> vPoints;
							uint32_t nNumPoints;
							if (fread(&nNumPoints, sizeof(uint32_t), 1, fPtr.get()) == 1)
							{

								for (uint32_t k = 0; k < nNumPoints && bSuccess; ++k)
								{
									SPoint sPoint;

									if (fread(sPoint.m_afData, sizeof(float), 3, fPtr.get()) == 3)
									{
										vPoints.emplace_back(sPoint);

										if (j == 0 && k == 0)
										{

											memcpy(sShape.m_afMin, sPoint.m_afData, 3 * sizeof(float));
											memcpy(sShape.m_afMax, sPoint.m_afData, 3 * sizeof(float));
										}
										else
										{

											for (int32_t l = 0; l < 3; ++l)
											{
												if (sPoint.m_afData[l] < sShape.m_afMin[l])
													sShape.m_afMin[l] = sPoint.m_afData[l];
												else if (sPoint.m_afData[l] > sShape.m_afMax[l])
													sShape.m_afMax[l] = sPoint.m_afData[l];
											}
										}
									}
									else
									{
										m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read point";
										bSuccess = false;
									}
								}
								sShape.m_vContours.emplace_back(vPoints);
							}
							else
							{
								m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read number of points";
								bSuccess = false;
							}
						}
						m_vBoundaries.emplace_back(sShape);
					}
					else
					{
						m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read number of contours";
						bSuccess = false;
					}
				}
			}
			else
			{
				m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read number of boundaries";
				bSuccess = false;
			}
		}
		else
		{
			m_strCurrentError = string("Error in CBoundaryShapeManager::LoadBsfFile(): Could not open ") + string(pszFilename);
			bSuccess = false;
		}
	}

	catch(...)
	{
		m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Unknown exception";
	}

	return bSuccess;
}
bool CBoundaryShapeManager::PointInside(float fX, float fY)
{
	bool bInside = false;

	for (uint32_t i = 0; i < m_vBoundaries.size() && !bInside; ++i)
		bInside = PointInShape(m_vBoundaries[i], fX, fY);

	return bInside;
}
bool CBoundaryShapeManager::PointInShape(SBoundaryShape& sShape, float fX, float fY)
{
	bool bInside = false;

	for (uint32_t k = 0; k < sShape.m_vContours.size(); ++k)
	{
		for (uint32_t i = 0, j = sShape.m_vContours[k].size() - 1; i < sShape.m_vContours[k].size(); j = i++)
		{
			if ((((sShape.m_vContours[k][i][1] <= fY) && (fY < sShape.m_vContours[k][j][1])) ||
				 ((sShape.m_vContours[k][j][1] <= fY) && (fY < sShape.m_vContours[k][i][1]))) &&
				  (fX < (sShape.m_vContours[k][i][0] - sShape.m_vContours[k][i][0]) * (fY - sShape.m_vContours[k][i][1]) / (sShape.m_vContours[k][j][1] - sShape.m_vContours[k][i][1]) + sShape.m_vContours[k][i][0]))
			{
				bInside = !bInside;
			}
		}
	}

	return bInside;
}
bool CBoundaryShapeManager::RandomPoint(float& fX, float& fY)
{
	bool bSuccess = false;

	if (!m_vBoundaries.empty())
	{

		int32_t nIndex = random_range(0, m_vBoundaries.size() - 1);
		SBoundaryShape& sShape = m_vBoundaries[nIndex];
		fX = frandom(sShape.m_afMin[0], sShape.m_afMax[0]);
		fY = frandom(sShape.m_afMin[1], sShape.m_afMax[1]);
		bSuccess = PointInShape(sShape, fX, fY);
	}

	return bSuccess;
}
