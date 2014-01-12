/******************************************************************************

	menu/mvs.c

	PSP メニュー (NEOGEO)

******************************************************************************/

#if defined(INCLUDE_GAMECFG_STRUCT)

/*-----------------------------------------------------------------------------
	gamecfg 構造体
-----------------------------------------------------------------------------*/

gamecfg_t gamecfg_neogeo[] =
{
	{ "Machine Region", &neogeo_region,        CFG_RESTART,  0, 3,  { "Default","Japan","USA","Europe" } },
	{ "Machine Mode",   &neogeo_machine_mode,  CFG_RESTART,  0, 2,  { "Default","Console (AES)","Arcade (MVS)" } },
	MENU_BLANK,
	{ "Raster Effects", &neogeo_raster_enable, CFG_RESTART,  0, 1,  { "Off", "On" } },
	MENU_BLANK,
	{ "Stretch Screen", &option_stretch,       CFG_CONTINUE, 0, 3,  { "Off", "Zoom (4:3)", "Zoom (14:9)", "Zoom (16:9)" } },
	{ "Video sync",     &option_vsync,         CFG_CONTINUE, 0, 1,  { "Off", "On" } },
	{ "Auto frameskip", &option_autoframeskip, CFG_CONTINUE, 0, 1,  { "Disable", "Enable" } },
	{ "Frameskip",      &option_frameskip,     CFG_CONTINUE, 0, 11, { "0","1","2","3","4","5","6","7","8","9","10","11" } },
	{ "Show FPS",       &option_showfps,       CFG_CONTINUE, 0, 1,  { "Off", "On" } },
	{ "60fps limit",    &option_speedlimit,    CFG_CONTINUE, 0, 1,  { "Off","On" } },
	MENU_BLANK,
	{ "Enable Sound",   &option_sound_enable,  CFG_RESTART,  0, 1,  { "No","Yes" } },
	{ "Sample Rate",    &option_samplerate,    CFG_RESTART,  0, 2,  {"11025Hz","22050Hz","44100Hz"} },
	{ "Sound Volume",   &option_sound_volume,  CFG_CONTINUE, 0, 10, {"0%","10%","20%","30%","40%","50%","60%","70%","80%","90%","100%"} },
	MENU_BLANK,
	{ "Controller",     &option_controller,    CFG_CONTINUE, 0, 1,  {"Player 1","Player 2"} },
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

	gamecfg = gamecfg_neogeo;

#elif defined(INCLUDE_KEYCFG_STRUCT)

/*-----------------------------------------------------------------------------
	keycfg 構造体
-----------------------------------------------------------------------------*/

static keycfg_menu_t keycfg_default[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP       },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN     },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT     },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT    },
	{ "Button A",           KEYCFG_BUTTON, 0, P1_BUTTONA  },
	{ "Button B",           KEYCFG_BUTTON, 0, P1_BUTTONB  },
	{ "Button C",           KEYCFG_BUTTON, 0, P1_BUTTONC  },
	{ "Button D",           KEYCFG_BUTTON, 0, P1_BUTTOND  },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START    },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN     },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Test Switch",        KEYCFG_BUTTON, 0, TEST_SWITCH },
	MENU_BLANK,
	{ "Autofire A",         KEYCFG_BUTTON, 0, P1_AF_A     },
	{ "Autofire B",         KEYCFG_BUTTON, 0, P1_AF_B     },
	{ "Autofire C",         KEYCFG_BUTTON, 0, P1_AF_C     },
	{ "Autofire D",         KEYCFG_BUTTON, 0, P1_AF_D     },
	{ "Autofire Interval",  KEYCFG_NUMBER, 0, 0           },
	MENU_BLANK,
	{ "Hotkey A+B",         KEYCFG_BUTTON, 0, P1_AB       },
	{ "Hotkey A+C",         KEYCFG_BUTTON, 0, P1_AC       },
	{ "Hotkey A+D",         KEYCFG_BUTTON, 0, P1_AD       },
	{ "Hotkey B+C",         KEYCFG_BUTTON, 0, P1_BC       },
	{ "Hotkey B+D",         KEYCFG_BUTTON, 0, P1_BD       },
	{ "Hotkey C+D",         KEYCFG_BUTTON, 0, P1_CD       },
	{ "Hotkey A+B+C",       KEYCFG_BUTTON, 0, P1_ABC      },
	{ "Hotkey A+B+D",       KEYCFG_BUTTON, 0, P1_ABD      },
	{ "Hotkey A+C+D",       KEYCFG_BUTTON, 0, P1_ACD      },
	{ "Hotkey B+C+D",       KEYCFG_BUTTON, 0, P1_BCD      },
	{ "Hotkey A+B+C+D",     KEYCFG_BUTTON, 0, P1_ABCD     },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT    },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER    },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_irrmaze[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP       },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN     },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT     },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT    },
	{ "Button A",           KEYCFG_BUTTON, 0, P1_BUTTONA  },
	{ "Button B",           KEYCFG_BUTTON, 0, P1_BUTTONB  },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START    },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN     },
	MENU_BLANK,
	{ "Analog Sensitivity", KEYCFG_ANALOG, 0, 0           },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Test Switch",        KEYCFG_BUTTON, 0, TEST_SWITCH },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT    },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER    },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_popbounc[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP       },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN     },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT     },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT    },
	{ "Button A",           KEYCFG_BUTTON, 0, P1_BUTTONA  },
	{ "Button B",           KEYCFG_BUTTON, 0, P1_BUTTONB  },
	{ "Button C",           KEYCFG_BUTTON, 0, P1_BUTTONC  },
	{ "Start",              KEYCFG_BUTTON, 0, P1_START    },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN     },
	MENU_BLANK,
	{ "Analog Sensitivity", KEYCFG_ANALOG, 0, 0           },
	MENU_BLANK,
	{ "Service Coin",       KEYCFG_BUTTON, 0, SERV_COIN   },
	{ "Test Switch",        KEYCFG_BUTTON, 0, TEST_SWITCH },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT    },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER    },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_vliner[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP       },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN     },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT     },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT    },
	{ "Big",                KEYCFG_BUTTON, 0, P1_BUTTONA  },
	{ "Small",              KEYCFG_BUTTON, 0, P1_BUTTONB  },
	{ "Double Up",          KEYCFG_BUTTON, 0, P1_BUTTONC  },
	{ "Start/Collect",      KEYCFG_BUTTON, 0, P1_BUTTOND  },
	{ "Payout",             KEYCFG_BUTTON, 0, P1_START    },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN     },
	MENU_BLANK,
	{ "Operator Menu",      KEYCFG_BUTTON, 0, OTHER1      },
	{ "Clear Credit",       KEYCFG_BUTTON, 0, OTHER2      },
	{ "Hopper Out",         KEYCFG_BUTTON, 0, OTHER3      },
	{ "Test Switch",        KEYCFG_BUTTON, 0, TEST_SWITCH },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT    },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

