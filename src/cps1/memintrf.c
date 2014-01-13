/******************************************************************************

	memintrf.c

	CPS1�������C���^�t�F�[�X�֐�

******************************************************************************/

#include "cps1.h"


#define M68K_AMASK 0x00ffffff
#define Z80_AMASK 0x0000ffff

#define READ_BYTE(mem, offset)			mem[offset ^ 1]
#define READ_WORD(mem, offset)			*(u16 *)&mem[offset]
#define WRITE_BYTE(mem, offset, data)	mem[offset ^ 1] = data
#define WRITE_WORD(mem, offset, data)	*(u16 *)&mem[offset] = data

#define str_cmp(s1, s2)		strnicmp(s1, s2, strlen(s2))

enum
{
	REGION_CPU1 = 0,
	REGION_CPU2,
	REGION_GFX1,
	REGION_SOUND1,
	REGION_USER1,
	REGION_SKIP
};

#define MAX_CPU1ROM		8
#define MAX_CPU2ROM		8
#define MAX_GFX1ROM		32
#define MAX_SND1ROM		8


/******************************************************************************
	�O���[�o���\����/�ϐ�
******************************************************************************/

u8 *memory_region_cpu1;
u8 *memory_region_cpu2;
u8 *memory_region_gfx1;
u8 *memory_region_sound1;
u8 *memory_region_user1;
u8 *memory_region_user2;

u32 memory_length_cpu1;
u32 memory_length_cpu2;
u32 memory_length_gfx1;
u32 memory_length_sound1;
u32 memory_length_user1;
u32 memory_length_user2;

u8  ALIGN_DATA cps1_ram[0x10000];
u16 ALIGN_DATA cps1_gfxram[0x30000 >> 1];
u16 ALIGN_DATA cps1_output[0x100 >> 1];

u8 *qsound_sharedram1;
u8 *qsound_sharedram2;


/******************************************************************************
	���[�J���\����/�ϐ�
******************************************************************************/

static struct rom_t cpu1rom[MAX_CPU1ROM];
static struct rom_t cpu2rom[MAX_CPU2ROM];
static struct rom_t gfx1rom[MAX_GFX1ROM];
static struct rom_t snd1rom[MAX_SND1ROM];

static int num_cpu1rom;
static int num_cpu2rom;
static int num_gfx1rom;
static int num_snd1rom;

static u8 *static_ram1;
static u8 *static_ram2;


/******************************************************************************
	�v���g�^�C�v
******************************************************************************/

u8 (*z80_read_memory_8)(u32 offset);
void (*z80_write_memory_8)(u32 offset, u8 data);

static u8 cps1_sound_readmem(u32 offset);
static void cps1_sound_writemem(u32 offset, u8 data);

static u8 cps1_qsound_readmem(u32 offset);
static void cps1_qsound_writemem(u32 offset, u8 data);


/******************************************************************************
	�G���[���b�Z�[�W�\��
******************************************************************************/

/*------------------------------------------------------
	�������m�ۃG���[���b�Z�[�W�\��
------------------------------------------------------*/

