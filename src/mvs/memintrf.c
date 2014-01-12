/******************************************************************************

	memintrf.c

	MVSメモリインタフェース関数

******************************************************************************/

#include "mvs.h"

#define M68K_AMASK 0x00ffffff
#define Z80_AMASK 0x0000ffff

#define READ_BYTE(mem, offset)			mem[offset ^ 1]
#define READ_WORD(mem, offset)			*(u16 *)&mem[offset]
#define WRITE_BYTE(mem, offset, data)	mem[offset ^ 1] = data
#define WRITE_WORD(mem, offset, data)	*(u16 *)&mem[offset] = data

#define READ_MIRROR_BYTE(mem, offset, amask)			mem[(offset & amask) ^ 1]
#define READ_MIRROR_WORD(mem, offset, amask)			*(u16 *)&mem[offset & amask]
#define WRITE_MIRROR_BYTE(mem, offset, data, amask)		mem[(offset & amask) ^ 1] = data
#define WRITE_MIRROR_WORD(mem, offset, data, amask)		*(u16 *)&mem[offset & amask] = data

#define str_cmp(s1, s2)		strnicmp(s1, s2, strlen(s2))

enum
{
	REGION_CPU1 = 0,
	REGION_CPU2,
	REGION_GFX1,
	REGION_GFX2,
	REGION_GFX3,
	REGION_SOUND1,
	REGION_SOUND2,
	REGION_USER1,
	REGION_SKIP
};

#define MAX_CPU1ROM		8
#define MAX_CPU2ROM		8
#define MAX_GFX2ROM		4
#define MAX_GFX3ROM		16
#define MAX_SND1ROM		8
#define MAX_SND2ROM		8
#define MAX_USR1ROM		1


/******************************************************************************
	グローバル変数
******************************************************************************/

u8 *memory_region_cpu1;
u8 *memory_region_cpu2;
u8 *memory_region_gfx1;
u8 *memory_region_gfx2;
u8 *memory_region_gfx3;
u8 *memory_region_sound1;
u8 *memory_region_sound2;
u8 *memory_region_user1;

u32 memory_length_cpu1;
u32 memory_length_cpu2;
u32 memory_length_gfx1;
u32 memory_length_gfx2;
u32 memory_length_gfx3;
u32 memory_length_sound1;
u32 memory_length_sound2;
u32 memory_length_user1;

u8 ALIGN_DATA neogeo_memcard[0x800];
u8 ALIGN_DATA neogeo_ram[0x10000];
u16 ALIGN_DATA neogeo_sram16[0x8000];

int neogeo_machine_mode;


/******************************************************************************
	ローカル構造体/変数
******************************************************************************/

static struct rom_t cpu1rom[MAX_CPU1ROM];
static struct rom_t cpu2rom[MAX_CPU2ROM];
static struct rom_t gfx2rom[MAX_GFX2ROM];
static struct rom_t gfx3rom[MAX_GFX3ROM];
static struct rom_t snd1rom[MAX_SND1ROM];
static struct rom_t snd2rom[MAX_SND2ROM];
static struct rom_t usr1rom[MAX_USR1ROM];

static int num_cpu1rom;
static int num_cpu2rom;
static int num_gfx2rom;
static int num_gfx3rom;
static int num_snd1rom;
static int num_snd2rom;
static int num_usr1rom;

static int encrypt_cpu1;
static int encrypt_snd1;
static int encrypt_gfx;
static int encrypt_usr1;

static u32 bios_amask;

static u8 *neogeo_sram;

int disable_sound;
static int neogeo_save_sound_flag;


/******************************************************************************
	プロトタイプ
******************************************************************************/

static u16 (*neogeo_protection_16_r)(u32 offset, u16 mem_mask);
static void (*neogeo_protection_16_w)(u32 offset, u16 data, u16 mem_mask);


/****************************************************************************
	Decode GFX
****************************************************************************/

/*------------------------------------------------------
	Decode sprite (SPR)
------------------------------------------------------*/

static void neogeo_decode_spr(u8 *mem, u32 length, u8 *usage)
{
	u32 tileno, numtiles = length / 128;

	for (tileno = 0; tileno < numtiles; tileno++)
	{
		u8 swap[128];
		u8 *gfxdata;
		int x,y;
		u32 pen;
		int opaque = 0;

		gfxdata = &mem[128 * tileno];

		memcpy(swap, gfxdata, 128);

		for (y = 0;y < 16;y++)
		{
			u32 dw, data;

			dw = 0;
			for (x = 0;x < 8;x++)
			{
				pen  = ((swap[64 + 4*y + 3] >> x) & 1) << 3;
				pen |= ((swap[64 + 4*y + 1] >> x) & 1) << 2;
				pen |= ((swap[64 + 4*y + 2] >> x) & 1) << 1;
				pen |= ((swap[64 + 4*y + 0] >> x) & 1) << 0;
				opaque += (pen & 0x0f) != 0;
				dw |= pen << 4*x;
			}

			data = ((dw & 0x0000000f) >>  0) | ((dw & 0x000000f0) <<  4)
				 | ((dw & 0x00000f00) <<  8) | ((dw & 0x0000f000) << 12)
				 | ((dw & 0x000f0000) >> 12) | ((dw & 0x00f00000) >>  8)
				 | ((dw & 0x0f000000) >>  4) | ((dw & 0xf0000000) >>  0);

			*(gfxdata++) = data >>  0;
			*(gfxdata++) = data >>  8;
			*(gfxdata++) = data >> 16;
			*(gfxdata++) = data >> 24;

			dw = 0;
			for (x = 0;x < 8;x++)
			{
				pen  = ((swap[4*y + 3] >> x) & 1) << 3;
				pen |= ((swap[4*y + 1] >> x) & 1) << 2;
				pen |= ((swap[4*y + 2] >> x) & 1) << 1;
				pen |= ((swap[4*y + 0] >> x) & 1) << 0;
				opaque += (pen & 0x0f) != 0;
				dw |= pen << 4*x;
			}

			data = ((dw & 0x0000000f) >>  0) | ((dw & 0x000000f0) <<  4)
				 | ((dw & 0x00000f00) <<  8) | ((dw & 0x0000f000) << 12)
				 | ((dw & 0x000f0000) >> 12) | ((dw & 0x00f00000) >>  8)
				 | ((dw & 0x0f000000) >>  4) | ((dw & 0xf0000000) >>  0);

			*(gfxdata++) = data >>  0;
			*(gfxdata++) = data >>  8;
			*(gfxdata++) = data >> 16;
			*(gfxdata++) = data >> 24;
		}

		if (opaque)
			*usage = (opaque == 256) ? 1 : 2;
		else
			*usage = 0;
		usage++;
	}
}

