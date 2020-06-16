#include "../include/NetEngine.hpp"

namespace net_engine
{
    Packet::Packet(std::shared_ptr<PacketDefinition> definition)
        : _definition(definition),
        _data(_definition->GetSize()),
        _dynamicData(),
        _subPackets() {
        std::fill(_data.begin(), _data.end(), 0);

        // todo: Create instances of sub type fields
        for (auto field : definition->GetFields()) {
            if (field.second.isSubType) {
                auto count = field.second.count;
                if (count == 0) count = 1;

                _subPackets[field.first] =
                    std::vector<std::shared_ptr<Packet>>(count);
                for (auto i = 0; i < count; i++) {
                    _subPackets[field.first][i] = std::make_shared<Packet>(
                        definition->GetNestedType(field.second.subType));
                }
            }
        }
    }

    Packet::~Packet() {}

    void Packet::InitializeSubPackets() {
        // Copy sub type data to the corresponding instances
        for (auto& subPacket : _subPackets) {
  //          CORE_LOGGING(trace)
  //              << "Copying data for sub packet " << subPacket.first;
            auto field = _definition->GetField(subPacket.first);
            int i = 0;
            for (auto& packet : subPacket.second) {
  //              CORE_LOGGING(trace) << "Copy entry id " << i << " " << packet
 //                   ? "valid ptr"
//                    : "invalid ptr";
                // Copy data of this sub-packet to our internal buffer
                std::copy(
                    std::next(_data.begin(),
                        field.position + field.length / field.count * i),
                    std::next(
                        _data.begin(),
                        field.position + field.length / field.count * (i + 1)),
                    packet->_data.begin());

                i++;
            }
        }
    }

    void Packet::CopyData(std::vector<uint8_t>& data, unsigned int offset) {
        std::copy_n(data.begin(), _data.size() - offset,
            _data.begin() + offset);
//        CORE_LOGGING(trace)
//            << "Copied " << (_data.size() - offset) << " (packet data)";

        InitializeSubPackets();

        if (_definition->IsDynamicSized()) {
            auto size =
                static_cast<uint16_t>(data.size() - _data.size() + offset);
            if (HasSequence()) {
                size -= 1;
            }

            _dynamicData.reserve(size);
            std::copy_n(data.begin() + _data.size() - offset, size,
                std::back_inserter(_dynamicData));
 //           CORE_LOGGING(trace) << "Copied " << size << " (dynamic data)";

            size += _data.size();
            if (HasSequence()) {
                size += 1;
            }
            _data[0] = static_cast<uint8_t>(size);
            _data[1] = static_cast<uint8_t>(size >> 8);
        }

        if (HasSequence()) {
            _sequence = data[data.size() - 1];
   //         CORE_LOGGING(trace) << "Copied 1 (sequence)";
        }
    }

    void Packet::CopyData(asio::streambuf& buffer) {
        auto copied = asio::buffer_copy(asio::buffer(_data),
            buffer.data(), _data.size());
        buffer.consume(copied);
//        CORE_LOGGING(trace) << "Consumed " << copied << " (packet data)";

        InitializeSubPackets();

        if (_definition->IsDynamicSized()) {
            auto data = buffer.data();
            auto size = buffer.size();
            if (HasSequence()) {
                size -= 1;
            }

            _dynamicData.resize(size);
            copied = asio::buffer_copy(asio::buffer(_dynamicData),
                data, size);
            buffer.consume(copied);
//            CORE_LOGGING(trace) << "Consumed " << copied << " (dynamic data)";
        }

        if (_definition->HasSequence()) {
            char buf[1];
            std::istream stream(&buffer);
            stream.read(buf, 1);
            _sequence = buf[0];

            buffer.consume(1);
  //          CORE_LOGGING(trace) << "Consumed 1 (sequence)";
        }
    }

    std::vector<uint8_t> Packet::GetData() {
        SyncInternalBuffer();

        if (!_definition->IsDynamicSized()) return _data;

        // Create combined data buffer
        std::vector<uint8_t> ret;
        ret.reserve(_data.size() + _dynamicData.size());
        ret.insert(ret.end(), _data.begin(), _data.end());
        ret.insert(ret.end(), _dynamicData.begin(), _dynamicData.end());
        return ret;
    }

