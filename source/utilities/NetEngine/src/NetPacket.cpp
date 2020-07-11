#include "../include/NetEngine.hpp"

namespace net_engine
{   
	Packet::Packet(std::shared_ptr <PacketDefinition> definition) :
		m_definition(definition),
		m_data(m_definition->GetSize()),
		m_dynamicData(),
		m_subPackets()
	{
		std::fill(m_data.begin(), m_data.end(), 0);

		// todo: Create instances of sub type fields
		for (auto field : definition->GetFields())
		{
			if (field.second.isSubType)
			{
				auto count = field.second.count;
				if (count == 0)
					count = 1;

				m_subPackets[field.first] = std::vector<std::shared_ptr<Packet>>(count);

				for (auto i = 0u; i < count; i++) 
				{
					m_subPackets[field.first][i] = std::make_shared<Packet>(
						definition->GetNestedType(field.second.subType)
					);
				}
			}
		}
	}
	Packet::~Packet()
	{
	}


	void Packet::__InitializeSubPackets()
	{
		// Copy sub type data to the corresponding instances
		for (auto& subPacket : m_subPackets)
		{
			NET_LOG(LL_SYS, "Copying data for sub packet ", subPacket.first.c_str());

			auto field = m_definition->GetField(subPacket.first);
			int i = 0;

			for (auto& packet : subPacket.second)
			{
				NET_LOG(LL_SYS, "Copy entry id: %d %s", i, packet ? "valid_ptr" : "invalid_ptr");

				// Copy data of this sub-packet to our internal buffer
				std::copy(
					std::next(m_data.begin(), field.position + field.length / field.count * i),
					std::next(m_data.begin(), field.position + field.length / field.count * (i + 1)),
					packet->m_data.begin()
				);

				i++;
			}
		}
	}

	void Packet::__SyncInternalBuffer()
	{
		if (m_definition->IsDynamicSized())
		{
			// Update size
			SetField<uint16_t>("size", GetSize() + m_dynamicData.size() + 1);  // we have to add the header too
		}

		// Copy sub type fields to internal buffer
		for (auto& subPacket : m_subPackets)
		{
			auto field = m_definition->GetField(subPacket.first);
			int i = 0;

			for (auto& packet : subPacket.second)
			{
				// Copy data of this sub-packet to our internal buffer
				std::copy(
					packet->m_data.begin(), packet->m_data.end(),
					std::next(m_data.begin(), field.position + field.length / field.count * i)
				);

				i++;
			}
		}
	}


	void Packet::CopyData(std::vector<uint8_t>& data, uint32_t offset)
	{
		std::copy_n(data.begin(), m_data.size() - offset, m_data.begin() + offset);

		NET_LOG(LL_SYS, "Copied %u (packet data)", m_data.size() - offset);

		__InitializeSubPackets();

		if (m_definition->IsDynamicSized())
		{
			auto size = static_cast<uint16_t>(data.size() - m_data.size() + offset);
			if (HasSequence())
				size -= 1;

			m_dynamicData.reserve(size);
			std::copy_n(data.begin() + m_data.size() - offset, size, std::back_inserter(m_dynamicData));
			NET_LOG(LL_SYS, "Copied %u (dynamic data)", size);

			size += m_data.size();
			if (HasSequence())
				size += 1;
			
			m_data[0] = static_cast<uint8_t>(size);
			m_data[1] = static_cast<uint8_t>(size >> 8);
		}

		if (HasSequence())
		{
			m_sequence = data[data.size() - 1];
			NET_LOG(LL_SYS, "Copied 1 (sequence)");
		}
	}

	void Packet::CopyData(asio::streambuf& buffer)
	{
		auto copied = asio::buffer_copy(asio::buffer(m_data), buffer.data(), m_data.size());
		buffer.consume(copied);

		NET_LOG(LL_SYS, "Consumed %u (packet data)", copied);

		__InitializeSubPackets();

		if (m_definition->IsDynamicSized())
		{
			auto data = buffer.data();
			auto size = buffer.size();

			if (HasSequence())
				size -= 1;

			m_dynamicData.resize(size);
			copied = asio::buffer_copy(asio::buffer(m_dynamicData), data, size);
			buffer.consume(copied);

			NET_LOG(LL_SYS, "Consumed %u (dynamic data)", copied);
		}

		if (m_definition->HasSequence())
		{
			char buf[1];
			std::istream stream(&buffer);
			stream.read(buf, 1);
			m_sequence = buf[0];

			buffer.consume(1);
			NET_LOG(LL_SYS, "Consumed 1 (sequence)");
		}
	}


	std::vector <uint8_t> Packet::GetData()
	{
		__SyncInternalBuffer();

		if (!m_definition->IsDynamicSized())
			return m_data;

		// Create combined data buffer
		std::vector<uint8_t> ret;
		ret.reserve(m_data.size() + m_dynamicData.size());
		ret.insert(ret.end(), m_data.begin(), m_data.end());
		ret.insert(ret.end(), m_dynamicData.begin(), m_dynamicData.end());
		return ret;
	}

	uint32_t Packet::GetSize() const
	{
		return m_definition->GetSize();
	}

	uint8_t Packet::GetHeader() const
	{
		return m_definition->GetHeader();
	}

	void* Packet::GetField(std::string name)
	{
		auto field = m_definition->GetField(name);
		return static_cast<void*>(&m_data[field.position]);
	}

