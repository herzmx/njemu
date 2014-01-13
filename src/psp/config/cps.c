/******************************************************************************

	config/cps.c

	�A�v���P�[�V�����ݒ�t�@�C���Ǘ� (CPS1/CPS2����)

******************************************************************************/

#if defined(INCLUDE_INIFILENAME)

/******************************************************************************
	ini�t�@�C����
******************************************************************************/

#if (EMU_SYSTEM == CPS1)
static const char *inifile_name = "cps1psp.ini";
#else
static const char *inifile_name = "cps2psp.ini";
#endif

#elif defined(INCLUDE_CONFIG_STRUCT)

#if (EMU_SYSTEM == CPS1)
#define DEFAULT_SAMPLERATE	0	// 11025Hz
#else
#define DEFAULT_SAMPLERATE	2	// 44100Hz
#endif

/******************************************************************************
	�\����
******************************************************************************/

static cfg_type gamecfg_2buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},
#if (EMU_SYSTEM == CPS1)
	{ CFG_INT,	"DipSwitchA",			&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",			&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",			&cps1_dipswitch[2],		0xff,	0xff	},
#endif

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_2buttons_rot[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"RotateScreen",			&cps_rotate_screen,		1,	1	},
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},
#if (EMU_SYSTEM == CPS1)
	{ CFG_INT,	"DipSwitchA",			&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",			&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",			&cps1_dipswitch[2],		0xff,	0xff	},
#endif

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_3buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},
#if (EMU_SYSTEM == CPS1)
	{ CFG_INT,	"DipSwitchA",			&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",			&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",			&cps1_dipswitch[2],		0xff,	0xff	},
#endif

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

#if (EMU_SYSTEM == CPS2)
static cfg_type gamecfg_3buttons_rot[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"RotateScreen",			&cps_rotate_screen,		1,	1	},
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_4buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};
#endif

static cfg_type gamecfg_6buttons[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button5",				&input_map[P1_BUTTON5],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Button6",				&input_map[P1_BUTTON6],	PSP_CTRL_RTRIGGER,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},
#if (EMU_SYSTEM == CPS1)
	{ CFG_INT,	"DipSwitchA",			&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",			&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",			&cps1_dipswitch[2],		0xff,	0xff	},
#endif

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_PAD,	"Autofire4",			&input_map[P1_AF_4],	0,	0	},
	{ CFG_PAD,	"Autofire5",			&input_map[P1_AF_5],	0,	0	},
	{ CFG_PAD,	"Autofire6",			&input_map[P1_AF_6],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_quiz[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	1	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},
#if (EMU_SYSTEM == CPS1)
	{ CFG_INT,	"DipSwitchA",			&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",			&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",			&cps1_dipswitch[2],		0xff,	0xff	},
#endif

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	PSP_CTRL_LTRIGGER,	0	},

	{ CFG_NONE, NULL, }
};

