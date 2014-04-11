#ifndef PAYLOADPACKET_H
#define PAYLOADPACKET_H

#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include "stdint.h"
#include "TimeoutSerial.h"
#include "functions.h"

#define byte(a) a*8
#define add(a,b) a+=b
#define start 0x7e

using namespace std;

class PayloadPacket
{
    public:
        enum Data{ Size, Destination, Origin, TransactionID, MessageType };

        PayloadPacket(): SOF(start) {};
        PayloadPacket(bitset<byte(2)> destination, bitset<byte(2)> origin, bitset<byte(2)> transactionID,
                      bitset<byte(2)> messageType, boost::dynamic_bitset<> message);
        virtual ~PayloadPacket();

        bool read(TimeoutSerial &serial);
        void write(TimeoutSerial &serial);

        boost::dynamic_bitset<> getMessage() { return message; }
        bitset<byte(2)> getData(Data value);

    protected:
    private:
        int16_t generateCRC();

        template<size_t N>
        void readIn(bitset<N>& binary, vector<char>& values, int first, int last);

        const bitset<byte(1)> SOF;
        bitset<byte(2)> packetSize;
        bitset<byte(2)> destination;
        bitset<byte(2)> origin;
        bitset<byte(2)> transactionID;
        bitset<byte(2)> messageType;
        boost::dynamic_bitset<> message;
        bitset<byte(2)> CRC;

};

#endif // PAYLOADPACKET_H