/*------------------------------------------------------
	Decode fixed sprite (FIX)
------------------------------------------------------*/

#define decode_fix(n)				\
{									\
	tile = buf[n];					\
	*p++ = tile;					\
	opaque += (tile & 0x0f) != 0;	\
	opaque += (tile >> 4) != 0;		\
}

static void neogeo_decode_fix(u8 *mem, u32 length, u8 *usage)
{
	u32 i, j;
	u8 tile, opaque;
	u8 *p, buf[32];
	u32 *gfx = (u32 *)mem;

	for (i = 0; i < length; i += 32)
	{
		opaque  = 0;

		memcpy(buf, &mem[i], 32);
		p = &mem[i];

		for (j = 0; j < 8; j++)
		{
			decode_fix(j + 16);
			decode_fix(j + 24);
			decode_fix(j +  0);
			decode_fix(j +  8);
		}

		if (opaque)
			*usage = (opaque == 64) ? 1 : 2;
		else
			*usage = 0;
		*usage++;
	}

	for (i = 0; i < length/4; i++)
	{
		u32 dw = gfx[i];
		u32 data = ((dw & 0x0000000f) >>  0) | ((dw & 0x000000f0) <<  4)
				 | ((dw & 0x00000f00) <<  8) | ((dw & 0x0000f000) << 12)
				 | ((dw & 0x000f0000) >> 12) | ((dw & 0x00f00000) >>  8)
				 | ((dw & 0x0f000000) >>  4) | ((dw & 0xf0000000) >>  0);
		gfx[i] = data;
	}
}


/******************************************************************************
	ROM読み込み
******************************************************************************/

/*--------------------------------------------------------
	CPU1 (M68000 program ROM)
--------------------------------------------------------*/

static int load_rom_cpu1(void)
{
	if ((memory_region_cpu1 = memalign(MEM_ALIGN, memory_length_cpu1)) == NULL)
	{
		error_memory("REGION_CPU1");
		return 0;
	}
	memset(memory_region_cpu1, 0, memory_length_cpu1);

	if (encrypt_cpu1)
	{
		msg_printf(TEXT(LOADING_DECRYPTED_CPU1_ROM));
		if (cachefile_open("prom") == -1)
		{
			error_file("CPU1");
			return 0;
		}
		file_read(memory_region_cpu1, memory_length_cpu1);
		file_close();
	}
	else
	{
		int i;
		char fname[32], *parent;

		parent = strlen(parent_name) ? parent_name : NULL;

		for (i = 0; i < num_cpu1rom; )
		{
			if (file_open(game_name, parent, cpu1rom[i].crc, fname) == -1)
			{
				error_crc("CPU1");
				return 0;
			}

			msg_printf(TEXT(LOADING), fname);

			i = rom_load(cpu1rom, memory_region_cpu1, i, num_cpu1rom);

			file_close();
		}
	}

	neogeo_ngh = m68000_read_memory_16(0x0108);

	return 1;
}

/*--------------------------------------------------------
	CPU2 (Z80 program ROM)
--------------------------------------------------------*/

static int load_rom_cpu2(void)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_cpu2 = memalign(MEM_ALIGN, memory_length_cpu2)) == NULL)
	{
		error_memory("REGION_CPU2");
		return 0;
	}
	memset(memory_region_cpu2, 0, memory_length_cpu2);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_cpu2rom; )
	{
		if (file_open(game_name, parent, cpu2rom[i].crc, fname) == -1)
		{
			error_crc("CPU2");
			return 0;
		}

		msg_printf(TEXT(LOADING), fname);

		i = rom_load(cpu2rom, memory_region_cpu2, i, num_cpu2rom);

		file_close();
	}

	memcpy(memory_region_cpu2, &memory_region_cpu2[0x10000], 0x10000);

	return 1;
}

/*--------------------------------------------------------
	GFX1 (System FIX sprite ROM)
--------------------------------------------------------*/

static int load_rom_gfx1(void)
{
	char fname[32];

	if ((memory_region_gfx1 = memalign(MEM_ALIGN, memory_length_gfx1)) == NULL)
	{
		error_memory("REGION_GFX1");
		return 0;
	}
	memset(memory_region_gfx1, 0, memory_length_gfx1);

	if ((gfx_pen_usage[0] = memalign(MEM_ALIGN, memory_length_gfx1 / 32)) == NULL)
	{
		error_memory("GFX_PEN_USAGE (sfix)");
		return 0;
	}

	msg_printf(TEXT(LOADING_SFIX));
	if (file_open(game_name, "neogeo", sfix_crc, fname) == -1)
	{
		error_crc("sfix.sfx");
		return 0;
	}
	file_read(memory_region_gfx1, memory_length_gfx1);
	file_close();

	neogeo_decode_fix(memory_region_gfx1, memory_length_gfx1, gfx_pen_usage[0]);

	return 1;
}

