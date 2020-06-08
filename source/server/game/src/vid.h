#pragma once
class VID
{
public:
	VID() : m_id(0), m_crc(0)
	{
	}

	VID(uint32_t id, uint32_t crc)
	{
		m_id = id;
		m_crc = crc;
	}

	VID(const VID& rvid)
	{
		*this = rvid;
	}

	const VID& operator = (const VID& rhs)
	{
		m_id = rhs.m_id;
		m_crc = rhs.m_crc;
		return *this;
	}

	bool operator == (const VID& rhs) const
	{
		return (m_id == rhs.m_id) && (m_crc == rhs.m_crc);
	}

	bool operator != (const VID& rhs) const
	{
		return !(*this == rhs);
	}

	operator uint32_t() const
	{
		return m_id;
	}

	void Reset()
	{
		m_id = 0, m_crc = 0;
	}

	uint32_t getCRC() const
	{
		return m_crc;
	}

private:
	uint32_t m_id;
	uint32_t m_crc;
};

namespace std
{
	template <>
	struct hash<VID>
	{
		std::size_t operator()(const VID& k) const
		{
			return k.getCRC();
		}
	};
}