static void error_memory(const char *mem_name)
{
	zip_close();
	msg_printf("ERROR: Could not allocate %s memory.\n", mem_name);
	msg_printf("Press any button.");
	pad_wait_press(-1);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	ROM�t�@�C���G���[���b�Z�[�W�\��
------------------------------------------------------*/

static void error_rom(const char *rom_name)
{
	zip_close();
	msg_printf("ERROR: File not found or CRC32 not correct. \"%s\"\n", rom_name);
	msg_printf("Press any button.\n");
	pad_wait_press(-1);
	Loop = LOOP_BROWSER;
}


/******************************************************************************
	ROM�ǂݍ���
******************************************************************************/

/*--------------------------------------------------------
	CPU1 (M68000 program ROM / encrypted)
--------------------------------------------------------*/

static int load_rom_cpu1(void)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_cpu1 = memalign(MEM_ALIGN, memory_length_cpu1)) == NULL)
	{
		error_memory("REGION_CPU1");
		return 0;
	}
	memset(memory_region_cpu1, 0, memory_length_cpu1);

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
			error_rom("CPU2");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

		i = rom_load(cpu2rom, memory_region_cpu2, i, num_cpu2rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	GFX1 (graphic ROM)
--------------------------------------------------------*/

static int load_rom_gfx1(void)
{
	int i;
	char fname[32], *parent;

	if ((memory_region_gfx1 = memalign(MEM_ALIGN, memory_length_gfx1)) == NULL)
	{
		error_memory("REGION_GFX1");
		return 0;
	}
	memset(memory_region_gfx1, 0, memory_length_gfx1);

	parent = strlen(parent_name) ? parent_name : NULL;

	for (i = 0; i < num_gfx1rom; )
	{
		if (file_open(game_name, parent, gfx1rom[i].crc, fname) == -1)
		{
			error_rom("GFX1");
			return 0;
		}

		msg_printf("Loading \"%s\"\n", fname);

		i = rom_load(gfx1rom, memory_region_gfx1, i, num_gfx1rom);

		file_close();
	}

	return 1;
}


/*--------------------------------------------------------
	SOUND1 (OKI6295 ADPCM / Q-SOUND PCM ROM)
--------------------------------------------------------*/

static int load_rom_sound1(void)
{
	int i;
	char fname[32], *parent;

	if (!memory_length_sound1) return 1;

	if ((memory_region_sound1 = memalign(MEM_ALIGN, memory_length_sound1)) == NULL)
	{
		error_memory("REGION_SOUND1");
		return 0;
	}
	memset(memory_region_sound1, 0, memory_length_sound1);

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

	return 1;
}


/*--------------------------------------------------------
	USER1 (Encrypted CPU2 ROM)
 -------------------------------------------------------*/

static int load_rom_user1(void)
{
	if (memory_length_user1 == 0) return 1;

	if ((memory_region_user1 = memalign(MEM_ALIGN, memory_length_user1)) == NULL)
	{
		error_memory("REGION_USER1");
		return 0;
	}
	memset(memory_region_user1, 0, memory_length_user1);

	memcpy(memory_region_user1, memory_region_cpu2, 0x8000);

	return 1;
}


/*--------------------------------------------------------
	ROM�����f�[�^�x�[�X�ŉ��
 -------------------------------------------------------*/

int load_rom_info(const char *game_name)
{
	FILE *fp;
	char path[MAX_PATH];
	char buf[256];
	int rom_start = 0;
	int region = 0;

	num_cpu1rom = 0;
	num_cpu2rom = 0;
	num_gfx1rom = 0;
	num_snd1rom = 0;

	machine_driver_type = 0;
	machine_input_type   = 0;
	machine_init_type    = 0;
	machine_screen_type  = 0;

	sprintf(path, "%srominfo.cps1", launchDir);

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
					// ���s
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
						if (str_cmp(parent, "cps1") == 0)
							parent_name[0] = '\0';
						else
							strcpy(parent_name, parent);

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

					strtok(&buf[1], " ");
					size = strtok(NULL, " ,");
					type = strtok(NULL, " ,");
					flag = strtok(NULL, " ");

					if (strcmp(type, "CPU1") == 0)
					{
						sscanf(size, "%x", &memory_length_cpu1);
						region = REGION_CPU1;
					}
					else if (strcmp(type, "CPU2") == 0)
					{
						sscanf(size, "%x", &memory_length_cpu2);
						region = REGION_CPU2;
					}
					else if (strcmp(type, "GFX1") == 0)
					{
						sscanf(size, "%x", &memory_length_gfx1);
						region = REGION_GFX1;
					}
					else if (strcmp(type, "SOUND1") == 0)
					{
						sscanf(size, "%x", &memory_length_sound1);
						region = REGION_SOUND1;
					}
					else if (strcmp(type, "USER1") == 0)
					{
						sscanf(size, "%x", &memory_length_user1);
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

					case REGION_GFX1:
						sscanf(type, "%x", &gfx1rom[num_gfx1rom].type);
						sscanf(offset, "%x", &gfx1rom[num_gfx1rom].offset);
						sscanf(length, "%x", &gfx1rom[num_gfx1rom].length);
						sscanf(crc, "%x", &gfx1rom[num_gfx1rom].crc);
						gfx1rom[num_gfx1rom].group = 0;
						gfx1rom[num_gfx1rom].skip = 0;
						num_gfx1rom++;
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

					case REGION_CPU2:
						msg_printf("ERROR in REGION_CPU2: ROMX() not supported.\n");
						break;

					case REGION_GFX1:
						sscanf(type, "%x", &gfx1rom[num_gfx1rom].type);
						sscanf(offset, "%x", &gfx1rom[num_gfx1rom].offset);
						sscanf(length, "%x", &gfx1rom[num_gfx1rom].length);
						sscanf(crc, "%x", &gfx1rom[num_gfx1rom].crc);
						sscanf(group, "%x", &gfx1rom[num_gfx1rom].group);
						sscanf(skip, "%x", &gfx1rom[num_gfx1rom].skip);
						num_gfx1rom++;
						break;

					case REGION_SOUND1:
						msg_printf("ERROR in REGION_SOUND1: ROMX() not supported.\n");
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
	�������C���^�t�F�[�X�֐�
******************************************************************************/

/*------------------------------------------------------
	�������C���^�t�F�[�X������
------------------------------------------------------*/

int memory_init(void)
{
	int i, res;

	memory_region_cpu1   = NULL;
	memory_region_cpu2   = NULL;
	memory_region_gfx1   = NULL;
	memory_region_sound1 = NULL;
	memory_region_user1  = NULL;
	memory_region_user2  = NULL;

	memory_length_cpu1   = 0;
	memory_length_cpu2   = 0;
	memory_length_gfx1   = 0;
	memory_length_sound1 = 0;
	memory_length_user1  = 0;
	memory_length_user2  = 0;

	pad_wait_clear();
	video_clear_screen();
	msg_screen_init("Load ROM");

	msg_printf("Checking ROM info...\n");

	if ((res = load_rom_info(game_name)) != 0)
	{
		switch (res)
		{
		case 1: msg_printf("ERROR: This game not supported.\n"); break;
		case 2: msg_printf("ERROR: ROM not found. (zip file name incorrect)\n"); break;
		case 3: msg_printf("ERROR: rominfo.cps1 not found.\n"); break;
		}
		msg_printf("Press any button.\n");
		pad_wait_press(-1);
		Loop = LOOP_BROWSER;
		return 0;
	}

	i = 0;
	driver = NULL;
	while (CPS1_driver[i].name)
	{
		if (!strcmp(game_name, CPS1_driver[i].name))
		{
			driver = &CPS1_driver[i];
			break;
		}
		i++;
	}
	if (!driver)
	{
		msg_printf("ERROR: Driver for \"%s\" not found.\n", game_name);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (parent_name[0])
		msg_printf("ROM set \"%s\" (parent: %s).\n", game_name, parent_name);
	else
		msg_printf("ROM set \"%s\".\n", game_name);

	load_gamecfg(game_name);

	if (load_rom_cpu1() == 0) return 0;
	if (load_rom_cpu2() == 0) return 0;
	if (load_rom_gfx1() == 0) return 0;
	if (load_rom_sound1() == 0) return 0;
	if (load_rom_user1() == 0) return 0;

	static_ram1 = (u8 *)cps1_ram    - 0xff0000;
	static_ram2 = (u8 *)cps1_gfxram - 0x900000;

	qsound_sharedram1 = &memory_region_cpu2[0xc000];
	qsound_sharedram2 = &memory_region_cpu2[0xf000];

	memset(cps1_ram, 0, sizeof(cps1_ram));
	memset(cps1_gfxram, 0, sizeof(cps1_gfxram));
	memset(cps1_output, 0, sizeof(cps1_output));

	switch (machine_driver_type)
	{
	case MACHINE_qsound:
		machine_sound_type = SOUND_QSOUND;
		z80_read_memory_8 = cps1_qsound_readmem;
		z80_write_memory_8 = cps1_qsound_writemem;
		memory_length_user2 = 0x8000;
		if ((memory_region_user2 = (u8 *)calloc(1, 0x8000)) == NULL)
		{
			fatalerror("Could not allocate memory. (0x8000 bytes)");
			return 0;
		}
		break;

	case MACHINE_forgottn:
		machine_sound_type = SOUND_YM2151_TYPE2;
		z80_read_memory_8 = cps1_sound_readmem;
		z80_write_memory_8 = cps1_sound_writemem;
		memory_region_user2 = memory_region_cpu2;
		break;

	default:
		machine_sound_type = SOUND_YM2151_TYPE1;
		z80_read_memory_8 = cps1_sound_readmem;
		z80_write_memory_8 = cps1_sound_writemem;
		memory_region_user2 = memory_region_cpu2;
		break;
	}

	switch (machine_init_type)
	{
	case INIT_wof:      wof_decode();      break;
	case INIT_dino:     dino_decode();     break;
	case INIT_punisher: punisher_decode(); break;
	case INIT_slammast: slammast_decode(); break;
	case INIT_pang3:    pang3_decode();    break;
	}

	msg_printf("Done.\n");

	msg_screen_clear();
	video_clear_screen();

	return 1;
}


/*------------------------------------------------------
	�������C���^�t�F�[�X�I��
------------------------------------------------------*/

void memory_shutdown(void)
{
	if (memory_region_cpu1)   free(memory_region_cpu1);
	if (memory_region_cpu2)   free(memory_region_cpu2);
	if (memory_region_gfx1)   free(memory_region_gfx1);
	if (memory_region_sound1) free(memory_region_sound1);
	if (memory_region_user1)  free(memory_region_user1);
	if (memory_length_user2)  free(memory_region_user2);
}


/******************************************************************************
	M68000 ���������[�h/���C�g�֐�
******************************************************************************/

/*------------------------------------------------------
	M68000���������[�h (byte)
------------------------------------------------------*/

u8 m68000_read_memory_8(u32 offset)
{
	int shift;
	u16 mem_mask;

	offset &= M68K_AMASK;

	if (offset < 0x200000)
	{
		return READ_BYTE(memory_region_cpu1, offset);
	}

	shift = (~offset & 1) << 3;
	mem_mask = ~(0xff << shift);

	switch (offset & 0xff0000)
	{
	case 0xff0000:
		return READ_BYTE(static_ram1, offset);

	case 0x900000:
	case 0x910000:
	case 0x920000:
		return READ_BYTE(static_ram2, offset);

	case 0xf00000:
		return qsound_rom_r(offset >> 1, mem_mask) >> shift;

	case 0x800000:
		switch (offset & 0xffe)
		{
		case 0x000: return cps1_inputport1_r(0, mem_mask) >> shift;
		case 0x010: return cps1_inputport1_r(0, mem_mask) >> shift;
		case 0x018: return cps1_inputport0_r(0, mem_mask) >> shift;
		case 0x01a: return cps1_dsw_a_r(0, mem_mask) >> shift;
		case 0x01c: return cps1_dsw_b_r(0, mem_mask) >> shift;
		case 0x01e: return cps1_dsw_c_r(0, mem_mask) >> shift;
		case 0x020: return 0;
		case 0x052: return forgottn_dial_0_r(0, mem_mask) >> shift;
		case 0x054: return forgottn_dial_0_r(1, mem_mask) >> shift;
		case 0x05a: return forgottn_dial_1_r(0, mem_mask) >> shift;
		case 0x05c: return forgottn_dial_1_r(1, mem_mask) >> shift;
		case 0x176: return cps1_inputport2_r(0, mem_mask) >> shift;
		case 0x178: return cps1_inputport3_r(0, mem_mask) >> shift;
		case 0x1fc: return cps1_inputport2_r(0, mem_mask) >> shift;
		default:
			if (offset >= 0x800100 && offset <= 0x8001ff)
			{
				return cps1_output_r(offset >> 1, mem_mask) >> shift;
			}
			break;
		}
		break;

	case 0xf10000:
		switch (offset & 0xf000)
		{
		case 0x8000:
		case 0x9000:
			return qsound_sharedram1_r(offset >> 1, mem_mask) >> shift;

		case 0xe000:
		case 0xf000:
			return qsound_sharedram2_r(offset >> 1, mem_mask) >> shift;

		case 0xc000:
			switch (offset & 0xffe)
			{
			case 0x00: return cps1_inputport2_r(0, mem_mask) >> shift;
			case 0x02: return cps1_inputport3_r(0, mem_mask) >> shift;
			case 0x06: return cps1_eeprom_port_r(offset >> 1, mem_mask) >> shift;
			}
			break;
		}
		break;
	}

	return 0xff;
}


/*------------------------------------------------------
	M68000���[�h������ (word)
------------------------------------------------------*/

u16 m68000_read_memory_16(u32 offset)
{
	offset &= M68K_AMASK;

	if (offset < 0x200000)
	{
		return READ_WORD(memory_region_cpu1, offset);
	}

	switch (offset & 0xff0000)
	{
	case 0xff0000:
		return READ_WORD(static_ram1, offset);

	case 0x900000:
	case 0x910000:
	case 0x920000:
		return READ_WORD(static_ram2, offset);

	case 0xf00000:
		return qsound_rom_r(offset >> 1, 0);

	case 0x800000:
		switch (offset & 0xffe)
		{
		case 0x000: return cps1_inputport1_r(0, 0);
		case 0x010: return cps1_inputport1_r(0, 0);
		case 0x018: return cps1_inputport0_r(0, 0);
		case 0x01a: return cps1_dsw_a_r(0, 0);
		case 0x01c: return cps1_dsw_b_r(0, 0);
		case 0x01e: return cps1_dsw_c_r(0, 0);
		case 0x020: return 0;
		case 0x052: return forgottn_dial_0_r(0, 0);
		case 0x054: return forgottn_dial_0_r(1, 0);
		case 0x05a: return forgottn_dial_1_r(0, 0);
		case 0x05c: return forgottn_dial_1_r(1, 0);
		case 0x176: return cps1_inputport2_r(0, 0);
		case 0x178: return cps1_inputport3_r(0, 0);
		case 0x1fc: return cps1_inputport2_r(0, 0);
		default:
			if (offset >= 0x800100 && offset <= 0x8001ff)
			{
				return cps1_output_r(offset >> 1, 0);
			}
			break;
		}
		break;

	case 0xf10000:
		switch (offset & 0xf000)
		{
		case 0x8000:
		case 0x9000:
			return qsound_sharedram1_r(offset >> 1, 0);

		case 0xe000:
		case 0xf000:
			return qsound_sharedram2_r(offset >> 1, 0);

		case 0xc000:
			switch (offset & 0xffe)
			{
			case 0x000: return cps1_inputport2_r(0, 0);
			case 0x002: return cps1_inputport3_r(0, 0);
			case 0x006: return cps1_eeprom_port_r(offset >> 1, 0);
			}
			break;
		}
		break;
	}

	return 0xffff;
}


/*------------------------------------------------------
	M68000���C�g������ (byte)
------------------------------------------------------*/

void m68000_write_memory_8(u32 offset, u8 data)
{
	int shift = (~offset & 1) << 3;
	u16 mem_mask = ~(0xff << shift);

	offset &= M68K_AMASK;

	switch (offset & 0xff0000)
	{
	case 0xff0000:
		WRITE_BYTE(static_ram1, offset, data);
		return;

	case 0x900000:
	case 0x910000:
	case 0x920000:
		WRITE_BYTE(static_ram2, offset, data);
		return;

	case 0x800000:
		switch (offset & 0xffe)
		{
		case 0x030: cps1_coinctrl_w(offset >> 1, data << shift, mem_mask); return;
		case 0x040: forgottn_dial_0_reset_w(0, 0, mem_mask); return;
		case 0x048: forgottn_dial_1_reset_w(0, 0, mem_mask); return;
		case 0x180: cps1_sound_command_w(offset >> 1, data << shift, mem_mask); return;
		case 0x188: cps1_sound_fade_timer_w(offset >> 1, data << shift, mem_mask); return;
		default:
			if (offset >= 0x800100 && offset <= 0x8001ff)
			{
				cps1_output_w(offset >> 1, data << shift, mem_mask);
				return;
			}
			break;
		}
		break;

	case 0xf10000:
		switch (offset & 0xf000)
		{
		case 0x8000:
		case 0x9000:
			qsound_sharedram1_w(offset >> 1, data << shift, mem_mask);
			return;

		case 0xe000:
		case 0xf000:
			qsound_sharedram2_w(offset >> 1, data << shift, mem_mask);
			return;

		case 0xc000:
			switch (offset & 0xffe)
			{
			case 0x004: cps1_coinctrl2_w(offset >> 1, data << shift, mem_mask); return;
			case 0x006: cps1_eeprom_port_w(offset >> 1, data << shift, mem_mask); return;
			}
			break;
		}
		break;
	}
}


/*------------------------------------------------------
	M68000���C�g������ (word)
------------------------------------------------------*/

void m68000_write_memory_16(u32 offset, u16 data)
{
	offset &= M68K_AMASK;

	switch (offset & 0xff0000)
	{
	case 0x900000:
	case 0x910000:
	case 0x920000:
		WRITE_WORD(static_ram2, offset, data);
		return;

	case 0xff0000:
		WRITE_WORD(static_ram1, offset, data);
		return;

	case 0x800000:
		switch (offset & 0xffe)
		{
		case 0x030: cps1_coinctrl_w(offset >> 1, data, 0); return;
		case 0x040: forgottn_dial_0_reset_w(0, 0, 0); return;
		case 0x048: forgottn_dial_1_reset_w(0, 0, 0); return;
		case 0x180: cps1_sound_command_w(offset >> 1, data, 0); return;
		case 0x188: cps1_sound_fade_timer_w(offset >> 1, data, 0); return;
		default:
			if (offset >= 0x800100 && offset <= 0x8001ff)
			{
				cps1_output_w(offset >> 1, data, 0);
				return;
			}
			break;
		}
		break;

	case 0xf10000:
		switch (offset & 0xf000)
		{
		case 0x8000:
		case 0x9000:
			qsound_sharedram1_w(offset >> 1, data, 0);
			return;

		case 0xe000:
		case 0xf000:
			qsound_sharedram2_w(offset >> 1, data, 0);
			return;

		case 0xc000:
			switch (offset & 0xffe)
			{
			case 0x004: cps1_coinctrl2_w(offset >> 1, data, 0); return;
			case 0x006: cps1_eeprom_port_w(offset >> 1, data, 0); return;
			}
			break;
		}
		break;
	}
}


/******************************************************************************
	Z80 ���������[�h/���C�g�֐�
******************************************************************************/

/*------------------------------------------------------
	Z80���[�h������ (byte - YM2151 + OKIM6295)
------------------------------------------------------*/

static u8 cps1_sound_readmem(u32 offset)
{
	offset &= Z80_AMASK;

	if (offset < 0xd800)
	{
		return memory_region_cpu2[offset];
	}
	switch (offset)
	{
	case 0xf001: return YM2151_status_port_r(0);
	case 0xf002: return OKIM6295_status_r(0);
	case 0xf008: return cps1_sound_command_r(0);
	case 0xf00a: return cps1_sound_fade_timer_r(0);
	}

	return 0;
}


/*------------------------------------------------------
	Z80���C�g������ (byte - YM2151 + OKIM6295)
------------------------------------------------------*/

static void cps1_sound_writemem(u32 offset, u8 data)
{
	offset &= Z80_AMASK;

	if (offset >= 0xc000 && offset < 0xd800)
	{
		memory_region_cpu2[offset] = data;
		return;
	}
	switch (offset)
	{
	case 0xf000: YM2151_register_port_w(0, data); return;
	case 0xf001: YM2151_data_port_w(0, data); return;
	case 0xf002: OKIM6295_data_w(0, data); return;
	case 0xf004: cps1_snd_bankswitch_w(0, data); return;
	}
}


/*------------------------------------------------------
	Z80���[�h������ (byte - QSOUND)
------------------------------------------------------*/

static u8 cps1_qsound_readmem(u32 offset)
{
	offset &= Z80_AMASK;

	switch (offset & 0xf000)
	{
	case 0xd000:
		if (offset == 0xd007) return 0x80;
		break;

	default:
		// 0000-7fff: ROM
		// 8000-bfff: banked ROM
		// c000-cfff: QSOUND shared RAM 1
		// f000-ffff: QSOUND shared RAM 2
		return memory_region_cpu2[offset];
	}

	return 0;
}


/*------------------------------------------------------
	Z80���C�g������ (byte - QSOUND)
------------------------------------------------------*/

static void cps1_qsound_writemem(u32 offset, u8 data)
{
	offset &= Z80_AMASK;

	switch (offset & 0xf000)
	{
	case 0xc000: case 0xf000:
		// c000-cfff: QSOUND shared RAM 1
		// f000-ffff: QSOUND shared RAM 2
		memory_region_cpu2[offset] = data;
		break;

	case 0xd000:
		switch (offset)
		{
		case 0xd000: qsound_data_h_w(0, data); break;
		case 0xd001: qsound_data_l_w(0, data); break;
		case 0xd002: qsound_cmd_w(0, data); break;
		case 0xd003: qsound_banksw_w(0, data); break;
		}
		break;
	}
}


/******************************************************************************
	�Z�[�u/���[�h �X�e�[�g
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( memory )
{
	state_save_byte(cps1_ram, sizeof(cps1_ram));
	state_save_byte(cps1_gfxram, sizeof(cps1_gfxram));
	state_save_byte(cps1_output, sizeof(cps1_output));
	if (machine_sound_type == SOUND_QSOUND)
	{
		state_save_byte(qsound_sharedram1, 0x1000);
		state_save_byte(qsound_sharedram2, 0x1000);
	}
	else
	{
		state_save_byte(&memory_region_cpu2[0xc000], 0x1800);
	}
}

STATE_LOAD( memory )
{
	state_load_byte(cps1_ram, sizeof(cps1_ram));
	state_load_byte(cps1_gfxram, sizeof(cps1_gfxram));
	state_load_byte(cps1_output, sizeof(cps1_output));
	if (machine_sound_type == SOUND_QSOUND)
	{
		state_load_byte(qsound_sharedram1, 0x1000);
		state_load_byte(qsound_sharedram2, 0x1000);
	}
	else
	{
		state_load_byte(&memory_region_cpu2[0xc000], 0x1800);
	}
}

#endif /* SAVE_STATE */