/*--------------------------------------------------------
	GFX2 (FIX sprite ROM)
--------------------------------------------------------*/

static int load_rom_gfx2(void)
{
	if ((memory_region_gfx2 = memalign(MEM_ALIGN, memory_length_gfx2)) == NULL)
	{
		error_memory("REGION_GFX2");
		return 0;
	}
	memset(memory_region_gfx2, 0, memory_length_gfx2);

	if ((gfx_pen_usage[1] = memalign(MEM_ALIGN, memory_length_gfx2 / 32)) == NULL)
	{
		error_memory("GFX_PEN_USAGE (fix)");
		return 0;
	}

	if (encrypt_gfx)
	{
		msg_printf(TEXT(LOADING_DECRYPTED_GFX2_ROM));
		if (cachefile_open("srom") == -1)
		{
			error_file("GFX2");
			return 0;
		}
		file_read(memory_region_gfx2, memory_length_gfx2);
		file_close();
	}
	else
	{
		int i;
		char fname[32], *parent;

		parent = strlen(parent_name) ? parent_name : NULL;

		for (i = 0; i < num_gfx2rom; )
		{
			if (file_open(game_name, parent, gfx2rom[i].crc, fname) == -1)
			{
				error_crc("GFX2");
				return 0;
			}

			msg_printf(TEXT(LOADING), fname);

			i = rom_load(gfx2rom, memory_region_gfx2, i, num_gfx2rom);

			file_close();
		}
	}

	neogeo_decode_fix(memory_region_gfx2, memory_length_gfx2, gfx_pen_usage[1]);

	return 1;
}

/*--------------------------------------------------------
	GFX3 (OBJ sprite ROM)
--------------------------------------------------------*/

static int load_rom_gfx3(void)
{
	if ((gfx_pen_usage[2] = memalign(MEM_ALIGN, memory_length_gfx3 / 128)) == NULL)
	{
		error_memory("GFX_PEN_USAGE (spr)");
		return 0;
	}

	if (encrypt_gfx)
	{
		msg_printf(TEXT(LOADING_DECRYPTED_GFX3_ROM));
	}
	else
	{
		if ((memory_region_gfx3 = memalign(MEM_ALIGN, memory_length_gfx3)) != NULL)
		{
			int i;
			char fname[32], *parent;

			memset(memory_region_gfx3, 0, memory_length_gfx3);

			parent = strlen(parent_name) ? parent_name : NULL;

			for (i = 0; i < num_gfx3rom; )
			{
				if (file_open(game_name, parent, gfx3rom[i].crc, fname) == -1)
				{
					error_crc("GFX3");
					return 0;
				}

				msg_printf(TEXT(LOADING), fname);

				i = rom_load(gfx3rom, memory_region_gfx3, i, num_gfx3rom);

				file_close();
			}

			neogeo_decode_spr(memory_region_gfx3, memory_length_gfx3, gfx_pen_usage[2]);
		}
		else
		{
			msg_printf(TEXT(COULD_NOT_ALLOCATE_MEMORY_FOR_SPRITE_DATA));
			msg_printf(TEXT(TRY_TO_USE_SPRITE_CACHE));
		}
	}

	if (memory_region_gfx3 == NULL)
	{
		if (cache_start() == 0)
		{
			msg_printf(TEXT(PRESS_ANY_BUTTON2));
			pad_wait_press(PAD_WAIT_INFINITY);
			Loop = LOOP_BROWSER;
			return 0;
		}
	}

	return 1;
}

/*--------------------------------------------------------
	SOUND1 (YM2610 ADPCM-A ROM)
--------------------------------------------------------*/

static int load_rom_sound1(void)
{
	if (!option_sound_enable)
	{
		memory_length_sound1 = 0;
		return 1;
	}
	if (disable_sound)
	{
		return 1;
	}

	if ((memory_region_sound1 = memalign(MEM_ALIGN, memory_length_sound1)) == NULL)
	{
		error_memory("REGION_SOUND1");
		return 0;
	}
	memset(memory_region_sound1, 0, memory_length_sound1);

	if (encrypt_snd1)
	{
		msg_printf(TEXT(LOADING_DECRYPTED_SOUND1_ROM));
		if (cachefile_open("vrom") == -1)
		{
			error_file("SOUND1");
			return 0;
		}
		file_read(memory_region_sound1, memory_length_sound1);
		file_close();
	}
	else
	{
		int i;
		char fname[32], *parent;

		parent = strlen(parent_name) ? parent_name : NULL;

		for (i = 0; i < num_snd1rom; )
		{
			if (file_open(game_name, parent, snd1rom[i].crc, fname) == -1)
			{
				error_crc("SOUND1");
				return 0;
			}

			msg_printf(TEXT(LOADING), fname);

			i = rom_load(snd1rom, memory_region_sound1, i, num_snd1rom);

			file_close();
		}
	}

	return 1;
}

/*--------------------------------------------------------
	SOUND2 (YM2610 Delta-T PCM ROM)
--------------------------------------------------------*/

