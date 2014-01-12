/******************************************************************************

	bmp.c

	PSP windows bitmap file I/O functions.

******************************************************************************/

#include "psp.h"


/******************************************************************************
	Windows bitmap structures
******************************************************************************/

#define BFH_SIZE	(2+4+2+2+4)
#define BIH_SIZE	(4+4+4+2+2+4+4+4+4+4+4)

typedef unsigned char BYTE;
typedef unsigned DWORD;
typedef int LONG;
typedef unsigned short WORD;

typedef struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER
{
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;


/*--------------------------------------------------------
	Save BMP
--------------------------------------------------------*/

int save_bmp(const char *name)
{
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	int x, y, y2;
	u16 *vptr, *src;
	FILE *fp;

	if ((fp = fopen(name, "wb")) == NULL)
		return 0;

	bf.bfType          = 0x4d42;
	bf.bfReserved1     = 0;
	bf.bfReserved2     = 0;
	bf.bfOffBits       = BFH_SIZE + BIH_SIZE;

	bi.biSize          = BIH_SIZE;
	bi.biWidth         = SCR_WIDTH;
	bi.biHeight        = SCR_HEIGHT;
	bi.biPlanes        = 1;
	bi.biBitCount      = 24;
	bi.biCompression   = 0;
	bi.biSizeImage     = bi.biWidth * bi.biHeight * 3;
	bi.biXPelsPerMeter = 3780;
	bi.biYPelsPerMeter = 3780;
	bi.biClrUsed       = 0;
	bi.biClrImportant  = 0;

	bf.bfSize          = bf.bfOffBits + bi.biSizeImage;

	// BITMAPFILEHEADER
	fwrite(&bf.bfType,      1, 2, fp);
	fwrite(&bf.bfSize,      1, 4, fp);
	fwrite(&bf.bfReserved1, 1, 2, fp);
	fwrite(&bf.bfReserved2, 1, 2, fp);
	fwrite(&bf.bfOffBits,   1, 4, fp);

	// BITMAPINFOHEADER
	fwrite(&bi.biSize,          1, 4, fp);
	fwrite(&bi.biWidth,         1, 4, fp);
	fwrite(&bi.biHeight,        1, 4, fp);
	fwrite(&bi.biPlanes,        1, 2, fp);
	fwrite(&bi.biBitCount,      1, 2, fp);
	fwrite(&bi.biCompression,   1, 4, fp);
	fwrite(&bi.biSizeImage,     1, 4, fp);
	fwrite(&bi.biXPelsPerMeter, 1, 4, fp);
	fwrite(&bi.biYPelsPerMeter, 1, 4, fp);
	fwrite(&bi.biClrUsed,       1, 4, fp);
	fwrite(&bi.biClrImportant,  1, 4, fp);

	vptr = video_frame_addr(show_frame, 0, 0);

	for (y = 0; y < SCR_HEIGHT; y++)
	{
		y2 = (SCR_HEIGHT - 1) - y;

		src = &vptr[y2 * BUF_WIDTH];

		for (x = 0; x < SCR_WIDTH; x++)
		{
			u16 color = src[x];
			int r = GETR15(color);
			int g = GETG15(color);
			int b = GETB15(color);

			fputc(b, fp);
			fputc(g, fp);
			fputc(r, fp);
		}
	}

	fclose(fp);

	return 1;
}
