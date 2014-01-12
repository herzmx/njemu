/******************************************************************************

	driver.c

	CPS1 ドライバ

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	グローバル構造体
******************************************************************************/

//                 CPSB ID    multiply protection  priority masks         layer enable masks          ctrl unknwn
#define CPS_B_01 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x68,0x6a,0x6c,0x6e}, {0x02,0x04,0x08,0x30,0x30}, 0x66, 0x70
#define CPS_B_02 0x60,0x0002, 0x00,0x00,0x00,0x00, {0x6a,0x68,0x66,0x64}, {0x02,0x04,0x08,0x00,0x00}, 0x6c, 0x62
#define CPS_B_03 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x6e,0x6c,0x6a,0x68}, {0x20,0x10,0x08,0x00,0x00}, 0x70, 0x66
#define CPS_B_04 0x60,0x0004, 0x00,0x00,0x00,0x00, {0x66,0x70,0x68,0x72}, {0x02,0x0c,0x0c,0x00,0x00}, 0x6e, 0x6a
#define CPS_B_05 0x60,0x0005, 0x00,0x00,0x00,0x00, {0x6a,0x6c,0x6e,0x70}, {0x02,0x08,0x20,0x14,0x14}, 0x68, 0x72
#define CPS_B_11 0x72,0x0401, 0x00,0x00,0x00,0x00, {0x68,0x6a,0x6c,0x6e}, {0x08,0x10,0x20,0x00,0x00}, 0x66, 0x70
#define CPS_B_12 0x60,0x0402, 0x00,0x00,0x00,0x00, {0x6a,0x68,0x66,0x64}, {0x02,0x04,0x08,0x00,0x00}, 0x6c, 0x62
#define CPS_B_13 0x6e,0x0403, 0x00,0x00,0x00,0x00, {0x64,0x66,0x68,0x6a}, {0x20,0x02,0x04,0x00,0x00}, 0x62, 0x6c
#define CPS_B_14 0x5e,0x0404, 0x00,0x00,0x00,0x00, {0x54,0x56,0x58,0x5a}, {0x08,0x20,0x10,0x00,0x00}, 0x52, 0x5c
#define CPS_B_15 0x4e,0x0405, 0x00,0x00,0x00,0x00, {0x44,0x46,0x48,0x4a}, {0x04,0x02,0x20,0x00,0x00}, 0x42, 0x4c
#define CPS_B_16 0x40,0x0406, 0x00,0x00,0x00,0x00, {0x4a,0x48,0x46,0x44}, {0x10,0x0a,0x0a,0x00,0x00}, 0x4c, 0x42
#define CPS_B_17 0x48,0x0407, 0x00,0x00,0x00,0x00, {0x52,0x50,0x4e,0x4c}, {0x08,0x10,0x02,0x00,0x00}, 0x54, 0x4a
#define CPS_B_18 0xd0,0x0408, 0x00,0x00,0x00,0x00, {0xda,0xd8,0xd6,0xd4}, {0x10,0x08,0x02,0x00,0x00}, 0xdc, 0xd2
#define NOBATTRY 0x00,0x0000, 0x40,0x42,0x44,0x46, {0x68,0x6a,0x6c,0x6e}, {0x02,0x04,0x08,0x30,0x30}, 0x66, 0x70
#define BATTRY_1 0x72,0x0800, 0x4e,0x4c,0x4a,0x48, {0x66,0x64,0x62,0x60}, {0x20,0x04,0x08,0x12,0x12}, 0x68, 0x70
#define BATTRY_2 0x00,0x0000, 0x5e,0x5c,0x5a,0x58, {0x6e,0x6c,0x6a,0x68}, {0x30,0x08,0x30,0x00,0x00}, 0x60, 0x70
#define BATTRY_3 0x00,0x0000, 0x46,0x44,0x42,0x40, {0x6e,0x6c,0x6a,0x68}, {0x20,0x12,0x12,0x00,0x00}, 0x60, 0x70
#define BATTRY_4 0x00,0x0000, 0x46,0x44,0x42,0x40, {0x66,0x64,0x62,0x60}, {0x20,0x10,0x02,0x00,0x00}, 0x68, 0x70
#define BATTRY_5 0x00,0x0000, 0x4e,0x4c,0x4a,0x48, {0x6e,0x6c,0x6a,0x68}, {0x20,0x06,0x06,0x00,0x00}, 0x60, 0x70
#define BATTRY_6 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x6e,0x6c,0x6a,0x68}, {0x20,0x14,0x14,0x00,0x00}, 0x60, 0x70
#define BATTRY_7 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x00,0x00,0x00,0x00}, {0x14,0x02,0x14,0x00,0x00}, 0x6c, 0x52
#define QSOUND_1 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x64,0x66,0x68,0x6a}, {0x10,0x08,0x04,0x00,0x00}, 0x62, 0x6c
#define QSOUND_2 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x4c,0x4e,0x40,0x42}, {0x16,0x16,0x16,0x00,0x00}, 0x4a, 0x44
#define QSOUND_3 0x4e,0x0c00, 0x00,0x00,0x00,0x00, {0x54,0x56,0x48,0x4a}, {0x04,0x02,0x20,0x00,0x00}, 0x52, 0x4c
#define QSOUND_4 0x6e,0x0c01, 0x00,0x00,0x00,0x00, {0x40,0x42,0x68,0x6a}, {0x04,0x08,0x10,0x00,0x00}, 0x56, 0x6c
#define QSOUND_5 0x5e,0x0c02, 0x00,0x00,0x00,0x00, {0x6c,0x6e,0x70,0x72}, {0x04,0x08,0x10,0x00,0x00}, 0x6a, 0x5c
#define HACK_B_1 0x00,0x0000, 0x00,0x00,0x00,0x00, {0x52,0x50,0x4e,0x4c}, {0xff,0xff,0xff,0x00,0x00}, 0x54, 0x5c

