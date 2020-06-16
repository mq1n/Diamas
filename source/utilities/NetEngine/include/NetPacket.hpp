#pragma once
#include <cstdint>
#include <list>
#include "NetConstants.hpp"
#include "NetLogHelper.hpp"
#include "NetPeer.hpp"
#include "NetPacketDefinition.hpp"

namespace net_engine
{
	using TNetOpcode   = uint16_t;
	using THandlerFunc = std::function <std::size_t /* sent_length */(const void* /* data */, std::size_t /* length */)>;

	enum EPacketTypes : uint8_t
	{
		PacketTypeNone,
		PacketTypeIncoming,
		PacketTypeOutgoing,
		PacketTypeBoth
	};
	struct SPacketContext
	{
		TNetOpcode header;
		std::string name;
		uint8_t type;
		THandlerFunc handler;
		bool is_dynamic;

		SPacketContext(TNetOpcode header, std::string name, uint8_t type, THandlerFunc handler, bool is_dynamic)
		{
			this->header = header;
			this->name = name;
			this->type = type;
			this->handler = handler;
			this->is_dynamic = is_dynamic;
		}
	};

	class Packet
	{
	public:
		explicit Packet(std::shared_ptr<PacketDefinition> definition);
		virtual ~Packet();

		void CopyData(std::vector<uint8_t>& data, unsigned int offset = 0);
		void CopyData(asio::streambuf& buffer);
		std::vector<uint8_t> GetData();
		unsigned int GetSize();
		uint8_t GetHeader();
		const std::string& GetName() const { return _definition->GetName(); }

		void* GetField(std::string name);

		template <typename T>
		T GetField(std::string name) {
			return *reinterpret_cast<T*>(GetField(name));
		}

		std::string GetString(std::string name);

		std::shared_ptr<Packet> GetRepeatedSubField(std::string name,
			unsigned int index);
		std::shared_ptr<Packet> GetSubField(std::string name);

		void SetRepeatedField(std::string name, unsigned int index,
			uint8_t* value, unsigned int length);
		void SetField(std::string name, uint8_t* value, unsigned int length);

		void SetString(std::string name, const char* value,
			unsigned int length);
		void SetString(std::string name, std::string value);

		template <typename T>
		void SetField(std::string name, T value) {
			SetField(name, reinterpret_cast<uint8_t*>(&value), sizeof(T));
		}

		template <typename T>
		void SetRepeatedField(std::string name, unsigned int index, T value) {
			SetRepeatedField(name, index, reinterpret_cast<uint8_t*>(&value),
				sizeof(T));
		}

		void SetDynamicString(const std::string& str);
		std::string GetDynamicString();
		void SetDynamicData(std::vector<uint8_t> data);

		bool IsReply();
		bool IsRequest();
		bool IsDynamicSized();
		bool HasSequence();
		uint64_t GetReferenceId();

	private:
		void InitializeSubPackets();
		void SyncInternalBuffer();

	private:
		std::shared_ptr<PacketDefinition> _definition;
		std::map<std::string, std::vector<std::shared_ptr<Packet>>> _subPackets;

		std::vector<uint8_t> _data;
		std::vector<uint8_t> _dynamicData;
		uint8_t _sequence;
	};


	enum Direction { Incoming, Outgoing };

	class PacketManager {
	public:
		PacketManager();
		virtual ~PacketManager();

		static PacketManager* InstancePtr();
		static PacketManager& Instance();

		std::shared_ptr<PacketDefinition> RegisterPacket(
			const std::string& name, uint8_t header, bool incoming,
			bool outgoing);

		std::shared_ptr<Packet> CreatePacket(uint8_t header,
			Direction direction = Outgoing);

	private:
		std::map<uint8_t, std::shared_ptr<PacketDefinition>> _incomingPackets;
		std::map<uint8_t, std::shared_ptr<PacketDefinition>> _outgoingPackets;
	};









	class CPacketContainer
	{
		public:
			CPacketContainer();
			~CPacketContainer();

			static CPacketContainer* InstancePtr();
			static CPacketContainer& Instance();

			std::shared_ptr <SPacketContext> GetPacket(TNetOpcode target_header);
			std::string GetPacketName(TNetOpcode target_header);
			std::string GetPacketName(const char* buf);
			void AppendPacket(TNetOpcode header, const std::string& name, uint8_t type, THandlerFunc handler, bool is_dynamic);
			void RemovePacket(TNetOpcode header, const std::string& name);

		private:
			mutable std::recursive_mutex m_rec_mutex;
			std::list <std::shared_ptr<SPacketContext>> m_packets;
	};
	

#pragma pack(1)
	struct SPacketHeader
	{
		TNetOpcode opcode;
		uint8_t version;
		uint32_t flags;
		uint32_t size;

		SPacketHeader() : 
			version(0), flags(0), opcode(0), size(0)
		{
			// default ctor
		}
		explicit SPacketHeader(uint8_t version_, uint32_t flags_, TNetOpcode opcode_, uint32_t size_)
		{
			version = version_;
			flags = flags_;
			opcode = opcode_;
			size = size_;
		}
		explicit SPacketHeader(TNetOpcode opcode_)
		{
			version = PACKET_VERSION;
			flags = 0; // PACKET_FLAG_RAW;
			size = 0;
			opcode = opcode_;
		}
		explicit SPacketHeader(const char* buf_)
		{
			version = PACKET_VERSION;
			flags = 0; // PACKET_FLAG_RAW;
			size = 0;
			opcode = *(TNetOpcode*)buf_;
		}
	};

	struct SNetPacket
	{
		SPacketHeader m_header;

		explicit SNetPacket(TNetOpcode header_) :
			m_header(header_)
		{
			// ctor
		}
		explicit SNetPacket(const char* buf)
		{
			// copy
			memcpy(&m_header, buf, sizeof(m_header));
		}

		const char* data() const
		{
			return reinterpret_cast<const char*>(this);
		}

		size_t size() const
		{
			return m_header.size;
		}
		void set_size(size_t len_)
		{
			m_header.size = len_;
		}

		const char* body() const
		{
			return reinterpret_cast<const char*>(this) + sizeof(SNetPacket);
		}
	};
#pragma pack()

	constexpr auto packet_header_min_size = sizeof(SPacketHeader);
}

#define NET_DECLARE_PACKET(header, context)\
	context() : SNetPacket(header)\
	{\
		set_size(sizeof(context));\
	}