static int load_rom_sound2(void)
{
	int i;
	char fname[32], *parent;

	if (memory_length_sound2 == 0 || !option_sound_enable || disable_sound)
	{
		memory_length_sound2 = 0;
		return 1;
	}

	if ((memory_region_sound2 = memalign(MEM_ALIGN, memory_length_sound2)) == NULL)
	{
		error_memory("REGION_SOUND2");
		return 0;
	}
	memset(memory_region_sound2, 0, memory_length_sound2);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_snd2rom; )
	{
		if (file_open(game_name, parent, snd2rom[i].crc, fname) == -1)
		{
			error_crc("SOUND2");
			return 0;
		}

		msg_printf(TEXT(LOADING), fname);

		i = rom_load(snd2rom, memory_region_sound2, i, num_snd2rom);

		file_close();
	}

	return 1;
}

/*--------------------------------------------------------
	USER1 (M68000 BIOS ROM)
--------------------------------------------------------*/

static int load_rom_user1(int reload)
{
	char fname[32];
	char *parent;
	u32 patch = 0;

	if (!reload)
	{
		if ((memory_region_user1 = memalign(MEM_ALIGN, memory_length_user1)) == NULL)
		{
			error_memory("REGION_USER1");
			return 0;
		}
		memset(memory_region_user1, 0, memory_length_user1);
	}

	parent = strlen(parent_name) ? parent_name : (char *)"neogeo";

	if (!num_usr1rom)
	{
		patch = bios_patch_address[neogeo_bios];

		if (file_open(game_name, "neogeo", bios_crc[neogeo_bios], fname) == -1)
		{
			error_crc(bios_name[neogeo_bios]);
			neogeo_bios = -1;
			return 0;
		}
		if (!reload) msg_printf(TEXT(LOADING_BIOS), fname, bios_name[neogeo_bios]);
		file_read(memory_region_user1, memory_length_user1);
		file_close();
	}
	else if (!reload)
	{
		if (encrypt_usr1)
		{
			// JAMMA PCB
			msg_printf(TEXT(LOADING_DECRYPTED_BIOS_ROM));
			if (cachefile_open("biosrom") == -1)
			{
				error_file("BIOS (biosrom)");
				return 0;
			}
		}
		else
		{
			// irrmaze
			if (!strcmp(game_name, "irrmaze")) patch = 0x010d8c;

			if (file_open(game_name, parent, usr1rom[0].crc, fname) == -1)
			{
				sprintf(fname, "%s BIOS", game_name);
				error_crc(fname);
				return 0;
			}
			msg_printf(TEXT(LOADING), fname);
		}
		file_read(memory_region_user1, memory_length_user1);
		file_close();
	}

	bios_amask = memory_length_user1 - 1;

	if (patch)
	{
		u16 *mem16 = (u16 *)memory_region_user1;
		u16 value;

		if (!neogeo_region)
			value = mem16[0x00400 >> 1] & 0x03;
		else
			value = neogeo_region - 1;

		if (!neogeo_machine_mode)
			value |= mem16[0x00400 >> 1] & 0x8000;
		else
			value |= (neogeo_machine_mode - 1) ? 0x8000 : 0;

		mem16[0x00400 >> 1] = value;
		mem16[(patch + 0) >> 1] = 0x4e71;
		mem16[(patch + 2) >> 1] = 0x4e71;
	}

	return 1;
}

/*--------------------------------------------------------
	ROM情報をデータベースで解析
--------------------------------------------------------*/

