#ifndef RESPONSE_H
#define RESPONSE_H

#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include "stdint.h"
#include <boost/dynamic_bitset.hpp>
#include "TimeoutSerial.h"

using namespace std;

class Response
{
    public:
        static const int BYTE = 8;
        Response(bitset<1*BYTE> type, bitset<4*BYTE> ID, boost::dynamic_bitset<> message);
        virtual ~Response();

        const std::vector<int8_t> packet();

        static Response read(TimeoutSerial &serial);

    protected:
    private:
        int16_t createCRC();
        int16_t computeIncrementally(int16_t crc, bool a);

         // [1 byte] Defines ACK or NAK Data Type
        bitset<1*BYTE> type;
        // [4 bytes] ID value of command
        bitset<4*BYTE> ID;
        // [2 bytes] size of message
        bitset<2*BYTE> datasize;
        // [max 196 bytes] Level 1 ACK or NAK data
        boost::dynamic_bitset<> message;
        // [2 bytes] crc
        bitset<2*BYTE> CRC;

};

#endif // RESPONSE_H
