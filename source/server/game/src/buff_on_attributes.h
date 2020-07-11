#pragma once

class CHARACTER;

class CBuffOnAttributes
{
public:
	CBuffOnAttributes(LPCHARACTER pOwner, uint8_t m_point_type, std::vector <uint8_t>* vec_buff_targets);
	~CBuffOnAttributes();

	void RemoveBuffFromItem(LPITEM pItem);
	void AddBuffFromItem(LPITEM pItem);
	void ChangeBuffValue(uint8_t bNewValue);

	bool On(uint8_t bValue);
	void Off();

	void Initialize();
private:
	LPCHARACTER m_pBuffOwner;
	uint8_t m_bPointType;
	uint8_t m_bBuffValue;
	std::vector <uint8_t>* m_p_vec_buff_wear_targets;

	// apply_type, apply_value
	typedef std::map <uint8_t, int32_t> TMapAttr;
	TMapAttr m_map_additional_attrs;

};