	std::string Packet::GetString(std::string name) const 
	{
		auto field = m_definition->GetField(name);
		auto strPos = reinterpret_cast<const char*>(&m_data[field.position]);
		return std::string(strPos, strnlen(strPos, field.length));
	}

	std::shared_ptr <Packet> Packet::GetRepeatedSubField(std::string name, uint32_t index)
	{
		return m_subPackets[name][index];
	}

	std::shared_ptr <Packet> Packet::GetSubField(std::string name)
	{
		return GetRepeatedSubField(name, 0);
	}

	void Packet::SetRepeatedField(std::string name, uint32_t index, uint8_t* value, uint32_t length)
	{
		auto field = m_definition->GetField(name);
		if (index >= field.count)
			throw std::invalid_argument("Index out of range");

		std::copy(
			value,
			value + length,
			std::next(m_data.begin(), field.position + (field.length / field.count) * index)
		);
	}

	void Packet::SetField(std::string name, uint8_t* value, uint32_t length)
	{
		auto field = m_definition->GetField(name);
		std::copy(value, value + length, std::next(m_data.begin(), field.position));
	}

	void Packet::SetString(std::string name, const char* value, uint32_t length)
	{
		auto field = m_definition->GetField(name);
		if (length > field.length)
		{
			throw std::overflow_error("value is too long to fit");
		}

		std::copy(value, value + length, std::next(m_data.begin(), field.position));
	}

	void Packet::SetString(std::string name, std::string value)
	{
		SetString(name, value.c_str(), value.length());
	}

	void Packet::SetDynamicString(const std::string& str)
	{
		m_dynamicData.assign(str.begin(), str.end());
		m_dynamicData.push_back(0x00);
	}

	std::string Packet::GetDynamicString() const
	{
		std::string str(m_dynamicData.begin(), m_dynamicData.end());
		str.erase(std::find(str.begin(), str.end(), '\0'), str.end());
		return str;
	}

	void Packet::SetDynamicData(std::vector<uint8_t> data)
	{
		m_dynamicData.assign(data.begin(), data.end());
	}

	bool Packet::IsReply() 
	{
		return GetField<uint8_t>("__REFERENCE_TYPE") == 1;
	}

	bool Packet::IsRequest()
	{
		return GetField<uint8_t>("__REFERENCE_TYPE") == 0;
	}

	bool Packet::IsDynamicSized() const
	{
		return m_definition->IsDynamicSized();
	}

	bool Packet::HasSequence() const
	{
		return m_definition->HasSequence();
	}

	uint64_t Packet::GetReferenceId()
	{
		return GetField<uint64_t>("__REFERENCE_ID");
	}





	static PacketManager* gs_pNetPacketManagerInstance = nullptr;

	PacketManager* PacketManager::InstancePtr()
	{
		return gs_pNetPacketManagerInstance;
	}
	PacketManager& PacketManager::Instance()
	{
		assert(gs_pNetPacketManagerInstance);
		return *gs_pNetPacketManagerInstance;
	}

	PacketManager::PacketManager()
	{
		assert(!gs_pNetPacketManagerInstance);
		gs_pNetPacketManagerInstance = this;

	}
	PacketManager::~PacketManager()
	{
		assert(gs_pNetPacketManagerInstance == this);
		gs_pNetPacketManagerInstance = nullptr;
	}

	std::shared_ptr <PacketDefinition> PacketManager::RegisterPacket(const std::string& name, uint8_t header, bool incoming, bool outgoing)
	{
		if (!incoming && !outgoing)
		{
			NET_LOG(LL_ERR, "Tried to register packet %u with incoming & outgoing traffic disabled", header);
			return nullptr;
		}

		if (incoming)
		{
			if (m_incomingPackets.find(header) != m_incomingPackets.end())
			{
				NET_LOG(LL_ERR, "Tried to register packet %u for incoming which is already in use", header);
				return nullptr;
			}
		}
		if (outgoing)
		{
			if (m_incomingPackets.find(header) != m_incomingPackets.end())
			{
				NET_LOG(LL_ERR, "Tried to register packet %u for outgoing which is already in use", header);
				return nullptr;
			}
		}

		auto packet = std::make_shared<PacketDefinition>(name, header);

		NET_LOG(LL_SYS, "Registered %s packet: %s (%u/0x%X)", incoming ? "'incoming'" : "'outgoing'", name.c_str(), header, header);

		if (incoming)
			m_incomingPackets[header] = packet;
		if (outgoing)
			m_outgoingPackets[header] = packet;

		return packet;
	}

	std::shared_ptr <Packet> PacketManager::CreatePacket(uint8_t header, EPacketDirection direction)
	{
		if (direction == EPacketDirection::Incoming)
		{
			if (m_incomingPackets.find(header) != m_incomingPackets.end())
			{
				return std::make_shared<Packet>(m_incomingPackets[header]);
			}
			else
			{
				NET_LOG(LL_ERR, "Failed to find packet %u in incoming packet types", header);
			}
		}
		else if (direction == EPacketDirection::Outgoing)
		{
			if (m_outgoingPackets.find(header) != m_outgoingPackets.end())
			{
				return std::make_shared<Packet>(m_outgoingPackets[header]);
			}
			else
			{
				NET_LOG(LL_ERR, "Failed to find packet %u in outgoing packet types", header);
			}
		}
		return nullptr;
	}
};