    unsigned int Packet::GetSize() { return _definition->GetSize(); }

    uint8_t Packet::GetHeader() { return _definition->GetHeader(); }

    void* Packet::GetField(std::string name) {
        auto field = _definition->GetField(name);
        return static_cast<void*>(&_data[field.position]);
    }

    std::string Packet::GetString(std::string name) {
        auto field = _definition->GetField(name);
        auto strPos = reinterpret_cast<const char*>(&_data[field.position]);
        return std::string(strPos, strnlen(strPos, field.length));
    }

    std::shared_ptr<Packet> Packet::GetRepeatedSubField(std::string name,
        unsigned int index) {
        return _subPackets[name][index];
    }

    std::shared_ptr<Packet> Packet::GetSubField(std::string name) {
        return GetRepeatedSubField(name, 0);
    }

    void Packet::SetRepeatedField(std::string name, unsigned int index,
        uint8_t* value, unsigned int length) {
        auto field = _definition->GetField(name);
        if (index >= field.count)
            throw std::invalid_argument("Index out of range");
        std::copy(
            value, value + length,
            std::next(_data.begin(),
                field.position + (field.length / field.count) * index));
    }

    void Packet::SetField(std::string name, uint8_t* value,
        unsigned int length) {
        auto field = _definition->GetField(name);
        std::copy(value, value + length,
            std::next(_data.begin(), field.position));
    }

    void Packet::SetString(std::string name, const char* value,
        unsigned int length) {
        auto field = _definition->GetField(name);
        if (length > field.length) {
            throw std::overflow_error("value is too long to fit");
        }

        std::copy(value, value + length,
            std::next(_data.begin(), field.position));
    }

    void Packet::SetString(std::string name, std::string value) {
        SetString(name, value.c_str(), value.length());
    }

    void Packet::SetDynamicString(const std::string& str) {
        _dynamicData.assign(str.begin(), str.end());
        _dynamicData.push_back(0x00);
    }

    std::string Packet::GetDynamicString() {
        std::string str(_dynamicData.begin(), _dynamicData.end());
        str.erase(std::find(str.begin(), str.end(), '\0'), str.end());
        return str;
    }

    void Packet::SetDynamicData(std::vector<uint8_t> data) {
        _dynamicData.assign(data.begin(), data.end());
    }

    bool Packet::IsReply() {
        return GetField<uint8_t>("__REFERENCE_TYPE") == 1;
    }

    bool Packet::IsRequest() {
        return GetField<uint8_t>("__REFERENCE_TYPE") == 0;
    }

    bool Packet::IsDynamicSized() { return _definition->IsDynamicSized(); }

    bool Packet::HasSequence() { return _definition->HasSequence(); }

    uint64_t Packet::GetReferenceId() {
        return GetField<uint64_t>("__REFERENCE_ID");
    }

