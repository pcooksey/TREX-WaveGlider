#include <iostream>
#include <vector>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <boost/asio.hpp>
#include "TimeoutSerial.h"
#include "payloadpacket.h"
#include "commandpacket.h"
#include "response.h"
#include "functions.h"

using namespace std;
using namespace boost::asio;

enum Commands { LightOn = 0x0220, LightOff = 0x0120,
                RightRudder = 0x0400, LeftRudder = 0x0500, CenterRudder = 0x0600,
                FixedHeading = 0x0800, HoldStation =0x0200};

void enumerateMessage(TimeoutSerial &serial, PayloadPacket &payload);
void requestStatus(TimeoutSerial &serial, PayloadPacket &payload);
void pollingLoop(TimeoutSerial &serial);
void sendCommand(TimeoutSerial &serial, PayloadPacket &payload, Commands command);
void newWayPoint(TimeoutSerial &serial, PayloadPacket &payload);
void readWayPoint(TimeoutSerial &serial, PayloadPacket &payload);

int queued = 3;

int main()
{
    try {
        // These are the values our port needs to connect
        #ifdef _WIN32
            TimeoutSerial serial("COM3",115200);
        #else
            TimeoutSerial serial("/dev/ttyO2",115200);
        #endif
        // Starting polling for messages from the C&C
        pollingLoop(serial);

        //Clost serial port when finished
        serial.close();

    } catch(boost::system::system_error& e){
        cout<<"Error: "<<e.what()<<endl;
        return 1;
    }
    return 0;
}

void pollingLoop(TimeoutSerial &serial)
{
    while(true)
    {
        PayloadPacket payload;
        payload.read(serial);
        bitset<16> messageType = payload.getData(PayloadPacket::MessageType);
        // Request status packet
        switch(messageType.to_ulong())
        {
            case 0x2200:
                cout<<"--------- Request Packet ------------"<<endl;
                requestStatus(serial,payload);
                break;
            case 0x0100:
                cout<<"----------- ACK Packet --------------"<<endl;
                break;
            case 0x4000:
                cout<<"----------- Command Packet --------------"<<endl;

                //Rudder Test
                switch(queued)
                {
                    case 1:
                        sendCommand(serial,payload, RightRudder);
                        break;
                    case 2:
                        sendCommand(serial,payload, LeftRudder);
                        break;
                    case 3:
                        sendCommand(serial,payload, CenterRudder);
                        break;
                    default:
                        sendCommand(serial,payload, CenterRudder);
                }

                //newWayPoint(serial, payload);
                //readWayPoint(serial,payload);
                //sendCommand(serial,payload,HoldStation);
                queued--;
                break;
            case 0x1000:
                // Start enumeration process with the C&C
                cout<<"----------- Starting enumeration process -----------"<<endl;
                enumerateMessage(serial,payload);
                cout<<"----------- Finished enumeration process -----------"<<endl;
                break;
            default:
                cout<<"----------- Unknown Message -----------"<<endl;
        }
    }
}

void enumerateMessage(TimeoutSerial &serial, PayloadPacket &payload)
{
    // Enumeration process starts with reading message
    bitset<2*Command::BYTE> destination (0x0000);
    bitset<2*Command::BYTE> origin(0x0001);
    bitset<2*Command::BYTE> transactionID = payload.getData(PayloadPacket::TransactionID);
    bitset<2*Command::BYTE> messageType(0x0100);
    bitset<2*Command::BYTE> sourceType(0x1000);
    bitset<2*Command::BYTE> numberResponding(0x0100);
    bitset<2*Command::BYTE> numberMessages(0x0100);
    bitset<2*Command::BYTE> version(0x0100);
    bitset<2*Command::BYTE> deviceType(0x0110);
    bitset<2*Command::BYTE> address(0x0001);
    bitset<7*Command::BYTE> serialAddress = (bitset<7*Command::BYTE>(0x31323334) << 24) | bitset<7*Command::BYTE>(0x353600);
    bitset<1*Command::BYTE> port(0x00);
    bitset<4*Command::BYTE> polling(0x00000000);
    bitset<1*Command::BYTE> want(0x02);
    bitset<1*Command::BYTE> firmwareMajor(0x01);
    bitset<1*Command::BYTE> firmwareMinor(0x80);
    bitset<20*Command::BYTE> description = (((((((bitset<20*Command::BYTE>(0x0146756c) << 32 ) | bitset<20*Command::BYTE>(0x6c205049)) << 32 ) |
                                           bitset<20*Command::BYTE>(0x42202020 )) << 32 ) | bitset<20*Command::BYTE>(0x20202020 )) << 32 ) |
                                            bitset<20*Command::BYTE>(0x20202020);
    bitset<1*Command::BYTE> extra(0x20);
    string temp = sourceType.to_string()+numberResponding.to_string()+numberMessages.to_string()+
                  version.to_string()+deviceType.to_string()+address.to_string()+serialAddress.to_string()+
                  port.to_string()+polling.to_string()+want.to_string()+firmwareMajor.to_string()+
                  firmwareMinor.to_string()+description.to_string()+extra.to_string();
    boost::dynamic_bitset<> message(temp);
    PayloadPacket packet(destination,origin, transactionID, messageType, message);
    packet.write(serial);
}

