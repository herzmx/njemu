/******************************************************************************

	icon_s.c

	Small icon data.

******************************************************************************/

#include "psp/psp.h"

#define NUM_FONTS	0x0e

/*------------------------------------------------------
	gryph data
------------------------------------------------------*/

static const u8 ALIGN_DATA icon_s[] = {
	0x00,0x00,0x21,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0xdc,0x5b,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xd8,0xed,0xee,0x07,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x10,0x53,0xeb,0x3e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0xe5,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0xe3,
	0xbe,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x71,0x58,0x33,0xeb,0xee,0x6d,0x00,0x00,
	0x00,0x00,0x00,0x00,0x40,0xee,0xdd,0xee,0xee,0xfe,0x4c,0x00,0x00,0x00,0x00,0x00,
	0x00,0xc5,0xfe,0xce,0xfe,0xee,0xef,0x3a,0x30,0x55,0x01,0x00,0x00,0x10,0x65,0x04,
	0xb3,0xff,0xee,0xef,0xdb,0xee,0x5c,0x00,0x00,0x00,0x00,0x00,0x00,0xc5,0xef,0xee,
	0xee,0xdd,0xee,0x05,0x00,0x00,0x00,0x00,0x00,0x10,0xd7,0xee,0xce,0x34,0x96,0x18,
	0x00,0x00,0x00,0x00,0x00,0x00,0x10,0xea,0x4e,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0xf6,0x5e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xd3,
	0xbe,0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0xee,0xde,0x8d,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xb6,0xcd,0x3a,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x10,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x04,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x71,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x89,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x87,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x00,0x00,0x00,0x00,0x00,0x00,
	0x61,0x57,0x24,0x00,0x54,0x10,0x43,0x86,0x17,0x00,0x10,0xfb,0xff,0xde,0xdd,0xcc,
	0xdd,0xee,0xce,0xae,0x00,0x60,0xce,0x67,0xfc,0xee,0xff,0xee,0xdc,0x08,0xe8,0x04,
	0xb0,0x4e,0x64,0xc2,0xee,0xab,0xbf,0x40,0x18,0xf8,0x0a,0xe3,0x2b,0xfe,0x84,0xef,
	0xc9,0xcf,0x51,0xce,0x76,0x2d,0xe6,0x1c,0xca,0x92,0xef,0xde,0xee,0xad,0xb7,0x00,
	0x5d,0xe7,0x8e,0x11,0xe5,0xee,0x98,0xee,0x4e,0x80,0x88,0x6e,0xe7,0xed,0xbc,0xee,
	0xee,0xed,0xee,0xae,0xc6,0xde,0x5d,0xe5,0xdd,0xee,0xee,0xee,0xee,0xee,0xee,0xde,
	0xdd,0x3d,0x80,0xdd,0xdd,0xcc,0xbb,0xbb,0xbb,0xcb,0xdd,0xcd,0x06,0x00,0x42,0x24,
	0x01,0x00,0x00,0x00,0x00,0x32,0x13,0x00,0x00,0x53,0x55,0x25,0x00,0x00,0x00,0x00,
	0x00,0x00,0x30,0xbc,0xbb,0xcb,0x01,0x00,0x00,0x00,0x00,0x00,0xb2,0x04,0x00,0x70,
	0x1a,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0xb9,0xaa,0xaa,0xaa,0x4b,0x00,
	0x58,0x00,0x00,0x00,0x30,0x33,0x22,0x22,0x79,0x00,0x58,0x90,0xab,0xba,0xaa,0xaa,
	0xaa,0xaa,0xcc,0x4a,0x48,0xe2,0xee,0xfe,0xff,0xff,0xee,0xee,0xdd,0x3e,0x48,0xe5,
	0xee,0xee,0xee,0xee,0xee,0xde,0xdd,0x1c,0x58,0xe8,0xee,0xee,0xee,0xee,0xee,0xde,
	0xed,0x0a,0x68,0xeb,0xee,0xee,0xee,0xee,0xee,0xde,0xed,0x07,0x88,0xed,0xee,0xee,
	0xee,0xee,0xee,0xde,0xdd,0x04,0xc8,0xdd,0xed,0xee,0xee,0xee,0xee,0xdd,0xcd,0x01,
	0xe8,0xdd,0xdd,0xee,0xee,0xee,0xde,0xdd,0xae,0x00,0xe8,0xed,0xee,0xee,0xee,0xee,
	0xee,0xee,0x8e,0x00,0xb6,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0x4b,0x00,0x10,0x54,
	0x66,0x56,0x55,0x55,0x56,0x45,0x52,0x66,0x56,0x04,0x51,0xc8,0xbb,0xcc,0xa8,0xcc,
	0xcb,0x8c,0xa6,0xbd,0xcc,0x07,0x95,0xaf,0x77,0xe7,0x6a,0xcf,0x76,0xee,0xf8,0x69,
	0xe7,0x08,0xf7,0x4a,0x02,0x95,0x58,0xbe,0x54,0xfb,0xec,0x45,0x95,0x07,0xfa,0x27,
	0x00,0x41,0x54,0xbe,0x66,0xbd,0xfa,0x8a,0x67,0x05,0xfb,0x17,0x00,0x00,0x50,0xce,
	0xdb,0x6c,0xd6,0xff,0xdf,0x08,0xfa,0x37,0x00,0x00,0x50,0xbe,0x76,0x46,0x65,0x87,
	0xfa,0x0c,0xf7,0x5c,0x03,0x31,0x54,0xbe,0x04,0x30,0x77,0x23,0xd5,0x0d,0xa5,0xbf,
	0x57,0x96,0x58,0xcf,0x35,0x20,0xcb,0x45,0xf8,0x09,0x52,0xe9,0xcd,0xcc,0xa8,0xde,
	0x59,0x30,0xe9,0xbc,0xac,0x05,0x10,0x65,0x87,0x67,0x76,0x66,0x57,0x20,0x66,0x77,
	0x56,0x01,0x00,0x10,0x11,0x01,0x10,0x00,0x01,0x00,0x10,0x11,0x01,0x00,0x00,0x86,
	0x88,0x88,0x88,0x88,0x88,0x88,0x36,0x00,0x00,0x00,0xeb,0xee,0xee,0xee,0xee,0xee,
	0xee,0xde,0x18,0x00,0x00,0xda,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xed,0xae,0x00,0x00,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0xb4,0xed,0x06,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x20,0xec,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe9,0x2d,
	0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0xe8,0x2d,0x00,0x00,0x89,0x00,0x00,
	0x00,0x00,0x00,0x10,0xec,0x0c,0x00,0xa2,0x8f,0x00,0x00,0x00,0x00,0x00,0xa2,0xee,
	0x07,0x40,0xec,0xde,0xbb,0xbb,0xbb,0xbb,0xbb,0xed,0xbe,0x01,0xb1,0xde,0xed,0xff,
	0xff,0xff,0xff,0xff,0xee,0x2a,0x00,0x10,0xea,0xce,0xaa,0xaa,0xaa,0xaa,0xaa,0x48,
	0x00,0x00,0x00,0x80,0x8f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x87,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x30,0x44,0x44,0x55,0x55,0x45,0x44,0x44,0x03,0x00,0x10,0xda,
	0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xad,0x01,0x90,0xee,0x06,0x00,0x00,0x00,0x00,
	0x00,0x40,0xed,0x09,0xd3,0xed,0x05,0x00,0x00,0x00,0x00,0x00,0x30,0xdd,0x4d,0xe7,
	0xed,0x06,0x00,0x00,0x00,0x00,0x00,0x30,0xdd,0x7e,0xe9,0xed,0x06,0x00,0x00,0x00,
	0x00,0x00,0x30,0xdd,0x9d,0xe9,0xed,0x06,0x00,0x00,0x00,0x00,0x00,0x30,0xdd,0xad,
	0xe8,0xed,0x06,0x00,0x00,0x00,0x00,0x00,0x30,0xdd,0x9e,0xe6,0xed,0x06,0x00,0x00,
	0x00,0x00,0x00,0x30,0xdd,0x6e,0xc2,0xed,0x05,0x00,0x00,0x00,0x00,0x00,0x30,0xdd,
	0x2d,0x60,0xee,0x48,0x44,0x44,0x44,0x44,0x44,0x74,0xee,0x06,0x00,0xb5,0xcc,0xcc,
	0xcc,0xcc,0xcc,0xcc,0xcc,0x6b,0x00,0xcb,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
	0xac,0xdd,0xcc,0xed,0xdd,0xed,0xde,0xdc,0xde,0xcc,0xcd,0x8d,0x66,0xd7,0x13,0xb1,
	0x6b,0x67,0x7d,0x67,0xc9,0x8d,0xdb,0xd8,0x02,0xb0,0xba,0x8e,0x8c,0xbd,0xc9,0x8d,
	0xcb,0xd8,0x02,0xb0,0xab,0x8d,0x8c,0xac,0xc9,0x8d,0x00,0xe5,0x02,0xb0,0x0a,0x30,
	0x5e,0x00,0xc8,0x8d,0x00,0xe5,0x02,0xb0,0x0b,0x30,0x4e,0x00,0xc8,0x8d,0x00,0xe5,
	0x02,0xb0,0x0b,0x30,0x4e,0x00,0xc8,0x8d,0x00,0xe5,0x02,0xb0,0x0a,0x30,0x4e,0x00,
	0xc8,0x8d,0x00,0xd5,0xb7,0xb8,0x0a,0x30,0x4e,0x00,0xc8,0x7d,0x00,0xd5,0xf9,0xbb,
	0x0a,0x30,0x4e,0x00,0xc8,0x8d,0x00,0xd6,0x96,0xb6,0x0a,0x30,0x5e,0x00,0xc9,0xdd,
	0xcc,0xed,0xbc,0xdc,0xcd,0xcc,0xcd,0xcc,0xcd,0xdc,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
	0xdd,0xdd,0xbd,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x00,0x90,0x05,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe6,0x3d,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x40,0xdd,0xce,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xd3,0xde,0xfe,0x0a,0x00,
	0x00,0x00,0x00,0x00,0x00,0xa8,0xec,0xad,0xab,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xf7,0x1d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf7,0x1d,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xf7,0x1d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf7,0x2d,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe5,0x6e,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xd2,0xde,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0xee,0xce,0xbb,
	0xbb,0x1a,0xa7,0x72,0x32,0x00,0x00,0xe8,0xee,0xee,0xee,0x2e,0xd9,0xa3,0x53,0x00,
	0x00,0x40,0xca,0xdd,0xdd,0x2c,0xc8,0x92,0x43,0x73,0x76,0x77,0x77,0x77,0x77,0x77,
	0x67,0x66,0x06,0xf8,0xee,0xee,0xff,0xff,0xff,0xee,0xee,0xee,0x1d,0xe7,0xdd,0xed,
	0xee,0xee,0xee,0xde,0xdd,0xdd,0x1c,0xe7,0xdd,0xee,0xee,0xee,0xee,0xee,0xdd,0xdd,
	0x1c,0xe7,0xdd,0xee,0xee,0xee,0xee,0xee,0xde,0xdd,0x1c,0xe7,0xed,0xee,0xee,0xee,
	0xee,0xee,0xde,0xdd,0x1c,0xe7,0xed,0xee,0xee,0xee,0xee,0xee,0xde,0xdd,0x1c,0xe7,
	0x8d,0xed,0xee,0xee,0xee,0xee,0xde,0xdd,0x1c,0xf8,0x0a,0xec,0xee,0xee,0xee,0xee,
	0xde,0xdd,0x1c,0xe4,0x8d,0xec,0xee,0xee,0xee,0xee,0xdd,0xdd,0x1c,0x50,0xee,0xee,
	0xee,0xee,0xee,0xee,0xdd,0xdd,0x1d,0x00,0xd5,0xdd,0xed,0xee,0xee,0xdd,0xdd,0xdd,
	0x1c,0x00,0x20,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x03,0x70,0xbc,0xcc,0xcc,0xcc,
	0xcc,0xac,0x66,0x56,0x00,0xb4,0xaa,0xaa,0xaa,0xaa,0xaa,0xdb,0xde,0xde,0x00,0x00,
	0x20,0x22,0x22,0x22,0x22,0x20,0x95,0x48,0x00,0x00,0xe9,0xee,0xee,0xee,0xee,0x04,
	0xd1,0x0a,0x00,0x00,0xa5,0xaa,0xaa,0xaa,0x9a,0x01,0xe2,0x0a,0x00,0x00,0xd9,0xdd,
	0xdd,0xdd,0xdd,0x04,0xe2,0x0a,0x00,0x00,0xa5,0xaa,0xaa,0xaa,0x9a,0x01,0xe2,0x0a,
	0x00,0x00,0xe9,0xdd,0xee,0xee,0xee,0x04,0xe2,0x0a,0x00,0x00,0x52,0x55,0x55,0x55,
	0x55,0x10,0x83,0x27,0x00,0x95,0x89,0x88,0x99,0x99,0x99,0xd9,0xde,0xcd,0x00,0xa1,
	0xde,0xed,0xee,0xee,0xee,0xce,0x98,0x69,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xb0,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0x86,0x58,0x04,0x00,0x00,0x00,
	0x00,0x00,0x00,0xe1,0xde,0xed,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x44,0x44,
	0x03,0x00,0x76,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x16,0x10,0xed,0xee,0xee,
	0xee,0xee,0xee,0xee,0xee,0xed,0x2d,0x10,0x9d,0x01,0x00,0x11,0x00,0x10,0x01,0x00,
	0x71,0x2d,0x30,0x8d,0x60,0x76,0x02,0x66,0x06,0x72,0x56,0x60,0x2d,0xd6,0x8e,0xe0,
	0xff,0x25,0xff,0x2f,0xf6,0xdf,0x60,0x2d,0xe7,0x8e,0xd0,0xee,0x25,0xee,0x2e,0xe5,
	0xce,0x60,0x2d,0xe6,0x8e,0xd0,0xee,0x25,0xee,0x2e,0xe5,0xce,0x60,0x2d,0xe6,0x8e,
	0xd0,0xee,0x25,0xee,0x2e,0xe5,0xce,0x60,0x2d,0xe7,0x8e,0xd0,0xee,0x25,0xee,0x2e,
	0xe5,0xce,0x60,0x2d,0xd6,0x8e,0xd0,0xff,0x25,0xff,0x2e,0xf6,0xce,0x60,0x2d,0x30,
	0x8d,0x50,0x55,0x01,0x55,0x05,0x62,0x45,0x50,0x2d,0x10,0x9d,0x12,0x11,0x22,0x11,
	0x21,0x12,0x11,0x72,0x2d,0x10,0xed,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xed,0x2d,
	0x00,0x65,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x15,0x00,0x76,0x77,0x77,0x77,
	0x77,0x77,0x77,0x77,0x77,0x16,0x10,0xed,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xed,
	0x2d,0x10,0x9d,0x11,0x11,0x11,0x00,0x10,0x01,0x00,0x71,0x2d,0x30,0x8d,0x00,0x00,
	0x10,0x66,0x06,0x72,0x56,0x60,0x2d,0xd6,0x8e,0x00,0x00,0x30,0xff,0x2f,0xf6,0xdf,
	0x60,0x2d,0xe7,0x8e,0x00,0x00,0x30,0xee,0x2e,0xe5,0xce,0x60,0x2d,0xe6,0x8e,0x00,
	0x00,0x30,0xee,0x2e,0xe5,0xce,0x60,0x2d,0xe6,0x8e,0x00,0x00,0x30,0xee,0x2e,0xe5,
	0xce,0x60,0x2d,0xe7,0x8e,0x00,0x00,0x30,0xee,0x2e,0xe5,0xce,0x60,0x2d,0xd6,0x8e,
	0x00,0x00,0x30,0xff,0x2e,0xf6,0xce,0x60,0x2d,0x30,0x8d,0x00,0x00,0x00,0x55,0x05,
	0x62,0x45,0x60,0x2d,0x10,0x9d,0x22,0x22,0x22,0x11,0x21,0x12,0x11,0x72,0x2d,0x10,
	0xed,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xed,0x2d,0x00,0x65,0x66,0x66,0x66,0x66,
	0x66,0x66,0x66,0x66,0x15,0x00,0x76,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x16,
	0x10,0xed,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xed,0x2d,0x10,0x9d,0x11,0x11,0x11,
	0x11,0x11,0x01,0x00,0x71,0x2d,0x30,0x8d,0x00,0x00,0x00,0x00,0x00,0x72,0x56,0x60,
	0x2d,0xd6,0x8e,0x00,0x00,0x00,0x00,0x00,0xf6,0xdf,0x60,0x2d,0xe7,0x8e,0x00,0x00,
	0x00,0x00,0x00,0xe6,0xce,0x60,0x2d,0xe6,0x8e,0x00,0x00,0x00,0x00,0x00,0xe6,0xce,
	0x60,0x2d,0xe6,0x8e,0x00,0x00,0x00,0x00,0x00,0xe6,0xce,0x60,0x2d,0xe7,0x8e,0x00,
	0x00,0x00,0x00,0x00,0xe6,0xce,0x60,0x2d,0xd6,0x8e,0x00,0x00,0x00,0x00,0x00,0xf6,
	0xce,0x60,0x2d,0x30,0x8d,0x00,0x00,0x00,0x00,0x00,0x62,0x45,0x60,0x2d,0x10,0x9d,
	0x22,0x22,0x22,0x22,0x22,0x12,0x11,0x72,0x2d,0x10,0xed,0xee,0xee,0xee,0xee,0xee,
	0xee,0xee,0xed,0x2d,0x00,0x65,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x15,0x00,
	0x76,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x16,0x10,0xed,0xee,0xee,0xee,0xee,
	0xee,0xee,0xee,0xed,0x2d,0x10,0x9d,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x71,0x2d,
	0x30,0x8d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x2d,0xd6,0x8e,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x60,0x2d,0xe7,0x8e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,
	0x2d,0xe6,0x8e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x2d,0xe6,0x8e,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x60,0x2d,0xe7,0x8e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x60,0x2d,0xd6,0x8e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x2d,0x30,0x8d,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x2d,0x10,0x9d,0x22,0x22,0x22,0x22,0x22,0x22,
	0x22,0x72,0x2d,0x10,0xed,0xee,0xee,0xee,0xee,0xee,0xee,0xee,0xed,0x2d,0x00,0x65,
	0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const int ALIGN_DATA icon_s_pos[NUM_FONTS] = {
	0x000000,0x0000d8,0x000188,0x00021e,0x0002ae,0x000353,0x0003d7,0x00046d,0x0004f9,0x00057b,0x000611,0x0006ab,0x000745,0x0007df
};

static const s8 ALIGN_DATA icon_s_width[NUM_FONTS] = {
	24,22,20,24,22,22,20,20,20,20,22,22,22,22
};

static const s8 ALIGN_DATA icon_s_height[NUM_FONTS] = {
	18,16,15,12,15,12,15,14,13,15,14,14,14,14
};

static const s8 ALIGN_DATA icon_s_skipx[NUM_FONTS] = {
	 0, 1, 2, 0, 1, 1, 2, 2, 2, 3, 1, 1, 1, 1
};

static const s8 ALIGN_DATA icon_s_skipy[NUM_FONTS] = {
	 0, 1, 1, 3, 2, 3, 2, 2, 3, 2, 2, 2, 2, 2
};


/*------------------------------------------------------
	functions
------------------------------------------------------*/

int icon_s_get_gryph(struct font_t *font, u16 code)
{
	if (code < NUM_FONTS)
	{
		font->data   = &icon_s[icon_s_pos[code]];
		font->width  = icon_s_width[code];
		font->height = icon_s_height[code];
		font->pitch  = 32;
		font->skipx  = icon_s_skipx[code];
		font->skipy  = icon_s_skipy[code];
		return 1;
	}
	return 0;
}
