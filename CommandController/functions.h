#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include <string>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "stdint.h"

using namespace std;

/** CRC data for calculation */
#define                 P_16        0xA001
#define FALSE           0
#define TRUE            1
static int              crc_tab16_init          = FALSE;
static unsigned short   crc_tab16[1024];
/** End of CRC */

inline std::vector<int8_t> binary(string binary_text)
{
    std::vector<int8_t> binary;
    int8_t byte = 0;
    for(int i=0; i<binary_text.size(); ++i)
    {
        if(binary_text[i]=='1')
        {
            byte = (byte << 1) | 1;
        }
        else
        {
            byte = byte << 1;
        }
        if((i+1)%8==0 && i!=0)
        {
            binary.push_back(byte);
            byte = 0;
        }
    }
    return binary;
}

template<size_t N>
inline string littleEndian(bitset<N> binarySet)
{
    string bin = binarySet.to_string(), temp;
    for(int i= (N/8)-1; i>=0; --i)
    {
        temp += bin.substr(i*8, 8);
    }
    return temp;
}

static inline void init_crc16_tab( void ) {

    int i, j;
    unsigned short crc, c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = (unsigned short) i;

        for (j=0; j<8; j++) {

            if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ P_16;
            else                      crc =   crc >> 1;

            c = c >> 1;
        }

        crc_tab16[i] = crc;
    }

    crc_tab16_init = TRUE;

}

unsigned inline short update_crc_16( unsigned short crc, char c ) {

    unsigned short tmp, short_c;

    short_c = 0x00ff & (unsigned short) c;

    if ( ! crc_tab16_init ) init_crc16_tab();

    tmp =  crc       ^ short_c;
    crc = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];

    return crc;

}

template <size_t  N>
int16_t inline iterateBitset(int16_t crc, bitset<N> a)
{
    std::vector<int8_t> packet = binary(a.to_string());
    for(int i=0; i<packet.size(); ++i) {
        crc = update_crc_16(crc, packet[i]);
    }
    return crc;
}

int16_t inline iterateBitset(int16_t crc, string str)
{
    std::vector<int8_t> packet = binary(str);
    for(int i=0; i<packet.size(); ++i) {
        crc = update_crc_16(crc, packet[i]);
    }
    return crc;
}
int16_t inline iterateBitset(int16_t crc, boost::dynamic_bitset<> a)
{
    string str;
    boost::to_string(a,str);
    std::vector<int8_t> packet = binary(str);
    for(int i=0; i<packet.size(); ++i) {
        crc = update_crc_16(crc, packet[i]);
    }
    return crc;
}

#endif // FUNCTIONS_H_INCLUDED