void requestStatus(TimeoutSerial &serial, PayloadPacket &payload)
{
    bitset<2*Command::BYTE> destination (0x0000);
    bitset<2*Command::BYTE> origin(0x0001);
    bitset<2*Command::BYTE> transactionID = payload.getData(PayloadPacket::TransactionID);
    bitset<2*Command::BYTE> messageType(0x0100);
    bitset<2*Command::BYTE> sourceType(0x2200);
    bitset<2*Command::BYTE> numberResponding(0x0100);
    bitset<2*Command::BYTE> numberResponding1(0x0100);
    bitset<4*Command::BYTE> numberResponding2(0x00800001);
    bitset<4*Command::BYTE> numberResponding3(0x00000001);
    string temp = sourceType.to_string()+numberResponding.to_string()+
                    numberResponding1.to_string()+
                    ((queued>0)?numberResponding2.to_string():numberResponding3.to_string());
    boost::dynamic_bitset<> message(temp);
    PayloadPacket starter(destination,origin, transactionID, messageType, message);
    starter.write(serial);
}

void sendCommand(TimeoutSerial &serial, PayloadPacket &payload, Commands commandValue)
{
    bitset<2*Command::BYTE> destination (0x0100); // Process number on pg:145 of document
    bitset<2*Command::BYTE> origin(0x0001);
    bitset<2*Command::BYTE> transactionID = payload.getData(PayloadPacket::TransactionID);
    bitset<2*Command::BYTE> messageType(0x0100);
    bitset<4*Command::BYTE> originalMesgType(0x40000100);
    // Creating a rudder command
    bitset<1*Command::BYTE> ID (0x10);
    bitset<2*Command::BYTE> value (commandValue);
    //bitset<5*Command::BYTE> message (0xFF000000+0x01000000); Holding Station Test
    bitset<5*Command::BYTE> message (0x00);
    bitset<4*Command::BYTE> longitude (0x0); //4
    bitset<4*Command::BYTE> latitude (0x0); //4
    Command level1(ID,value,message,longitude,latitude);
    CommandPacket command(level1);
    // End of creating command

    string temp = originalMesgType.to_string()+command.stringPacket();
    boost::dynamic_bitset<> fullMessage(temp);

    // Create payload packet
    PayloadPacket starter(destination,origin, transactionID, messageType, fullMessage);
    starter.write(serial);
}

void newWayPoint(TimeoutSerial &serial, PayloadPacket &payload)
{
    bitset<2*Command::BYTE> destination (0x0100); // Process number on pg:145 of document
    bitset<2*Command::BYTE> origin(0x0001);
    bitset<2*Command::BYTE> transactionID = payload.getData(PayloadPacket::TransactionID);
    bitset<2*Command::BYTE> messageType(0x0100);
    bitset<4*Command::BYTE> originalMesgType(0x40000100);
    // Creating a rudder command
    bitset<1*Command::BYTE> ID (0x10);
    bitset<2*Command::BYTE> value (0x0080);
    bitset<5*Command::BYTE> message (0x01); //5
    bitset<4*Command::BYTE> longitude (40.80230094); //4
    bitset<4*Command::BYTE> latitude (-125.7877388); //4
    Command level1(ID,value,message,longitude,latitude);
    CommandPacket command(level1);
    // End of creating command

    string temp = originalMesgType.to_string()+command.stringPacket();
    boost::dynamic_bitset<> fullMessage(temp);

    // Create payload packet
    PayloadPacket starter(destination,origin, transactionID, messageType, fullMessage);
    starter.write(serial);
}

void readWayPoint(TimeoutSerial &serial, PayloadPacket &payload)
{
    bitset<2*Command::BYTE> destination (0x0100); // Process number on pg:145 of document
    bitset<2*Command::BYTE> origin(0x0001);
    bitset<2*Command::BYTE> transactionID = payload.getData(PayloadPacket::TransactionID);
    bitset<2*Command::BYTE> messageType(0x0100);
    bitset<4*Command::BYTE> originalMesgType(0x40000100);
    // Creating a rudder command
    bitset<1*Command::BYTE> ID (0x10);
    bitset<2*Command::BYTE> value (0x0380);
    bitset<5*Command::BYTE> message (0x01); //5
    bitset<4*Command::BYTE> longitude (0x0); //4
    bitset<4*Command::BYTE> latitude (0x0); //4
    Command level1(ID,value,message,longitude,latitude);
    CommandPacket command(level1);
    // End of creating command

    string temp = originalMesgType.to_string()+command.stringPacket();
    boost::dynamic_bitset<> fullMessage(temp);

    // Create payload packet
    PayloadPacket starter(destination,origin, transactionID, messageType, fullMessage);
    starter.write(serial);
}
