#ifndef COMMANDPACKET_H
#define COMMANDPACKET_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include "stdint.h"


using namespace std;

// [max 217 bytes] The Level 1 command Packet

class Command {
    public:
        static const int BYTE = 8;
        Command(bitset<1*BYTE> ID, bitset<2*BYTE>  value, bitset<5*BYTE>  message,
                bitset<4*BYTE>  longitude, bitset<4*BYTE>  latitude);
        virtual ~Command();
        // [1 byte] Defines the expected size and format of Command Data
        bitset<1*BYTE> ID;
        // [2 bytes] A number specific to Command Data Type
        bitset<2*BYTE> value;
        // [5 bytes] message for command
        bitset<5*BYTE> message;
        // [4 bytes] longitude
        bitset<4*BYTE> longitude;
        // [4 bytes] latitude
        bitset<4*BYTE> latitude;
        // [2 bytes] Size of data
        int dataSize() const;
        bitset<16*BYTE> fullCommand() const;
        string fullCommand();
    private:
        int size;
};

class CommandPacket
{
    public:
        CommandPacket(const Command &command);
        virtual ~CommandPacket();

        const std::vector<int8_t> packet();
        boost::dynamic_bitset<> dynamicPacket();
        string stringPacket();
    protected:
    private:

        int16_t createCRC();
        // [1 byte] Indicated a Command Data Packet
        bitset<1*Command::BYTE> header;
        // [4 bytes] LRI-definted ID stored in WGMS database
        bitset<4*Command::BYTE> ID;
        // [2 bytes] The size of the Command that follows
        bitset<2*Command::BYTE> dataSize;
        // [max 217 bytes] Level 1 command
        const Command &command;
        // [2 bytes] CRC16
        bitset<2*Command::BYTE> CRC;
};

#endif // COMMANDPACKET_H
