#pragma once
#include <cstdint>
#include <list>
#include "NetConstants.hpp"
#include "NetLogHelper.hpp"
#include "NetPeer.hpp"
#include "NetPacketDefinition.hpp"
#include "../../../common/singleton.h"
#include "../../../common/packets.h"

namespace net_engine
{
	using TNetOpcode = uint8_t;

	struct SPacketID
	{
		TNetOpcode header{0};
		bool incoming{false};
		bool outgoing{false};
	};

	enum ESecurityLevels : uint8_t
	{
		SECURITY_LEVEL_NONE = 0,  // No handshake
		SECURITY_LEVEL_BASIC = 1, // Basic handshake (No keys or Diffie-Hellman)
		SECURITY_LEVEL_XTEA = 2,  // Key/Pong keys
		SECURITY_LEVEL_KEY_AGREEMENT = 3 // Diffie-Hellman Key agreement
	};

	class Packet
	{
	public:
		explicit Packet(std::shared_ptr <PacketDefinition> definition);
		virtual ~Packet();

		void CopyData(std::vector <uint8_t>& data, uint32_t offset = 0);
		void CopyData(asio::streambuf& buffer);

		void* GetField(std::string name);
		template <typename T>
		T GetField(std::string name) {
			return *reinterpret_cast<T*>(GetField(name));
		}

		void SetField(std::string name, uint8_t* value, uint32_t length);
		template <typename T>
		void SetField(std::string name, T value) {
			SetField(name, reinterpret_cast<uint8_t*>(&value), sizeof(T));
		}

		void SetRepeatedField(std::string name, uint32_t index, uint8_t* value, uint32_t length);
		template <typename T>
		void SetRepeatedField(std::string name, uint32_t index, T value) {
			SetRepeatedField(name, index, reinterpret_cast<uint8_t*>(&value), sizeof(T));
		}

		std::shared_ptr <Packet> GetSubField(std::string name);
		std::shared_ptr <Packet> GetRepeatedSubField(std::string name, uint32_t index);

		std::string GetString(std::string name) const;
		void SetString(std::string name, const char* value, uint32_t length);
		void SetString(std::string name, std::string value);

		void SetDynamicString(const std::string& str);
		std::string GetDynamicString() const;
		void SetDynamicData(std::vector <uint8_t> data);

		bool IsReply();
		bool IsRequest();
		bool IsDynamicSized() const;
		bool HasSequence() const;
		uint64_t GetReferenceId();

		std::vector <uint8_t> GetData();
		uint32_t GetSize() const;
		uint8_t GetHeader() const;
		const std::string& GetName() const { return m_definition->GetName(); }

	private:
		void __InitializeSubPackets();
		void __SyncInternalBuffer();

	private:
		std::shared_ptr <PacketDefinition> m_definition;
		std::map <std::string, std::vector <std::shared_ptr<Packet>>> m_subPackets;

		std::vector <uint8_t> m_data;
		std::vector <uint8_t> m_dynamicData;
		uint8_t m_sequence;
	};

	inline SPacketID CreateIncomingPacketID(TNetOpcode header)
	{
		SPacketID id { header, true, false };
		return id;
	}
	inline SPacketID CreateOutgoingPacketID(TNetOpcode header)
	{
		SPacketID id { header, false, true };
		return id;
	}

	class NetPacketManager : public CSingleton <NetPacketManager>
	{
		using TOnPacketRegister	 = std::function <void(std::shared_ptr <PacketDefinition>)>;

		public:
			NetPacketManager() = default;
			virtual ~NetPacketManager() = default;

			bool IsRegistiredPacket(const SPacketID& packet_id);
			std::shared_ptr <PacketDefinition> GetPacketDefination(const SPacketID& packet_id);

			std::shared_ptr <PacketDefinition> RegisterPacket(const std::string& name, const SPacketID& packet_id, TOnPacketRegister on_register, const std::string& from_func);
			bool DeregisterPacket(const SPacketID& packet_id);

			std::shared_ptr <Packet> CreatePacket(const SPacketID& packet_id);

			void RegisterPackets(bool is_server);

		private:
			std::map <uint8_t, std::shared_ptr <PacketDefinition>> m_incomingPackets;
			std::map <uint8_t, std::shared_ptr <PacketDefinition>> m_outgoingPackets;
	};
};

#ifndef REGISTER_PACKET
	#define REGISTER_PACKET(header, incoming, on_register)\
	if (net_engine::NetPacketManager::InstancePtr())\
	{ net_engine::NetPacketManager::Instance().RegisterPacket(NAMEOF(header).c_str(), { header, incoming, !incoming }, on_register, __FUNCTION__); }
#endif
