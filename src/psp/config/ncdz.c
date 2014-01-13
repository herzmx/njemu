/******************************************************************************

	config/ncdz.c

	�A�v���P�[�V�����ݒ�t�@�C���Ǘ� (NEOGEO CDZ)

******************************************************************************/

#if defined(INCLUDE_INIFILENAME)

/******************************************************************************
	ini�t�@�C����
******************************************************************************/

static const char *inifile_name = "ncdzpsp.ini";

#elif defined(INCLUDE_CONFIG_STRUCT)

#define DEFAULT_SAMPLERATE	1	// 22050Hz

/******************************************************************************
	�\����
******************************************************************************/

static cfg_type gamecfg_ncdz[] =
{
	{ CFG_NONE,	"[System Settings]", },
	{ CFG_INT,	"PSPClock",				&psp_cpuclock,	PSPCLOCK_333,	PSPCLOCK_333 },

	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&neogeo_raster_enable,	0,	1	},
	{ CFG_BOOL,	"EnableLoadScreen",		&neogeo_loadscreen,		0,	1	},
	{ CFG_BOOL,	"CDROMSpeedLimit",		&neogeo_cdspeed_limit,	0,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		1,	3	},
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},
	{ CFG_BOOL,	"EnableCDDA",			&option_mp3_enable,		1,	1	},
	{ CFG_INT,	"CDDAVolume",			&option_mp3_volume,		8,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	1	},

	{ CFG_NONE,	"[NEOGEO Settings]", },
	{ CFG_PAD,	"Up",			&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",			&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",			&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",		&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"ButtonA",		&input_map[P1_BUTTONA],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"ButtonB",		&input_map[P1_BUTTONB],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"ButtonC",		&input_map[P1_BUTTONC],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"ButtonD",		&input_map[P1_BUTTOND],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Start",		&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",			&input_map[P1_SELECT],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"AutofireA",	&input_map[P1_AF_A],	0,	0	},
	{ CFG_PAD,	"AutofireB",	&input_map[P1_AF_B],	0,	0	},
	{ CFG_PAD,	"AutofireC",	&input_map[P1_AF_C],	0,	0	},
	{ CFG_PAD,	"AutofireD",	&input_map[P1_AF_D],	0,	0	},
	{ CFG_INT,	"AFInterval",	&af_interval,			1,	10	},

	{ CFG_NONE,	"[Hotkey Settings]", },
	{ CFG_PAD,	"HotkeyeAB",	&input_map[P1_AB],		0,	0	},
	{ CFG_PAD,	"HotkeyeAC",	&input_map[P1_AC],		0,	0	},
	{ CFG_PAD,	"HotkeyeAD",	&input_map[P1_AD],		0,	0	},
	{ CFG_PAD,	"HotkeyeBC",	&input_map[P1_BC],		0,	0	},
	{ CFG_PAD,	"HotkeyeBD",	&input_map[P1_BD],		0,	0	},
	{ CFG_PAD,	"HotkeyeCD",	&input_map[P1_CD],		0,	0	},
	{ CFG_PAD,	"HotkeyeABC",	&input_map[P1_ABC],		0,	0	},
	{ CFG_PAD,	"HotkeyeABD",	&input_map[P1_ABD],		0,	0	},
	{ CFG_PAD,	"HotkeyeACD",	&input_map[P1_ACD],		0,	0	},
	{ CFG_PAD,	"HotkeyeBCD",	&input_map[P1_BCD],		0,	0	},
	{ CFG_PAD,	"HotkeyeABCD",	&input_map[P1_ABCD],	0,	0	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",		&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",	&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

#elif defined(INCLUDE_SETUP_CONFIG_STRUCT)

/******************************************************************************
	config�\���̂̐ݒ�
******************************************************************************/

	gamecfg = gamecfg_ncdz;

#endif
