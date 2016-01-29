#include "tiff_image.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <QDebug>

tiff_image::tiff_image()
{

}

//WORD* tiff_image::ReadTIFFile(const char * name, int* width, int* height, int* BitPix)
//{
// void * file = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
// if (file == INVALID_HANDLE_VALUE) return NULL;
// int i,j,k,size, Line;
// short off, Compression=1, Predictor=1, PhotometricInterpretation;
// long offset,count, RowsPerStrip, *StripOffsets=NULL, *StripByteCounts=NULL;
// BYTE *q,*buf;
// WORD *pw, *pImage=NULL, *pwPal=NULL;	// Palette pointer
// WORD cIFD, tag, FieldType, cnt, Strips, wColors=256;
//        // Create file mapping
// if ((i=GetFileSize(file, NULL))==0) { // Zero-length file
//     CloseHandle(file);
//     ShowError(IDS_NOT_TIFF);
//     return NULL;
// }
// void * hMap = CreateFileMapping(file, NULL, PAGE_READONLY, 0,0, NULL);
// BYTE*  pMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0,0,0);
// bool	bMAC = false;
// pEnd=pMap+i;	// Pointer end of file

// if (NULL==pMap) goto err;
// if (strncmp((const char *)pMap, "MM\x0*", 2)==0) bMAC=true;
// else if (strncmp((const char *)pMap, "II*\x0", 2)) {
//err: goto out;
// }
// offset = GetLong(pMap+4, bMAC);
// cIFD = GetWord(pfile=pMap+offset, bMAC);	pfile+=2;
//        // Reading IFD-entries
// for (i=0; i<cIFD; i++) {
//    tag		 = GetWord(pfile, bMAC);
//    FieldType= GetWord(pfile+2, bMAC);
//    count	 = GetLong(pfile+4, bMAC);  cnt = (WORD)count;
//    offset	 = GetLong(pfile+8, bMAC);  off = (short)GetWord(pfile+8, bMAC);	pfile+=12;
//    switch (tag) {
//    case 256: *width = off; break;
//    case 257: RowsPerStrip = *height= off; break;
//    case 258: if (off!=16) { goto out; }
//              else *BitPix = off; break;
//    case 259: if (off != 1 && off != 5) { goto out; }
//              else Compression = off; break;
//    case 262: if (off==2) { goto out; }
//              else 	PhotometricInterpretation = off; break;
//    case 273: if (!GetData((void**)(&StripOffsets),cnt*4, offset, pMap, bMAC)) goto err; Strips=cnt; break;
//    case 278: RowsPerStrip = off; break;
//    case 279: if (!GetData((void**)(&StripByteCounts),cnt*4, offset, pMap, bMAC)) goto err; break;
//    case 284: break;
//    case 317: Predictor = off; break;
//    case 320: pwPal = (WORD*)(pMap+offset); wColors = cnt/3;
//    }
// }
// if (Compression==5 &&
//    (NULL == (dict= (DICT*)malloc(4096*sizeof(DICT))) ||
//     NULL == (stack=(BYTE*)malloc(1024*sizeof(BYTE))) ||
//     NULL == (buf = (BYTE*)malloc(BUF_SIZE*sizeof(BYTE))))) { goto out; }
// switch (*BitPix) {
// case 16:  Line = *width*2;
//    if (NULL == (pImage = (WORD*)malloc((size=*width**height)*sizeof(WORD))))  { goto out; }
//    for (pw=pImage,k=i=0; i<Strips; i++) {
//        pfile = pMap+StripOffsets[i];
//        if (Compression == 5) { if (!LZWUnPack(q=buf)) { goto out; }}	// unpack the file
//        else q=pfile;
//        for (j=0; j<RowsPerStrip; j++, q+=Line, pw+=*width) {
//            memcpy(pw, q, *width*2);
//            if (bMAC) MacData((BYTE*)pw, 2, *width);
//            if (++k==*height) break;
//        }
//    }
// }
//out:
// if (Compression==5) {
//     free(dict);
//     free(stack);
//     free(buf);
// }
// if (StripOffsets)		free(StripOffsets);
// if (StripByteCounts)	free(StripByteCounts);

