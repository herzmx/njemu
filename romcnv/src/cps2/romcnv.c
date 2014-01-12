/*****************************************************************************

	romcnv.c

	ROM converter for CPS2PSP

******************************************************************************/

#include "romcnv.h"


#define SPR_BLANK		0x00
#define SPR_NOT_BLANK	0x02
#define SPR_OPAQUE		0x01

#define MAX_GFX1ROM		32

enum
{
	REGION_GFX1 = 0,
	REGION_SKIP
};

enum
{
	ROM_LOAD = 0,
	ROM_CONTINUE,
	ROM_WORDSWAP,
	MAP_MAX
};

enum
{
	TILE08 = 0,
	TILE16,
	TILE32,
	TILE_SIZE_MAX
};


/******************************************************************************
	ローカル変数
******************************************************************************/

static char delimiter = '/';

static u8  *memory_region_gfx1;
static u32 memory_length_gfx1;

static u32 gfx_total_elements[TILE_SIZE_MAX];
static u8  *gfx_pen_usage[TILE_SIZE_MAX];

static char game_dir[MAX_PATH];
static char zip_dir[MAX_PATH];
static char launchDir[MAX_PATH];

static char game_name[16];
static char parent_name[16];
static char cache_name[16];

static u8 block_empty[0x200];

static u8 null_tile[128] =
{
	0x67,0x66,0x66,0x66,0x66,0x66,0x66,0x56,
	0x56,0x55,0x55,0x55,0x55,0x55,0x55,0x45,
	0x56,0x51,0x15,0x51,0x11,0x15,0x51,0x45,
	0x56,0x11,0x15,0x51,0x11,0x15,0x51,0x45,
	0x56,0x11,0x11,0x51,0x11,0x15,0x51,0x45,
	0x56,0x11,0x15,0x51,0x11,0x15,0x51,0x45,
	0x56,0x11,0x55,0x51,0x11,0x11,0x51,0x45,
	0x56,0x55,0x55,0x55,0x55,0x55,0x55,0x45,
	0x56,0x11,0x55,0x55,0x11,0x55,0x55,0x45,
	0x56,0x11,0x55,0x55,0x11,0x55,0x55,0x45,
	0x56,0x11,0x55,0x55,0x11,0x55,0x55,0x45,
	0x56,0x11,0x55,0x55,0x11,0x55,0x55,0x45,
	0x56,0x11,0x11,0x51,0x11,0x11,0x51,0x45,
	0x56,0x55,0x55,0x55,0x55,0x55,0x55,0x45,
	0x56,0x55,0x55,0x55,0x55,0x55,0x55,0x45,
	0x45,0x44,0x44,0x44,0x44,0x44,0x44,0x34
};

