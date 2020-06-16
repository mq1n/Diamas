#include "../include/NetEngine.hpp"

#include <utility>

namespace net_engine {
    PacketDefinition::PacketDefinition(std::string name, unsigned char header)
        : _header(header),
          _name(std::move(name)),
          _size(0),
          _fields(),
          _sequence(false),
          _dynamicSizedPacket(false) {}

    PacketDefinition::~PacketDefinition() = default;

    uint8_t PacketDefinition::GetHeader() { return _header; }

    unsigned int PacketDefinition::GetSize() { return _size; }

    const std::string &PacketDefinition::GetName() { return _name; }

    Field PacketDefinition::GetField(std::string name) {
        if (_fields.find(name) == _fields.end()) {
            throw std::runtime_error("Field is not definied");
        }

        return _fields[name];
    }

    const std::map<std::string, Field> &PacketDefinition::GetFields() {
        return _fields;
    }

    std::shared_ptr<PacketDefinition> PacketDefinition::GetNestedType(
        std::string name) {
        return _subDefinitions[name];
    }

    bool PacketDefinition::IsDynamicSized() { return _dynamicSizedPacket; }

    void PacketDefinition::AddField(std::string name, unsigned int length) {
        if (_fields.find(name) != _fields.end()) {
 //           CORE_LOGGING(warning)
//                << "Field " << name << " already exists in " << _name;
            return;  // todo: throw exception
        }

        Field field;
        field.position = _size;
        field.length = length;
        _fields[name] = field;

        _size += length;

 //       CORE_LOGGING(trace)
 //           << "Added field " << name << " at " << field.position
 //           << " with length " << field.length << " in packet " << _name;
    }

    void PacketDefinition::AddRepeatedField(std::string name,
                                            unsigned int count,
                                            unsigned int entryLength) {
        if (_fields.find(name) != _fields.end()) {
 //           CORE_LOGGING(warning)
 //               << "Field " << name << " already exists in " << _name;
            return;  // todo: throw exception
        }

        Field field;
        field.position = _size;
        field.length = entryLength * count;
        field.count = count;
        _fields[name] = field;

        _size += entryLength * count;

 //       CORE_LOGGING(trace)
//            << "Added repeated field " << name << " at " << field.position
//            << " with length " << field.length << " in packet " << _name;
    }

    void PacketDefinition::AddString(std::string name, unsigned int length) {
        if (_fields.find(name) != _fields.end()) {
 //           CORE_LOGGING(warning)
 //               << "Field " << name << " already exists in " << _name;
            return;  // todo: throw exception
        }

        Field field;
        field.length = length;
        field.position = _size;

        _fields[name] = field;

        _size += length;

  //      CORE_LOGGING(trace)
  //          << "Added string field " << name << " at " << field.position
 //           << " with length " << field.length << " in packet " << _name;
    }

    void PacketDefinition::AddRepeatedString(std::string name,
                                             unsigned int count,
                                             unsigned int entryLength) {
        if (_fields.find(name) != _fields.end()) {
 //           CORE_LOGGING(warning)
//                << "Field " << name << " already exists in " << _name;
            return;  // todo: throw exception
        }

        Field field;
        field.length = count * entryLength;
        field.count = count;
        field.position = _size;

        _fields[name] = field;

        _size += count * entryLength;

  //      CORE_LOGGING(trace) << "Added repeated string field " << name << " at "
 //                           << field.position << " with length " << field.length
  //                          << " in packet " << _name;
    }

    void PacketDefinition::AddSubField(
        std::string name, std::shared_ptr<PacketDefinition> subType) {
        if (_fields.find(name) != _fields.end()) {
 //           CORE_LOGGING(warning)
  //              << "Field " << name << " already exists in " << _name;
            return;  // todo: throw exception
        }

        Field field;
        field.length = subType->GetSize();
        field.position = _size;
        field.isSubType = true;
        field.subType = subType->GetName();

        _fields[name] = field;

        _size += subType->GetSize();

 //       CORE_LOGGING(trace)
 //           << "Added sub type (" << subType->GetName() << ") field " << name
 //           << " at " << field.position << " with length " << field.length
  //          << " in packet " << _name;
    }

    void PacketDefinition::AddRepeatedSubField(
        std::string name, unsigned int count,
        std::shared_ptr<PacketDefinition> subType) {
        if (_fields.find(name) != _fields.end()) {
 //           CORE_LOGGING(warning)
//                << "Field " << name << " already exists in " << _name;
            return;  // todo: throw exception
        }

        Field field;
        field.length = subType->GetSize() * count;
        field.count = count;
        field.position = _size;
        field.isSubType = true;
        field.subType = subType->GetName();

        _fields[name] = field;

        _size += subType->GetSize() * count;

  //      CORE_LOGGING(trace)
  //          << "Added repeated sub type (" << subType->GetName() << ") field "
 //           << name << " at " << field.position << " with length "
//           << field.length << " in packet " << _name;
    }

    std::shared_ptr<PacketDefinition> PacketDefinition::CreateNestedType(
        const std::string &name) {
        auto definition = std::make_shared<PacketDefinition>(name, 0);
        _subDefinitions[name] = definition;
        return definition;
    }

    void PacketDefinition::EnableDirectResponse() {
        AddField<uint8_t>("__REFERENCE_TYPE");  // 0 = request, 1 = reply
        AddField<uint64_t>("__REFERENCE_ID");   // unique message id to identify
    }

    void PacketDefinition::EnableSequence() { _sequence = true; }

    bool PacketDefinition::HasSequence() { return _sequence; }

    void PacketDefinition::EnableDynamicData() { _dynamicSizedPacket = true; }
}  // namespace core::networking
