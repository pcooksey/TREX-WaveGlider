#include "response.h"
#include "functions.h"

Response::Response(bitset<1*BYTE> type, bitset<4*BYTE> ID, boost::dynamic_bitset<> message)
    :type(type), ID(ID), message(message), datasize(message.size()/8)
{
    CRC = bitset<2*BYTE>(createCRC());
}

Response::~Response()
{
    //dtor
}

const std::vector<int8_t> Response::packet()
{
    string crc;
    boost::to_string(message,crc);
    string temp = type.to_string()+ID.to_string()+
                  datasize.to_string()+((message.count()==0)?"":crc)+
                  CRC.to_string();

    return binary(temp);
}

int16_t Response::createCRC()
{
    int16_t crc = 0;
    int size = type.size();
    for(unsigned int idx=0; size-- > 0; ++idx)
    {
        crc = computeIncrementally(crc, type[idx]);
    }
    // ID binary
    size = ID.size();
    for(unsigned int idx=0; size-- > 0; ++idx)
    {
        crc = computeIncrementally(crc, ID[idx]);
    }
    // DataSize binary
    size = datasize.size();
    for(unsigned int idx=0; size-- > 0; ++idx)
    {
        crc = computeIncrementally(crc, datasize[idx]);
    }
    // Level 1 command binary
    size = message.size();
    for(unsigned int idx=0; size-- > 0; ++idx)
    {
        crc = computeIncrementally(crc, message[idx]);
    }
    return crc;
}

int16_t Response::computeIncrementally(int16_t crc, bool a)
{
    crc ^= a;
    if((crc & 1)==1)
    {
        crc = (int16_t)((crc >> 1) ^ 0xA001);
    }
    else
    {
        crc = (int16_t)(crc >> 1);
    }
    return crc;
}

Response Response::read(TimeoutSerial &serial)
{
    int readNum = 0;
    // Read the Type, Command ID, and Size
    vector<char> readValues = serial.read(7);
    int readType = static_cast<int>(readValues[readNum++]);
    bitset<1*Response::BYTE> type(readType);
    cout<<type<<endl;
    bitset<4*Response::BYTE> ID;
    int temp = 0;
    for(;readNum<5;++readNum)
    {
        ID <<= sizeof(temp);
        temp = static_cast<int>(readValues[readNum]);
        ID |= temp;
    }
    cout<<ID<<endl;
    int16_t mesgSize = 0;
    mesgSize = static_cast<int>(readValues[readNum++]);
    mesgSize <<= 8;
    mesgSize |= static_cast<int>(readValues[readNum]);
    cout<<bitset<16>(mesgSize)<<endl;
    // Get the message
    boost::dynamic_bitset<> message;
    readValues = serial.read(mesgSize);
    for(int i=readValues.size()-1; i>=0; --i)
    {
        int8_t value = static_cast<int8_t>(readValues[i]);
        boost::dynamic_bitset<> temp(8,value);
        for(int j=0; j<8; ++j)
            message.push_back(temp[j]);
    }
    cout<<message<<endl;

    // Get the CRC
    readValues = serial.read(2);
    bitset<2*Response::BYTE> CRC;

    for(int i=0;i<2;++i)
    {
        CRC <<= sizeof(temp);
        temp = static_cast<int>(readValues[i]);
        CRC |= temp;
    }
    cout<<CRC<<endl;
    return Response(type, ID, message);
}