static u8 blank_tile[128] =
{
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x11,0x11,0x11,0x11,0x11,0x11,0xff,
	0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0xf1,
	0x1f,0xff,0xff,0x1f,0xff,0xff,0xf1,0xf1,
	0x1f,0xff,0xff,0xff,0xf1,0x1f,0x1f,0xf1,
	0x1f,0xff,0xff,0xff,0xff,0xff,0xf1,0xf1,
	0x1f,0xff,0xff,0x1f,0xff,0xff,0xff,0xf1,
	0x1f,0xff,0xff,0x1f,0xff,0xff,0xff,0xf1,
	0x1f,0xff,0xf1,0xff,0xf1,0x1f,0xff,0xf1,
	0x1f,0x1f,0xff,0xff,0xf1,0xff,0xf1,0xf1,
	0x1f,0x1f,0xff,0xff,0x1f,0xff,0xf1,0xf1,
	0x1f,0x1f,0x1f,0xff,0x1f,0xff,0xf1,0xf1,
	0x1f,0xff,0xff,0x11,0xf1,0xff,0xff,0xf1,
	0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0xf1,
	0xff,0x11,0x11,0x11,0x11,0x11,0x11,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

struct rom_t
{
	u32 type;
	char name[16];
	u32 offset;
	u32 length;
	u32 crc;
	int group;
	int skip;
};

static struct rom_t gfx1rom[MAX_GFX1ROM];
static int num_gfx1rom;
static int rom_fd = -1;


struct cacheinfo_t
{
	const char *name;
	u32  object_start;
	u32  object_end;
	u32  scroll1_start;
	u32  scroll1_end;
	u32  scroll2_start;
	u32  scroll2_end;
	u32  scroll3_start;
	u32  scroll3_end;
	u32  object2_start;
	u32  object2_end;
};

struct cacheinfo_t *cacheinfo;

struct cacheinfo_t CPS2_cacheinfo[] =
{
//    name        object              scroll1             scroll2             scroll3             object/scroll2
	{ "ssf2",     0x000000, 0x7fffff, 0x800000, 0x88ffff, 0x900000, 0xabffff, 0xac0000, 0xbbffff, 0,         0         },
	{ "ddtod",    0x000000, 0x7fffff, 0x800000, 0x8fffff, 0x900000, 0xafffff, 0xac0000, 0xbfffff, 0,         0         },
	{ "ecofghtr", 0x000000, 0x7fffff, 0x800000, 0x83ffff, 0x880000, 0x99ffff, 0xa00000, 0xabffff, 0,         0         },
	{ "ssf2t",    0x000000, 0x7fffff, 0x800000, 0x88ffff, 0x900000, 0xabffff, 0xac0000, 0xffffff, 0,         0         },
	{ "xmcota",   0x000000, 0x7dffff, 0x800000, 0x8dffff, 0xb00000, 0xfdffff, 0x8e0000, 0xafffff, 0x1000000, 0x1ffffff },
	{ "armwar",   0x000000, 0x7fffff, 0x800000, 0x85ffff, 0x860000, 0x9bffff, 0x9c0000, 0xa5ffff, 0xa60000,  0x12fffff },
	{ "avsp",     0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0x9fffff, 0xa00000, 0xafffff, 0,         0         },
	{ "dstlk",    0x000000, 0x7cffff, 0x800000, 0x87ffff, 0x880000, 0x9bffff, 0x9c0000, 0xabffff, 0xac0000,  0x13fffff },
	{ "ringdest", 0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0x9fffff, 0xac0000, 0xcfffff, 0xd40000,  0x11fffff },
	{ "cybots",   0x000000, 0x7dffff, 0x800000, 0x8bffff, 0x8c0000, 0xb3ffff, 0xb40000, 0xcbffff, 0xcc0000,  0x1ffffff },
	{ "msh",      0x000000, 0x7fffff, 0x800000, 0x8cffff, 0xb00000, 0xffffff, 0x8e0000, 0xafffff, 0x1000000, 0x1ffffff },
	{ "nwarr",    0x000000, 0x7cffff, 0x800000, 0x87ffff, 0x880000, 0x9bffff, 0x9c0000, 0xabffff, 0xac0000,  0x1f8ffff },
	{ "sfa",      0x000000, 0x000000, 0x800000, 0x81ffff, 0x820000, 0xf8ffff, 0xfa0000, 0xfeffff, 0,         0         },
	{ "mmancp2u", 0x000000, 0x000000, 0x800000, 0x85ffff, 0x860000, 0xe6ffff, 0xe80000, 0xfeffff, 0,         0         },
	{ "19xx",     0x000000, 0x16ffff, 0x800000, 0x83ffff, 0x840000, 0x9bffff, 0x9c0000, 0xafffff, 0xb00000,  0xffffff  },
	{ "ddsom",    0x000000, 0x7dffff, 0x800000, 0x8bffff, 0x8c0000, 0xbdffff, 0xbe0000, 0xdbffff, 0xde0000,  0x179ffff },
	{ "megaman2", 0x000000, 0x000000, 0x800000, 0x85ffff, 0x860000, 0xecffff, 0xee0000, 0xffffff, 0,         0         },
	{ "qndream",  0x000000, 0x000000, 0x800000, 0x81ffff, 0x840000, 0xefffff, 0x820000, 0x83ffff, 0,         0         },
	{ "sfa2",     0x000000, 0x79ffff, 0x800000, 0x91ffff, 0xa40000, 0xccffff, 0x920000, 0xa3ffff, 0xd20000,  0x138ffff },
	{ "spf2t",    0x000000, 0x000000, 0x800000, 0x82ffff, 0x840000, 0xb8ffff, 0xb90000, 0xbcffff, 0,         0         },
	{ "xmvsf",    0x000000, 0x7effff, 0x800000, 0x8fffff, 0xaa0000, 0xffffff, 0x900000, 0xa7ffff, 0x1000000, 0x1ffffff },
	{ "batcir",   0x000000, 0x7dffff, 0x800000, 0x817fff, 0x818000, 0x937fff, 0x938000, 0xa3ffff, 0xa48000,  0xd8ffff  },
	{ "csclub",   0x000000, 0x000000, 0x8c0000, 0x8fffff, 0x900000, 0xffffff, 0x800000, 0x8bffff, 0,         0         },
	{ "mshvsf",   0x000000, 0x7fffff, 0x800000, 0x8dffff, 0xa80000, 0xfeffff, 0x8e0000, 0xa6ffff, 0x1000000, 0x1feffff },
	{ "sgemf",    0x000000, 0x7fffff, 0x800000, 0x8d1fff, 0xa22000, 0xfdffff, 0x8d2000, 0xa21fff, 0x1000000, 0x13fffff },
	{ "vhunt2",   0x000000, 0x7affff, 0x800000, 0x8affff, 0xa10000, 0xfdffff, 0x8c0000, 0xa0ffff, 0x1000000, 0x1fdffff },
	{ "vsav",     0x000000, 0x7fffff, 0x800000, 0x8bffff, 0x9c0000, 0xffffff, 0x8c0000, 0x9bffff, 0x1000000, 0x1feffff },
	{ "vsav2",    0x000000, 0x7fffff, 0x800000, 0x8affff, 0xa10000, 0xfdffff, 0x8c0000, 0xa0ffff, 0x1000000, 0x1fdffff },
	{ "mvsc",     0x000000, 0x7cffff, 0x800000, 0x91ffff, 0xb40000, 0xd0ffff, 0x920000, 0xb2ffff, 0xd20000,  0x1feffff },
	{ "sfa3",     0x000000, 0x7dffff, 0x800000, 0x95ffff, 0xb60000, 0xffffff, 0x960000, 0xb5ffff, 0x1000000, 0x1fcffff },
	{ "jyangoku", 0x000000, 0x7fffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0,         0         },
	{ "hsf2",     0x000000, 0x7fffff, 0x800000, 0x1ffffff,0x800000, 0x1ffffff,0x800000, 0x1ffffff,0,         0         },
	{ "gigawing", 0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0xa7ffff, 0xa80000, 0xdcffff, 0xe00000,  0xffffff  },
	{ "mmatrix",  0x000000, 0x7fffff, 0x800000, 0x8fffff, 0x800000, 0xd677ff, 0x800000, 0xd677ff, 0x1000000, 0x1ffffff },
	{ "mpangj",   0x000000, 0x000000, 0x800000, 0x82ffff, 0x840000, 0x9dffff, 0xa00000, 0xbdffff, 0xc00000,  0xffffff  },
	{ "mpang",    0x000000, 0x000000, 0x800000, 0x82ffff, 0x840000, 0x9dffff, 0xa00000, 0xbdffff, 0xc00000,  0xffffff  },
	{ "pzloop2",  0x000000, 0x81ffff, 0x800000, 0x97ffff, 0xa00000, 0xc8ffff, 0xd80000, 0xebffff, 0,         0         },
	{ "choko",    0x000000, 0x7fffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0x800000, 0xffffff, 0,         0         },
	{ "dimahoo",  0x000000, 0x7fffff, 0x800000, 0x8bffff, 0xb80000, 0xffffff, 0x8e0000, 0xb6ffff, 0,         0         },
	{ "1944",     0x000000, 0x7fffff, 0x800000, 0x87ffff, 0x880000, 0xcdffff, 0xd00000, 0xfeffff, 0x1000000, 0x13bffff },
	{ "progear",  0x000000, 0x7fffff, 0x800000, 0xa0afff, 0xa0b000, 0xd86fff, 0xd87000, 0xffffff, 0,         0         },
	{ NULL }
};


/******************************************************************************
	ローカル関数
******************************************************************************/

#ifdef WIN32

static int is_win9x = 0;

/*--------------------------------------------------------
	「ファイルを開く」ダイアログを表示
--------------------------------------------------------*/

static int file_dialog(HWND hwnd, LPCSTR filter, char *fname, u32 flags)
{
	OPENFILENAME OFN;

	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner   = hwnd;
	OFN.lpstrFilter = filter;
	OFN.lpstrFile   = fname;
	OFN.nMaxFile    = MAX_PATH * 2;
	OFN.Flags       = flags;
	OFN.lpstrTitle  = "Select zipped ROM file.";

	return GetOpenFileName(&OFN);
}


/*--------------------------------------------------------
	「フォルダを開く」ダイアログを表示
--------------------------------------------------------*/

static int folder_dialog(HWND hwnd, char *path)
{
	BROWSEINFO BINFO;
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc;
	int res = 0;

	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		memset(&BINFO, 0, sizeof(BINFO));
		BINFO.hwndOwner = hwnd;
		BINFO.lpszTitle = "Select ROM folder";
		BINFO.ulFlags = BIF_RETURNONLYFSDIRS;

		pidl = SHBrowseForFolder(&BINFO);
		if (pidl)
		{
			res = SHGetPathFromIDList(pidl, path);
			IMalloc_Free(pMalloc, pidl);
		}
		IMalloc_Release(pMalloc);
	}
	return res;
}


