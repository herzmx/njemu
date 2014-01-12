/******************************************************************************

	menu/cps.c

	PSP メニュー (CPS1/CPS2共通)

******************************************************************************/

#if defined(INCLUDE_GAMECFG_STRUCT)

/*-----------------------------------------------------------------------------
	gamecfg 構造体
-----------------------------------------------------------------------------*/

static gamecfg_t gamecfg_normal[] =
{
	{ "Raster Effects", &cps_raster_enable,    CFG_CONTINUE, 0, 1,  {"Off","On"} },
	MENU_BLANK,
	{ "Stretch Screen", &option_stretch,       CFG_CONTINUE, 0, 3,  {"Off","Zoom (4:3)","Zoom (24:17)","Zoom (16:9)"} },
	{ "Video sync",     &option_vsync,         CFG_CONTINUE, 0, 1,  {"Off","On" } },
	{ "Auto frameskip", &option_autoframeskip, CFG_CONTINUE, 0, 1,  {"Disable","Enable" } },
	{ "Frameskip",      &option_frameskip,     CFG_CONTINUE, 0, 11, {"0","1","2","3","4","5","6","7","8","9","10","11"} },
	{ "Show FPS",       &option_showfps,       CFG_CONTINUE, 0, 1,  {"Off","On"} },
	{ "60fps limit",    &option_speedlimit,    CFG_CONTINUE, 0, 1,  {"Off","On"} },
	MENU_BLANK,
	{ "Enable Sound",   &option_sound_enable,  CFG_RESTART,  0, 1,  {"No","Yes"} },
#if (EMU_SYSTEM == CPS1)
	{ "Sample Rate",    &option_samplerate,    CFG_RESTART,  0, 2,  {"11025Hz","22050Hz","44100Hz"} },
#else
	{ "Sample Rate",    &option_samplerate,    CFG_CONTINUE, 0, 2,  {"11025Hz","22050Hz","44100Hz"} },
#endif
	{ "Sound Volume",   &option_sound_volume,  CFG_CONTINUE, 0, 10, {"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%"} },
	MENU_BLANK,
	{ "Controller",     &option_controller,    CFG_CONTINUE, 0, 3,  {"Player 1","Player 2","Player 3","Player 4"} },
	MENU_BLANK,
	{ "PSP clock",      &psp_cpuclock,         CFG_CONTINUE, 0, 2,  {"222MHz","266MHz","333MHz"} },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static gamecfg_t gamecfg_vertical[] =
{
	{ "Raster Effects", &cps_raster_enable,    CFG_CONTINUE, 0, 1,  {"Off","On"} },
	MENU_BLANK,
	{ "Stretch Screen", &option_stretch,       CFG_CONTINUE, 0, 3,  {"Off","Zoom (4:3)","Zoom (24:17)","Zoom (16:9)"} },
	{ "Rotate Screen",  &cps_rotate_screen,    CFG_CONTINUE, 0, 1,  {"No","Yes"} },
	{ "Video sync",     &option_vsync,         CFG_CONTINUE, 0, 1,  {"Off","On" } },
	{ "Auto frameskip", &option_autoframeskip, CFG_CONTINUE, 0, 1,  {"Disable","Enable" } },
	{ "Frameskip",      &option_frameskip,     CFG_CONTINUE, 0, 11, {"0","1","2","3","4","5","6","7","8","9","10","11"} },
	{ "Show FPS",       &option_showfps,       CFG_CONTINUE, 0, 1,  {"Off","On"} },
	{ "60fps limit",    &option_speedlimit,    CFG_CONTINUE, 0, 1,  {"Off","On"} },
	MENU_BLANK,
	{ "Enable Sound",   &option_sound_enable,  CFG_RESTART,  0, 1,  {"No","Yes"} },
#if (EMU_SYSTEM == CPS1)
	{ "Sample Rate",    &option_samplerate,    CFG_RESTART,  0, 2,  {"11025Hz","22050Hz","44100Hz"} },
#else
	{ "Sample Rate",    &option_samplerate,    CFG_CONTINUE, 0, 2,  {"11025Hz","22050Hz","44100Hz"} },
#endif
	{ "Sound Volume",   &option_sound_volume,  CFG_CONTINUE, 0, 10, {"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%"} },
	MENU_BLANK,
	{ "Controller",     &option_controller,    CFG_CONTINUE, 0, 2,  {"Player 1","Player 2"} },
	MENU_BLANK,
	{ "PSP clock",      &psp_cpuclock,         CFG_CONTINUE, 0, 2,  {"222MHz","266MHz","333MHz"} },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

#elif defined(INCLUDE_GAMECFG_MENU)

/*-----------------------------------------------------------------------------
	gamecfg menu 初期化
-----------------------------------------------------------------------------*/

	if (machine_screen_type)
	{
		gamecfg = gamecfg_vertical;
	}
	else
	{
		gamecfg = gamecfg_normal;
		gamecfg[13].value_max = input_max_players - 1;
	}

	if (option_controller >= input_max_players)
		option_controller = INPUT_PLAYER1;

#elif defined(INCLUDE_KEYCFG_STRUCT)

/*-----------------------------------------------------------------------------
	keycfg 構造体
-----------------------------------------------------------------------------*/

static keycfg_menu_t keycfg_2buttons[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire 2",         KEYCFG_BUTTON, 0, P1_AF_2    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_3buttons[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Button 3",           KEYCFG_BUTTON, 0, P1_BUTTON3 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire 2",         KEYCFG_BUTTON, 0, P1_AF_2    },
	{ "Autofire 3",         KEYCFG_BUTTON, 0, P1_AF_3    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

#if (EMU_SYSTEM == CPS2)
static keycfg_menu_t keycfg_4buttons[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Button 3",           KEYCFG_BUTTON, 0, P1_BUTTON3 },
	{ "Button 4",           KEYCFG_BUTTON, 0, P1_BUTTON4 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire 2",         KEYCFG_BUTTON, 0, P1_AF_2    },
	{ "Autofire 3",         KEYCFG_BUTTON, 0, P1_AF_3    },
	{ "Autofire 4",         KEYCFG_BUTTON, 0, P1_AF_4    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};
#endif

static keycfg_menu_t keycfg_6buttons[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Button 3",           KEYCFG_BUTTON, 0, P1_BUTTON3 },
	{ "Button 4",           KEYCFG_BUTTON, 0, P1_BUTTON4 },
	{ "Button 5",           KEYCFG_BUTTON, 0, P1_BUTTON5 },
	{ "Button 6",           KEYCFG_BUTTON, 0, P1_BUTTON6 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire 2",         KEYCFG_BUTTON, 0, P1_AF_2    },
	{ "Autofire 3",         KEYCFG_BUTTON, 0, P1_AF_3    },
	{ "Autofire 4",         KEYCFG_BUTTON, 0, P1_AF_4    },
	{ "Autofire 5",         KEYCFG_BUTTON, 0, P1_AF_5    },
	{ "Autofire 6",         KEYCFG_BUTTON, 0, P1_AF_6    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_quiz[] =
{
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Button 3",           KEYCFG_BUTTON, 0, P1_BUTTON3 },
	{ "Button 4",           KEYCFG_BUTTON, 0, P1_BUTTON4 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

#if (EMU_SYSTEM == CPS1)
static keycfg_menu_t keycfg_sfzch[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Button 3",           KEYCFG_BUTTON, 0, P1_BUTTON3 },
	{ "Button 4",           KEYCFG_BUTTON, 0, P1_BUTTON4 },
	{ "Button 5",           KEYCFG_BUTTON, 0, P1_BUTTON5 },
	{ "Button 6",           KEYCFG_BUTTON, 0, P1_BUTTON6 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Pause",              KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire 2",         KEYCFG_BUTTON, 0, P1_AF_2    },
	{ "Autofire 3",         KEYCFG_BUTTON, 0, P1_AF_3    },
	{ "Autofire 4",         KEYCFG_BUTTON, 0, P1_AF_4    },
	{ "Autofire 5",         KEYCFG_BUTTON, 0, P1_AF_5    },
	{ "Autofire 6",         KEYCFG_BUTTON, 0, P1_AF_6    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_forgottn[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Dial (Left)",        KEYCFG_BUTTON, 0, P1_DIAL_L  },
	{ "Dial (Right)",       KEYCFG_BUTTON, 0, P1_DIAL_R  },
	{ "Analog Sensitivity", KEYCFG_ANALOG, 0, 0          },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};
#else
static keycfg_menu_t keycfg_progear[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Button 2",           KEYCFG_BUTTON, 0, P1_BUTTON2 },
	{ "Button 3",           KEYCFG_BUTTON, 0, P1_BUTTON3 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "P2 Start",           KEYCFG_BUTTON, 0, P2_START   },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Autofire 1",         KEYCFG_BUTTON, 0, P1_AF_1    },
	{ "Autofire 2",         KEYCFG_BUTTON, 0, P1_AF_2    },
	{ "Autofire 3",         KEYCFG_BUTTON, 0, P1_AF_3    },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0          },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_puzloop2[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP      },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN    },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT    },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT   },
	{ "Button 1",           KEYCFG_BUTTON, 0, P1_BUTTON1 },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START   },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN    },
	MENU_BLANK,
	{ "Paddle (Left)",      KEYCFG_BUTTON, 0, P1_DIAL_L  },
	{ "Paddle (Right)",     KEYCFG_BUTTON, 0, P1_DIAL_R  },
	{ "Analog Sensitivity", KEYCFG_ANALOG, 0, 0          },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Service Switch",     KEYCFG_BUTTON, 0, SERV_SWITCH },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT   },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};
#endif

#elif defined(INCLUDE_KEYCFG_MENU)

/*-----------------------------------------------------------------------------
	keycfg menu 初期化
-----------------------------------------------------------------------------*/

#if (EMU_SYSTEM == CPS1)
	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
		keycfg = keycfg_forgottn;
		break;

	case INPTYPE_dynwar:
	case INPTYPE_ffight:
	case INPTYPE_mtwins:
	case INPTYPE_3wonders:
	case INPTYPE_pnickj:
	case INPTYPE_pang3:
	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
	case INPTYPE_slammast:
		keycfg = keycfg_3buttons;
		break;

	case INPTYPE_sf2:
	case INPTYPE_sf2j:
		keycfg = keycfg_6buttons;
		break;

	case INPTYPE_sfzch:
		keycfg = keycfg_sfzch;
		break;

	case INPTYPE_cworld2j:
	case INPTYPE_qad:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
		keycfg = keycfg_quiz;
		break;

	default:
		keycfg = keycfg_2buttons;
		break;
	}
#else
	switch (machine_input_type)
	{
	case INPTYPE_19xx:
	case INPTYPE_batcir:
		keycfg = keycfg_2buttons;
		break;

	case INPTYPE_cybots:
	case INPTYPE_ddtod:
		keycfg = keycfg_4buttons;
		break;

	case INPTYPE_cps2:
	case INPTYPE_ssf2:
		keycfg = keycfg_6buttons;
		break;

	case INPTYPE_qndream:
		keycfg = keycfg_quiz;
		break;

	case INPTYPE_puzloop2:
		keycfg = keycfg_puzloop2;
		break;

	default:
		keycfg = keycfg_3buttons;
		break;
	}

	if (!strcmp(driver->name, "progear"))
		keycfg = keycfg_progear;
#endif

#elif defined(INCLUDE_LOAD_DIPSWITCH)

/*-----------------------------------------------------------------------------
	dipswitch menu 初期化
-----------------------------------------------------------------------------*/

#if (EMU_SYSTEM == CPS1)
	int old_value1, old_value2, old_value3;

	if ((dipswitch = load_dipswitch(&sx)) == NULL)
	{
		ui_popup("This game has no DIP switches.");
		return 0;
	}

	old_value1 = cps1_dipswitch[DIP_A] & 0xff;
	old_value2 = cps1_dipswitch[DIP_B] & 0xff;
	old_value3 = cps1_dipswitch[DIP_C] & 0xff;
#endif

#elif defined(INCLUDE_SAVE_DIPSWITCH)

/*-----------------------------------------------------------------------------
	dipswitch menu 終了
-----------------------------------------------------------------------------*/

#if (EMU_SYSTEM == CPS1)
	save_dipswitch();

	cps1_dipswitch[DIP_A] &= 0xff;
	cps1_dipswitch[DIP_B] &= 0xff;
	cps1_dipswitch[DIP_C] &= 0xff;

	if (cps1_dipswitch[DIP_A] != old_value1
	||	cps1_dipswitch[DIP_B] != old_value2
	||	cps1_dipswitch[DIP_C] != old_value3)
	{
		menu_restart();
		return 1;
	}
#endif

#endif
