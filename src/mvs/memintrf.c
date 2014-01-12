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

static int disable_sound;
static int neogeo_save_sound_flag;


/******************************************************************************
	プロトタイプ
******************************************************************************/

static u16 (*neogeo_protection_16_r)(u32 offset, u16 mem_mask);
static void (*neogeo_protection_16_w)(u32 offset, u16 data, u16 mem_mask);


/******************************************************************************
	エラーメッセージ表示
******************************************************************************/

/*------------------------------------------------------
	メモリ確保エラーメッセージ表示
------------------------------------------------------*/

static void error_memory(const char *mem_name)
{
	zip_close();
	msg_printf("ERROR: Could not allocate %s memory.\n", mem_name);
	msg_printf("Press any button.\n");
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	ファイルオープンエラーメッセージ表示
------------------------------------------------------*/

static void error_rom(const char *rom_name)
{
	zip_close();
	msg_printf("ERROR: CRC32 not correct. \"%s\"\n", rom_name);
	msg_printf("Press any button.\n");
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	ROMファイルエラーメッセージ表示
------------------------------------------------------*/

static void error_file(const char *rom_name)
{
	zip_close();
	msg_printf("ERROR: File not found or CRC32 not correct. \"%s\"\n", rom_name);
	msg_printf("Press any button.\n");
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/****************************************************************************
	Decode GFX
****************************************************************************/

/*------------------------------------------------------
	Decode sprite (SPR)
------------------------------------------------------*/

static void neogeo_decode_spr(u8 *mem, u32 length, u8 *usage)
{
	int tileno, numtiles = length / 128;

	for (tileno = 0;tileno < numtiles;tileno++)
	{
		unsigned char swap[128];
		u8 *gfxdata;
		int x,y;
		unsigned int pen;
		int opaque = 0;

		gfxdata = &mem[128 * tileno];

		memcpy(swap,gfxdata,128);

		for (y = 0;y < 16;y++)
		{
			u32 dw;

			dw = 0;
			for (x = 0;x < 8;x++)
			{
				pen  = ((swap[64 + 4*y + 3] >> x) & 1) << 3;
				pen |= ((swap[64 + 4*y + 1] >> x) & 1) << 2;
				pen |= ((swap[64 + 4*y + 2] >> x) & 1) << 1;
				pen |=	(swap[64 + 4*y	  ] >> x) & 1;
				opaque += (pen & 0x0f) != 0;
				dw |= pen << 4*x;
			}
			*(gfxdata++) = dw>>0;
			*(gfxdata++) = dw>>8;
			*(gfxdata++) = dw>>16;
			*(gfxdata++) = dw>>24;

			dw = 0;
			for (x = 0;x < 8;x++)
			{
				pen  = ((swap[4*y + 3] >> x) & 1) << 3;
				pen |= ((swap[4*y + 1] >> x) & 1) << 2;
				pen |= ((swap[4*y + 2] >> x) & 1) << 1;
				pen |=	(swap[4*y	 ] >> x) & 1;
				opaque += (pen & 0x0f) != 0;
				dw |= pen << 4*x;
			}
			*(gfxdata++) = dw>>0;
			*(gfxdata++) = dw>>8;
			*(gfxdata++) = dw>>16;
			*(gfxdata++) = dw>>24;
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
	int i, j;
	u8 tile, opaque;
	u8 *p, buf[32];

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
}


/******************************************************************************
	ROM読み込み
******************************************************************************/

/*--------------------------------------------------------
	CPU1 (M68000 program ROM)
--------------------------------------------------------*/

static int load_rom_cpu1(void)
{
	if ((memory_region_cpu1 = memalign(64, memory_length_cpu1)) == NULL)
	{
		error_memory("REGION_CPU1");
		return 0;
	}
	memset(memory_region_cpu1, 0, memory_length_cpu1);

	if (encrypt_cpu1)
	{
		msg_printf("Loading decrypted CPU1 ROM...\n");
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
				error_rom("CPU1");
				return 0;
			}

			msg_printf("Loading \"%s\"\n", fname);

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

	if ((memory_region_cpu2 = memalign(64, memory_length_cpu2)) == NULL)
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
			error_rom("CPU2");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

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

	if ((memory_region_gfx1 = memalign(64, memory_length_gfx1)) == NULL)
	{
		error_memory("REGION_GFX1");
		return 0;
	}
	memset(memory_region_gfx1, 0, memory_length_gfx1);

	if ((video_fix_usage[0] = memalign(64, memory_length_gfx1 / 32)) == NULL)
	{
		error_memory("GFX_PEN_USAGE (sfix)");
		return 0;
	}

	msg_printf("Loading \"sfix.sfx\"\n");
	if (file_open(game_name, "neogeo", sfix_crc, fname) == -1)
	{
		error_rom("sfix.sfx");
		return 0;
	}
	file_read(memory_region_gfx1, memory_length_gfx1);
	file_close();

	neogeo_decode_fix(memory_region_gfx1, memory_length_gfx1, video_fix_usage[0]);
}

/*--------------------------------------------------------
	GFX2 (FIX sprite ROM)
--------------------------------------------------------*/

static int load_rom_gfx2(void)
{
	if ((memory_region_gfx2 = memalign(64, memory_length_gfx2)) == NULL)
	{
		error_memory("REGION_GFX2");
		return 0;
	}
	memset(memory_region_gfx2, 0, memory_length_gfx2);

	if ((video_fix_usage[1] = memalign(64, memory_length_gfx2 / 32)) == NULL)
	{
		error_memory("GFX_PEN_USAGE (fix)");
		return 0;
	}

	if (encrypt_gfx)
	{
		msg_printf("Loading decrypted GFX2 ROM...\n");
		if (cachefile_open("srom") == -1)
		{
			error_file("GFX2");
			return 0;
		}
		file_read(memory_region_gfx2, memory_length_gfx2);
		file_close();

		if (cachefile_open("fix_usage") == -1)
		{
			error_file("fix_usage (fix pen usage)");
			return 0;
		}
		file_read(video_fix_usage[1], memory_length_gfx2 / 32);
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
				error_rom("GFX2");
				return 0;
			}

			msg_printf("Loading \"%s\"\n", fname);

			i = rom_load(gfx2rom, memory_region_gfx2, i, num_gfx2rom);

			file_close();
		}

		neogeo_decode_fix(memory_region_gfx2, memory_length_gfx2, video_fix_usage[1]);
	}

	return 1;
}

/*--------------------------------------------------------
	GFX3 (OBJ sprite ROM)
--------------------------------------------------------*/

static int load_rom_gfx3(void)
{
	if ((video_spr_usage = memalign(64, memory_length_gfx3 / 128)) == NULL)
	{
		error_memory("GFX_PEN_USAGE (spr)");
		return 0;
	}

	if (encrypt_gfx)
	{
		msg_printf("Loading decrypted GFX3 ROM...\n");
	}
	else
	{
		if ((memory_region_gfx3 = memalign(64, memory_length_gfx3)) != NULL)
		{
			int i;
			char fname[32], *parent;

			memset(memory_region_gfx3, 0, memory_length_gfx3);

			parent = strlen(parent_name) ? parent_name : NULL;

			for (i = 0; i < num_gfx3rom; )
			{
				if (file_open(game_name, parent, gfx3rom[i].crc, fname) == -1)
				{
					error_rom("GFX3");
					return 0;
				}

				msg_printf("Loading \"%s\"\n", fname);

				i = rom_load(gfx3rom, memory_region_gfx3, i, num_gfx3rom);

				file_close();
			}

			neogeo_decode_spr(memory_region_gfx3, memory_length_gfx3, video_spr_usage);
		}
		else
		{
			msg_printf("Could not allocate memory for sprite data.\n");
			msg_printf("Try to use sprite cache...\n");
		}
	}

	if (memory_region_gfx3 == NULL)
	{
		if (cachefile_open("spr_usage") == -1)
		{
			error_file("spr_usage (spr pen usage)");
			return 0;
		}
		file_read(video_spr_usage, memory_length_gfx3 / 128);
		file_close();

		if (cache_start() == 0)
		{
			msg_printf("Press any button.\n");
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
	if (!option_sound_enable || disable_sound)
	{
		memory_length_sound1 = 0;
		return 1;
	}

	if ((memory_region_sound1 = memalign(64, memory_length_sound1)) == NULL)
	{
		error_memory("REGION_SOUND1");
		return 0;
	}
	memset(memory_region_sound1, 0, memory_length_sound1);

	if (encrypt_snd1)
	{
		msg_printf("Loading decrypted SOUND1 ROM...\n");
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
				error_rom("SOUND1");
				return 0;
			}

			msg_printf("Loading \"%s\"\n", fname);

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

	if ((memory_region_sound2 = memalign(64, memory_length_sound2)) == NULL)
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
			error_rom("SOUND2");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

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
		if ((memory_region_user1 = memalign(1, memory_length_user1)) == NULL)
		{
			error_memory("REGION_USER1");
			return 0;
		}
		memset(memory_region_user1, 0, memory_length_user1);
	}

	parent = strlen(parent_name) ? parent_name : "neogeo";

	if (!num_usr1rom)
	{
		patch = bios_patch_address[neogeo_bios];

		if (file_open(game_name, "neogeo", bios_crc[neogeo_bios], fname) == -1)
		{
			error_rom(bios_name[neogeo_bios]);
			neogeo_bios = -1;
			return 0;
		}
		if (!reload) msg_printf("Loading \"%s (%s)\"\n", fname, bios_name[neogeo_bios]);
		file_read(memory_region_user1, memory_length_user1);
		file_close();
	}
	else if (!reload)
	{
		if (encrypt_usr1)
		{
			// JAMMA PCB
			msg_printf("Loading decrypted BIOS ROM...\n");
			if (cachefile_open("biosrom") == -1)
			{
				error_file("BIOS (biosrom)");
				return 0;
			}
		}
		else
		{
			// irrmaze
			patch = 0x010d8c;

			if (file_open(game_name, parent, usr1rom[0].crc, fname) == -1)
			{
				sprintf(fname, "%s BIOS", game_name);
				error_rom(fname);
				return 0;
			}
			msg_printf("Loading \"%s\"\n", fname);
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

int load_rom_info(const char *game_name)
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

	video_fix_usage[0] = NULL;
	video_fix_usage[1] = NULL;
	video_spr_usage    = NULL;

	cache_init();
	pad_wait_clear();
	video_clear_screen();

#ifdef SOUND_TEST
	if (sound_test)
	{
		msg_screen_init("Load ROM (Sound Test)");
	}
	else
	{
#endif
		msg_screen_init("Load ROM");

		load_gamecfg(game_name);

		msg_printf("Checking BIOS...\n");

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
			msg_screen_init("Load ROM");
			msg_printf("Checking BIOS...\n");
			msg_printf("All NVRAM files are removed.\n");
		}
#ifdef SOUND_TEST
	}
#endif

	msg_printf("Checking ROM info...\n");

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: msg_printf("ERROR: This game not supported.\n"); break;
		case 2: msg_printf("ERROR: ROM not found. (zip file name incorrect)\n"); break;
		case 3: msg_printf("ERROR: rominfo.mvs not found.\n"); break;
		}
		msg_printf("Press any button.\n");
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (parent_name[0])
		msg_printf("ROM set \"%s\" (parent: %s).\n", game_name, parent_name);
	else
		msg_printf("ROM set \"%s\".\n", game_name);

#ifdef SOUND_TEST
	if (sound_test)
	{
		option_sound_enable = 1;	// Enable
		option_samplerate   = 2;	// 44100Hz
		option_sound_volume = 10;	// 100%

		disable_sound = 0;
		neogeo_save_sound_flag = 0;
		brza_sram = NULL;

		if (load_rom_cpu2() == 0) return 0;
		if (load_rom_sound1() == 0) return 0;
		if (load_rom_sound2() == 0) return 0;
	}
	else
#endif
	{
		// Brezzasoft用の設定 - メモリを確保するので先に処理
		if (machine_init_type == INIT_vliner || machine_init_type == INIT_jockeygp)
		{
			msg_printf("Allocate \"brza_sram\" memory.\n");
			if ((brza_sram = (u16 *)malloc(0x2000)) == NULL)
			{
				msg_printf("ERROR: Could not allocate \"brza_sram\" memory.\n");
				msg_printf("Press any button.\n");
				pad_wait_press(PAD_WAIT_INFINITY);
				Loop = LOOP_BROWSER;
				return 0;
			}
			memset(brza_sram, 0, 0x2000);
		}
		else brza_sram = NULL;

		if (disable_sound)
		{
			msg_printf("This game only work without sound.\n");
			neogeo_save_sound_flag = option_sound_enable;
			option_sound_enable = 0;
		}
		else
		{
			neogeo_save_sound_flag = 0;
		}

		if (load_rom_user1(0) == 0) return 0;
		if (load_rom_cpu1() == 0) return 0;
		if (load_rom_cpu2() == 0) return 0;
		if (load_rom_sound1() == 0) return 0;
		if (load_rom_sound2() == 0) return 0;
		if (load_rom_gfx1() == 0) return 0;
		if (load_rom_gfx2() == 0) return 0;
		if (load_rom_gfx3() == 0) return 0;

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

		memset(neogeo_memcard, 0, 0x800);
		memset(neogeo_ram, 0, 0x10000);
		memset(neogeo_sram16, 0, 0x10000);

		neogeo_sram = (u8 *)neogeo_sram16;
	}

	msg_printf("Done.\n");

	msg_screen_clear();
	video_clear_screen();

	return 1;
}


/*------------------------------------------------------
	メモリインタフェース終了
------------------------------------------------------*/

void memory_shutdown(void)
{
	if (neogeo_save_sound_flag) option_sound_enable = 1;

	cache_shutdown();

	if (brza_sram) free(brza_sram);

	if (video_fix_usage[0]) free(video_fix_usage[0]);
	if (video_fix_usage[1]) free(video_fix_usage[1]);
	if (video_spr_usage)    free(video_spr_usage);

	if (memory_region_cpu1)   free(memory_region_cpu1);
	if (memory_region_cpu2)   free(memory_region_cpu2);
	if (memory_region_gfx1)   free(memory_region_gfx1);
	if (memory_region_gfx2)   free(memory_region_gfx2);
	if (memory_region_gfx3)   free(memory_region_gfx3);
	if (memory_region_sound1) free(memory_region_sound1);
	if (memory_region_sound2) free(memory_region_sound2);
	if (memory_region_user1)  free(memory_region_user1);
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

	switch (offset & 0xf00000)
	{
	case 0x000000: return READ_BYTE(memory_region_cpu1, offset);
	case 0x100000: return READ_MIRROR_BYTE(neogeo_ram, offset, 0x00ffff);
	case 0xc00000: return READ_MIRROR_BYTE(memory_region_user1, offset, bios_amask);
	case 0xd00000: return READ_MIRROR_BYTE(neogeo_sram, offset, 0x00ffff);

	case 0x200000: return (*neogeo_protection_16_r)(offset >> 1, mem_mask) >> shift;
	case 0x400000: return neogeo_paletteram16_r(offset >> 1, mem_mask) >> shift;
	case 0x800000: return neogeo_memcard16_r(offset >> 1, mem_mask) >> shift;

	case 0x300000:
		switch (offset & 0xff0000)
		{
		case 0x300000: return neogeo_controller1and4_16_r(offset >> 1, mem_mask) >> shift;
		case 0x320000: return neogeo_timer16_r(offset >> 1, mem_mask) >> shift;
		case 0x340000: return neogeo_controller2_16_r(offset >> 1, mem_mask) >> shift;
		case 0x380000: return neogeo_controller3_16_r(offset >> 1, mem_mask) >> shift;
		case 0x3c0000: return neogeo_video_16_r(offset >> 1, mem_mask) >> shift;
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

	switch (offset & 0xf00000)
	{
	case 0x000000: return READ_WORD(memory_region_cpu1, offset);
	case 0x100000: return READ_MIRROR_WORD(neogeo_ram, offset, 0x00ffff);
	case 0xc00000: return READ_MIRROR_WORD(memory_region_user1, offset, bios_amask);
	case 0xd00000: return READ_MIRROR_WORD(neogeo_sram, offset, 0x00ffff);

	case 0x200000: return (*neogeo_protection_16_r)(offset >> 1, 0);
	case 0x400000: return neogeo_paletteram16_r(offset >> 1, 0);
	case 0x800000: return neogeo_memcard16_r(offset >> 1, 0);

	case 0x300000:
		switch (offset & 0xff0000)
		{
		case 0x300000: return neogeo_controller1and4_16_r(offset >> 1, 0);
		case 0x320000: return neogeo_timer16_r(offset >> 1, 0);
		case 0x340000: return neogeo_controller2_16_r(offset >> 1, 0);
		case 0x380000: return neogeo_controller3_16_r(offset >> 1, 0);
		case 0x3c0000: return neogeo_video_16_r(offset >> 1, 0);
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

	switch (offset & 0xf00000)
	{
	case 0x000000: WRITE_BYTE(memory_region_cpu1, offset, data); return;
	case 0x100000: WRITE_MIRROR_BYTE(neogeo_ram, offset, data, 0x00ffff); return;

	case 0x200000: (*neogeo_protection_16_w)(offset >> 1, data << shift, mem_mask); return;
	case 0x400000: neogeo_paletteram16_w(offset >> 1, data << shift, mem_mask); return;
	case 0x800000: neogeo_memcard16_w(offset >> 1, data << shift, mem_mask); return;
	case 0xd00000: neogeo_sram16_w(offset >> 1, data << shift, mem_mask); return;

	case 0x300000:
		switch (offset & 0xff0000)
		{
		case 0x300000: watchdog_reset_16_w(offset >> 1, data << shift, mem_mask); return;
		case 0x320000: neogeo_z80_w(offset >> 1, data << shift, mem_mask); return;
		case 0x380000: neogeo_syscontrol1_16_w(offset >> 1, data << shift, mem_mask); return;
		case 0x3a0000: neogeo_syscontrol2_16_w(offset >> 1, data << shift, mem_mask); return;
		case 0x3c0000: neogeo_video_16_w(offset >> 1, data << shift, mem_mask); return;
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

	switch (offset & 0xf00000)
	{
	case 0x000000: WRITE_WORD(memory_region_cpu1, offset, data); return;
	case 0x100000: WRITE_MIRROR_WORD(neogeo_ram, offset, data, 0x00ffff); return;

	case 0x200000: (*neogeo_protection_16_w)(offset >> 1, data, 0); return;
	case 0x400000: neogeo_paletteram16_w(offset >> 1, data, 0); return;
	case 0x800000: neogeo_memcard16_w(offset >> 1, data, 0); return;
	case 0xd00000: neogeo_sram16_w(offset >> 1, data, 0); return;

	case 0x300000:
		switch (offset & 0xff0000)
		{
		case 0x300000: watchdog_reset_16_w(offset >> 1, data , 0); return;
		case 0x320000: neogeo_z80_w(offset >> 1, data, 0); return;
		case 0x380000: neogeo_syscontrol1_16_w(offset >> 1, data, 0); return;
		case 0x3a0000: neogeo_syscontrol2_16_w(offset >> 1, data, 0); return;
		case 0x3c0000: neogeo_video_16_w(offset >> 1, data, 0); return;
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
	offset &= Z80_AMASK;

	return memory_region_cpu2[offset];
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
