#pragma once
#include <cstdint>
#include <list>
#include "NetConstants.hpp"
#include "NetLogHelper.hpp"
#include "NetPeer.hpp"
#include "NetPacketDefinition.hpp"

namespace net_engine
{
	using TNetOpcode = uint8_t;

	enum class EPacketDirection
	{
		Incoming,
		Outgoing
	};

	enum EReservedCommonHeaders : uint8_t
	{
		HEADER_KEY_AGREEMENT = 254,
		HEADER_HANDSHAKE = 255
	};

	enum ReservedGCHeaders : uint8_t
	{
		HEADER_GC_KEY_AGREEMENT_COMPLETED = 252,
		HEADER_GC_PHASE = 253,
		HEADER_GC_KEY_AGREEMENT = HEADER_KEY_AGREEMENT,
		HEADER_GC_HANDSHAKE = HEADER_HANDSHAKE
	};

	enum ReservedCGHeaders : uint8_t
	{
		HEADER_CG_KEY_AGREEMENT = HEADER_KEY_AGREEMENT,
		HEADER_CG_HANDSHAKE = HEADER_HANDSHAKE
	};

	enum EPhases : uint8_t
	{
		PHASE_HANDSHAKE = 1,
		PHASE_LOGIN = 2,
		PHASE_SELECT = 3,
		PHASE_LOADING = 4,
		PHASE_GAME = 5,
		PHASE_AUTH = 10
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

	class PacketManager
	{
		public:
			PacketManager();
			virtual ~PacketManager();

			static PacketManager* InstancePtr();
			static PacketManager& Instance();

			std::shared_ptr <PacketDefinition> RegisterPacket(const std::string& name, uint8_t header, bool incoming, bool outgoing);
			bool DeregisterPacket(uint8_t header, bool incoming, bool outgoing);

			std::shared_ptr <Packet> CreatePacket(uint8_t header, EPacketDirection direction);

		private:
			std::map <uint8_t, std::shared_ptr <PacketDefinition>> m_incomingPackets;
			std::map <uint8_t, std::shared_ptr <PacketDefinition>> m_outgoingPackets;
	};


	using THandlerFunc 		 = std::function <void(std::shared_ptr <Packet> packet)>;
	using TOnPacketRegister	 = std::function <void(std::shared_ptr <PacketDefinition>)>;

	static inline bool RegisterPacket(const std::string& name, TNetOpcode header, THandlerFunc handler, bool incoming, bool outgoing, TOnPacketRegister on_register)
	{ 
		std::shared_ptr <PacketDefinition> def;
		if ((def = PacketManager::Instance().RegisterPacket(name, header, incoming, outgoing)))
		{
			on_register(def);
			return true;
		}
		return false;
	}
	static inline bool DeregisterPacket(TNetOpcode header, bool incoming, bool outgoing)
	{
		return PacketManager::Instance().DeregisterPacket(header, incoming, outgoing);
	}

	void SetupPackets(bool is_server);
};
