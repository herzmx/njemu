/******************************************************************************

	biosmenu.c

	MVS BIOS�I�����j���[

******************************************************************************/

#ifndef MVS_BIOS_MENU_H
#define MVS_BIOS_MENU_H

enum
{
	// official BIOS
	EUROPE_V2 = 0,
	EUROPE_V1,
	USA_V2,
	USA_V1,
	ASIA_V3,
	JAPAN_V3,
	JAPAN_V2,
	JAPAN_V1,
	ASIA_AES,

	// hack BIOS
	UNI_V10,
	UNI_V11,
	UNI_V12old,
	UNI_V12,
	UNI_V13,

	UNI_V20,
	UNI_V21,
	UNI_V22,

	DEBUG_BIOS,

	BIOS_MAX
};


extern const char *bios_name[BIOS_MAX];
extern const u32 bios_crc[BIOS_MAX];
extern const u32 bios_patch_address[BIOS_MAX];
//extern const u32 sm1_crc;
extern const u32 sfix_crc;
extern const u32 lorom_crc;

void bios_select(int flag);

#endif /* MVS_BIOS_MENU_H */