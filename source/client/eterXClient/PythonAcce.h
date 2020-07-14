#pragma once

#ifdef ENABLE_ACCE_SYSTEM

class CPythonAcce : public CSingleton<CPythonAcce>
{
public:
	uint32_t	dwPrice;
	typedef std::vector<TAcceMaterial> TAcceMaterials;

public:
	CPythonAcce();
	virtual ~CPythonAcce();

	void	Clear();
	void	AddMaterial(uint32_t dwRefPrice, uint8_t bPos, TItemPos tPos);
	void	AddResult(uint32_t dwItemVnum, uint32_t dwMinAbs, uint32_t dwMaxAbs);
	void	RemoveMaterial(uint32_t dwRefPrice, uint8_t bPos);
	uint32_t	GetPrice() {return dwPrice;}
	bool	GetAttachedItem(uint8_t bPos, uint8_t & bHere, uint16_t & wCell);
	void	GetResultItem(uint32_t & dwItemVnum, uint32_t & dwMinAbs, uint32_t & dwMaxAbs);

protected:
	TAcceResult	m_vAcceResult;
	TAcceMaterials	m_vAcceMaterials;
};
#endif