/*--------------------------------------------------------
	デリミタを変換
--------------------------------------------------------*/

#define issjis1(c)	(((c) >= 0x81 && (c) <= 0x9f) | ((c) >= 0xe0 && (c) <= 0xfc))

static void convert_delimiter(char *path)
{
	if (!is_win9x)
	{
		char *p = path;
		int i, len = strlen(path);

		for (i = 0; i < len; i++)
		{
			if (*p == '\\')
			{
				if (i == 0 || !issjis1(*(u8 *)(p - 1)))
					*p = '/';
			}
			p++;
		}
	}
}

#endif /* WIN32 */


/*--------------------------------------------------------
	エラーメッセージ表示
--------------------------------------------------------*/

static void error_memory(const char *mem_name, int pause)
{
	zip_close();
	printf("ERROR: Could not allocate %s memory.\n", mem_name);
	if (pause)
	{
		printf("Press any button.\n");
		getch();
	}
}


static void error_rom(const char *rom_name, int pause)
{
	zip_close();
	printf("ERROR: File not found or CRC32 not correct. \"%s\"\n", rom_name);
	if (pause)
	{
		printf("Press any button.\n");
		getch();
	}
}


/*--------------------------------------------------------
	ROMファイルを閉じる
--------------------------------------------------------*/

static void file_close(void)
{
	if (rom_fd != -1)
	{
		zclose(rom_fd);
		zip_close();
		rom_fd = -1;
	}
}


/*--------------------------------------------------------
	ROMファイルを開く
--------------------------------------------------------*/

static int file_open(const char *fname1, const char *fname2, const u32 crc, char *fname)
{
	int found = 0;
	struct zip_find_t file;
	char path[MAX_PATH];

	file_close();

	sprintf(path, "%s%c%s.zip", zip_dir, delimiter, fname1);

	if (zip_open(path, "rb") != -1)
	{
		if (zip_findfirst(&file))
		{
			if (file.crc32 == crc)
			{
				found = 1;
			}
			else
			{
				while (zip_findnext(&file))
				{
					if (file.crc32 == crc)
					{
						found = 1;
						break;
					}
				}
			}
		}
		if (!found) zip_close();
	}

	if (!found && fname2 != NULL)
	{
		sprintf(path, "%s%c%s.zip", zip_dir, delimiter, fname2);

		if (zip_open(path, "rb") != -1)
		{
			if (zip_findfirst(&file))
			{
				if (file.crc32 == crc)
				{
					found = 2;
				}
				else
				{
					while (zip_findnext(&file))
					{
						if (file.crc32 == crc)
						{
							found = 2;
							break;
						}
					}
				}
			}
			if (!found) zip_close();
		}
	}

	if (found)
	{
		if (fname) strcpy(fname, file.name);
		rom_fd = zopen(file.name);
		return rom_fd;
	}

	return -1;
}


/*--------------------------------------------------------
	ROMファイルを指定バイト読み込む
--------------------------------------------------------*/

static int file_read(void *buf, size_t length)
{
	if (rom_fd != -1)
		return zread(rom_fd, buf, length);
	return -1;
}


/*--------------------------------------------------------
	ROMファイルを1バイト読み込む
--------------------------------------------------------*/

static int file_getc(void)
{
	if (rom_fd != -1)
		return zgetc(rom_fd);
	return -1;
}


/*--------------------------------------------------------
	ROMを指定メモリエリアに読み込む
--------------------------------------------------------*/

static int rom_load(struct rom_t *rom, u8 *mem, int idx, int max)
{
	int offset, length;

_continue:
	offset = rom[idx].offset;

	if (rom[idx].skip == 0)
	{
		file_read(&mem[offset], rom[idx].length);

		if (rom[idx].type == ROM_WORDSWAP)
			swab(&mem[offset], &mem[offset], rom[idx].length);
	}
	else
	{
		int c;
		int skip = rom[idx].skip + rom[idx].group;

		length = 0;

		if (rom[idx].group == 1)
		{
			if (rom[idx].type == ROM_WORDSWAP)
				offset ^= 1;

			while (length < rom[idx].length)
			{
				if ((c = file_getc()) == EOF) break;
				mem[offset] = c;
				offset += skip;
				length++;
			}
		}
		else
		{
			while (length < rom[idx].length)
			{
				if ((c = file_getc()) == EOF) break;
				mem[offset + 0] = c;
				if ((c = file_getc()) == EOF) break;
				mem[offset + 1] = c;
				offset += skip;
				length += 2;
			}
		}
	}

	if (++idx != max)
	{
		if (rom[idx].type == ROM_CONTINUE)
		{
			goto _continue;
		}
	}

	return idx;
}


/*--------------------------------------------------------
	文字列の比較
--------------------------------------------------------*/

static int str_cmp(const char *s1, const char *s2)
{
	return strnicmp(s1, s2, strlen(s2));
}


/******************************************************************************
	CPS2用関数
******************************************************************************/

static void unshuffle(u64 *buf, int len)
{
	int i;
	u64 t;

	if (len == 2) return;

	len /= 2;

	unshuffle(buf, len);
	unshuffle(buf + len, len);

	for (i = 0; i < len / 2; i++)
	{
		t = buf[len / 2 + i];
		buf[len / 2 + i] = buf[len + i];
		buf[len + i] = t;
	}
}