//                      stars banks  gfx limit  scroll1 limit    scroll2 limit    scroll3 limit
#define GFX_FORGOTTN	1,    0,0,0, 0,        {0xf000,0xffff}, {0x0800,0x1fff}, {0x0800,0x0fff}
#define GFX_GHOULS		0,    0,0,0, 0,        {0x2000,0x3fff}, {0x2000,0x3fff}, {0x1000,0x13ff}
#define GFX_STRIDER		1,    1,0,1, 0,        {0x7000,0x7fff}, {0x2800,0x5fff}, {0x0000,0x0dff}
#define GFX_DYNWAR		0,    0,1,1, 0,        {0x6000,0x7fff}, {0x2000,0x3fff}, {0x0000,0x07ff}
#define GFX_WILLOW		0,    0,1,0, 0x2fffff, {0x7000,0x7fff}, {0x0000,0x1fff}, {0x0a00,0x0dff}
#define GFX_FFIGHT		0,    0,0,0, 0,        {0x4400,0x4bff}, {0x3000,0x3fff}, {0x0980,0x0bff}
#define GFX_1941		0,    0,0,0, 0,        {0x4000,0x47ff}, {0x2400,0x3fff}, {0x0400,0x07ff}
#define GFX_UNSQUAD		0,    0,0,0, 0,        {0x3000,0x3fff}, {0x2000,0x2fff}, {0x0c00,0x0fff}
#define GFX_MERCS		0,    0,0,0, 0x2fffff, {0x0000,0x0bff}, {0x0600,0x1dff}, {0x0780,0x097f}
#define GFX_MSWORD		0,    0,0,0, 0,        {0x4000,0x4fff}, {0x2800,0x37ff}, {0x0e00,0x0fff}
#define GFX_MTWINS		0,    0,0,0, 0,        {0x3000,0x3fff}, {0x2000,0x37ff}, {0x0e00,0x0fff}
#define GFX_NEMO		0,    0,0,0, 0,        {0x4000,0x47ff}, {0x2400,0x33ff}, {0x0d00,0x0fff}
#define GFX_CAWING		0,    0,0,0, 0,        {0x5000,0x57ff}, {0x2c00,0x3fff}, {0x0600,0x09ff}
#define GFX_SF2			0,    2,2,2, 0,        {0x4000,0x4fff}, {0x2800,0x3fff}, {0x0400,0x07ff}
#define GFX_3WONDERS	0,    0,1,0, 0,        {0x5400,0x6fff}, {0x1400,0x7fff}, {0x0e00,0x14ff}
#define GFX_KOD			0,    0,0,0, 0,        {0xc000,0xd7ff}, {0x4800,0x5fff}, {0x1b00,0x1fff}
#define GFX_CAPTCOMM	0,    0,0,0, 0,        {0xc000,0xcfff}, {0x6800,0x7fff}, {0x1400,0x17ff}
#define GFX_KNIGHTS		0,    0,0,0, 0,        {0x8800,0x97ff}, {0x4c00,0x67ff}, {0x1a00,0x1fff}
#define GFX_VARTH		0,    0,0,0, 0,        {0x5800,0x5fff}, {0x1600,0x2bff}, {0x0c00,0x0fff}
#define GFX_CWORLD2J	0,    0,0,0, 0,        {0x7800,0x7fff}, {0x0000,0x37ff}, {0x0e00,0x0eff}
#define GFX_WOF			0,    0,0,0, 0,        {0xd000,0xdfff}, {0x4800,0x67ff}, {0x1c00,0x1fff}
#define GFX_DINO		0,    0,0,0, 0,        {0x0000,0x0fff}, {0x5800,0x6fff}, {0x1c00,0x1fff}
#define GFX_PUNISHER	0,    0,0,0, 0,        {0x0000,0x0fff}, {0x5400,0x6dff}, {0x1b80,0x1fff}
#define GFX_SLAMMAST	0,    0,0,0, 0,        {0x0000,0x0fff}, {0xa000,0xb3ff}, {0x2d00,0x2fff}
#define GFX_SF2HF		0,    2,2,2, 0,        {0x4000,0x4fff}, {0x2800,0x3fff}, {0x0400,0x07ff}
#define GFX_QAD			0,    0,0,0, 0x0fffff, {0x0000,0x07ff}, {0x0400,0x13ff}, {0x0500,0x07ff}
#define GFX_QADJ		0,    0,0,0, 0,        {0x0000,0x07ff}, {0x1000,0x3fff}, {0x0100,0x03ff}
#define GFX_QTONO2		0,    0,0,0, 0x37ffff, {0x0000,0x0fff}, {0x2000,0x6fff}, {0x0200,0x07ff}
#define GFX_MEGAMAN		0,    0,0,0, 0x7effff, {0x0000,0x17ff}, {0xb000,0xcdff}, {0x3400,0x3f7f}
#define GFX_PUNICKJ		0,    0,0,0, 0x1bffff, {0x0000,0x0fff}, {0x0800,0x2fff}, {0x0c00,0x0dff}
#define GFX_PANG3		0,    0,0,0, 0,        {0x0000,0xffff}, {0x0000,0x7fff}, {0x0000,0x1fff}
#define GFX_SFZCH		0,    0,0,0, 0x7effff, {0x0000,0x07ff}, {0xd400,0xf1ff}, {0x3d00,0x3f7f}

