#ifndef CRC16_H
#define CRC16_H

unsigned long reflect (unsigned short crc, int bitnum);

unsigned short crcbitbybit(unsigned char* p, unsigned short len);

unsigned short crc16(unsigned char *pcBlock, unsigned short len);

#endif // CRC16_H
