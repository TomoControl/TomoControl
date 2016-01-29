#pragma once

#define TABLE_SIZE	5003
#define BUF_SIZE	16384
#define EMPTY		-1

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef bool BOOL;

typedef struct {	// Dictionary to unpack
 short prefix;
 BYTE c;
} DICT;

typedef struct {	// Dictionary to pack
 short	entry;
 WORD	prefix;
 BYTE	c;
} DIC;




WORD* ReadTIFFile(LPCTSTR name, int* width, int* height, int* BitPix);
BOOL  WriteTIFF(LPCTSTR, WORD*, int,int,int bpp=16, double res=1., short bps=16);