struct driver_t CPS1_driver[] =
{
	/* name       CPSB    kludge                gfx type */
	{"forgottn",CPS_B_01, CPS1_KLUDGE_FORGOTTN, GFX_FORGOTTN },
	{"lostwrld",CPS_B_01, CPS1_KLUDGE_FORGOTTN, GFX_FORGOTTN },
	{"ghouls",  CPS_B_01, CPS1_KLUDGE_GHOULS,   GFX_GHOULS   },
	{"ghoulsu", CPS_B_01, CPS1_KLUDGE_GHOULS,   GFX_GHOULS   },
	{"daimakai",CPS_B_01, CPS1_KLUDGE_GHOULS,   GFX_GHOULS   },
	{"strider", CPS_B_01, 0,                    GFX_STRIDER  },
	{"stridrua",CPS_B_01, 0,                    GFX_STRIDER  },
	{"striderj",CPS_B_01, 0,                    GFX_STRIDER  },
	{"stridrja",CPS_B_01, 0,                    GFX_STRIDER  },
	{"dynwar",  CPS_B_02, 0,                    GFX_DYNWAR   },
	{"dynwarj", CPS_B_02, 0,                    GFX_DYNWAR   },
	{"willow",  CPS_B_03, 0,                    GFX_WILLOW   },
	{"willowj", CPS_B_03, 0,                    GFX_WILLOW   },
	{"willowje",CPS_B_03, 0,                    GFX_WILLOW   },
	{"ffight",  CPS_B_04, 0,                    GFX_FFIGHT   },
	{"ffightu", CPS_B_01, 0,                    GFX_FFIGHT   },
	{"ffightua",CPS_B_05, 0,                    GFX_FFIGHT   },
	{"ffightj", CPS_B_04, 0,                    GFX_FFIGHT   },
	{"ffightj1",CPS_B_02, 0,                    GFX_FFIGHT   },
	{"1941",    CPS_B_05, 0,                    GFX_1941     },
	{"1941j",   CPS_B_05, 0,                    GFX_1941     },
	{"unsquad", CPS_B_11, 0,                    GFX_UNSQUAD  },
	{"area88",  CPS_B_11, 0,                    GFX_UNSQUAD  },
	{"mercs",   CPS_B_12, CPS1_KLUDGE_MERCS,    GFX_MERCS    },
	{"mercsu",  CPS_B_12, CPS1_KLUDGE_MERCS,    GFX_MERCS    },
	{"mercsua", CPS_B_12, CPS1_KLUDGE_MERCS,    GFX_MERCS    },
	{"mercsj",  CPS_B_12, CPS1_KLUDGE_MERCS,    GFX_MERCS    },
	{"msword",  CPS_B_13, 0,                    GFX_MSWORD   },
	{"mswordr1",CPS_B_13, 0,                    GFX_MSWORD   },
	{"mswordu", CPS_B_13, 0,                    GFX_MSWORD   },
	{"mswordj", CPS_B_13, 0,                    GFX_MSWORD   },
	{"mtwins",  CPS_B_14, 0,                    GFX_MTWINS   },
	{"chikij",  CPS_B_14, 0,                    GFX_MTWINS   },
	{"nemo",    CPS_B_15, 0,                    GFX_NEMO     },
	{"nemoj",   CPS_B_15, 0,                    GFX_NEMO     },
	{"cawing",  CPS_B_16, 0,                    GFX_CAWING   },
	{"cawingr1",CPS_B_16, 0,                    GFX_CAWING   },
	{"cawingu", CPS_B_16, 0,                    GFX_CAWING   },
	{"cawingj", CPS_B_16, 0,                    GFX_CAWING   },
	{"sf2",     CPS_B_11, 0,                    GFX_SF2      },
	{"sf2eb",   CPS_B_17, 0,                    GFX_SF2      },
	{"sf2ua",   CPS_B_17, 0,                    GFX_SF2      },
	{"sf2ub",   CPS_B_17, 0,                    GFX_SF2      },
	{"sf2ud",   CPS_B_05, 0,                    GFX_SF2      },
	{"sf2ue",   CPS_B_18, 0,                    GFX_SF2      },
	{"sf2uf",   CPS_B_15, 0,                    GFX_SF2      },
	{"sf2ui",   CPS_B_14, 0,                    GFX_SF2      },
	{"sf2uk",   CPS_B_17, 0,                    GFX_SF2      },
	{"sf2j",    CPS_B_13, 0,                    GFX_SF2      },
	{"sf2ja",   CPS_B_17, 0,                    GFX_SF2      },
	{"sf2jc",   CPS_B_12, 0,                    GFX_SF2      },
	{"3wonders",BATTRY_1, CPS1_KLUDGE_3WONDERS, GFX_3WONDERS },
	{"3wonderu",BATTRY_1, CPS1_KLUDGE_3WONDERS, GFX_3WONDERS },
	{"wonder3", BATTRY_1, CPS1_KLUDGE_3WONDERS, GFX_3WONDERS },
	{"kod",     BATTRY_2, 0,                    GFX_KOD      },
	{"kodu",    BATTRY_2, 0,                    GFX_KOD      },
	{"kodj",    BATTRY_2, 0,                    GFX_KOD      },
#if !RELEASE
	{"kodb",    BATTRY_2, 0,                    GFX_KOD      },
#endif
	{"captcomm",BATTRY_3, 0,                    GFX_CAPTCOMM },
	{"captcomu",BATTRY_3, 0,                    GFX_CAPTCOMM },
	{"captcomj",BATTRY_3, 0,                    GFX_CAPTCOMM },
	{"knights", BATTRY_4, 0,                    GFX_KNIGHTS  },
	{"knightsu",BATTRY_4, 0,                    GFX_KNIGHTS  },
	{"knightsj",BATTRY_4, 0,                    GFX_KNIGHTS  },
	{"sf2ce",   NOBATTRY, 0,                    GFX_SF2      },
	{"sf2ceua", NOBATTRY, 0,                    GFX_SF2      },
	{"sf2ceub", NOBATTRY, 0,                    GFX_SF2      },
	{"sf2ceuc", NOBATTRY, 0,                    GFX_SF2      },
	{"sf2cej",  NOBATTRY, 0,                    GFX_SF2      },
#if !RELEASE
	{"sf2rb",   NOBATTRY, 0,                    GFX_SF2      },
	{"sf2rb2",  NOBATTRY, 0,                    GFX_SF2      },
	{"sf2red",  NOBATTRY, 0,                    GFX_SF2      },
	{"sf2v004", NOBATTRY, 0,                    GFX_SF2      },
	{"sf2accp2",NOBATTRY, 0,                    GFX_SF2      },
	{"sf2m1",   NOBATTRY, 0,                    GFX_SF2      },
	{"sf2m2",   NOBATTRY, 0,                    GFX_SF2      },
	{"sf2m3",   NOBATTRY, 0,                    GFX_SF2      },
	{"sf2m4",   HACK_B_1, CPS1_KLUDGE_SF2CEB,   GFX_SF2      },
	{"sf2m5",   NOBATTRY, CPS1_KLUDGE_SF2CEB,   GFX_SF2      },
	{"sf2m6",   NOBATTRY, CPS1_KLUDGE_SF2CEB,   GFX_SF2      },
	{"sf2m7",   NOBATTRY, CPS1_KLUDGE_SF2CEB,   GFX_SF2      },
	{"sf2yyc",  NOBATTRY, CPS1_KLUDGE_SF2CEB,   GFX_SF2      },
	{"sf2koryu",NOBATTRY, CPS1_KLUDGE_SF2CEB,   GFX_SF2      },
#endif
	{"varth",   CPS_B_04, 0,                    GFX_VARTH    },
	{"varthr1", CPS_B_04, 0,                    GFX_VARTH    },
	{"varthu",  CPS_B_04, 0,                    GFX_VARTH    },
	{"varthj",  BATTRY_5, 0,                    GFX_VARTH    },
	{"cworld2j",BATTRY_6, 0,                    GFX_CWORLD2J },
	{"wof",     NOBATTRY, 0,                    GFX_WOF      },
	{"wofa",    NOBATTRY, 0,                    GFX_WOF      },
	{"wofu",    QSOUND_1, 0,                    GFX_WOF      },
	{"wofj",    QSOUND_1, 0,                    GFX_WOF      },
	{"dino",    QSOUND_2, 0,                    GFX_DINO     },
	{"dinou",   QSOUND_2, 0,                    GFX_DINO     },
	{"dinoj",   QSOUND_2, 0,                    GFX_DINO     },
	{"punisher",QSOUND_3, 0,                    GFX_PUNISHER },
	{"punishru",QSOUND_3, 0,                    GFX_PUNISHER },
	{"punishrj",QSOUND_3, 0,                    GFX_PUNISHER },
	{"slammast",QSOUND_4, 0,                    GFX_SLAMMAST },
	{"slammasu",QSOUND_4, 0,                    GFX_SLAMMAST },
	{"mbomberj",QSOUND_4, 0,                    GFX_SLAMMAST },
	{"mbombrd", QSOUND_5, 0,                    GFX_SLAMMAST },
	{"mbombrdj",QSOUND_5, 0,                    GFX_SLAMMAST },
	{"sf2hf",   NOBATTRY, 0,                    GFX_SF2HF    },
	{"sf2t",    NOBATTRY, 0,                    GFX_SF2HF    },
	{"sf2tj",   NOBATTRY, 0,                    GFX_SF2HF    },
	{"qad",     BATTRY_7, 0,                    GFX_QAD      },
	{"qadj",    NOBATTRY, 0,                    GFX_QADJ     },
	{"qtono2",  NOBATTRY, 0,                    GFX_QTONO2   },
	{"megaman", NOBATTRY, 0,                    GFX_MEGAMAN  },
	{"rockmanj",NOBATTRY, 0,                    GFX_MEGAMAN  },
	{"pnickj",  NOBATTRY, 0,                    GFX_PUNICKJ  },
	{"pang3",   NOBATTRY, CPS1_KLUDGE_PANG3,    GFX_PANG3    },
	{"pang3j",  NOBATTRY, CPS1_KLUDGE_PANG3,    GFX_PANG3    },
	{"sfzch",   NOBATTRY, 0,                    GFX_SFZCH    },
	{NULL}
};