// return pImage;
//}

bool tiff_image::WriteTIFF(string name, WORD* pImage, int width, int height, int bpp, double res, short BitsPerSample)
{
 FILE*  fp = fopen(name.data(), "wb");
 if (fp)
 {
    bool	bResult = false;
    short	PhotometricInterpretation=1, ResolutionUnit=2, Compression = 1;
    char	head[]="II*\x0\x8\x0\x0\x0\xE\x0\xFE\x0\x4\x0\x1\x0\x0\x0\x0\x0\x0\x0";
    WORD	i,j,nu, resid, Strips, cIFD = 14;		// # of directory entries
    long	RowsPerStrip, *StripOffsets, *StripByteCounts;
    long	XResolution[2], YResolution[2], end=26+12*cIFD;
    BYTE	*stack;
    DWORD	dwLine;
    QString str = "INKCT";
    QTime	t;


    stack=NULL;
    head[8]=(char)cIFD;
    switch (BitsPerSample) {
    case 8:  dwLine = width;			break;
    case 10: dwLine = (5*width+3)/4;	break;
    case 12: dwLine = (3*width+1)/2;	break;
    case 14: dwLine = (7*width+3)/4;	break;
    case 16: dwLine = width*2;
    }

    XResolution[0] = YResolution[0] = (long)(res*1000);
    XResolution[1] = YResolution[1] = 1000;
    RowsPerStrip = height;		if (RowsPerStrip==0) RowsPerStrip=1;
    nu = (WORD)RowsPerStrip;
    Strips	   = (WORD)(height/RowsPerStrip);
    if (resid = (WORD)(height%RowsPerStrip)) Strips++;
    StripOffsets	= (long*)malloc(Strips*sizeof(long));
    StripByteCounts= (long*)malloc(Strips*sizeof(long));


    if (fwrite(head, 1, 22, fp) != 22) return false;		// Write TIFF-header
    for (i=0; i<cIFD; i++)
        if (fwrite(head+10, 1, 12, fp) != 12) return false;	// Write empty IFD
    end=ftell(fp);
    fseek(fp, 22, SEEK_SET);
    if (PutData(256, 4, 1, &width, fp) ||					// Write IFD truely
         PutData(257, 4, 1, &height,fp) ||
         PutData(258, 3, 1, &BitsPerSample, fp)				||
         PutData(259, 3, 1, &Compression, fp)				||
         PutData(262, 3, 1, &PhotometricInterpretation, fp) ||
         PutData(273, 4, Strips, StripOffsets, fp, &end)	||
         PutData(278, 4, 1, &RowsPerStrip, fp)				||
         PutData(279, 4, Strips, StripByteCounts, fp, &end)	||
         PutData(282, 5, 1, XResolution, fp, &end)			||
         PutData(283, 5, 1, YResolution, fp, &end)			||
         PutData(296, 3, 1, &ResolutionUnit, fp)			||
         PutData(305, 2, str.size()+1, (void*)str.data(), fp, &end)) goto out;
    t = QTime::currentTime();	 str=t.toString();
    if (PutData(306, 2, 20, (void*)str.data(), fp, &end)) goto out;	// Date & time
    if (fwrite("\x0\x0\x0\x0", 1, 4, fp) != 4) goto out;	// Next IFD offset
    fseek(fp, 0, SEEK_END);


    //stack = (BYTE*)malloc(dwLine*sizeof(BYTE));
     stack = new BYTE [width*2];

    if (NULL == (stack)) { goto out; }
            // Write Image pixels
    for (i=0; i<Strips; i++)
    {

        StripOffsets[i] = ftell(fp);

        if (i==Strips-1 && resid)  nu = resid;

        StripByteCounts[i] = nu*dwLine;

        for (j=0; j<nu; j++, pImage+=width)
        {
            PackPixels(stack, pImage, width, bpp, BitsPerSample);

            if (fwrite(stack, dwLine, 1, fp) != 1) goto out;

        }
    }

    ReWriteStrips(fp, 90, Strips, StripOffsets);		// Rewrite strips info
    ReWriteStrips(fp,114, Strips, StripByteCounts);
    bResult = true;
out:

    fclose(fp);
    if (stack)	{ free(stack);	stack=NULL;	}
    free(StripOffsets);		free(StripByteCounts);
    return bResult;
 } else {
    return false;
 }
}