static keycfg_menu_t keycfg_jockeygp[] =
{
	{ FONT_UPARROW,         KEYCFG_BUTTON, 0, P1_UP       },
	{ FONT_DOWNARROW,       KEYCFG_BUTTON, 0, P1_DOWN     },
	{ FONT_LEFTARROW,       KEYCFG_BUTTON, 0, P1_LEFT     },
	{ FONT_RIGHTARROW,      KEYCFG_BUTTON, 0, P1_RIGHT    },
	{ "Bet",                KEYCFG_BUTTON, 0, P1_BUTTONA  },
	{ "Cancel",             KEYCFG_BUTTON, 0, P1_BUTTONB  },
	{ "Bet/Cancel All",     KEYCFG_BUTTON, 0, P1_BUTTONC  },
	{ "Payout",             KEYCFG_BUTTON, 0, P1_BUTTOND  },
	{ "Coin",               KEYCFG_BUTTON, 0, P1_COIN     },
	MENU_BLANK,
	{ "Test Switch",        KEYCFG_BUTTON, 0, TEST_SWITCH },
	MENU_BLANK,
	{ "Save snapshot",      KEYCFG_BUTTON, 0, SNAPSHOT    },
	{ "Switch player",      KEYCFG_BUTTON, 0, SWPLAYER   },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END
};

#elif defined(INCLUDE_KEYCFG_MENU)

/*-----------------------------------------------------------------------------
	keycfg menu 初期化
-----------------------------------------------------------------------------*/

	switch (neogeo_ngh)
	{
	case NGH_irrmaze:  keycfg = keycfg_irrmaze; break;
	case NGH_popbounc: keycfg = keycfg_popbounc; break;
	case NGH_vliner:   keycfg = keycfg_vliner; break;
	case NGH_jockeygp: keycfg = keycfg_jockeygp; break;
	default: keycfg = keycfg_default; break;
	}

#elif defined(INCLUDE_LOAD_DIPSWITCH)

/*-----------------------------------------------------------------------------
	dipswitch menu 初期化
-----------------------------------------------------------------------------*/

	int old_value = neogeo_dipswitch & 0xff;

	dipswitch = load_dipswitch();

#elif defined(INCLUDE_SAVE_DIPSWITCH)

/*-----------------------------------------------------------------------------
	dipswitch menu 終了
-----------------------------------------------------------------------------*/

	save_dipswitch();

	if (neogeo_dipswitch != old_value)
	{
		menu_restart();
		return 1;
	}

#endif
