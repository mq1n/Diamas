// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
// Source: https://gitlab.com/quantum-core/core
// License https://gitlab.com/quantum-core/core/-/blob/master/LICENSE?ref_type=heads

#include "../include/NetEngine.hpp"
#include <utility>

namespace net_engine
{
	PacketDefinition::PacketDefinition(std::string name, uint8_t header) :
		m_header(header),
		m_name(std::move(name)),
		m_size(0),
		m_fields(),
		m_sequence(false),
		m_dynamicSizedPacket(false)
	{
	}
	PacketDefinition::~PacketDefinition() = default;

	uint8_t PacketDefinition::GetHeader() const
	{
		return m_header;
	}
	uint32_t PacketDefinition::GetSize() const
	{
		return m_size;
	}
	const std::string &PacketDefinition::GetName()
	{
		return m_name;
	}
	Field PacketDefinition::GetField(const std::string& name)
	{
		if (m_fields.find(name) == m_fields.end())
		{
			throw std::runtime_error("Field is not definied");
		}

		return m_fields[name];
	}
	const std::map <std::string, Field> &PacketDefinition::GetFields()
	{
		return m_fields;
	}
	std::shared_ptr <PacketDefinition> PacketDefinition::GetNestedType(const std::string& name)
	{
		return m_subDefinitions[name];
	}
	bool PacketDefinition::IsDynamicSized() const
	{
		return m_dynamicSizedPacket;
	}
	bool PacketDefinition::HasSequence() const
	{
		return m_sequence;
	}

	void PacketDefinition::AddField(std::string name, uint32_t length)
	{
		if (m_fields.find(name) != m_fields.end())
		{
			NET_LOG(LL_ERR, "Field: %s already exist in: %s", name.c_str(), m_name.c_str());
			return;  // todo: throw exception
		}

		Field field;
		field.position = m_size;
		field.length = length;
		m_fields[name] = field;

		m_size += length;

		NET_LOG(LL_TRACE, "Added field %s at %u with length %u in packet %s", name.c_str(), field.position, field.length, m_name.c_str());
	}

	void PacketDefinition::AddRepeatedField(std::string name, uint32_t count, uint32_t entryLength)
	{
		if (m_fields.find(name) != m_fields.end())
		{
			NET_LOG(LL_ERR, "Field: %s already exist in: %s", name.c_str(), m_name.c_str());
			return;  // todo: throw exception
		}

		Field field;
		field.position = m_size;
		field.length = entryLength * count;
		field.count = count;
		m_fields[name] = field;

		m_size += entryLength * count;

		NET_LOG(LL_TRACE, "Added repeated field %s at %u with length %u in packet %s", name.c_str(), field.position, field.length, m_name.c_str());
	}

	void PacketDefinition::AddString(std::string name, uint32_t length)
	{
		if (m_fields.find(name) != m_fields.end())
		{
			NET_LOG(LL_ERR, "Field: %s already exist in: %s", name.c_str(), m_name.c_str());
			return;  // todo: throw exception
		}

		Field field;
		field.length = length;
		field.position = m_size;

		m_fields[name] = field;

		m_size += length;

 		NET_LOG(LL_TRACE, "Added string field %s at %u with length %u in packet %s", name.c_str(), field.position, field.length, m_name.c_str());
	}

	void PacketDefinition::AddRepeatedString(std::string name, uint32_t count, uint32_t entryLength)
	{
		if (m_fields.find(name) != m_fields.end())
		{
			NET_LOG(LL_ERR, "Field: %s already exist in: %s", name.c_str(), m_name.c_str());
			return;  // todo: throw exception
		}

		Field field;
		field.length = count * entryLength;
		field.count = count;
		field.position = m_size;

		m_fields[name] = field;

		m_size += count * entryLength;

  		NET_LOG(LL_TRACE, "Added repeated string field %s at %u with length %u in packet %s", name.c_str(), field.position, field.length, m_name.c_str());
	}

	void PacketDefinition::AddSubField(std::string name, std::shared_ptr<PacketDefinition> subType)
	{
		if (m_fields.find(name) != m_fields.end())
		{
			NET_LOG(LL_ERR, "Field: %s already exist in: %s", name.c_str(), m_name.c_str());
			return;  // todo: throw exception
		}

		Field field;
		field.length = subType->GetSize();
		field.position = m_size;
		field.isSubType = true;
		field.subType = subType->GetName();

		m_fields[name] = field;

		m_size += subType->GetSize();

  		NET_LOG(LL_TRACE, "Added sub type ( %s ) field %s at %u with length %u in packet %s",
		  	subType->GetName().c_str(), name.c_str(), field.position, field.length, m_name.c_str());
	}

	void PacketDefinition::AddRepeatedSubField(std::string name, uint32_t count, std::shared_ptr <PacketDefinition> subType)
	{
		if (m_fields.find(name) != m_fields.end())
		{
			NET_LOG(LL_ERR, "Field: %s already exist in: %s", name.c_str(), m_name.c_str());
			return;  // todo: throw exception
		}

		Field field;
		field.length = subType->GetSize() * count;
		field.count = count;
		field.position = m_size;
		field.isSubType = true;
		field.subType = subType->GetName();

		m_fields[name] = field;

		m_size += subType->GetSize() * count;

   		NET_LOG(LL_TRACE, "Added repeated sub type ( %s ) field %s at %u with length %u in packet %s",
		  	subType->GetName().c_str(), name.c_str(), field.position, field.length, m_name.c_str());
	}

	std::shared_ptr <PacketDefinition> PacketDefinition::CreateNestedType(const std::string &name)
	{
		auto definition = std::make_shared<PacketDefinition>(name, 0);
		m_subDefinitions[name] = definition;
		return definition;
	}

	void PacketDefinition::EnableDirectResponse()
	{
		AddField<uint8_t>("__REFERENCE_TYPE");  // 0 = request, 1 = reply
		AddField<uint64_t>("__REFERENCE_ID");   // unique message id to identify
	}

	void PacketDefinition::EnableSequence()
	{
		m_sequence = true;
	}
	void PacketDefinition::EnableDynamicData()
	{
		m_dynamicSizedPacket = true;
	}
}