    void Packet::SyncInternalBuffer() {
        if (_definition->IsDynamicSized()) {
            // Update size
            SetField<unsigned short>("size",
                GetSize() + _dynamicData.size() +
                1);  // we have to add the header too
        }

        // Copy sub type fields to internal buffer
        for (auto& subPacket : _subPackets) {
            auto field = _definition->GetField(subPacket.first);
            int i = 0;
            for (auto& packet : subPacket.second) {
                // Copy data of this sub-packet to our internal buffer
                std::copy(
                    packet->_data.begin(), packet->_data.end(),
                    std::next(_data.begin(),
                        field.position + field.length / field.count * i));

                i++;
            }
        }
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

    std::shared_ptr<PacketDefinition> PacketManager::RegisterPacket(
        const std::string& name, uint8_t header, bool incoming, bool outgoing) {
        if (!incoming && !outgoing) {
 //           CORE_LOGGING(warning)
 //               << "Tried to register packet " << header
 //               << " with incoming & outgoing traffic disabled";
            return nullptr;  // todo: throw exception
        }

        if (incoming) {
            if (_incomingPackets.find(header) != _incomingPackets.end()) {
  //              CORE_LOGGING(warning)
  //                  << "Tried to register packet " << header
 //                   << " for incoming which is already in use";
                return nullptr;  // todo: throw exception
            }
        }
        if (outgoing) {
            if (_outgoingPackets.find(header) != _outgoingPackets.end()) {
 //               CORE_LOGGING(warning)
 //                   << "Tried to register packet " << header
  //                  << " for outgoing which is already in use";
                return nullptr;  // todo: throw exception
            }
        }

        auto packet = std::make_shared<PacketDefinition>(name, header);

//        CORE_LOGGING(trace) << "Registered packet " << name << " ("
 //           << (boost::format("0x%02x") % (int)header) << ")";

        if (incoming) _incomingPackets[header] = packet;
        if (outgoing) _outgoingPackets[header] = packet;

        return packet;
    }

    std::shared_ptr<Packet> PacketManager::CreatePacket(uint8_t header,
        Direction direction) {
        if (direction == Direction::Incoming) {
            if (_incomingPackets.find(header) != _incomingPackets.end()) {
                return std::make_shared<Packet>(_incomingPackets[header]);
            }
            else {
 //               CORE_LOGGING(warning) << "Failed to find packet "
 //                   << (boost::format("0x%02x") % (int)header)
  //                  << " in incoming packet types";
            }
        }
        else if (direction == Direction::Outgoing) {
            if (_outgoingPackets.find(header) != _outgoingPackets.end()) {
                return std::make_shared<Packet>(_outgoingPackets[header]);
            }
            else {
 //               CORE_LOGGING(warning) << "Failed to find packet "
  //                  << (boost::format("0x%02x") % (int)header)
  //                  << " in outgoing packet types";
            }
        }
        return nullptr;
    }










	static CPacketContainer* gs_pNetPacketContainerInstance = nullptr;

	CPacketContainer* CPacketContainer::InstancePtr()
	{
		return gs_pNetPacketContainerInstance;
	}
	CPacketContainer& CPacketContainer::Instance()
	{
		assert(gs_pNetPacketContainerInstance);
		return *gs_pNetPacketContainerInstance;
	}

    CPacketContainer::CPacketContainer()
    {
		assert(!gs_pNetPacketContainerInstance);
		gs_pNetPacketContainerInstance = this;

    }
    CPacketContainer::~CPacketContainer()
    {
		assert(gs_pNetPacketContainerInstance == this);
		gs_pNetPacketContainerInstance = nullptr;
    }

	std::shared_ptr <SPacketContext> CPacketContainer::GetPacket(TNetOpcode target_header)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_rec_mutex);

		for (auto packet : m_packets)
		{
			if (packet.get() && packet->header == target_header)
			{
				return packet;
			}
		}
		return {};
	}
	std::string CPacketContainer::GetPacketName(TNetOpcode target_header)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_rec_mutex);

		auto packet = GetPacket(target_header);
		return packet ? packet->name : "";
	}
	std::string CPacketContainer::GetPacketName(const char* buf)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_rec_mutex);

		return GetPacketName(*(TNetOpcode*)buf);
	}
	void CPacketContainer::AppendPacket(TNetOpcode header, const std::string& name, uint8_t type, THandlerFunc handler, bool is_dynamic)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_rec_mutex);

		if (type != PacketTypeIncoming && type != PacketTypeOutgoing && type != PacketTypeBoth)
		{
			NET_LOG(LL_CRI, "Unknown packet type: %u for packet: %u(%s)", type, header, name.c_str());
			return;
		}
		if (GetPacketName(header).length())
		{
			NET_LOG(LL_CRI, "Already registired packet: %u(%s)", header, name.c_str());
			return;
		}

		auto data = std::make_shared<SPacketContext>(header, name, type, handler, is_dynamic);
		m_packets.emplace_back(data);
		NET_LOG(LL_SYS, "Registired packet: %u|0x%02x(%s)", header, header, name.c_str());
	}

	void CPacketContainer::RemovePacket(TNetOpcode header, const std::string& name)
	{
		std::lock_guard <std::recursive_mutex> __lock(m_rec_mutex);

		for (const auto& packet : m_packets)
		{
			if (packet.get() && packet->header == header)
			{
				m_packets.remove(packet);
				NET_LOG(LL_SYS, "Removed packet: %u|0x%02x(%s)", header, header, name.c_str());
				return;
			}
		}
		NET_LOG(LL_CRI, "Not registired packet: %u(%s)", header, name.c_str());
	}
};
