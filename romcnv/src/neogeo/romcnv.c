#include "romcnv.h"
#include "zip32j.h"

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

enum
{
	ROM_LOAD = 0,
	ROM_CONTINUE,
	ROM_WORDSWAP,
	MAP_MAX
};

#define MAX_CPU1ROM		8
#define MAX_CPU2ROM		8
#define MAX_GFX2ROM		4
#define MAX_GFX3ROM		16
#define MAX_SND1ROM		8
#define MAX_SND2ROM		8
#define MAX_USR1ROM		1

u8 *memory_region_cpu1;
u8 *memory_region_gfx2;
u8 *memory_region_gfx3;
u8 *memory_region_sound1;
u8 *memory_region_user1;

u32 memory_length_cpu1;
u32 memory_length_cpu2;
u32 memory_length_gfx1;
u32 memory_length_gfx2;
u32 memory_length_gfx3;
u32 memory_length_sound1;
u32 memory_length_sound2;
u32 memory_length_user1;

static u8 *video_fix_usage;
static u8 *video_spr_usage;

static int disable_sound;
static int machine_driver_type;
static int machine_init_type;
static int machine_input_type;
static int machine_screen_type;

static char game_dir[MAX_PATH];
static char zip_dir[MAX_PATH];
static char launchDir[MAX_PATH];
static char game_name[16];
static char parent_name[16];

struct rom_t
{
	u32 type;
	u32 offset;
	u32 length;
	u32 crc;
	int group;
	int skip;
};

static int rom_fd;

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


int file_dialog(HWND hwnd, LPCSTR filter, char *fname, u32 flags)
{
	OPENFILENAME OFN;

	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner   = hwnd;
	OFN.lpstrFilter = filter;
	OFN.lpstrFile   = fname;
	OFN.nMaxFile    = MAX_PATH*2;
	OFN.Flags       = flags;
	OFN.lpstrTitle  = "Select zipped ROM file.";

	return GetOpenFileName(&OFN);
}


void neogeo_decode_spr(u8 *mem, u32 length, u8 *usage)
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


int file_open(const char *fname1, const char *fname2, const u32 crc, char *fname)
{
	int found = 0;
	struct zip_find_t file;
	char path[MAX_PATH];

	sprintf(path, "%s\\%s.zip", zip_dir, fname1);

	if (zip_open(path) != -1)
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
		sprintf(path, "%s\\%s.zip", zip_dir, fname2);

		if (zip_open(path) != -1)
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
	}

	if (found)
	{
		if (fname) strcpy(fname, file.name);
		rom_fd = zopen(file.name);
		return rom_fd;
	}

	return -1;
}


void file_close(void)
{
	if (rom_fd != -1)
	{
		zclose(rom_fd);
		zip_close();
		rom_fd = -1;
	}
}


int file_read(void *buf, size_t length)
{
	if (rom_fd != -1)
		return zread(rom_fd, buf, length);
	return -1;
}


int file_getc(void)
{
	if (rom_fd != -1)
		return zgetc(rom_fd);
	return -1;
}


int rom_load(struct rom_t *rom, u8 *mem, int idx, int max)
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


static int load_rom_cpu1(void)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_cpu1 = calloc(1, memory_length_cpu1)) == NULL)
	{
		printf("Could not allocate memory. (REGION_CPU1)\n");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_cpu1rom; )
	{
		if (file_open(game_name, parent, cpu1rom[i].crc, fname) == -1)
		{
			printf("Could not open file. (CPU1)\n");
			return 0;
		}

		printf("Loading \"%s\"\n", fname);

		i = rom_load(cpu1rom, memory_region_cpu1, i, num_cpu1rom);

		file_close();
	}

	return 1;
}


static int load_rom_gfx2(void)
{
	if ((memory_region_gfx2 = calloc(1, memory_length_gfx2)) == NULL)
	{
		printf("Could not allocate memory. (REGION_GFX2)\n");
		return 0;
	}
	if ((video_fix_usage = calloc(1, memory_length_gfx2 / 32)) == NULL)
	{
		printf("Could not allocate memory. (FIX_PEN_USAGE)");
		return 0;
	}

	return 1;
}


static int load_rom_gfx3(int decode)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_gfx3 = calloc(1, memory_length_gfx3)) == NULL)
	{
		printf("Could not allocate memory. (REGION_GFX3)\n");
		return 0;
	}
	if ((video_spr_usage = calloc(1, memory_length_gfx3 / 128)) == NULL)
	{
		printf("Could not allocate memory. (SPR_PEN_USAGE)");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_gfx3rom; )
	{
		if (file_open(game_name, parent, gfx3rom[i].crc, fname) == -1)
		{
			printf("Could not open file. (GFX3)\n");
			return 0;
		}

		printf("Loading \"%s\"\n", fname);

		i = rom_load(gfx3rom, memory_region_gfx3, i, num_gfx3rom);

		file_close();
	}

	if (decode)
		neogeo_decode_spr(memory_region_gfx3, memory_length_gfx3, video_spr_usage);

	return 1;
}