#if (EMU_SYSTEM == CPS1)
static cfg_type gamecfg_forgottn[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[CPS Analog Input Settings]", },
	{ CFG_PAD,	"DialLeft",				&input_map[P1_DIAL_L],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"DialRight",			&input_map[P1_DIAL_R],	PSP_CTRL_RTRIGGER,	0	},
	{ CFG_INT,	"Sensitivity",			&analog_sensitivity,	1,					2	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},
	{ CFG_INT,	"DipSwitchA",			&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",			&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",			&cps1_dipswitch[2],		0xff,	0xff	},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_sfzch[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"Button4",				&input_map[P1_BUTTON4],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Button5",				&input_map[P1_BUTTON5],	PSP_CTRL_CIRCLE,	0	},
	{ CFG_PAD,	"Button6",				&input_map[P1_BUTTON6],	PSP_CTRL_RTRIGGER,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Pause",				&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",	&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",&input_map[SERV_SWITCH],0,		0		},
	{ CFG_INT,	"DipSwitchA",	&cps1_dipswitch[0],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchB",	&cps1_dipswitch[1],		0xff,	0xff	},
	{ CFG_INT,	"DipSwitchC",	&cps1_dipswitch[2],		0xff,	0xff	},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",	&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",	&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",	&input_map[P1_AF_3],	0,	0	},
	{ CFG_PAD,	"Autofire4",	&input_map[P1_AF_4],	0,	0	},
	{ CFG_PAD,	"Autofire5",	&input_map[P1_AF_5],	0,	0	},
	{ CFG_PAD,	"Autofire6",	&input_map[P1_AF_6],	0,	0	},
	{ CFG_INT,	"AFInterval",	&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",		&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",	&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};
#else
static cfg_type gamecfg_progear[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Button2",				&input_map[P1_BUTTON2],	PSP_CTRL_TRIANGLE,	0	},
	{ CFG_PAD,	"Button3",				&input_map[P1_BUTTON3],	PSP_CTRL_CROSS,		0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Start2",				&input_map[P2_START],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[Autofire Settings]", },
	{ CFG_PAD,	"Autofire1",			&input_map[P1_AF_1],	0,	0	},
	{ CFG_PAD,	"Autofire2",			&input_map[P1_AF_2],	0,	0	},
	{ CFG_PAD,	"Autofire3",			&input_map[P1_AF_3],	0,	0	},
	{ CFG_INT,	"AFInterval",			&af_interval,			1,	10	},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};

static cfg_type gamecfg_pzloop2[] =
{
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"RasterEnable",			&cps_raster_enable,		1,	1	},

	{ CFG_NONE,	"[Video Settings]", },
	{ CFG_INT,	"StretchScreen",		&option_stretch,		2,	3	},
#if (EMU_SYSTEM == CPS2)
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			1,	1	},
#else
	{ CFG_BOOL,	"VideoSync",			&option_vsync,			0,	1	},
#endif
	{ CFG_BOOL,	"AutoFrameSkip",		&option_autoframeskip,	0,	1	},
	{ CFG_INT,	"FrameSkipLevel",		&option_frameskip,		0,	11	},
	{ CFG_BOOL,	"ShowFPS",				&option_showfps,		0,	1	},
	{ CFG_BOOL,	"60FPSLimit",			&option_speedlimit,		1,	1	},

	{ CFG_NONE,	"[Audio Settings]", },
	{ CFG_BOOL,	"EnableSound",			&option_sound_enable,	1,	1	},
	{ CFG_INT,	"SampleRate",			&option_samplerate,		DEFAULT_SAMPLERATE,	2	},
	{ CFG_INT,	"SoundVolume",			&option_sound_volume,	10,	10	},

	{ CFG_NONE,	"[Input Settings]", },
	{ CFG_INT,	"Controller",			&option_controller,		0,	3	},

	{ CFG_NONE,	"[CPS Settings]", },
	{ CFG_PAD,	"Up",					&input_map[P1_UP],		PSP_CTRL_UP,		0	},
	{ CFG_PAD,	"Down",					&input_map[P1_DOWN],	PSP_CTRL_DOWN,		0	},
	{ CFG_PAD,	"Left",					&input_map[P1_LEFT],	PSP_CTRL_LEFT,		0	},
	{ CFG_PAD,	"Right",				&input_map[P1_RIGHT],	PSP_CTRL_RIGHT,		0	},
	{ CFG_PAD,	"Button1",				&input_map[P1_BUTTON1],	PSP_CTRL_SQUARE,	0	},
	{ CFG_PAD,	"Start",				&input_map[P1_START],	PSP_CTRL_START,		0	},
	{ CFG_PAD,	"Coin",					&input_map[P1_COIN],	PSP_CTRL_SELECT,	0	},

	{ CFG_NONE,	"[CPS Analog Input Settings]", },
	{ CFG_PAD,	"PaddleLeft",			&input_map[P1_DIAL_L],	PSP_CTRL_LTRIGGER,	0	},
	{ CFG_PAD,	"PaddleRight",			&input_map[P1_DIAL_R],	PSP_CTRL_RTRIGGER,	0	},
	{ CFG_INT,	"Sensitivity",			&analog_sensitivity,	1,					2	},

	{ CFG_NONE,	"[Switch Settings]", },
	{ CFG_PAD,	"ServiceCoin",			&input_map[SERV_COIN],	0,		0		},
	{ CFG_PAD,	"ServiceSwitch",		&input_map[SERV_SWITCH],0,		0		},

	{ CFG_NONE,	"[System Key Settings]", },
	{ CFG_PAD,	"Snapshot",				&input_map[SNAPSHOT],	0,	0	},
	{ CFG_PAD,	"SwitchPlayer",			&input_map[SWPLAYER],	0,	0	},

	{ CFG_NONE, NULL, }
};
#endif

#elif defined(INCLUDE_SETUP_CONFIG_STRUCT)

/******************************************************************************
	config�\���̂̐ݒ�
******************************************************************************/

#if (EMU_SYSTEM == CPS1)
	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
		gamecfg = gamecfg_forgottn;
		break;

	case INPTYPE_dynwar:
	case INPTYPE_ffight:	// button 3 (cheat)
	case INPTYPE_mtwins:
	case INPTYPE_3wonders:
	case INPTYPE_pnickj:
	case INPTYPE_pang3:
	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
	case INPTYPE_slammast:
#if !RELEASE
	case INPTYPE_knightsh:
#endif
		gamecfg = gamecfg_3buttons;
		break;

	case INPTYPE_sf2:
	case INPTYPE_sf2j:
		gamecfg = gamecfg_6buttons;
		break;

	case INPTYPE_sfzch:
		gamecfg = gamecfg_sfzch;
		break;

	case INPTYPE_cworld2j:
	case INPTYPE_qad:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
		gamecfg = gamecfg_quiz;
		break;

	default:
		if (machine_screen_type)
			gamecfg = gamecfg_2buttons_rot;
		else
			gamecfg = gamecfg_2buttons;
		break;
	}
#else
	switch (machine_input_type)
	{
	case INPTYPE_19xx:
		gamecfg = gamecfg_2buttons_rot;
		break;

	case INPTYPE_daimahoo:
		gamecfg = gamecfg_3buttons_rot;
		break;

	case INPTYPE_batcir:
		gamecfg = gamecfg_2buttons;
		break;

	case INPTYPE_cybots:
	case INPTYPE_ddtod:
		gamecfg = gamecfg_4buttons;
		break;

	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		gamecfg = gamecfg_6buttons;
		break;

	case INPTYPE_qndream:
		gamecfg = gamecfg_quiz;
		break;

	case INPTYPE_pzloop2:
		gamecfg = gamecfg_pzloop2;
		break;

	default:
		gamecfg = gamecfg_3buttons;
		break;
	}

	if (!strcmp(driver->name, "progear"))
		gamecfg = gamecfg_progear;
#endif

#elif defined(INCLUDE_SETUP_DIPSWITCH)

/******************************************************************************
	DIP switch�̐ݒ� (CPS1�̂�)
******************************************************************************/

#if (EMU_SYSTEM == CPS1)
#ifdef ADHOC
	if (adhoc_enable)
	{
		cps1_dipswitch[DIP_A] = 0xff;
		cps1_dipswitch[DIP_B] = 0xff;
		cps1_dipswitch[DIP_C] = 0xff;
	}
#endif

	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
	case INPTYPE_unsquad:
	case INPTYPE_1941:
	case INPTYPE_mercs:
	case INPTYPE_cawing:
	case INPTYPE_nemo:
	case INPTYPE_sf2:
	case INPTYPE_kod:
	case INPTYPE_kodj:
	case INPTYPE_knights:	// Enemy's Attack Frequency
#if !RELEASE
	case INPTYPE_knightsh:
#endif
	case INPTYPE_qtono2:
		cps1_dipswitch[DIP_B] &= ~0x07;
		cps1_dipswitch[DIP_B] |= 0x04;	// Difficulty
		break;

	case INPTYPE_strider:
	case INPTYPE_stridrua:
		cps1_dipswitch[DIP_B] &= ~0xc0;	// Life Loss
		break;

	case INPTYPE_ffight:		// Difficulty 1 / Difficulty 2
	case INPTYPE_captcomm:		// Difficulty 1 / Difficulty 2
	case INPTYPE_varth:			// Difficulty / Bonus Life
	case INPTYPE_qad:			// Difficulty / Wisdom
		cps1_dipswitch[DIP_B] &= ~(0x07 | 0x18);
		cps1_dipswitch[DIP_B] |= (0x04 | 0x10);
		break;

	case INPTYPE_mtwins:	// Difficulty / Lives
		cps1_dipswitch[DIP_B] &= ~(0x07 | 0x38);
		cps1_dipswitch[DIP_B] |= (0x04 | 0x18);
		break;

	case INPTYPE_msword:	// Player's vitality consumption / Stage Select
		cps1_dipswitch[DIP_B] &= ~(0x07 | 0x40);
		cps1_dipswitch[DIP_B] |= (0x04 | 0x00);
		break;

	case INPTYPE_sf2j:		// Difficulty / 2 Players Game
	case INPTYPE_pnickj:	// Difficulty / Unknown
		cps1_dipswitch[DIP_B] &= ~(0x07 | 0x08);
		cps1_dipswitch[DIP_B] |= (0x04 | 0x00);
		break;

	case INPTYPE_3wonders:	// Lives / Difficulty
		cps1_dipswitch[DIP_B] &= ~(0x03 | 0x0c);	// Midnight Wonders
		cps1_dipswitch[DIP_B] |= (0x02 | 0x08);
		cps1_dipswitch[DIP_B] &= ~(0x30 | 0xc0);	// Chariot
		cps1_dipswitch[DIP_B] |= (0x10 | 0x80);
		cps1_dipswitch[DIP_C] &= ~(0x03 | 0x0c);	// Don't Pull
		cps1_dipswitch[DIP_C] |= (0x01 | 0x08);
		break;

	case INPTYPE_cworld2j:
		cps1_dipswitch[DIP_B] &= ~0x07;
		cps1_dipswitch[DIP_B] |= 0x06;	// Difficulty
		break;

	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
		cps1_dipswitch[DIP_B] &= ~0x03;
		cps1_dipswitch[DIP_B] |= 0x02;	// Difficulty
		break;
	}

	switch (machine_input_type)
	{
	case INPTYPE_ghouls:
	case INPTYPE_ghoulsu:
	case INPTYPE_daimakai:
	case INPTYPE_strider:
	case INPTYPE_stridrua:
	case INPTYPE_dynwar:
	case INPTYPE_willow:
	case INPTYPE_qad:
	case INPTYPE_megaman:
	case INPTYPE_pang3:
		break;

	case INPTYPE_forgottn:
	case INPTYPE_cworld2j:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
	case INPTYPE_pnickj:
		cps1_dipswitch[DIP_C] &= ~0x20;	// Demo Sounds = On
		break;

	case INPTYPE_rockmanj:
		cps1_dipswitch[DIP_C] &= ~0x04;	// Allow Continue = On
		break;

	case INPTYPE_wof:
	case INPTYPE_dino:
	case INPTYPE_punisher:
	case INPTYPE_slammast:
		*gamecfg[11].value = 1;		// samplerate = 22KHz
		break;

	default:
		cps1_dipswitch[DIP_C] &= ~0x20;	// Demo Sounds = On
		cps1_dipswitch[DIP_C] &= ~0x40;	// Allow Continue = On
		break;
	}

#ifdef ADHOC
	dip[0] = cps1_dipswitch[DIP_A];
	dip[1] = cps1_dipswitch[DIP_B];
	dip[2] = cps1_dipswitch[DIP_C];
#endif
#endif

#elif defined(INCLUDE_SET_DIPSWITCH_DEFAULT_VALUE)

/******************************************************************************
	DIP switch�������l�ɖ߂� (CPS1�̂�/AdHoc�p)
******************************************************************************/

#if (EMU_SYSTEM == CPS1)
#ifdef ADHOC
	if (adhoc_enable)
	{
		cps1_dipswitch[DIP_A] = dip[0];
		cps1_dipswitch[DIP_B] = dip[1];
		cps1_dipswitch[DIP_C] = dip[2];
	}
#endif
#endif

#endif
