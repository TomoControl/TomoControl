#ifndef TIFF_IMAGE_H
#define TIFF_IMAGE_H

#pragma once

#define TABLE_SIZE	5003
#define BUF_SIZE	16384
#define EMPTY		-1

//#include "stdafx.h"
#include <QMainWindow>
#include <QTime>

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
//typedef const char * LPCTSTR; //????
//typedef bool BOOL;


struct DICT
{	// Dictionary to unpack
     short prefix;
     BYTE c;
};

struct DIC
 {	// Dictionary to pack
     short	entry;
     WORD	prefix;
     BYTE	c;
};

class tiff_image
{
public:
    tiff_image();

    //WORD* ReadTIFFile(const char * name, int* width, int* height, int* BitPix);
    bool  WriteTIFF(string, WORD*, int,int,int bpp=16, double res=1., short bps=16);

    // Local functions
    bool LZWUnPack		(BYTE*);				// (for decompression)
    bool GetData(void **s, DWORD count, long offset, BYTE* pMap, bool bMAC);
    void InitializeTable(void);
    WORD GetNextCode	(bool*);
    BYTE WriteString	(WORD);
    bool StringToTable	(WORD, BYTE);
    long LZWPack		(BYTE*, BYTE*, long);	// (for compression)
    bool PutData(WORD tag, WORD type, long count, void *offset, FILE* fp, long* end=NULL);
    void ReWriteStrips	(FILE*, long, WORD, long*);
    void WriteCode		(WORD);
    void PackPixels		(BYTE* pBytes, WORD* pPixels, int width, int bpp, int BitsPerSample);
    void	MacData(BYTE *p, int sizeDATA, int count);

    BYTE *pfile;			// Current BYTE pointer in TIFF-file
    BYTE *pEnd;	 			// End file pointer
    DICT* dict;	 DIC* dic;	// Dictionaries
    WORD Bits, CurBit, Entry;
    BYTE *CurByte, *stack;


};

#endif // TIFF_IMAGE_H