static int load_rom_sound1(void)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_sound1 = calloc(1, memory_length_sound1)) == NULL)
	{
		printf("Could not allocate memory. (REGION_SOUND1)\n");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_snd1rom; )
	{
		if (file_open(game_name, parent, snd1rom[i].crc, fname) == -1)
		{
			printf("Could not open file. (SOUND1)\n");
			return 0;
		}

		printf("Loading \"%s\"\n", fname);

		i = rom_load(snd1rom, memory_region_sound1, i, num_snd1rom);

		file_close();
	}

	return 1;
}


static int load_rom_user1(void)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_user1 = calloc(1, memory_length_user1)) == NULL)
	{
		printf("Could not allocate memory. (REGION_USER1)\n");
		return 0;
	}

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_usr1rom; )
	{
		if (file_open(game_name, parent, usr1rom[i].crc, fname) == -1)
		{
			printf("Could not open file. (SOUND1)\n");
			return 0;
		}

		printf("Loading \"%s\"\n", fname);

		i = rom_load(usr1rom, memory_region_user1, i, num_usr1rom);

		file_close();
	}

	return 1;
}


int str_cmp(const char *s1, const char *s2)
{
	return strnicmp(s1, s2, strlen(s2));
}

int load_rom_info(const char *game_name)
{
	FILE *fp;
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

	machine_driver_type = 0;
	machine_input_type  = 0;
	machine_init_type   = 0;
	machine_screen_type = 0;

	encrypt_cpu1 = 0;
	encrypt_snd1 = 0;
	encrypt_gfx  = 0;
	encrypt_usr1 = 0;

	disable_sound = 0;

	if ((fp = fopen("rominfo.mvs", "r")) != NULL)
	{
		while (fgets(buf, 255, fp))
		{
			if (buf[0] == '/' && buf[1] == '/')
				continue;

			if (buf[0] != '\t')
			{
				if (buf[0] == '\r' || buf[0] == '\n')
				{
					// ‰üs
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


int main(int argc, char *argv[])
{
	u32 i, j, k, l, length, res;
	FILE *fp;
	char fname[MAX_PATH], zipname[MAX_PATH];
	char cmdline[MAX_PATH * 2], path[MAX_PATH];
	char *p, buffer[1024];
	int num = 0, num2 = 0;

	memory_region_cpu1   = NULL;
	memory_region_gfx2   = NULL;
	memory_region_gfx3   = NULL;
	memory_region_sound1 = NULL;
	memory_region_user1  = NULL;

	memory_length_cpu1   = 0;
	memory_length_cpu2   = 0;
	memory_length_gfx1   = 0;
	memory_length_gfx2   = 0;
	memory_length_gfx3   = 0;
	memory_length_sound1 = 0;
	memory_length_sound2 = 0;
	memory_length_user1  = 0;

	video_fix_usage      = NULL;
	video_spr_usage      = NULL;

	printf("-------------------------------------------\n");
	printf(" ROM converter for NEOGEO Emulator ver.0.5\n");
	printf("-------------------------------------------\n\n");

	if (argc == 2 && argv[1] != NULL)
	{
		strcpy(launchDir, argv[0]);
		*(strrchr(launchDir, '\\') + 1) = '\0';
	}
	else
		_getcwd(launchDir, MAX_PATH);

	if (_chdir("cache") != 0)
	{
		if (_mkdir("cache") != 0)
		{
			printf("Error: Could not create directory \"cache\".\n");
			printf("Press any key to exit.\n");
			getch();
			return 0;
		}
	}
	else _chdir("..");

	if (_chdir("temp") != 0)
	{
		if (_mkdir("temp") != 0)
		{
			printf("Error: Could not create directory \"temp\".\n");
			printf("Press any key to exit.\n");
			getch();
			return 0;
		}
	}
	else _chdir("..");

	if (argc == 2 && argv[1] != NULL)
	{
		strcpy(buffer, argv[1]);
		strcpy(game_dir, strtok(buffer, "\""));
	}
	else
	{
		printf("Please select ROM file.\n");

		if (!file_dialog(NULL, "zip file (*.zip)\0*.zip\0", game_dir, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY))
		{
			printf("Press any key to exit.\n");
			getch();
			return 0;
		}
	}

	if ((p = strrchr(game_dir, '\\')) != NULL)
	{
		strcpy(game_name, p + 1);
		strcpy(zip_dir, game_dir);
		*strrchr(zip_dir, '\\') = '\0';
	}
	else
	{
		strcpy(game_name, game_dir);
		strcpy(zip_dir, "");
	}

	printf("path: %s\n", zip_dir);
	printf("filename: %s\n", game_name);

	if ((p = strrchr(game_name, '.')) == NULL)
	{
		printf("Please input correct path.\n");
		printf("Press any key to exit.\n");
		getch();
		return 0;
	}
	*p = '\0';

	printf("Checking ROM file... (%s)\n", game_name);

	_chdir(launchDir);

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: printf("ERROR: This game is not supported.\n"); break;
		case 2: printf("ERROR: ROM not found. (zip file name incorrect)\n"); break;
		case 3: printf("ERROR: rominfo.mvs not found.\n"); break;
		}
		printf("Press any key.\n");
		getch();
		return 0;
	}

	if (strlen(parent_name))
		printf("Clone set (parent: %s)\n", parent_name);

	if (encrypt_usr1)
	{
		printf("decrypt biosrom\n");

		if (load_rom_user1() == 0)
		{
			printf("ERROR: Could not load BIOS program rom.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error;
		}

		switch (machine_init_type)
		{
		case INIT_kf2k3pcb:
			kof2003biosdecode();
			break;

		default:
			printf("This romset not supported.\n");
			printf("Press any key.\n");
			getch();
			goto error;
		}
	}

	if (encrypt_cpu1)
	{
		if (load_rom_cpu1() == 0)
		{
			printf("ERROR: Could not load program rom (p rom).\n");
			printf("Press any key to exit.\n");
			getch();
			goto error;
		}

		printf("decrypt prom\n");

		switch (machine_init_type)
		{
		case INIT_kof98:	kof98_decrypt_68k();	break;
		case INIT_kof99:	kof99_decrypt_68k();	break;
		case INIT_garou:	garou_decrypt_68k();	break;
		case INIT_garouo:	garouo_decrypt_68k();	break;
		case INIT_mslug3:	mslug3_decrypt_68k();	break;
		case INIT_kof2000:	kof2000_decrypt_68k();	break;
		case INIT_kof2002:	kof2002_decrypt_68k();	break;
		case INIT_mslug5:	mslug5_decrypt_68k();	break;
		case INIT_svchaosa:	svcchaos_px_decrypt();	break;
		case INIT_samsho5:	samsho5_decrypt_68k();	break;
		case INIT_kof2003:	kof2003_decrypt_68k();	break;
		case INIT_samsh5sp:	samsh5p_decrypt_68k();	break;
		case INIT_matrim:	matrim_decrypt_68k();	break;

		case INIT_ms5pcb:	mslug5_decrypt_68k();	break;
		case INIT_svcpcb:	svcchaos_px_decrypt();	break;
		case INIT_kf2k3pcb:	kf2k3pcb_decrypt_68k();	break;

		default:
			printf("This romset not supported.\n");
			printf("Press any key.\n");
			getch();
			goto error;
		}
	}

	if (encrypt_snd1)
	{
		if (load_rom_sound1() == 0)
		{
			printf("ERROR: Could not load program rom (v rom).\n");
			printf("Press any key to exit.\n");
			getch();
			goto error;
		}

		printf("decrypt vrom\n");

		switch (machine_init_type)
		{
		case INIT_kof2002:	neo_pcm2_swap(0);		break;
		case INIT_mslug5:	neo_pcm2_swap(2);		break;
		case INIT_svchaosa:	neo_pcm2_swap(3);		break;
		case INIT_samsho5:	neo_pcm2_swap(4);		break;
		case INIT_kof2003:	neo_pcm2_swap(5);		break;
		case INIT_samsh5sp:	neo_pcm2_swap(6);		break;
		case INIT_pnyaa:	neo_pcm2_snk_1999(4);	break;
		case INIT_mslug4:	neo_pcm2_snk_1999(8);	break;
		case INIT_rotd:		neo_pcm2_snk_1999(16);	break;
		case INIT_matrim:	neo_pcm2_swap(1);		break;

		case INIT_ms5pcb:	neo_pcm2_swap(2);		break;
		case INIT_svcpcb:	neo_pcm2_swap(3);		break;
		case INIT_kf2k3pcb:	neo_pcm2_swap(5);		break;

		default:
			printf("This romset not supported.\n");
			printf("Press any key.\n");
			getch();
			goto error;
		}

	}

	if (encrypt_gfx)
	{
		if (load_rom_gfx2() == 0)
		{
			printf("ERROR: Could not allocate memory (s rom).\n");
			printf("Press any key to exit.\n");
			getch();
			goto error;
		}
		if (load_rom_gfx3(0) == 0)
		{
			printf("ERROR: Could not load sprite rom (c rom).\n");
			printf("Press any key to exit.\n");
			getch();
			goto error;
		}

		printf("decrypt gfx\n");

		switch (machine_init_type)
		{
		case INIT_kof99:	kof99_neogeo_gfx_decrypt(0x00);		break;
		case INIT_kof99n:	kof99_neogeo_gfx_decrypt(0x00);		break;
		case INIT_garou:	kof99_neogeo_gfx_decrypt(0x06);		break;
		case INIT_garouo:	kof99_neogeo_gfx_decrypt(0x06);		break;
		case INIT_mslug3:	kof99_neogeo_gfx_decrypt(0xad);		break;
		case INIT_mslug3n:	kof99_neogeo_gfx_decrypt(0xad);		break;
		case INIT_kof2000:	kof2000_neogeo_gfx_decrypt(0x00);	break;
		case INIT_kof2000n:	kof2000_neogeo_gfx_decrypt(0x00);	break;
		case INIT_zupapa:	kof99_neogeo_gfx_decrypt(0xbd);		break;
		case INIT_sengoku3:	kof99_neogeo_gfx_decrypt(0xfe);		break;
		case INIT_kof2001:	kof2000_neogeo_gfx_decrypt(0x1e);	break;
		case INIT_kof2002:	kof2000_neogeo_gfx_decrypt(0xec);	break;
		case INIT_mslug5:	kof2000_neogeo_gfx_decrypt(0x19);	break;
		case INIT_svchaosa:	kof2000_neogeo_gfx_decrypt(0x57);	break;
		case INIT_samsho5:	kof2000_neogeo_gfx_decrypt(0x0f);	break;
		case INIT_kof2003:	kof2000_neogeo_gfx_decrypt(0x9d);	break;
		case INIT_samsh5sp:	kof2000_neogeo_gfx_decrypt(0x0d);	break;
		case INIT_nitd:		kof99_neogeo_gfx_decrypt(0xff);		break;
		case INIT_s1945p:	kof99_neogeo_gfx_decrypt(0x05);		break;
		case INIT_pnyaa:	kof2000_neogeo_gfx_decrypt(0x2e);	break;
		case INIT_preisle2:	kof99_neogeo_gfx_decrypt(0x9f);		break;
		case INIT_ganryu:	kof99_neogeo_gfx_decrypt(0x07);		break;
		case INIT_bangbead:	kof99_neogeo_gfx_decrypt(0xf8);		break;
		case INIT_mslug4:	kof2000_neogeo_gfx_decrypt(0x31);	break;
		case INIT_rotd:		kof2000_neogeo_gfx_decrypt(0x3f);	break;
		case INIT_matrim:	kof2000_neogeo_gfx_decrypt(0x6a);	break;

		case INIT_ms5pcb:
			svcpcb_gfx_decrypt();
			kof2000_neogeo_gfx_decrypt(0x19);
			svcpcb_s1data_decrypt();
			break;

		case INIT_svcpcb:
			svcpcb_gfx_decrypt();
			kof2000_neogeo_gfx_decrypt(0x57);
			svcpcb_s1data_decrypt();
			break;

		case INIT_kf2k3pcb:
			kf2k3pcb_gfx_decrypt();
			kof2000_neogeo_gfx_decrypt(0x9d);
			kf2k3pcb_decrypt_s1data();
			break;

		case INIT_jockeygp:	kof2000_neogeo_gfx_decrypt(0xac);	break;

		default:
			printf("This romset not supported.\n");
			printf("Press any key.\n");
			getch();
			goto error;
		}

		neogeo_decode_fix(memory_region_gfx2, memory_length_gfx2, video_fix_usage);
		neogeo_decode_spr(memory_region_gfx3, memory_length_gfx3, video_spr_usage);
	}
	else
	{
		if (load_rom_gfx3(1) == 0)
		{
			if (memory_region_cpu1) free(memory_region_cpu1);
			printf("ERROR: Could not load sprite rom (c rom).\n");
			printf("Press any key to exit.\n");
			getch();
			if (memory_region_gfx3) free(memory_region_gfx3);
			if (video_spr_usage) free(video_spr_usage);
			return 0;
		}
	}

	_chdir(launchDir);
	_chdir("temp");

	printf("Create cache file...\n");

	if ((fp = fopen("spr_usage", "wb")) == NULL)
	{
		printf("ERROR: Could not create file.\n");
		printf("Press any key to exit.\n");
		getch();
		goto error2;
	}
	fwrite(video_spr_usage, 1, memory_length_gfx3 / 128, fp);
	fclose(fp);
	num++;

	for (i = 0; i < memory_length_gfx3; i += 0x10000)
	{
		sprintf(fname, "%03x", i >> 16);
		if ((fp = fopen(fname, "wb")) == NULL)
		{
			printf("ERROR: Could not create file.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error2;
		}
		fwrite(&memory_region_gfx3[i], 1, 0x10000, fp);
		fclose(fp);
		num++;
	}

	if (encrypt_gfx)
	{
		if ((fp = fopen("fix_usage", "wb")) == NULL)
		{
			printf("ERROR: Could not create file.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error2;
		}
		fwrite(video_fix_usage, 1, memory_length_gfx2 / 32, fp);
		fclose(fp);
		num++;

		if ((fp = fopen("srom", "wb")) == NULL)
		{
			printf("ERROR: Could not create file.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error2;
		}
		fwrite(memory_region_gfx2, 1, memory_length_gfx2, fp);
		fclose(fp);
		num++;
	}

	if (encrypt_snd1)
	{
		if ((fp = fopen("vrom", "wb")) == NULL)
		{
			printf("ERROR: Could not create file.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error2;
		}
		fwrite(memory_region_sound1, 1, memory_length_sound1, fp);
		fclose(fp);
		num++;
	}

	if (encrypt_cpu1)
	{
		if ((fp = fopen("prom", "wb")) == NULL)
		{
			printf("ERROR: Could not create file.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error2;
		}
		fwrite(memory_region_cpu1, 1, memory_length_cpu1, fp);
		fclose(fp);
		num++;
	}

	if (encrypt_usr1)
	{
		if ((fp = fopen("biosrom", "wb")) == NULL)
		{
			printf("ERROR: Could not create file.\n");
			printf("Press any key to exit.\n");
			getch();
			goto error2;
		}
		fwrite(memory_region_user1, 1, memory_length_user1, fp);
		fclose(fp);
		num++;
	}

	if ((fp = fopen("response.txt", "w")) == NULL)
	{
		printf("ERROR: Could not create file.\n");
		printf("Press any key to exit.\n");
		getch();
		goto error2;
	}

	sprintf(path, "%s\\temp\\", launchDir);

	for (i = 0; i < memory_length_gfx3; i += 0x10000)
	{
		sprintf(fname, "%03x", i >> 16);
		fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, fname);
	}

	if (encrypt_gfx) fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, "srom");
	if (encrypt_usr1) fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, "biosrom");
	if (encrypt_cpu1) fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, "prom");
	if (encrypt_snd1) fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, "vrom");

	fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, "spr_usage");
	if (encrypt_gfx) fprintf(fp, "-D -j -r -9 \"%s\" \"%s\"\n", path, "fix_usage");

	fclose(fp);

	printf("Compress to zip file... \"cache\\%s_cache.zip\"\n", game_name);

	sprintf(zipname, "%s\\cache\\%s_cache.zip", launchDir, game_name);
	remove(zipname);

	sprintf(cmdline, "\"%s\" @\"%s\\response.txt\"", zipname, path);
	if (Zip(NULL, cmdline, NULL, 0) != 0)
	{
		printf("ERROR: Could not create zip file.\n");
		printf("Press any key to exit.\n");
		getch();
		goto error2;
	}

	printf("complete.\n");
	printf("Please copy this file to directory \"/PSP/GAMES/mvspsp/cache\".\n");
	printf("Press any key to exit.\n");
	getch();

error2:
	_chdir(launchDir);
	_chdir("temp");

	for (i = 0; i < memory_length_gfx3; i += 0x10000)
	{
		sprintf(fname, "%03x", i >> 16);
		remove(fname);
	}
	remove("spr_usage");

	if (encrypt_gfx)
	{
		remove("fix_usage");
		remove("srom");
	}
	if (encrypt_snd1) remove("vrom");
	if (encrypt_cpu1) remove("prom");
	if (encrypt_usr1) remove("biosrom");
	remove("response.txt");

error:
	if (memory_region_cpu1)   free(memory_region_cpu1);
	if (memory_region_gfx2)   free(memory_region_gfx2);
	if (memory_region_gfx3)   free(memory_region_gfx3);
	if (memory_region_sound1) free(memory_region_sound1);
	if (memory_region_user1)  free(memory_region_user1);
	if (video_spr_usage)      free(video_spr_usage);
	if (video_fix_usage)      free(video_fix_usage);

	return 1;
}
