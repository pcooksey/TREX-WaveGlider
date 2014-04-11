#include "payloadpacket.h"

PayloadPacket::PayloadPacket(bitset<byte(2)> destination, bitset<byte(2)> origin, bitset<byte(2)> transactionID,
                             bitset<byte(2)> messageType, boost::dynamic_bitset<> message)
    :SOF(start), destination(destination), origin(origin),
    transactionID(transactionID), messageType(messageType), message(message)
{
    // Add 2 bytes for the CRC and 2 bytes for the size itself
    int size = ((destination.size()+origin.size()+transactionID.size()+messageType.size()+message.size())/8) + 4;
    packetSize = bitset<byte(2)>(size);

    CRC = bitset<byte(2)>(generateCRC());
}

PayloadPacket::~PayloadPacket()
{
    //dtor
}

bool PayloadPacket::read(TimeoutSerial &serial)
{
    vector<char> header = serial.read(11);
    // Check for start of frame which indacates a packet is starting
    if(bitset<byte(1)>(header[0])==SOF)
    {
        cout<<"----------- Starting -----------"<<endl;
        // Read the size of the message reversed (little endian input)
        int8_t num = 0;
        for(int i=2; i>0; --i)
        {
            packetSize <<= 8;
            num = static_cast<int8_t>(header[i]);
            for(int bit=0; bit<8; ++bit)
            {
                if(((num) & (1<<(bit))))
                    packetSize[bit] = 1;
                else
                    packetSize[bit] = 0;
            }
        }
        cout<<"Packet Size: "<<packetSize<<endl;
        // Reading in the rest of the header
        int first = 3;
        readIn(destination, header, first, first+2);
        cout<<"Destination: "<<destination<<endl;
        add(first,2);
        readIn(origin, header, first, first+2);
        cout<<"Origin: "<<origin<<endl;
        add(first,2);
        readIn(transactionID, header, first, first+2);
        cout<<"Transaction ID: "<<transactionID<<endl;
        add(first,2);
        readIn(messageType, header, first, first+2);
        cout<<"MessageType: "<<messageType<<endl;
        // End of the header

        // Determine message size by minusing 10 for header and 2 for CRC
        int messageSize = packetSize.to_ulong()-10-2;

        // Message data starts
        vector<char> messageData = serial.read(messageSize);
        num = 0;
        for(int i=messageSize-1; i>=0; --i)
        {
            bitset<byte(1)> temp(messageData[i]);
            for(int j=0; j<8; ++j)
            {
                message.push_back(temp[j]);
            }
        }
        cout<<"Message: "<<message<<endl;
        // End of message data
        vector<char> crc = serial.read(2);
        readIn(CRC, crc, 0, 2 );
        cout<<"CRC(16): "<<CRC<<endl;

    } else {
        return false;
    }
    return true;
}

void PayloadPacket::write(TimeoutSerial &serial)
{
    string messageStr;
    boost::to_string(message,messageStr);
    string temp = SOF.to_string()+littleEndian(packetSize)+destination.to_string()+
                  origin.to_string()+transactionID.to_string()+messageType.to_string()+
                  messageStr+littleEndian(CRC);
    std::vector<int8_t> packet = binary(temp);
    serial.write(packet);
}

template<size_t N>
void PayloadPacket::readIn(bitset<N>& binary, vector<char>& values, int first, int last)
{
    int8_t num = 0;
    for(int i=first; i<last; ++i )
    {
        binary <<= 8;
        num = static_cast<int8_t>(values[i]);
        for(int bit=0; bit<8; ++bit)
        {
            // Checks if bit is 1
            if(((num) & (1<<(bit))))
                binary[bit] = 1;
            else
                binary[bit] = 0;
        }
    }
    return;
}

bitset<byte(2)> PayloadPacket::getData(Data value)
{
    switch(value)
    {
        case Size:
            return packetSize;
        case Destination:
            return destination;
        case Origin:
            return origin;
        case TransactionID:
            return transactionID;
        case MessageType:
            return messageType;
        default:
            return bitset<byte(2)>();
    }
}

int16_t PayloadPacket::generateCRC()
{
    int16_t crc = 0;

    crc = iterateBitset(crc, littleEndian(packetSize));
    crc = iterateBitset(crc, destination);
    crc = iterateBitset(crc, origin);
    crc = iterateBitset(crc, transactionID);
    crc = iterateBitset(crc, messageType);
    crc = iterateBitset(crc, message);
    return crc;
}