// Return true if error
bool tiff_image::PutData(WORD tag, WORD type, long count, void *offset, FILE* fp, long* end)
{
 if (fwrite(&tag,  2, 1, fp) != 1 ||	// Write IFD Entry
     fwrite(&type, 2, 1, fp) != 1 ||
     fwrite(&count,4, 1, fp) != 1) return true;
 long	old;
 WORD	i, zero=0, uns=(WORD)count;
 bool	b = (type==2 && count>4) || (type!=2 && count!=1L) || type==5;
 if (b) {
     if (fwrite(end, 4, 1, fp) != 1) return true;
     old = ftell(fp);
     fseek(fp, 0, SEEK_END); // Goto EOF
 }
 switch (type) {
 case 2: if (fwrite(offset, 1, uns, fp) != uns) return true;		// ASCII
         for (i=uns; i<4; i++)
             if (fputc('\x0', fp) == EOF) return true;
         break;
 case 3: if((fwrite(offset, 2, uns, fp) != uns) || (uns==1 &&		// SHORT
             fwrite(&zero, 2, 1, fp) != 1)) return true;
         break;
 case 4: if (fwrite(offset, 4, uns, fp) != uns) return true; break;	// LONG
 case 5: if (fwrite(offset, 8, uns, fp) != uns) return true;		// RATIONAL
 }
 if (b) { *end=ftell(fp);  fseek(fp, old, SEEK_SET); } // Save offset & return
 return false;
}

void tiff_image::ReWriteStrips(FILE* fp, long offset, WORD Strips, long* p)
{
 fseek(fp, offset, SEEK_SET);
 if (Strips!=1) { fread(&offset, 4, 1, fp);  fseek(fp, offset, SEEK_SET); }
 fwrite(p, 4, Strips, fp);
}

void tiff_image::PackPixels(BYTE* b, WORD* p, int width, int bpp, int BitsPerSample)
{

 int i,n=BitsPerSample-bpp;  WORD w;

 switch (BitsPerSample) {
 case 16:

    memcpy(b, p, width*sizeof(WORD));

    break;
 case 14:

    for (i=0; i<width; i++,p++)
    {
        w = (n>=0) ? *p	: *p>>(-n);
        switch (i%4) {
        case 0:	*b++ = (BYTE)(w>>6);		*b  = (BYTE)((w<<2) & 0x00FC); break;
        case 1: *b++ = (BYTE)(w>>12 | *b);	*b++= (BYTE)((w>>4) & 0x00FF);  *b = (BYTE)((w<<4) & 0x00F0); break;
        case 2: *b++ = (BYTE)(w>>10 | *b);	*b++= (BYTE)((w>>2) & 0x00FF);  *b = (BYTE)((w<<6) & 0x00C0); break;
        case 3: *b++ = (BYTE)(w>>8  | *b);	*b++= (BYTE)( w     & 0x00FF);
        }
    } break;
 case 12:

    for (i=0; i<width; i++,p++)
    {
        w = (n>=0) ? *p	: *p>>(-n);
        if (i%2) {	*b++ = (BYTE)(*b | w>>8);	*b++ = (BYTE)(w & 0x00FF); }
        else	 {	*b++ = (BYTE)(w>>4);		*b   = (BYTE)((w<<4) & 0x00F0); }
    } break;

 case 10:

    for (i=0; i<width; i++,p++)
    {
        w = (n>=0) ? *p	: *p>>(-n);
        switch (i%4) {
        case 0:	*b++ = (BYTE)(w>>2);		*b  = (BYTE)((w & 0x0003)<<6); break;
        case 1: *b++ = (BYTE)(w>>4 | *b);	*b  = (BYTE)((w & 0x000F)<<4); break;
        case 2: *b++ = (BYTE)(w>>6 | *b);	*b  = (BYTE)((w & 0x003F)<<2); break;
        case 3: *b++ = (BYTE)(w>>8 | *b);	*b++= (BYTE)( w & 0x00FF)    ;
        }
    }
 }

}


