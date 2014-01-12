/******************************************************************************

	driver.c

	CPS2 ドライバ

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	グローバル構造体
******************************************************************************/

#define SPRMASK_NONE { 0, -1, -1 }

#define SPRITE_MASK_NONE  { SPRMASK_NONE,SPRMASK_NONE,SPRMASK_NONE,SPRMASK_NONE }

#define SPRITE_MASK_SSF2T														\
{																				\
	{ MASK_CHECK_MASK, 3, 7 },													\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_MSH															\
{																				\
	{ MASK_CHECK_MASK | MASK_MSH, 4, 7 },										\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_DDSOM														\
{																				\
	{ MASK_CHECK_MASK | MASK_AFTER_DRAW, 2, 4 },								\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_SFA2														\
{																				\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ, 0, 7 },									\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_SFA3														\
{																				\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ, 3, 7 },									\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_CSCLUB														\
{																				\
	{ MASK_CHECK_MASK, 2, 7 },													\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_VSAV														\
{																				\
	{ MASK_CHECK_MASK | MASK_AFTER_DRAW, 6, 7 },								\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_GIGAWING													\
{																				\
	{ MASK_CHECK_MASK, 1, 3 },													\
	{ MASK_CHECK_MASK | MASK_AFTER_DRAW, 0, 3 },								\
	{ MASK_CHECK_OBJ | MASK_AFTER_DRAW, 4, 7 },									\
	{ MASK_CHECK_MASK | MASK_CHECK_ATTR, 1, 4 }									\
}

#define SPRITE_MASK_MPANGJ														\
{																				\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ, 1, 7 },									\
	SPRMASK_NONE,																\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_DIMAHOO														\
{																				\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ, 0, 7 },									\
	{ MASK_CHECK_MASK, 0, 7 },													\
	SPRMASK_NONE,																\
	SPRMASK_NONE																\
}

#define SPRITE_MASK_PROGEAR														\
{																				\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ, 0, 6 },									\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ | MASK_AFTER_DRAW, 5, 6 },				\
	{ MASK_CHECK_MASK | MASK_CHECK_OBJ | MASK_AFTER_DRAW, 0, 4 },				\
	{ MASK_CHECK_MASK | MASK_AFTER_DRAW, 4, 6 }									\
}


struct driver_t CPS2_driver[] =
{
//    name        cache      sprite info         kludge              flags mask                  player & coin chuter
	{ "ssf2",     0xb50000,  512, 208, 256, 256, CPS2_KLUDGE_SSF2,    1,   SPRITE_MASK_NONE,     0x00, { COIN_NONE } },
	{ "ddtod",    0xc00000,  512,  80, 288, 352, 0,                   0,   SPRITE_MASK_NONE,     0x0e, { COIN_4P4C, COIN_4P1C, COIN_3P3C, COIN_3P1C, COIN_2P1C } },
	{ "ecofghtr", 0x9c0000,  512,  96, 272, 352, 0,                   0,   SPRITE_MASK_NONE,     0x00, { COIN_NONE } },
	{ "ssf2t",    0xf30000,  512, 192, 240, 288, CPS2_KLUDGE_SSF2T,   1,   SPRITE_MASK_SSF2T,    0x00, { COIN_NONE } },
	{ "xmcota",   0x1fb0000, 464, 192, 256, 320, CPS2_KLUDGE_XMCOTA,  1|2, SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "armwar",   0x1300000, 512, 112, 288, 320, 0,                   0,   SPRITE_MASK_NONE,     0x06, { COIN_2P1C, COIN_2P2C, COIN_2P2C, COIN_3P1C, COIN_3P2C, COIN_3P3C, COIN_3P3C } },
	{ "avsp",     0xb00000,  512, 112, 320, 288, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_3P3C, COIN_3P1C, COIN_2P1C } },
	{ "dstlk",    0x13d0000, 512, 144, 256, 320, 0,                   0,   SPRITE_MASK_NONE,     0x00, { COIN_NONE } },
	{ "ringdest", 0x1100000, 512,  80, 256, 384, 0,                   0,   SPRITE_MASK_NONE,     0x06, { COIN_2P1C, COIN_2P2C, COIN_2P2C } },
	{ "cybots",   0x1f40000, 480, 192, 240, 320, 0,                   0,   SPRITE_MASK_NONE,     0x26, { COIN_2P1C, COIN_2P2C, COIN_2P2C } },
	{ "msh",      0x1ff0000, 480, 192, 272, 288, 0,                   1,   SPRITE_MASK_MSH,      0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "nwarr",    0x1ed0000, 512, 176, 288, 256, 0,                   0,   SPRITE_MASK_NONE,     0x1d, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "sfa",      0x7e0000,  512,  48, 352, 320, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "rckmanj",  0x790000,  512, 208, 256, 256, 0,                   0,   SPRITE_MASK_NONE,     0x00, { COIN_NONE } },
	{ "19xx",     0x970000,  512,  48, 288, 384, 0,                   0,   SPRITE_MASK_NONE,     0x26, { COIN_2P1C, COIN_2P2C, COIN_2P2C } },
	{ "ddsom",    0x16f0000, 512, 128, 272, 320, 0,                   0,   SPRITE_MASK_DDSOM,    0x01, { COIN_2P1C, COIN_2P2C, COIN_2P2C, COIN_3P1C, COIN_3P2C, COIN_3P3C, COIN_3P3C, COIN_4P1C, COIN_4P2C, COIN_4P2C, COIN_4P4C, COIN_4P4C } },
	{ "megaman2", 0x7f0000,  512,  64, 240, 416, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "qndream",  0x700000,  512, 144, 352, 224, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "sfa2",     0x12e0000, 512, 192, 272, 256, 0,                   0,   SPRITE_MASK_SFA2,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "spf2t",    0x3b0000,  512,  80, 416, 224, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "xmvsf",    0x1fd0000, 512, 208, 256, 256, 0,                   1|2, SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "batcir",   0xd70000,  512,  64, 368, 288, 0,                   0,   SPRITE_MASK_NONE,     0x26, { COIN_2P1C, COIN_2P2C, COIN_2P2C, COIN_4P1C, COIN_4P2C, COIN_4P2C, COIN_4P4C, COIN_4P4C } },
	{ "csclub",   0x7f0000,  512,  80, 224, 416, 0,                   0,   SPRITE_MASK_CSCLUB,   0x21, { COIN_2P1C, COIN_2P2C, COIN_2P2C } },
	{ "mshvsf",   0x1fd0000, 512, 192, 272, 256, 0,                   1|2, SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "sgemf",    0x13e0000, 512, 176, 256, 288, 0,                   2,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "vhunt2",   0x1f60000, 512, 144, 256, 320, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "vsav",     0x1fe0000, 512, 144, 256, 320, 0,                   0,   SPRITE_MASK_VSAV,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "vsav2",    0x1fb0000, 512, 144, 256, 320, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "mvsc",     0x1f90000, 464, 208, 240, 320, 0,                   1|2, SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "sfa3",     0x1f60000, 512, 160, 272, 288, 0,                   0,   SPRITE_MASK_SFA3,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "gigawing", 0xf90000,  464, 208, 272, 288, 0,                   0,   SPRITE_MASK_GIGAWING, 0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "mmatrix",  0x1d70000, 512,  48, 288, 384, CPS2_KLUDGE_MMATRIX, 0,   SPRITE_MASK_NONE,     0x11, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "mpangj",   0x790000,  512,  32, 304, 384, 0,                   0,   SPRITE_MASK_MPANGJ,   0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "pzloop2j", 0xab0000,  512, 144, 288, 288, CPS2_KLUDGE_PUZLOOP2,0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "choko",    0,         512, 208, 256, 256, 0,                   0,   SPRITE_MASK_NONE,     0x00, { COIN_NONE } },
	{ "dimahoo",  0xfb0000,  512,  96, 304, 320, CPS2_KLUDGE_DIMAHOO, 0,   SPRITE_MASK_DIMAHOO,  0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "1944",     0x1390000, 512,  80, 288, 352, 0,                   0,   SPRITE_MASK_NONE,     0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ "progear",  0xfe0000,  464, 224, 224, 320, 0,                   0,   SPRITE_MASK_PROGEAR,  0x05, { COIN_2P2C, COIN_2P2C, COIN_2P1C } },
	{ NULL }
};

struct driver_t *driver;


/******************************************************************************
	ローカル変数
******************************************************************************/

static int z80_bank;
static int readpaddle;
static int next_update_first_line;


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	Z80 ROMバンク切り替え
--------------------------------------------------------*/

void z80_set_bank(u32 offset)
{
	if (offset != z80_bank)
	{
		z80_bank = offset;
		memcpy(&memory_region_cpu2[0x8000], &memory_region_cpu2[offset], 0x4000);
	}
}


/******************************************************************************
	コールバック関数
******************************************************************************/

/*--------------------------------------------------------
	M68000割り込み
--------------------------------------------------------*/

TIMER_CALLBACK( cps2_raster_interrupt )
{
	int scanline = param & 0xffff;
	int reg = (param >> 16) ? 0x52/2 : 0x50/2;

	cps1_output[reg] = 0;
	m68000_set_irq_line(4, HOLD_LINE);

	if (!skip_this_frame())
	{
		if (scanline >= FIRST_VISIBLE_LINE && next_update_first_line <= LAST_VISIBLE_LINE)
		{
			int line = scanline;

			if (line > LAST_VISIBLE_LINE) line = LAST_VISIBLE_LINE;

			cps2_screenrefresh(next_update_first_line, line);
		}
	}

	next_update_first_line = scanline + 1;
}

TIMER_CALLBACK( cps2_vblank_interrupt )
{
	cps1_output[0x50/2] = scanline1;
	cps1_output[0x52/2] = scanline2;
	m68000_set_irq_line(2, HOLD_LINE);

	if (!skip_this_frame())
	{
		if (next_update_first_line <= LAST_VISIBLE_LINE)
		{
			cps2_screenrefresh(next_update_first_line, LAST_VISIBLE_LINE);
		}
		blit_finish();
	}

	cps2_objram_latch();
	next_update_first_line = FIRST_VISIBLE_LINE;
}


/******************************************************************************
	メモリハンドラ
******************************************************************************/

/*--------------------------------------------------------
	入力ポートリード
--------------------------------------------------------*/

READ16_HANDLER( cps2_inputport0_r )
{
	if (machine_input_type == INPTYPE_puzloop2)
	{
		if (!readpaddle) return cps2_port_value[3];
	}
	return cps2_port_value[0];
}

READ16_HANDLER( cps2_inputport1_r )
{
	return cps2_port_value[1];
}


/*--------------------------------------------------------
	Q-Sound
--------------------------------------------------------*/

READ16_HANDLER( qsound_sharedram1_r )
{
	offset &= 0xfff;
	return qsound_sharedram1[offset] | 0xff00;
}

WRITE16_HANDLER( qsound_sharedram1_w )
{
	if (ACCESSING_LSB)
	{
		offset &= 0xfff;
		qsound_sharedram1[offset] = data;
	}
}

WRITE8_HANDLER( qsound_banksw_w )
{
	/*
		Z80 bank register for music note data. It's odd that it isn't encrypted
		though.
	*/
	int bankaddress = 0x10000 + ((data & 0x0f) << 14);

	if (bankaddress >= memory_length_cpu2)
		bankaddress = 0x10000;

	z80_set_bank(bankaddress);
}

READ16_HANDLER( cps2_qsound_volume_r )
{
	/* Extra adapter memory (0x660000-0x663fff) available when bit 14 = 0 */
	/* Network adapter (ssf2tb) present when bit 15 = 0 */
	/* Only game known to use both these so far is SSF2TB */

	if (strncmp(game_name, "ssf2tb", 6) == 0)
		return 0x2021;
	else
		return 0xe021;
}


/********************************************************************
*
*  EEPROM
*  ======
*
*   The EEPROM is accessed by a serial protocol using the register
*   0xf1c006
*
********************************************************************/

static struct EEPROM_interface cps2_eeprom_interface =
{
	6,		/* address bits */
	16,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111"	/* erase command */
};


static void cps2_nvram_read_write(int read_or_write)
{
	char path[MAX_PATH];
	FILE *fp;

	sprintf(path, "%snvram/%s.nv", launchDir, game_name);

	if (read_or_write)
	{
		if ((fp = fopen(path, "wb")) != NULL)
		{
			EEPROM_save(fp);
			fclose(fp);
		}
	}
	else
	{
		if ((fp = fopen(path, "rb")) != NULL)
		{
			EEPROM_load(fp);
			fclose(fp);
		}
	}
}


READ16_HANDLER( cps2_eeprom_port_r )
{
	return (cps2_port_value[2] & 0xfffe) | EEPROM_read_bit();
}


WRITE16_HANDLER( cps2_eeprom_port_w )
{
	if (ACCESSING_MSB)
	{
		/* bit 0 - Unused */
		/* bit 1 - Unused */
		/* bit 2 - Unused */
		/* bit 3 - Unused? */
		/* bit 4 - Eeprom data  */
		/* bit 5 - Eeprom clock */
		/* bit 6 - */
		/* bit 7 - */

		/* EEPROM */
		EEPROM_write_bit(data & 0x1000);
		EEPROM_set_clock_line((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE);
		EEPROM_set_cs_line((data & 0x4000) ? CLEAR_LINE : ASSERT_LINE);
	}

	if (ACCESSING_LSB)
	{
		/* bit 0 - coin counter 1 */
		/* bit 0 - coin counter 2 */
		/* bit 2 - Unused */
		/* bit 3 - Allows access to Z80 address space (Z80 reset) */
		/* bit 4 - lock 1  */
		/* bit 5 - lock 2  */
		/* bit 6 - */
		/* bit 7 - */

		/* Z80 Reset */
		z80_set_reset_line((data & 0x0008) ? CLEAR_LINE : ASSERT_LINE);

		coin_counter_w(0, data & 0x0001);
		if (driver->kludge == CPS2_KLUDGE_PUZLOOP2)	// Puzz Loop 2 uses coin counter 2 input to switch between stick and paddle controls
			readpaddle = data & 0x0002;
		else
			coin_counter_w(1, data & 0x0002);

		if (driver->kludge == CPS2_KLUDGE_MMATRIX)	// Mars Matrix seems to require the coin lockout bit to be reversed
		{
			coin_lockout_w(0, data & 0x0010);
			coin_lockout_w(1, data & 0x0020);
			coin_lockout_w(2, data & 0x0040);
			coin_lockout_w(3, data & 0x0080);
		}
		else
		{
			coin_lockout_w(0, ~data & 0x0010);
			coin_lockout_w(1, ~data & 0x0020);
			coin_lockout_w(2, ~data & 0x0040);
			coin_lockout_w(3, ~data & 0x0080);
		}
	}
}


/******************************************************************************
	ドライバインタフェース
******************************************************************************/

void cps2_driver_init(void)
{
	int i, length = memory_length_user1;
	u16 *rom = (u16 *)memory_region_cpu1;
	u16 *xor = (u16 *)memory_region_user1;

	for (i = 0; i < length/2; i++)
		xor[i] ^= rom[i];

	m68000_init();
	m68000_set_encrypted_range(0, length - 1, xor);

	z80_init();
	z80_bank = -1;

	EEPROM_init(&cps2_eeprom_interface);
	cps2_nvram_read_write(0);

	if (!strcmp(driver->name, "sfa3"))
		EEPROM_write_data(0x75, 0x04);
}


void cps2_driver_exit(void)
{
	cps2_nvram_read_write(1);
}


void cps2_driver_reset(void)
{
	m68000_reset();
	z80_reset();

	coin_counter_reset();

	next_update_first_line = FIRST_VISIBLE_LINE;
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( driver )
{
	state_save_long(&z80_bank, 1);
}

STATE_LOAD( driver )
{
	int bank;

	state_load_long(&bank, 1);

	z80_bank = -1;
	z80_set_bank(bank);

	next_update_first_line = FIRST_VISIBLE_LINE;
}

#endif /* SAVE_STATE */
