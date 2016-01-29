#include "crc16.h"

/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: true
  XorOut: 0x0000
*/

#include <string.h>
#include <stdio.h>

const int order = 16;
const unsigned long polynom = 0x1021;
const int direct = 1;
const unsigned long crcinit = 0xffff;
const unsigned long crcxor = 0x0;
const int refin = 1;
const int refout = 1;

unsigned long crcmask;
unsigned long crchighbit;
unsigned long crcinit_direct;
unsigned long crcinit_nondirect;

unsigned long reflect (unsigned short crc, int bitnum) {

    unsigned long i, j=1, crcout=0;

    for (i=(unsigned long)1<<(bitnum-1); i; i>>=1) {
        if (crc & i) crcout|=j;
        j<<= 1;
    }
    return (crcout);
}

unsigned short crcbitbybit(unsigned char* p, unsigned short len) {

    unsigned long i, j, c, bit;
    unsigned short crc = crcinit_nondirect;

    for (i=0; i<len; i++) {

        c = (unsigned long)*p++;
        if (refin) c = reflect(c, 8);

        for (j=0x80; j; j>>=1) {

            bit = crc & crchighbit;
            crc<<= 1;
            if (c & j) crc|= 1;
            if (bit) crc^= polynom;
        }
    }

    for (i=0; i<order; i++) {

        bit = crc & crchighbit;
        crc<<= 1;
        if (bit) crc^= polynom;
    }

    if (refout) crc=reflect(crc, order);
    crc^= crcxor;
    crc&= crcmask;

    return(crc);
}



unsigned short crc16(unsigned char *pcBlock, unsigned short len)
{
    int i;
    unsigned long bit, crc;
    crcmask = ((((unsigned long)1<<(order-1))-1)<<1)|1;
    crchighbit = (unsigned long)1<<(order-1);

    if (!direct) {

            crcinit_nondirect = crcinit;
            crc = crcinit;
            for (i=0; i<order; i++) {

                bit = crc & crchighbit;
                crc<<= 1;
                if (bit) crc^= polynom;
            }
            crc&= crcmask;
            crcinit_direct = crc;
        }

        else {

            crcinit_direct = crcinit;
            crc = crcinit;
            for (i=0; i<order; i++) {

                bit = crc & 1;
                if (bit) crc^= polynom;
                crc >>= 1;
                if (bit) crc|= crchighbit;
            }
            crcinit_nondirect = crc;
        }

    unsigned short getcrc = crcbitbybit(pcBlock, len);
    return getcrc;

}












