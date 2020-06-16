#pragma once
#include <map>
#include <string>

namespace net_engine
{
    struct Field {
        unsigned int position = 0;
        unsigned int length = 0;
        unsigned int count = 1;

        bool isSubType = false;
        std::string subType = "";
    };

    class PacketDefinition {
       public:
        PacketDefinition(std::string name, unsigned char header);
        virtual ~PacketDefinition();

        uint8_t GetHeader();
        unsigned int GetSize();
        const std::string &GetName();
        Field GetField(std::string name);
        const std::map<std::string, Field> &GetFields();
        std::shared_ptr<PacketDefinition> GetNestedType(std::string name);
        bool IsDynamicSized();

        template <typename T>
        void AddField(std::string name) {
            AddField(name, sizeof(T));
        }

        template <typename T>
        void AddRepeatedField(std::string name, unsigned int count) {
            AddRepeatedField(name, count, sizeof(T));
        }

        void AddField(std::string name, unsigned int length);
        void AddRepeatedField(std::string name, unsigned int count,
                              unsigned int entryLength);
        void AddString(std::string name, unsigned int length);
        void AddRepeatedString(std::string name, unsigned int count,
                               unsigned int entryLength);

        void AddSubField(std::string name,
                         std::shared_ptr<PacketDefinition> subType);
        void AddRepeatedSubField(std::string name, unsigned int count,
                                 std::shared_ptr<PacketDefinition> subType);

        std::shared_ptr<PacketDefinition> CreateNestedType(
            const std::string &name);

        /// Adds internal field for packet identifying to allow direct responses
        /// Needs to be activated on request and reply packets!
        void EnableDirectResponse();

        /// Adds a byte field to the packet, used only on receiving packets
        /// Data is validated with stored array for each connection
        void EnableSequence();
        bool HasSequence();

        /// Allows to set data after the packet data, a field called "size" from
        /// type unsigned short must exists!
        void EnableDynamicData();

       private:
        std::string _name;
        unsigned char _header;

        bool _sequence;
        bool _dynamicSizedPacket;

        unsigned int _size;
        std::map<std::string, Field> _fields;

        std::map<std::string, std::shared_ptr<PacketDefinition>>
            _subDefinitions;
    };
}