struct driver_t *driver;


/******************************************************************************
	ローカル変数
******************************************************************************/

static u32 z80_bank;

static int cps1_sound_fade_timer;
static u8 sound_data;


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	Z80 ROMバンク切り替え
--------------------------------------------------------*/

static void z80_set_bank(u32 offset)
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

TIMER_CALLBACK( cps1_vblank_interrupt )
{
	m68000_set_irq_line(2, HOLD_LINE);
	if (!skip_this_frame())
	{
		cps1_screenrefresh();
		blit_finish();
	}
	cps1_objram_latch();
}


/******************************************************************************
	メモリハンドラ
******************************************************************************/

/*--------------------------------------------------------
	コインコントロール
--------------------------------------------------------*/

WRITE16_HANDLER( cps1_coinctrl_w )
{
	if (ACCESSING_MSB)
	{
		coin_counter_w(0,  data & 0x0100);
		coin_counter_w(1,  data & 0x0200);
		coin_lockout_w(0, ~data & 0x0400);
		coin_lockout_w(1, ~data & 0x0800);
	}
#if 0
	if (ACCESSING_LSB)
	{
		/* mercs sets bit 0 */
		set_led_status(0, data & 0x02);
		set_led_status(1, data & 0x04);
		set_led_status(2, data & 0x08);
	}
#endif
}