static void cps2_gfx_decode(void)
{
	int i, j;
	u8 *gfx = memory_region_gfx1;

	for (i = 0; i < memory_length_gfx1; i += 0x200000)
		unshuffle((u64 *)&memory_region_gfx1[i], 0x200000 / 8);

	for (i = 0; i < memory_length_gfx1 / 4; i++)
	{
		u32 src = gfx[4 * i] + (gfx[4 * i + 1] << 8) + (gfx[4 * i + 2] << 16) + (gfx[4 * i + 3] << 24);
		u32 dw = 0, data;

		for (j = 0; j < 8; j++)
		{
			int n = 0;
			u32 mask = (0x80808080 >> j) & src;

			if (mask & 0x000000ff) n |= 1;
			if (mask & 0x0000ff00) n |= 2;
			if (mask & 0x00ff0000) n |= 4;
			if (mask & 0xff000000) n |= 8;

			dw |= n << (j * 4);
		}

		data = ((dw & 0x0000000f) >>  0) | ((dw & 0x000000f0) <<  4)
			 | ((dw & 0x00000f00) <<  8) | ((dw & 0x0000f000) << 12)
			 | ((dw & 0x000f0000) >> 12) | ((dw & 0x00f00000) >>  8)
			 | ((dw & 0x0f000000) >>  4) | ((dw & 0xf0000000) >>  0);

		gfx[4 * i + 0] = data >>  0;
		gfx[4 * i + 1] = data >>  8;
		gfx[4 * i + 2] = data >> 16;
		gfx[4 * i + 3] = data >> 24;
	}
}


static void clear_empty_blocks(void)
{
	int i, j, size;
	u8 temp[512];
	int blocks_available = 0;

	memset(block_empty, 0, 0x200);

	for (i = 0; i < memory_length_gfx1; i += 128)
	{
		if (memcmp(&memory_region_gfx1[i], null_tile, 128) == 0
		||	memcmp(&memory_region_gfx1[i], blank_tile, 128) == 0)
			memset(&memory_region_gfx1[i], 0xff, 128);
	}

	if (!strcmp(cacheinfo->name, "avsp"))
	{
		for (i = 0xb0; i <= 0xff; i++)
			memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
	}
	else if (!strcmp(cacheinfo->name, "ddtod"))
	{
		memcpy(temp, &memory_region_gfx1[0x5be800], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x657a00], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x707800], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x710b80], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x77d080], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x780000], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x7b5580], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x7d7800], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x93bd00], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0x9a5380], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0xa3eb80], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0xa70300], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0xa84f00], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
		memcpy(temp, &memory_region_gfx1[0xb75000], 512);
		for (i = 0; i < memory_length_gfx1; i += 512)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
				memset(&memory_region_gfx1[i], 0xff, 512);
		}
		memcpy(temp, &memory_region_gfx1[0xb90600], 512);
		for (i = 0; i < memory_length_gfx1; i += 512)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
				memset(&memory_region_gfx1[i], 0xff, 512);
		}
		memcpy(temp, &memory_region_gfx1[0xbcb200], 512);
		for (i = 0; i < memory_length_gfx1; i += 512)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
				memset(&memory_region_gfx1[i], 0xff, 512);
		}
		memcpy(temp, &memory_region_gfx1[0xbd0000], 512);
		for (i = 0; i < memory_length_gfx1; i += 512)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 512) == 0)
				memset(&memory_region_gfx1[i], 0xff, 512);
		}
	}
	else if (!strcmp(cacheinfo->name, "dstlk") || !strcmp(cacheinfo->name, "nwarr"))
	{
		for (i = 0x7d; i <= 0x7f; i++)
			memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		memset(&memory_region_gfx1[0xff0000 + (16*29)*128], 0xff, 0x10000-(16*29)*128);
		memset(&memory_region_gfx1[0x13f0000 + (16*11)*128], 0xff, 0x10000-(16*11)*128);

		memcpy(temp, &memory_region_gfx1[0x10000], 128);
		for (i = 0; i < memory_length_gfx1; i += 128)
		{
			if (memcmp(&memory_region_gfx1[i], temp, 128) == 0)
				memset(&memory_region_gfx1[i], 0xff, 128);
		}
	}
	else if (!strcmp(cacheinfo->name, "ringdest"))
	{
		for (i = 0xa0; i <= 0xab; i++)
			memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0xd0; i <= 0xd3; i++)
			memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
	}
	else if (!strcmp(cacheinfo->name, "mpang") || !strcmp(cacheinfo->name, "mpangj"))
	{
		memset(&memory_region_gfx1[0x820000 + 16*11*128], 0xff, 16*21*128);
		memset(&memory_region_gfx1[0x830000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0x840000 + (16*31+13)*128], 0xff, 0x10000-(16*31+13)*128);
		memset(&memory_region_gfx1[0x850000], 0xff, 16*16*128);
		memset(&memory_region_gfx1[0x9d0000 + (16*22+13)*128], 0xff, 0x10000-(16*22+13)*128);
		memset(&memory_region_gfx1[0x9e0000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0x9f0000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0xbd0000 + (16*4+8)*128], 0xff, 0x10000-(16*4+8)*128);
		memset(&memory_region_gfx1[0xbe0000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0xbf0000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0xd50000 + (16*12)*128], 0xff, 0x10000-(16*12)*128);
		memset(&memory_region_gfx1[0xd60000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0xd70000], 0xff, 0x10000);
		memset(&memory_region_gfx1[0xdf0000 + (16*24)*128], 0xff, 0x10000-(16*24)*128);
		memset(&memory_region_gfx1[0xef0000 + (16*31)*128], 0xff, 0x10000-(16*31)*128);
		memset(&memory_region_gfx1[0xfb0000 + (16*14)*128], 0xff, 0x10000-(16*14)*128);
		memset(&memory_region_gfx1[0xff0000 + (16*12)*128], 0xff, 0x10000-(16*12)*128);
	}
	else if (!strcmp(cacheinfo->name, "mmatrix"))
	{
		memset(&memory_region_gfx1[0xd67600], 0xff, (16*17+4)*128);
		for (i = 0xd7; i <= 0xff; i++)
			memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
	}
	else if (!strcmp(cacheinfo->name, "pzloop2"))
	{
		memset(&memory_region_gfx1[0x170000 + 16*16*128], 0xff, 16*16*128);
		memset(&memory_region_gfx1[0x1c0000 + 16* 9*128], 0xff, 16*23*128);
		memset(&memory_region_gfx1[0x230000 + 16* 7*128], 0xff, 16*25*128);
		memset(&memory_region_gfx1[0x270000 + 16*17*128], 0xff, 16*15*128);
		memset(&memory_region_gfx1[0x290000 + 16*23*128], 0xff, 16* 9*128);
		memset(&memory_region_gfx1[0x2d0000 + 16*21*128], 0xff, 16*11*128);
		memset(&memory_region_gfx1[0x390000 + 16*30*128], 0xff, 16* 2*128);
		memset(&memory_region_gfx1[0x410000 + 16*17*128], 0xff, 16*15*128);
		memset(&memory_region_gfx1[0x530000 + 16* 6*128], 0xff, 16*26*128);
		memset(&memory_region_gfx1[0x590000 + 16* 4*128], 0xff, 16*28*128);
		memset(&memory_region_gfx1[0x670000 + 16* 9*128], 0xff, 16*23*128);
		memset(&memory_region_gfx1[0x730000 + 16*12*128], 0xff, 16*20*128);
		memset(&memory_region_gfx1[0x7a0000 + 16*10*128], 0xff, 16*22*128);
		memset(&memory_region_gfx1[0x802000 + 2*128], 0xff, 14*128);
		memset(&memory_region_gfx1[0x806800 + 4*128], 0xff, 12*128);
		memset(&memory_region_gfx1[0x810000 + 16*19*128 + 128], 0xff, 16*13*128 - 128);
		memset(&memory_region_gfx1[0xc80000 + 11*128], 0xff, 0x10000 - 11*128);
		memset(&memory_region_gfx1[0x970000 + (16*27+11)*128], 0xff, 0x10000 - (16*17+11)*128);
		memset(&memory_region_gfx1[0xeb0000 + (16*2+9)*512], 0xff, 0x10000 - (16*2+9)*512);

		for (i = 0x1d; i <= 0x1f; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x2a; i <= 0x2b; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x3a; i <= 0x3f; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x42; i <= 0x47; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x54; i <= 0x57; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x5a; i <= 0x5f; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x74; i <= 0x77; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x7b; i <= 0x7f; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x82; i <= 0x87; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0x98; i <= 0x9f; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0xc9; i <= 0xd7; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
		for (i = 0xec; i <= 0xff; i++) memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
	}
	else if (!strcmp(cacheinfo->name, "1944"))
	{
		for (i = 0x140; i <= 0x1ff; i++)
			memset(&memory_region_gfx1[i << 16], 0xff, 0x10000);
	}

	if (cacheinfo->object_end == 0)
	{
		memset(memory_region_gfx1, 0xff, 0x800000);
	}
	else if (cacheinfo->object_end != 0x7fffff)
	{
		for (i = cacheinfo->object_end + 1; i < 0x800000; i += 0x10000)
		{
			memset(&memory_region_gfx1[i], 0xff, 0x10000);
		}
	}

	for (i = 0; i < memory_length_gfx1 >> 16; i++)
	{
		int empty = 1;
		u32 offset = i << 16;

		for (j = 0; j < 0x10000; j++)
		{
			if (memory_region_gfx1[offset + j] != 0xff)
			{
				empty = 0;
				break;
			}
		}

		block_empty[i] = empty;
	}
	for (; i < 0x200; i++)
	{
		block_empty[i] = 1;
	}

	for (i = 0; i < memory_length_gfx1 >> 16; i++)
	{
		if (!block_empty[i]) blocks_available++;
	}
//	printf("cache required size = %x\n", blocks_available << 16);

	size = blocks_available << 16;
	if (size != memory_length_gfx1)
	{
		printf("remove empty tiles (total size: %d bytes -> %d bytes)\n", memory_length_gfx1, size);
	}
}