static int load_rom_info(const char *game_name)
{
	FILE *fp;
	char path[MAX_PATH];
	char buf[256];
	int rom_start = 0;
	int region = 0;

	num_cpu1rom = 0;
	num_cpu2rom = 0;
	num_gfx2rom = 0;
	num_gfx3rom = 0;
	num_snd1rom = 0;
	num_snd2rom = 0;
	num_usr1rom = 0;

	encrypt_cpu1 = 0;
	encrypt_snd1 = 0;
	encrypt_gfx  = 0;
	encrypt_usr1 = 0;

	disable_sound = 0;

	sprintf(path, "%srominfo.mvs", launchDir);

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
					// 改行
					continue;
				}
				else if (str_cmp(buf, "FILENAME(") == 0)
				{
					char *name, *parent;
					char *machine, *input, *init, *rotate;

					strtok(buf, " ");
					name    = strtok(NULL, " ,");
					parent  = strtok(NULL, " ,");
					machine = strtok(NULL, " ,");
					input   = strtok(NULL, " ,");
					init    = strtok(NULL, " ,");
					rotate  = strtok(NULL, " ");

					if (stricmp(name, game_name) == 0)
					{
						if (str_cmp(parent, "neogeo") == 0)
						{
							parent_name[0] = '\0';
						}
						else if (str_cmp(parent, "pcb") == 0)
						{
							parent_name[0] = '\0';
						}
						else
						{
							strcpy(parent_name, parent);
						}

						sscanf(machine, "%d", &machine_driver_type);
						sscanf(input, "%d", &machine_input_type);
						sscanf(init, "%d", &machine_init_type);
						sscanf(rotate, "%d", &machine_screen_type);
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
					char *size, *type, *flag;
					int encrypted = 0;

					strtok(&buf[1], " ");
					size = strtok(NULL, " ,");
					type = strtok(NULL, " ,");
					flag = strtok(NULL, " ");

					if (strstr(flag, "SOUND_DISABLE")) disable_sound = 1;
					if (strstr(flag, "ENCRYPTED")) encrypted = 1;

					if (strcmp(type, "CPU1") == 0)
					{
						sscanf(size, "%x", &memory_length_cpu1);
						encrypt_cpu1 = encrypted;
						region = REGION_CPU1;
					}
					else if (strcmp(type, "CPU2") == 0)
					{
						sscanf(size, "%x", &memory_length_cpu2);
						region = REGION_CPU2;
					}
					else if (strcmp(type, "GFX2") == 0)
					{
						sscanf(size, "%x", &memory_length_gfx2);
						region = REGION_GFX2;
					}
					else if (strcmp(type, "GFX3") == 0)
					{
						sscanf(size, "%x", &memory_length_gfx3);
						encrypt_gfx = encrypted;
						region = REGION_GFX3;
					}
					else if (strcmp(type, "SOUND1") == 0)
					{
						sscanf(size, "%x", &memory_length_sound1);
						encrypt_snd1 = encrypted;
						region = REGION_SOUND1;
					}
					else if (strcmp(type, "SOUND2") == 0)
					{
						sscanf(size, "%x", &memory_length_sound2);
						region = REGION_SOUND2;
					}
					else if (strcmp(type, "USER1") == 0)
					{
						sscanf(size, "%x", &memory_length_user1);
						encrypt_usr1 = encrypted;
						region = REGION_USER1;
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
					case REGION_CPU1:
						sscanf(type, "%x", &cpu1rom[num_cpu1rom].type);
						sscanf(offset, "%x", &cpu1rom[num_cpu1rom].offset);
						sscanf(length, "%x", &cpu1rom[num_cpu1rom].length);
						sscanf(crc, "%x", &cpu1rom[num_cpu1rom].crc);
						cpu1rom[num_cpu1rom].group = 0;
						cpu1rom[num_cpu1rom].skip = 0;
						num_cpu1rom++;
						break;

					case REGION_CPU2:
						sscanf(type, "%x", &cpu2rom[num_cpu2rom].type);
						sscanf(offset, "%x", &cpu2rom[num_cpu2rom].offset);
						sscanf(length, "%x", &cpu2rom[num_cpu2rom].length);
						sscanf(crc, "%x", &cpu2rom[num_cpu2rom].crc);
						cpu2rom[num_cpu2rom].group = 0;
						cpu2rom[num_cpu2rom].skip = 0;
						num_cpu2rom++;
						break;

					case REGION_GFX2:
						sscanf(type, "%x", &gfx2rom[num_gfx2rom].type);
						sscanf(offset, "%x", &gfx2rom[num_gfx2rom].offset);
						sscanf(length, "%x", &gfx2rom[num_gfx2rom].length);
						sscanf(crc, "%x", &gfx2rom[num_gfx2rom].crc);
						gfx2rom[num_gfx2rom].group = 0;
						gfx2rom[num_gfx2rom].skip = 0;
						num_gfx2rom++;
						break;

					case REGION_GFX3:
						sscanf(type, "%x", &gfx3rom[num_gfx3rom].type);
						sscanf(offset, "%x", &gfx3rom[num_gfx3rom].offset);
						sscanf(length, "%x", &gfx3rom[num_gfx3rom].length);
						sscanf(crc, "%x", &gfx3rom[num_gfx3rom].crc);
						gfx3rom[num_gfx3rom].group = 0;
						gfx3rom[num_gfx3rom].skip = 0;
						num_gfx3rom++;
						break;

					case REGION_SOUND1:
						sscanf(type, "%x", &snd1rom[num_snd1rom].type);
						sscanf(offset, "%x", &snd1rom[num_snd1rom].offset);
						sscanf(length, "%x", &snd1rom[num_snd1rom].length);
						sscanf(crc, "%x", &snd1rom[num_snd1rom].crc);
						snd1rom[num_snd1rom].group = 0;
						snd1rom[num_snd1rom].skip = 0;
						num_snd1rom++;
						break;

					case REGION_SOUND2:
						sscanf(type, "%x", &snd2rom[num_snd2rom].type);
						sscanf(offset, "%x", &snd2rom[num_snd2rom].offset);
						sscanf(length, "%x", &snd2rom[num_snd2rom].length);
						sscanf(crc, "%x", &snd2rom[num_snd2rom].crc);
						snd2rom[num_snd2rom].group = 0;
						snd2rom[num_snd2rom].skip = 0;
						num_snd2rom++;
						break;

					case REGION_USER1:
						sscanf(type, "%x", &usr1rom[num_usr1rom].type);
						sscanf(offset, "%x", &usr1rom[num_usr1rom].offset);
						sscanf(length, "%x", &usr1rom[num_usr1rom].length);
						sscanf(crc, "%x", &usr1rom[num_usr1rom].crc);
						usr1rom[num_usr1rom].group = 0;
						usr1rom[num_usr1rom].skip = 0;
						num_usr1rom++;
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
					case REGION_CPU1:
						sscanf(type, "%x", &cpu1rom[num_cpu1rom].type);
						sscanf(offset, "%x", &cpu1rom[num_cpu1rom].offset);
						sscanf(length, "%x", &cpu1rom[num_cpu1rom].length);
						sscanf(crc, "%x", &cpu1rom[num_cpu1rom].crc);
						sscanf(group, "%x", &cpu1rom[num_cpu1rom].group);
						sscanf(skip, "%x", &cpu1rom[num_cpu1rom].skip);
						num_cpu1rom++;
						break;

					case REGION_GFX3:
						sscanf(type, "%x", &gfx3rom[num_gfx3rom].type);
						sscanf(offset, "%x", &gfx3rom[num_gfx3rom].offset);
						sscanf(length, "%x", &gfx3rom[num_gfx3rom].length);
						sscanf(crc, "%x", &gfx3rom[num_gfx3rom].crc);
						sscanf(group, "%x", &gfx3rom[num_gfx3rom].group);
						sscanf(skip, "%x", &gfx3rom[num_gfx3rom].skip);
						num_gfx3rom++;
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


/******************************************************************************
	メモリインタフェース関数
******************************************************************************/

/*------------------------------------------------------
	メモリインタフェース初期化
-----------------------------------------------------*/

int memory_init(void)
{
	int res;

	memory_region_cpu1   = NULL;
	memory_region_cpu2   = NULL;
	memory_region_gfx1   = NULL;
	memory_region_gfx2   = NULL;
	memory_region_gfx3   = NULL;
	memory_region_sound1 = NULL;
	memory_region_sound2 = NULL;
	memory_region_user1  = NULL;

	memory_length_cpu1   = 0;
	memory_length_cpu2   = 0;
	memory_length_gfx1   = 0x20000;
	memory_length_gfx2   = 0;
	memory_length_gfx3   = 0;
	memory_length_sound1 = 0;
	memory_length_sound2 = 0;
	memory_length_user1  = 0x20000;

	gfx_pen_usage[0] = NULL;
	gfx_pen_usage[1] = NULL;
	gfx_pen_usage[2] = NULL;

	cache_init();
	pad_wait_clear();
	video_clear_screen();
	msg_screen_init(WP_LOGO, ICON_SYSTEM, TEXT(LOAD_ROM));

	load_gamecfg(game_name);

	memset(neogeo_memcard, 0, 0x800);
	memset(neogeo_ram, 0, 0x10000);
	memset(neogeo_sram16, 0, 0x10000);

#ifdef ADHOC
	if (adhoc_enable)
	{
		/* AdHoc通信時は一部オプションで固定の設定を使用 */
		neogeo_raster_enable = 0;
		option_vsync         = 0;
		option_autoframeskip = 0;
		option_frameskip     = 0;
		option_showfps       = 0;
		option_speedlimit    = 1;
		option_sound_enable  = 1;
		option_samplerate    = 1;
		neogeo_dipswitch     = 0xff;

		if (adhoc_server)
			option_controller = INPUT_PLAYER1;
		else
			option_controller = INPUT_PLAYER2;
	}
#endif

	msg_printf(TEXT(CHECKING_BIOS));

	res = neogeo_bios;
	if (res != -1)
	{
		res = file_open("neogeo", NULL, bios_crc[neogeo_bios], NULL);
		file_close();
	}
	if (res == -1)
	{
		pad_wait_clear();
		video_clear_screen();
		bios_select(1);
		if (neogeo_bios == -1)
		{
			Loop = LOOP_BROWSER;
			return 0;
		}

		pad_wait_clear();
		video_clear_screen();
		msg_screen_init(WP_LOGO, ICON_SYSTEM, TEXT(LOAD_ROM));
		msg_printf(TEXT(CHECKING_BIOS));
		msg_printf(TEXT(ALL_NVRAM_FILES_ARE_REMOVED));
	}

	msg_printf(TEXT(CHECKING_ROM_INFO));

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: msg_printf(TEXT(THIS_GAME_NOT_SUPPORTED)); break;
		case 2: msg_printf(TEXT(ROM_NOT_FOUND)); break;
		case 3: msg_printf(TEXT(ROMINFO_NOT_FOUND)); break;
		}
		msg_printf(TEXT(PRESS_ANY_BUTTON2));
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (parent_name[0])
		msg_printf(TEXT(ROMSET_x_PARENT_x), game_name, parent_name);
	else
		msg_printf(TEXT(ROMSET_x), game_name);

#ifdef ADHOC
	if (!adhoc_enable)
#endif
	{
#ifdef COMMAND_LIST
		if (parent_name[0])
			load_commandlist(game_name, parent_name);
		else
			load_commandlist(game_name, NULL);
#endif
	}

	// Brezzasoft用の設定 - メモリを確保するので先に処理
	if (machine_init_type == INIT_vliner || machine_init_type == INIT_jockeygp)
	{
		msg_printf(TEXT(ALLOCATE_BRZA_SRAM_MEMORY));
		if ((brza_sram = (u16 *)malloc(0x2000)) == NULL)
		{
			msg_printf(TEXT(COULD_NOT_ALLOCATE_BRZA_SRAM_MEMORY));
			msg_printf(TEXT(PRESS_ANY_BUTTON2));
			pad_wait_press(PAD_WAIT_INFINITY);
			Loop = LOOP_BROWSER;
			return 0;
		}
		memset(brza_sram, 0, 0x2000);
	}
	else brza_sram = NULL;

	if (load_rom_user1(0) == 0) return 0;
	if (load_rom_cpu1() == 0) return 0;
	if (load_rom_cpu2() == 0) return 0;
	if (load_rom_sound1() == 0) return 0;
	if (load_rom_sound2() == 0) return 0;
	if (load_rom_gfx1() == 0) return 0;
	if (load_rom_gfx2() == 0) return 0;
	if (load_rom_gfx3() == 0) return 0;

	if (disable_sound)
	{
		msg_printf(TEXT(THIS_GAME_ONLY_WORK_WITHOUT_SOUND));
		neogeo_save_sound_flag = option_sound_enable;
		option_sound_enable = 0;
	}
	else
	{
		neogeo_save_sound_flag = 0;
	}

	// FIXバンクタイプ設定
	switch (machine_init_type)
	{
	case INIT_garou:
	case INIT_garouo:
	case INIT_mslug4:
	case INIT_mslug3:
	case INIT_mslug3n:
	case INIT_samsho5:
	case INIT_samsh5sp:
		neogeo_fix_bank_type = 1;
		break;

	case INIT_kof2000:
	case INIT_kof2000n:
	case INIT_matrim:
	case INIT_svcpcb:
	case INIT_svchaosa:
	case INIT_kf2k3pcb:
	case INIT_kof2003:
		neogeo_fix_bank_type = 2;
		break;

	default:
		neogeo_fix_bank_type = 0;
		break;
	}

	switch (machine_init_type)
	{
	case INIT_fatfury2:
		neogeo_protection_16_r = fatfury2_protection_16_r;
		neogeo_protection_16_w = fatfury2_protection_16_w;
		break;

	case INIT_kof98:
		neogeo_protection_16_r = neogeo_secondbank_16_r;
		neogeo_protection_16_w = kof98_protection_16_w;
		break;

	case INIT_mslugx:
		mslugx_install_protection();
		neogeo_protection_16_r = neogeo_secondbank_16_r;
		neogeo_protection_16_w = neogeo_secondbank_16_w;
		break;

	case INIT_kof99:
		neogeo_protection_16_r = kof99_protection_16_r;
		neogeo_protection_16_w = kof99_protection_16_w;
		break;

	case INIT_garou:
		neogeo_protection_16_r = garou_protection_16_r;
		neogeo_protection_16_w = garou_protection_16_w;
		break;

	case INIT_garouo:
		neogeo_protection_16_r = garou_protection_16_r;
		neogeo_protection_16_w = garouo_protection_16_w;
		break;

	case INIT_mslug3:
		neogeo_protection_16_r = mslug3_protection_16_r;
		neogeo_protection_16_w = mslug3_protection_16_w;
		break;

	case INIT_kof2000:
		neogeo_protection_16_r = kof2000_protection_16_r;
		neogeo_protection_16_w = kof2000_protection_16_w;
		break;

	case INIT_mslug5:
	case INIT_ms5pcb:
	case INIT_svcpcb:
	case INIT_svchaosa:
	case INIT_kof2003:
	case INIT_kf2k3pcb:
		neogeo_protection_16_r = pvc_protection_16_r;
		neogeo_protection_16_w = pvc_protection_16_w;
		break;

	case INIT_vliner:
		neogeo_protection_16_r = vliner_16_r;
		neogeo_protection_16_w = brza_sram_16_w;
		neogeo_ngh = NGH_vliner;
		break;

	case INIT_jockeygp:
		neogeo_protection_16_r = brza_sram_16_r;
		neogeo_protection_16_w = brza_sram_16_w;
		neogeo_ngh = NGH_jockeygp;
		break;

	default:
		neogeo_protection_16_r = neogeo_secondbank_16_r;
		neogeo_protection_16_w = neogeo_secondbank_16_w;
		break;
	}

	neogeo_sram = (u8 *)neogeo_sram16;

	return 1;
}


/*------------------------------------------------------
	メモリインタフェース終了
------------------------------------------------------*/

void memory_shutdown(void)
{
	cache_shutdown();

	if (brza_sram) free(brza_sram);

	if (gfx_pen_usage[0]) free(gfx_pen_usage[0]);
	if (gfx_pen_usage[1]) free(gfx_pen_usage[1]);
	if (gfx_pen_usage[2]) free(gfx_pen_usage[2]);

	if (memory_region_cpu1)   free(memory_region_cpu1);
	if (memory_region_cpu2)   free(memory_region_cpu2);
	if (memory_region_gfx1)   free(memory_region_gfx1);
	if (memory_region_gfx2)   free(memory_region_gfx2);
	if (memory_region_gfx3)   free(memory_region_gfx3);
	if (memory_region_sound1) free(memory_region_sound1);
	if (memory_region_sound2) free(memory_region_sound2);
	if (memory_region_user1)  free(memory_region_user1);

	if (neogeo_save_sound_flag) option_sound_enable = 1;

#if PSP_VIDEO_32BPP
	GFX_MEMORY = NULL;
#endif
}


/******************************************************************************
	M68000 メモリリード/ライト関数
******************************************************************************/

/*------------------------------------------------------
	M68000メモリリード (byte)
------------------------------------------------------*/

u8 m68000_read_memory_8(u32 offset)
{
	int shift = (~offset & 1) << 3;
	u16 mem_mask = ~(0xff << shift);

	offset &= M68K_AMASK;

	switch (offset >> 20)
	{
	case 0x0: return READ_BYTE(memory_region_cpu1, offset);
	case 0x1: return READ_MIRROR_BYTE(neogeo_ram, offset, 0x00ffff);
	case 0xc: return READ_MIRROR_BYTE(memory_region_user1, offset, bios_amask);
	case 0xd: return READ_MIRROR_BYTE(neogeo_sram, offset, 0x00ffff);

	case 0x2: return (*neogeo_protection_16_r)(offset >> 1, mem_mask) >> shift;
	case 0x4: return neogeo_paletteram16_r(offset >> 1, mem_mask) >> shift;
	case 0x8: return neogeo_memcard16_r(offset >> 1, mem_mask) >> shift;

	case 0x3:
		switch (offset >> 16)
		{
		case 0x30: return neogeo_controller1and4_16_r(offset >> 1, mem_mask) >> shift;
		case 0x32: return neogeo_timer16_r(offset >> 1, mem_mask) >> shift;
		case 0x34: return neogeo_controller2_16_r(offset >> 1, mem_mask) >> shift;
		case 0x38: return neogeo_controller3_16_r(offset >> 1, mem_mask) >> shift;
		case 0x3c: return neogeo_video_16_r(offset >> 1, mem_mask) >> shift;
		}
		break;
	}
	return 0xff;
}


/*------------------------------------------------------
	M68000リードメモリ (word)
------------------------------------------------------*/

u16 m68000_read_memory_16(u32 offset)
{
	offset &= M68K_AMASK;

	switch (offset >> 20)
	{
	case 0x0: return READ_WORD(memory_region_cpu1, offset);
	case 0x1: return READ_MIRROR_WORD(neogeo_ram, offset, 0x00ffff);
	case 0xc: return READ_MIRROR_WORD(memory_region_user1, offset, bios_amask);
	case 0xd: return READ_MIRROR_WORD(neogeo_sram, offset, 0x00ffff);

	case 0x2: return (*neogeo_protection_16_r)(offset >> 1, 0);
	case 0x4: return neogeo_paletteram16_r(offset >> 1, 0);
	case 0x8: return neogeo_memcard16_r(offset >> 1, 0);

	case 0x3:
		switch (offset >> 16)
		{
		case 0x30: return neogeo_controller1and4_16_r(offset >> 1, 0);
		case 0x32: return neogeo_timer16_r(offset >> 1, 0);
		case 0x34: return neogeo_controller2_16_r(offset >> 1, 0);
		case 0x38: return neogeo_controller3_16_r(offset >> 1, 0);
		case 0x3c: return neogeo_video_16_r(offset >> 1, 0);
		}
		break;
	}
	return 0xffff;
}


/*------------------------------------------------------
	M68000ライトメモリ (byte)
------------------------------------------------------*/

void m68000_write_memory_8(u32 offset, u8 data)
{
	int shift = (~offset & 1) << 3;
	u16 mem_mask = ~(0xff << shift);

	offset &= M68K_AMASK;

	switch (offset >> 20)
	{
	case 0x1: WRITE_MIRROR_BYTE(neogeo_ram, offset, data, 0x00ffff); return;

	case 0x2: (*neogeo_protection_16_w)(offset >> 1, data << shift, mem_mask); return;
	case 0x4: neogeo_paletteram16_w(offset >> 1, data << shift, mem_mask); return;
	case 0x8: neogeo_memcard16_w(offset >> 1, data << shift, mem_mask); return;
	case 0xd: neogeo_sram16_w(offset >> 1, data << shift, mem_mask); return;

	case 0x3:
		switch (offset >> 16)
		{
		case 0x30: watchdog_reset_16_w(offset >> 1, data << shift, mem_mask); return;
		case 0x32: neogeo_z80_w(offset >> 1, data << shift, mem_mask); return;
		case 0x38: neogeo_syscontrol1_16_w(offset >> 1, data << shift, mem_mask); return;
		case 0x3a: neogeo_syscontrol2_16_w(offset >> 1, data << shift, mem_mask); return;
		case 0x3c: neogeo_video_16_w(offset >> 1, data << shift, mem_mask); return;
		}
		break;
	}
}


/*------------------------------------------------------
	M68000ライトメモリ (word)
------------------------------------------------------*/

void m68000_write_memory_16(u32 offset, u16 data)
{
	offset &= M68K_AMASK;

	switch (offset >> 20)
	{
	case 0x1: WRITE_MIRROR_WORD(neogeo_ram, offset, data, 0x00ffff); return;

	case 0x2: (*neogeo_protection_16_w)(offset >> 1, data, 0); return;
	case 0x4: neogeo_paletteram16_w(offset >> 1, data, 0); return;
	case 0x8: neogeo_memcard16_w(offset >> 1, data, 0); return;
	case 0xd: neogeo_sram16_w(offset >> 1, data, 0); return;

	case 0x3:
		switch (offset >> 16)
		{
		case 0x30: watchdog_reset_16_w(offset >> 1, data , 0); return;
		case 0x32: neogeo_z80_w(offset >> 1, data, 0); return;
		case 0x38: neogeo_syscontrol1_16_w(offset >> 1, data, 0); return;
		case 0x3a: neogeo_syscontrol2_16_w(offset >> 1, data, 0); return;
		case 0x3c: neogeo_video_16_w(offset >> 1, data, 0); return;
		}
		break;
	}
}


/******************************************************************************
	Z80 メモリリード/ライト関数
******************************************************************************/

/*------------------------------------------------------
	Z80リードメモリ (byte)
------------------------------------------------------*/

u8 z80_read_memory_8(u32 offset)
{
	return memory_region_cpu2[offset & Z80_AMASK];
}


/*------------------------------------------------------
	Z80ライトメモリ (byte)
------------------------------------------------------*/

void z80_write_memory_8(u32 offset, u8 data)
{
	offset &= Z80_AMASK;

	if (offset >= 0xf800)
		memory_region_cpu2[offset] = data;
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( memory )
{
	state_save_long(&neogeo_bios, 1);
	state_save_long(&neogeo_region, 1);
	state_save_long(&neogeo_machine_mode, 1);

	state_save_byte(neogeo_ram, 0x10000);
	state_save_byte(&memory_region_cpu2[0xf800], 0x800);
	state_save_byte(neogeo_sram16, 0x10000);

	if (brza_sram)
		state_save_byte(brza_sram, 0x2000);
}

STATE_LOAD( memory )
{
	int bios, region, mode;

	state_load_long(&bios, 1);
	state_load_long(&region, 1);
	state_load_long(&mode, 1);

	state_load_byte(neogeo_ram, 0x10000);
	state_load_byte(&memory_region_cpu2[0xf800], 0x800);
	state_load_byte(neogeo_sram16, 0x10000);

	if (brza_sram)
		state_load_byte(brza_sram, 0x2000);

	if (machine_init_type != INIT_ms5pcb
	&&	machine_init_type != INIT_svcpcb
	&&	machine_init_type != INIT_kf2k3pcb)
	{
		if (neogeo_bios != bios
		||	neogeo_region != region
		||	neogeo_machine_mode != mode)
		{
			neogeo_bios = bios;
			neogeo_region = region;
			neogeo_machine_mode = mode;
			state_reload_bios = 1;
		}
	}
}


int reload_bios(void)
{
	return load_rom_user1(1);
}

#endif /* SAVE_STATE */