WRITE16_HANDLER( cps1_coinctrl2_w )
{
	if (ACCESSING_LSB)
	{
		coin_counter_w(2,  data & 0x01);
		coin_lockout_w(2, ~data & 0x02);
		coin_counter_w(3,  data & 0x04);
		coin_lockout_w(3, ~data & 0x08);
	}
}


/*--------------------------------------------------------
	ディップスイッチリード
--------------------------------------------------------*/

READ16_HANDLER( cps1_dsw_a_r )
{
	return cps1_dipswitch[DIP_A] | (cps1_dipswitch[DIP_A] << 8);
}

READ16_HANDLER( cps1_dsw_b_r )
{
	return cps1_dipswitch[DIP_B] | (cps1_dipswitch[DIP_B] << 8);
}

READ16_HANDLER( cps1_dsw_c_r )
{
	return cps1_dipswitch[DIP_C] | (cps1_dipswitch[DIP_C] << 8);
}


/*--------------------------------------------------------
	入力ポートリード
--------------------------------------------------------*/

READ16_HANDLER( cps1_inputport0_r )
{
	return cps1_port_value[0];
}

READ16_HANDLER( cps1_inputport1_r )
{
	return cps1_port_value[1];
}

READ16_HANDLER( cps1_inputport2_r )
{
	return cps1_port_value[2];
}