static int calc_pen_usage(void)
{
	int i, j, k, size;
	u32 *tile, data;
	u32 s0 = cacheinfo->object_start;
	u32 e0 = cacheinfo->object_end;
	u32 s1 = cacheinfo->scroll1_start;
	u32 e1 = cacheinfo->scroll1_end;
	u32 s2 = cacheinfo->scroll2_start;
	u32 e2 = cacheinfo->scroll2_end;
	u32 s3 = cacheinfo->scroll3_start;
	u32 e3 = cacheinfo->scroll3_end;
	u32 s4 = cacheinfo->object2_start;
	u32 e4 = cacheinfo->object2_end;

	gfx_total_elements[TILE08] = (memory_length_gfx1 - 0x800000) >> 6;
	gfx_total_elements[TILE16] = memory_length_gfx1 >> 7;
	gfx_total_elements[TILE32] = (memory_length_gfx1 - 0x800000) >> 9;

	if (gfx_total_elements[TILE08] >= 0x10000) gfx_total_elements[TILE08] = 0x10000;
	if (gfx_total_elements[TILE32] >= 0x10000) gfx_total_elements[TILE32] = 0x10000;

	gfx_pen_usage[TILE08] = malloc(gfx_total_elements[TILE08]);
	gfx_pen_usage[TILE16] = malloc(gfx_total_elements[TILE16]);
	gfx_pen_usage[TILE32] = malloc(gfx_total_elements[TILE32]);

	if (!gfx_pen_usage[TILE08] || !gfx_pen_usage[TILE16] || !gfx_pen_usage[TILE32])
		return 0;

	memset(gfx_pen_usage[TILE08], 0, gfx_total_elements[TILE08]);
	memset(gfx_pen_usage[TILE16], 0, gfx_total_elements[TILE16]);
	memset(gfx_pen_usage[TILE32], 0, gfx_total_elements[TILE32]);

	for (i = 0; i < gfx_total_elements[TILE08]; i++)
	{
		int count = 0;
		u32 offset = (0x20000 + i) << 6;
		int s5 = 0x000000;
		int e5 = 0x000000;

		if (!strcmp(cacheinfo->name, "pzloop2"))
		{
			s5 = 0x802800;
			e5 = 0x87ffff;
		}

		if ((offset >= s1 && offset <= e1) && !(offset >= s5 && offset <= e5))
		{
			tile = (u32 *)&memory_region_gfx1[offset];

			for (j = 0; j < 8; j++)
			{
				tile++;
				data = *tile++;
				for (k = 0; k < 8; k++)
				{
					if ((data & 0x0f) == 0x0f)
						count++;
					data >>= 4;
				}
			}
			if (count == 0)
				gfx_pen_usage[TILE08][i] = SPR_OPAQUE;
			else if (count != 8*8)
				gfx_pen_usage[TILE08][i] = SPR_NOT_BLANK;
		}
	}

	for (i = 0; i < gfx_total_elements[TILE16]; i++)
	{
		u32 s5 = 0;
		u32 e5 = 0;
		u32 offset = i << 7;

		if (!strcmp(cacheinfo->name, "ssf2t"))
		{
			s5 = 0xc00000;
			e5 = 0xfaffff;
		}
		else if (!strcmp(cacheinfo->name, "gigawing"))
		{
			s5 = 0xc00000;
			e5 = 0xc7ffff;
		}
		else if (!strcmp(cacheinfo->name, "progear"))
		{
			s5 = 0xf27000;
			e5 = 0xf86fff;
		}

		if ((offset >= s0 && offset <= e0)
		||	(offset >= s2 && offset <= e2)
		||	(offset >= s4 && offset <= e4)
		||	(offset >= s5 && offset <= e5))
		{
			int count = 0;

			tile = (u32 *)&memory_region_gfx1[offset];

			for (j = 0; j < 2*16; j++)
			{
				data = *tile++;
				for (k = 0; k < 8; k++)
				{
					if ((data & 0x0f) == 0x0f)
						count++;
					data >>= 4;
				}
			}
			if (count == 0)
				gfx_pen_usage[TILE16][i] = SPR_OPAQUE;
			else if (count != 2*16*8)
				gfx_pen_usage[TILE16][i] = SPR_NOT_BLANK;
		}
	}

	for (i = 0; i < gfx_total_elements[TILE32]; i++)
	{
		int count  = 0;
		u32 offset = (0x4000 + i) << 9;

		if (!strcmp(cacheinfo->name, "ssf2t"))
		{
			if (offset >= 0xc00000 && offset <= 0xfaffff)
				continue;
		}
		else if (!strcmp(cacheinfo->name, "gigawing"))
		{
			if (offset >= 0xc00000 && offset <= 0xc7ffff)
				continue;
		}
		else if (!strcmp(cacheinfo->name, "progear"))
		{
			if (offset >= 0xf27000 && offset <= 0xf86fff)
				continue;
		}

		if (offset >= s3 && offset <= e3)
		{
			tile = (u32 *)&memory_region_gfx1[offset];

			for (j = 0; j < 4*32; j++)
			{
				data = *tile++;
				for (k = 0; k < 8; k++)
				{
					if ((data & 0x0f) == 0x0f)
						count++;
					data >>= 4;
				}
			}
			if (count == 0)
				gfx_pen_usage[TILE32][i] = SPR_OPAQUE;
			else if (count != 4*32*8)
				gfx_pen_usage[TILE32][i] = SPR_NOT_BLANK;
		}
	}

	return 1;
}


