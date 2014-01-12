/******************************************************************************

	loadrom.c

	ROMイメージファイルロード関数

******************************************************************************/

#ifndef LOAD_ROM_H
#define LOAD_ROM_H

enum
{
	ROM_LOAD = 0,
	ROM_CONTINUE,
	ROM_WORDSWAP,
	MAP_MAX
};

struct rom_t
{
	u32 type;
	u32 offset;
	u32 length;
	u32 crc;
	int group;
	int skip;
};

int  file_open(const char *fname1, const char *fname2, const u32 crc, char *fname);
void file_close(void);
int  file_read(void *buf, size_t length);
int  file_getc(void);
#ifdef USE_CACHE
int  cachefile_open(const char *fname);
#endif
int rom_load(struct rom_t *rom, u8 *mem, int idx, int max);

#endif /* LOAD_ROM_H */