READ16_HANDLER( cps1_inputport3_r )
{
	return cps1_port_value[3];
}


/*--------------------------------------------------------
	ロストワールド用 ダイアル入力
--------------------------------------------------------*/

static int dial[2];

READ16_HANDLER( forgottn_dial_0_r )
{
	return ((forgottn_read_dial0() - dial[0]) >> (offset << 3)) & 0xff;
}

READ16_HANDLER( forgottn_dial_1_r )
{
	return ((forgottn_read_dial1() - dial[1]) >> (offset << 3)) & 0xff;
}

WRITE16_HANDLER( forgottn_dial_0_reset_w )
{
	dial[0] = forgottn_read_dial0();
}

WRITE16_HANDLER( forgottn_dial_1_reset_w )
{
	dial[1] = forgottn_read_dial1();
}


/*--------------------------------------------------------
	Sound (YM2151 + OKIM6295)
--------------------------------------------------------*/

WRITE8_HANDLER( cps1_snd_bankswitch_w )
{
	int length = memory_length_cpu2 - 0x10000;
	int bankaddress = (data * 0x4000) & (length - 1);

	z80_set_bank(bankaddress + 0x10000);
}

WRITE8_HANDLER( cps1_oki_pin7_w )
{
	OKIM6295_set_pin7_w(0, (data & 1));
}