static int load_rom_gfx1(int pause)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_gfx1 = calloc(1, memory_length_gfx1)) == NULL)
	{
		error_memory("REGION_GFX1", pause);
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_gfx1rom; )
	{
		if (file_open(game_name, parent_name, gfx1rom[i].crc, fname) == -1)
		{
			error_rom("GFX1", pause);
			return 0;
		}

		printf("Loading \"%s\"\n", fname);

		i = rom_load(gfx1rom, memory_region_gfx1, i, num_gfx1rom);

		file_close();
	}

	return 1;
}


static int load_rom_info(const char *game_name)
{
	FILE *fp;
	char path[MAX_PATH];
	char buf[256];
	int rom_start = 0;
	int region = 0;

	num_gfx1rom = 0;

	sprintf(path, "%s%crominfo.cps2", launchDir, delimiter);

	if ((fp = fopen(path, "r")) != NULL)
	{
		while (fgets(buf, 255, fp))
		{
			if (buf[0] == '/' && buf[1] == '/')
				continue;

			if (buf[0] != '\t')
			{
				if (buf[0] == '\r' || buf[0] == '\n')
				{
					continue;
				}
				else if (str_cmp(buf, "FILENAME(") == 0)
				{
					char *name, *parent;

					strtok(buf, " ");
					name    = strtok(NULL, " ,");
					parent  = strtok(NULL, " ,");

					if (stricmp(name, game_name) == 0)
					{
						if (str_cmp(parent, "cps2") == 0)
							parent_name[0] = '\0';
						else
							strcpy(parent_name, parent);

						rom_start = 1;
					}
				}
				else if (rom_start && str_cmp(buf, "END") == 0)
				{
					fclose(fp);
					return 0;
				}
			}
			else if (rom_start)
			{
				if (str_cmp(&buf[1], "REGION(") == 0)
				{
					char *size, *type;

					strtok(&buf[1], " ");
					size = strtok(NULL, " ,");
					type = strtok(NULL, " ,");

					if (strcmp(type, "GFX1") == 0)
					{
						sscanf(size, "%x", &memory_length_gfx1);
						region = REGION_GFX1;
					}
					else
					{
						region = REGION_SKIP;
					}
				}
				else if (str_cmp(&buf[1], "ROM(") == 0)
				{
					char *type, *offset, *length, *crc;

					strtok(&buf[1], " ");
					type   = strtok(NULL, " ,");
					offset = strtok(NULL, " ,");
					length = strtok(NULL, " ,");
					crc    = strtok(NULL, " ");

					switch (region)
					{
					case REGION_GFX1:
						sscanf(type, "%x", &gfx1rom[num_gfx1rom].type);
						sscanf(offset, "%x", &gfx1rom[num_gfx1rom].offset);
						sscanf(length, "%x", &gfx1rom[num_gfx1rom].length);
						sscanf(crc, "%x", &gfx1rom[num_gfx1rom].crc);
						gfx1rom[num_gfx1rom].group = 0;
						gfx1rom[num_gfx1rom].skip = 0;
						num_gfx1rom++;
						break;
					}
				}
				else if (str_cmp(&buf[1], "ROMX(") == 0)
				{
					char *type, *offset, *length, *crc;
					char *group, *skip;

					strtok(&buf[1], " ");
					type   = strtok(NULL, " ,");
					offset = strtok(NULL, " ,");
					length = strtok(NULL, " ,");
					crc    = strtok(NULL, " ,");
					group  = strtok(NULL, " ,");
					skip   = strtok(NULL, " ");

					switch (region)
					{
					case REGION_GFX1:
						sscanf(type, "%x", &gfx1rom[num_gfx1rom].type);
						sscanf(offset, "%x", &gfx1rom[num_gfx1rom].offset);
						sscanf(length, "%x", &gfx1rom[num_gfx1rom].length);
						sscanf(crc, "%x", &gfx1rom[num_gfx1rom].crc);
						sscanf(group, "%x", &gfx1rom[num_gfx1rom].group);
						sscanf(skip, "%x", &gfx1rom[num_gfx1rom].skip);
						num_gfx1rom++;
						break;
					}
				}
			}
		}
		fclose(fp);
		return 2;
	}
	return 3;
}


