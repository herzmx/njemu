/******************************************************************************

	biosmenu.c

	MVS BIOSëIëÉÅÉjÉÖÅ[

******************************************************************************/

#ifndef MVS_BIOS_MENU_H
#define MVS_BIOS_MENU_H

enum
{
	EUROPE_V2 = 0,
	EUROPE_V1,
	USA_V2,
	USA_V1,
	ASIA_V3,
	JAPAN_V3,
	JAPAN_V2,
	JAPAN_V1,
	ASIA_AES,

	BIOS_MAX
};


extern const char *bios_name[BIOS_MAX];
extern const u32 bios_crc[BIOS_MAX];
extern const u32 bios_patch_address[BIOS_MAX];
extern const u32 sfix_crc;
extern const u32 lorom_crc;

void bios_select(int flag);

#endif /* MVS_BIOS_MENU_H */