READ8_HANDLER( cps1_sound_fade_timer_r )
{
	return cps1_sound_fade_timer;
}

WRITE16_HANDLER( cps1_sound_fade_timer_w )
{
	if (ACCESSING_LSB)
		cps1_sound_fade_timer = data & 0xff;
}

READ8_HANDLER( cps1_sound_command_r )
{
	// Z80
	return sound_data;
}

WRITE16_HANDLER( cps1_sound_command_w )
{
	// M68000
	if (ACCESSING_LSB) sound_data = data & 0xff;
}

void cps1_sound_interrupt(int state)
{
	z80_set_irq_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


/*--------------------------------------------------------
	Sound (Q Sound)
--------------------------------------------------------*/

READ16_HANDLER( qsound_rom_r )
{
	if (!memory_region_user1) return 0;

	offset &= 0x7fff;
	return memory_region_user1[offset] | 0xff00;
}

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

READ16_HANDLER( qsound_sharedram2_r )
{
	offset &= 0xfff;
	return qsound_sharedram2[offset] | 0xff00;
}

WRITE16_HANDLER( qsound_sharedram2_w )
{
	if (ACCESSING_LSB)
	{
		offset &= 0xfff;
		qsound_sharedram2[offset] = data;
	}
}

WRITE8_HANDLER( qsound_banksw_w )
{
	/*
		Z80 bank register for music note data. It's odd that it isn't encrypted
		though.
	*/
	u32 bankaddress = 0x10000 + ((data & 0x0f) << 14);

	if (bankaddress >= memory_length_cpu2)
		bankaddress = 0x10000;

	z80_set_bank(bankaddress);
}


/*--------------------------------------------------------
	EEPROM
--------------------------------------------------------*/

static struct EEPROM_interface qsound_eeprom_interface =
{
	7,		/* address bits */
	8,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111"	/* erase command */
};

static struct EEPROM_interface pang3_eeprom_interface =
{
	6,		/* address bits */
	16,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111"	/* erase command */
};

static void cps1_nvram_read_write(int read_or_write)
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

READ16_HANDLER( cps1_eeprom_port_r )
{
	return EEPROM_read_bit();
}

WRITE16_HANDLER( cps1_eeprom_port_w )
{
	if (ACCESSING_LSB)
	{
		/*
			bit 0 = data
			bit 6 = clock
			bit 7 = cs
		*/
		EEPROM_write_bit(data & 0x01);
		EEPROM_set_cs_line((data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
		EEPROM_set_clock_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}


/******************************************************************************
	Pang!3 (Japan) ROM暗号化解除
******************************************************************************/

void pang3_decode(void)
{
	u16 *rom = (u16 *)memory_region_cpu1;
	int addr, src, dst;

	for (addr = 0x80000; addr < 0x100000; addr += 2)
	{
		/* only the low 8 bits of each word are encrypted */
		src = rom[addr / 2];
		dst = src & 0xff00;
		if ( src & 0x01) dst ^= 0x04;
		if ( src & 0x02) dst ^= 0x21;
		if ( src & 0x04) dst ^= 0x01;
		if (~src & 0x08) dst ^= 0x50;
		if ( src & 0x10) dst ^= 0x40;
		if ( src & 0x20) dst ^= 0x06;
		if ( src & 0x40) dst ^= 0x08;
		if (~src & 0x80) dst ^= 0x88;
		rom[addr / 2] = dst;
	}
}


/******************************************************************************
	ドライバインタフェース
******************************************************************************/

int cps1_driver_init(void)
{
	m68000_init();

	z80_init();
	z80_bank = -1;

	if (machine_driver_type == MACHINE_qsound)
	{
		EEPROM_init(&qsound_eeprom_interface);
	}
	else
	{
		EEPROM_init(&pang3_eeprom_interface);
	}
	if (machine_driver_type == MACHINE_qsound
	||	machine_driver_type == MACHINE_pang3)
	{
#ifdef ADHOC
		if (adhoc_enable)
		{
			if (adhoc_server)
			{
				msg_printf(TEXT(SENDING_EEPROM_DATA));
				return adhoc_send_eeprom();
			}
			else
			{
				msg_printf(TEXT(RECIEVING_EEPROM_DATA));
				return adhoc_recv_eeprom();
			}
		}
		else
#endif
		{
			cps1_nvram_read_write(0);
		}
	}

	return 1;
}


void cps1_driver_exit(void)
{
#ifdef ADHOC
	if (!adhoc_enable || adhoc_server)
#endif
	{
		if (machine_driver_type == MACHINE_qsound
		||	machine_driver_type == MACHINE_pang3)
		{
			cps1_nvram_read_write(1);
		}
	}
}


void cps1_driver_reset(void)
{
	m68000_reset();
	z80_reset();

	coin_counter_reset();

	sound_data = 0x00;
	cps1_sound_fade_timer = 0;
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( driver )
{
	state_save_long(&z80_bank, 1);
	state_save_long(&sound_data, 1);
	state_save_long(&cps1_sound_fade_timer, 1);
}

STATE_LOAD( driver )
{
	int bank;

	state_load_long(&bank, 1);
	state_load_long(&sound_data, 1);
	state_load_long(&cps1_sound_fade_timer, 1);

	z80_bank = 0xffffffff;
	z80_set_bank(bank);

	coin_counter_reset();
}

#endif /* SAVE_STATE */