static void free_memory(void)
{
	if (memory_region_gfx1) free(memory_region_gfx1);
	if (gfx_pen_usage[TILE08]) free(gfx_pen_usage[TILE08]);
	if (gfx_pen_usage[TILE16]) free(gfx_pen_usage[TILE16]);
	if (gfx_pen_usage[TILE32]) free(gfx_pen_usage[TILE32]);
}


static int convert_rom(char *game_name, int pause)
{
	int i, res;

	printf("Checking ROM file... (%s)\n", game_name);

	memory_region_gfx1 = NULL;
	memory_length_gfx1 = 0;

	gfx_pen_usage[0] = NULL;
	gfx_pen_usage[1] = NULL;
	gfx_pen_usage[2] = NULL;

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: printf("ERROR: This game is not supported.\n"); break;
		case 2: printf("ERROR: ROM not found. (zip file name incorrect)\n"); break;
		case 3: printf("ERROR: rominfo.cps2 not found.\n"); break;
		}
		return 0;
	}

	if (!strcmp(game_name, "ssf2ta")
	||	!strcmp(game_name, "ssf2tu")
	||	!strcmp(game_name, "ssf2tur1")
	||	!strcmp(game_name, "ssf2xj"))
	{
		strcpy(cache_name, "ssf2t");
	}
	else if (!strcmp(game_name, "ssf2t"))
	{
		cache_name[0] = '\0';
	}
	else if (!strcmp(game_name, "mpangj"))
	{
		cache_name[0] = '\0';
	}
	else
	{
		strcpy(cache_name, parent_name);
	}

	if (strlen(parent_name))
		printf("Clone set (parent: %s)\n", parent_name);

	i = 0;
	cacheinfo = NULL;
	while (CPS2_cacheinfo[i].name)
	{
		if (!strcmp(game_name, CPS2_cacheinfo[i].name))
		{
			cacheinfo = &CPS2_cacheinfo[i];
			break;
		}
		if (!strcmp(cache_name, CPS2_cacheinfo[i].name))
		{
			cacheinfo = &CPS2_cacheinfo[i];
			break;
		}
		i++;
	}

	if (cacheinfo)
	{
		if (load_rom_gfx1(pause))
		{
			cps2_gfx_decode();
			clear_empty_blocks();
			if (calc_pen_usage()) return 1;
		}
		printf("ERROR: Could not allocate memory.\n");
	}
	else
	{
		printf("ERROR: Unknown romset.\n");
	}

	return 0;
}



static int create_raw_cache(char *game_name, int pause)
{
	FILE *fp;
	int i, offset;
	char version[8];
	u32 header_size, aligned_size, block[0x200];
	char fname[MAX_PATH];

	sprintf(version, "CPS2V%d%d\0", VERSION_MAJOR, VERSION_MINOR);

	chdir("cache");

	header_size = 8;
	header_size += gfx_total_elements[TILE08];
	header_size += gfx_total_elements[TILE16];
	header_size += gfx_total_elements[TILE32];
	header_size += 0x200 * sizeof(u32);

	aligned_size = (header_size + 0xffff) & ~0xffff;

	offset = aligned_size;
	for (i = 0; i < 0x200; i++)
	{
		if (block_empty[i])
		{
			block[i] = 0xffffffff;
		}
		else
		{
			block[i] = offset;
			offset += 0x10000;
		}
	}

	sprintf(fname, "%s.cache", game_name);
	if ((fp = fopen(fname, "wb")) == NULL)
	{
		chdir("..");
		printf("ERROR: Could not create file.\n");
		return 0;
	}

	printf("Create cache file...\n");

	fwrite(version, 1, sizeof(version), fp);
	fwrite(gfx_pen_usage[TILE08], 1, gfx_total_elements[TILE08], fp);
	fwrite(gfx_pen_usage[TILE16], 1, gfx_total_elements[TILE16], fp);
	fwrite(gfx_pen_usage[TILE32], 1, gfx_total_elements[TILE32], fp);
	fwrite(block, 1, 0x200 * sizeof(u32), fp);

	for (i = header_size; i < aligned_size; i++)
		fputc(0, fp);

	for (i = 0; i < 0x200; i++)
	{
		if (block_empty[i]) continue;

		fwrite(&memory_region_gfx1[i << 16], 1, 0x10000, fp);
	}

	fclose(fp);

	chdir("..");

	return 1;
}


static void print_progress(int count, int total)
{
	int i, progress = (count * 100) / total;

	printf("%3d%% [", progress);
	for (i = 0; i < progress/2; i++) printf("*");
	for (; i < 50; i++) printf(".");
	printf("]\r");
}


static int create_zip_cache(char *game_name, int pause)
{
	int fd;
	u32 block, res = 0, total = 0, count = 0;
	char version[8], fname[MAX_PATH], zipname[MAX_PATH];

	sprintf(version, "CPS2V%d%d\0", VERSION_MAJOR, VERSION_MINOR);

	chdir("cache");

	sprintf(zipname, "%s%ccache%c%s_cache.zip", launchDir, delimiter, delimiter, game_name);
	remove(zipname);

	printf("Create cache file...\n");

	if (zip_open(zipname, "wb") < 0)
	{
		printf("Error: Could not create zip file \"cache%c%s_cache.zip\".\n", delimiter, game_name);
		goto error;
	}

	printf("Compress to zip file... \"cache%c%s_cache.zip\"\n", delimiter, game_name);

	for (block = 0; block < 0x200; block++)
		if (!block_empty[block]) total++;
	total++;

	print_progress(0, total);

	for (block = 0; block < 0x200; block++)
	{
		if (block_empty[block]) continue;

		sprintf(fname, "%03x", block);
		if ((fd = zopen(fname)) < 0) goto error;
		zwrite(fd, &memory_region_gfx1[block << 16], 0x10000);
		zclose(fd);
		print_progress(++count, total);
	}

	if ((fd = zopen("cache_info")) < 0) goto error;
	zwrite(fd, version, 8);
	zwrite(fd, gfx_pen_usage[TILE08], gfx_total_elements[TILE08]);
	zwrite(fd, gfx_pen_usage[TILE16], gfx_total_elements[TILE16]);
	zwrite(fd, gfx_pen_usage[TILE32], gfx_total_elements[TILE32]);
	zwrite(fd, block_empty, 0x200);
	zclose(fd);

	print_progress(++count, total);
	printf("\n");

	res = 1;

error:
	zip_close();

	if (!res) printf("ERROR: Could not create file.\n");

	chdir("..");

	return res;
}


