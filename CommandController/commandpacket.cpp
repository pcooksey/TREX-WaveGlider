#include "commandpacket.h"
#include "functions.h"

Command::Command(bitset<1*BYTE> ID, bitset<2*BYTE>  value, bitset<5*BYTE>  message,
                    bitset<4*BYTE>  longitude, bitset<4*BYTE>  latitude)
{
    this->ID = ID;
    this->value = value;
    this->message = message;
    this->longitude = longitude;
    this->latitude = latitude;
    size = (ID.size()+value.size()+message.size()+longitude.size()+latitude.size())/8;
}

Command::~Command()
{
    //dtor
}

bitset<16*Command::BYTE> Command::fullCommand() const
{
    string text = ID.to_string()+value.to_string()+message.to_string()+longitude.to_string()+latitude.to_string();
    return bitset<16*Command::BYTE>(text);
}

string Command::fullCommand()
{
    return ID.to_string()+value.to_string()+message.to_string()+longitude.to_string()+latitude.to_string();
}

int Command::dataSize() const
{
    return size;
}

CommandPacket::CommandPacket(const Command &command):
    header({0x0A}), ID({0x0}), dataSize(command.dataSize()), command(command), CRC(createCRC())
{

}

CommandPacket::~CommandPacket()
{
    //dtor
}

const std::vector<int8_t> CommandPacket::packet()
{
    string temp = header.to_string()+ID.to_string()+
                  littleEndian(dataSize)+command.fullCommand().to_string()+
                  littleEndian(CRC);
    return binary(temp);
}

boost::dynamic_bitset<> CommandPacket::dynamicPacket()
{
    string temp = header.to_string()+ID.to_string()+
                  littleEndian(dataSize)+command.fullCommand().to_string()+
                  littleEndian(CRC);
    return boost::dynamic_bitset<>(temp);
}

string CommandPacket::stringPacket()
{
    return header.to_string()+ID.to_string()+
           littleEndian(dataSize)+command.fullCommand().to_string()+
           littleEndian(CRC);
}

int16_t CommandPacket::createCRC()
{
    int16_t crc = 0;
    crc = iterateBitset(crc, header);
    crc = iterateBitset(crc, ID);
    crc = iterateBitset(crc, littleEndian(dataSize));
    // Level 1 command binary
    bitset<16*Command::BYTE> temp = command.fullCommand();
    crc = iterateBitset(crc, temp);
    return crc;
}