bool tiff_image::LZWUnPack(BYTE* buf)
{
 WORD Code,OldCode;
 CurByte=buf; CurBit=0; Bits=9;
 bool bRes=true;
 while ((Code=GetNextCode(&bRes)) != 257) {
    if (Code == 256) {
        InitializeTable();
        if ((Code=GetNextCode(&bRes)) == 257) break;
        WriteString(Code);
    } else {
        if (Code < Entry) { if (!StringToTable(OldCode, WriteString(Code))) return false; }
        else { *CurByte=WriteString(OldCode);    if (!StringToTable(OldCode, *CurByte++)) return false; }
    }
    OldCode=Code;
 }
 return bRes;
}

void tiff_image::MacData(BYTE *p, int sizeDATA, int count)
{
 if (sizeDATA==1 || sizeDATA==8) return;	// Processing not need
 BYTE b;
 for (int i=0; i<count; i++,p+=sizeDATA) {
    if (sizeDATA==2) {
        b=*(p+1); *(p+1)=*p;  *p=b;			// 01 -> 10
    } else {
        b=*(p+3); *(p+3)=*p;	 *p=b;		// 0123 -> 3210
        b=*(p+2); *(p+2)=*(p+1); *(p+1)=b;
    }
 }
}

void tiff_image::InitializeTable()
{
 Bits=9; Entry=258;
 for (WORD i=0; i<256; i++) dict[i].c=(BYTE)(dict[i].prefix=i);
}

WORD tiff_image::GetNextCode(bool *bRes)
{
 WORD i,code;
 if (pfile > pEnd) goto er;
 i = CurBit+Bits;
 code = (WORD)(*pfile++ << (CurBit+8)) >> (16-Bits);
 if (i > 16) {
    code |= (*pfile++ << (i-16));
    code |= (*pfile   >> (24-i));
 } else
    code |= (*pfile   >> (16-i));
 CurBit=i%8;
 if (CurBit==0) {
    pfile++;
er:	if (pfile > pEnd) { *bRes=false; return 257; }
 }
 *bRes=true;
 return code;
}

bool tiff_image::GetData(void **s, DWORD count, long offset, BYTE* pMap, bool bMAC)
{
// if (NULL == (*s = malloc(count))) return false;
// if (count<5)
//    *((long*)(*s)) = GetLong((BYTE*)&offset, bMAC);
// else
//    for (DWORD i=0; i<count; i+=4)
//        *((long*)(*s)+i/4) = GetLong(pMap+offset+i, bMAC);
// return true;
}

BYTE tiff_image::WriteString(WORD Code)
{
 BYTE *v, c=dict[Code].c;
 if (Code < 256) *CurByte++=c;
 else {
   *(v=stack)=c;
   do { Code=dict[Code].prefix; *(++v)=dict[Code].c; }
   while (Code > 257);
   c=*v;
   while (v >= stack) *CurByte++=*v--;
 }
 return c;
}

bool tiff_image::StringToTable(WORD code, BYTE c)
{
 dict[Entry].prefix=code;
 dict[Entry++].c=c;
 if      (Entry== 511) Bits=10;
 else if (Entry==1023) Bits=11;
 else if (Entry==2047) Bits=12;
 return (Entry < 4096);
}