#ifdef WIN32
#define DELIMITER	'\\'
#else
#define DELIMITER	'/'
#endif

int main(int argc, char *argv[])
{
	char *p, path[MAX_PATH];
	int i, path_found = 0, all = 0, zip = 0, res = 1;
#ifdef WIN32
	int pause = 1;
	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	if (osvi.dwMajorVersion == 4)
	{
		is_win9x  = 1;
		delimiter = '\\';
	}
#else
	int pause = 0;
#endif

	printf("-------------------------------------------\n");
	printf(" ROM converter for CPS2 Emulator ver." VERSION_STR "\n");
	printf("-------------------------------------------\n\n");

	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if (!stricmp(argv[i], "-all"))
			{
				all = 1;
			}
			else if (!stricmp(argv[i], "-zip"))
			{
				zip = 1;
			}
			else if (!stricmp(argv[i], "-batch"))
			{
				pause = 0;
			}
#ifdef WIN32
			else if (strchr(argv[i], ':') != NULL || strchr(argv[i], DELIMITER) != NULL)
#else
			else if (strchr(argv[i], DELIMITER) != NULL)
#endif
			{
				path_found = i;
			}
		}
	}

#ifndef WIN32
	if (!path_found)
	{
		printf("usage: romcnv_cps2 fullpath%cgamename.zip [-all] [-zip] [-batch]\n", DELIMITER);
		return 0;
	}
#endif

	if (chdir("cache") != 0)
	{
		if (mkdir("cache") != 0)
		{
			printf("Error: Could not create directory \"cache\".\n");
			goto error;
		}
	}
	else chdir("..");

#ifdef WIN32
	strcpy(launchDir, argv[0]);
	convert_delimiter(launchDir);

	p = strrchr(launchDir, DELIMITER);
	if (p)
	{
		*(p + 1) = '\0';
	}
	else
#endif
	{
		getcwd(launchDir, MAX_PATH);
#ifdef WIN32
		convert_delimiter(launchDir);
		if (is_win9x)
			strcat(launchDir, "\\");
		else
#endif
			strcat(launchDir, "/");
	}

	if (all)
	{
#ifdef WIN32
		if (!folder_dialog(NULL, zip_dir)) goto error;
		convert_delimiter(zip_dir);
#endif
		strcpy(game_dir, zip_dir);
#ifdef WIN32
		if (is_win9x)
			strcat(game_dir, "\\");
		else
#endif
			strcat(game_dir, "/");

		for (i = 0; CPS2_cacheinfo[i].name; i++)
		{
			res = 1;

			strcpy(game_name, CPS2_cacheinfo[i].name);

			printf("\n-------------------------------------------\n");
			printf("  ROM set: %s\n", game_name);
			printf("-------------------------------------------\n\n");

			if (!strcmp(game_name, "choko")
			||	!strcmp(game_name, "jyangoku"))
			{
				printf("\nSkip \"%s\". - GAME_NOT_WORK\n\n", game_name);
				continue;
			}

			chdir(launchDir);
			if (!convert_rom(game_name, 0))
			{
				printf("ERROR: Convert failed. - Skip\n\n");
			}
			else
			{
				if (zip)
					res = create_zip_cache(game_name, 0);
				else
					res = create_raw_cache(game_name, 0);

				if (!res)
				{
					printf("ERROR: Create cache failed. - Skip\n\n");
				}
			}
			free_memory();
		}

		printf("\ncomplete.\n");
		printf("Please copy these files to directory \"/PSP/GAMES/cps2psp/cache\".\n");
	}
	else
	{
#ifdef WIN32
		if (!path_found)
		{
			printf("Please select ROM file.\n");

			if (!file_dialog(NULL, "zip file (*.zip)\0*.zip\0", game_dir, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY))
				goto error;
		}
		else
		{
			strcpy(path, argv[path_found]);
			strcpy(game_dir, strtok(path, "\""));
		}
		convert_delimiter(game_dir);
#else
		strcpy(game_dir, argv[path_found]);
#endif

		if ((p = strrchr(game_dir, delimiter)) != NULL)
		{
			strcpy(game_name, p + 1);
			strcpy(zip_dir, game_dir);
			*strrchr(zip_dir, delimiter) = '\0';
		}
		else
		{
			strcpy(game_name, game_dir);
			strcpy(zip_dir, "");
		}

		p = game_name;
		while (*p)
		{
			*p = tolower(*p);
			*p++;
		}

		printf("path: %s\n", zip_dir);
		printf("file name: %s\n", game_name);

		if ((p = strrchr(game_name, '.')) == NULL)
		{
			printf("Please input correct path.\n");
			goto error;
		}
		*p = '\0';

		if (zip)
			printf("cache name: cache%c%s_cache.zip\n", delimiter, game_name);
		else
			printf("cache name: cache%c%s.cache\n", delimiter, game_name);

		chdir(launchDir);
		if (!convert_rom(game_name, pause))
		{
			res = 0;
		}
		else
		{
			if (zip)
				res = create_zip_cache(game_name, pause);
			else
				res = create_raw_cache(game_name, pause);
		}
		if (res && pause)
		{
			printf("complete.\n");
			printf("Please copy \"cache%c%s.cache\" to directory \"/PSP/GAMES/cps2psp/cache\".\n", delimiter, game_name);
		}
		free_memory();
	}

error:
	if (pause)
	{
		printf("Press any key to exit.\n");
		getch();
	}
	return res;
}
