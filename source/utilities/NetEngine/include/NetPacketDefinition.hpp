#pragma once
#include <map>
#include <string>

namespace net_engine
{
	struct Field
	{
		uint32_t position = 0;
		uint32_t length = 0;
		uint32_t count = 1;

		bool isSubType = false;
		std::string subType = "";
	};

	class PacketDefinition
	{
	public:
		PacketDefinition(std::string name, uint8_t header);
		virtual ~PacketDefinition();

		uint8_t GetHeader() const;
		uint32_t GetSize() const;
		const std::string &GetName();
		Field GetField(const std::string& name);
		const std::map <std::string, Field> &GetFields();
		std::shared_ptr <PacketDefinition> GetNestedType(const std::string& name);
		bool IsDynamicSized() const;
		bool HasSequence() const;

		void AddField(std::string name, uint32_t length);
		template <typename T>
		void AddField(std::string name) {
			AddField(name, sizeof(T));
		}

		void AddRepeatedField(std::string name, uint32_t count, uint32_t entryLength);
		template <typename T>
		void AddRepeatedField(std::string name, uint32_t count) {
			AddRepeatedField(name, count, sizeof(T));
		}

		void AddString(std::string name, uint32_t length);
		void AddRepeatedString(std::string name, uint32_t count, uint32_t entryLength);
		void AddSubField(std::string name, std::shared_ptr<PacketDefinition> subType);
		void AddRepeatedSubField(std::string name, uint32_t count, std::shared_ptr<PacketDefinition> subType);

		std::shared_ptr<PacketDefinition> CreateNestedType(const std::string &name);

		/// Adds internal field for packet identifying to allow direct responses
		/// Needs to be activated on request and reply packets!
		void EnableDirectResponse();

		/// Adds a byte field to the packet, used only on receiving packets
		/// Data is validated with stored array for each connection
		void EnableSequence();

		/// Allows to set data after the packet data, a field called "size" from
		/// type unsigned short must exists!
		void EnableDynamicData();

	private:
		std::string m_name;
		uint8_t m_header;

		bool m_sequence;
		bool m_dynamicSizedPacket;

		uint32_t m_size;
		std::map <std::string, Field> m_fields;

		std::map <std::string, std::shared_ptr<PacketDefinition>> m_subDefinitions;
	};
}
