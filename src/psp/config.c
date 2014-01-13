/******************************************************************************

	config.c

	�A�v���P�[�V�����ݒ�t�@�C���Ǘ�

******************************************************************************/

#include "psp.h"
#include "emumain.h"

#define LINEBUF_SIZE	256


enum
{
	CFG_NONE = 0,
	CFG_INT,
	CFG_BOOL,
	CFG_PAD,
	CFG_STR
};

enum
{
	PAD_NONE = 0,
	PAD_UP,
	PAD_DOWN,
	PAD_LEFT,
	PAD_RIGHT,
	PAD_CIRCLE,
	PAD_CROSS,
	PAD_SQUARE,
	PAD_TRIANGLE,
	PAD_LTRIGGER,
	PAD_RTRIGGER,
	PAD_START,
	PAD_SELECT,
	PAD_MAX
};

typedef struct cfg_t
{
	int type;
	const char *name;
	int *value;
	int def;
	int max;
} cfg_type;

typedef struct cfg2_t
{
	int type;
	const char *name;
	char *value;
	int max_len;
} cfg2_type;


/******************************************************************************
	���[�J���\����/�ϐ�
******************************************************************************/

static int ini_version;

#define INCLUDE_INIVERSION

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "config/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "config/mvs.c"
#endif

#undef INCLUDE_INIVERSION

static cfg_type default_options[] =
{
	{ CFG_NONE,	"[System Settings]", },
	{ CFG_INT,	"INIFileVersion",	&ini_version,		INIVERSION,		INIVERSION		},
	{ CFG_INT,	"PSPClock",			&psp_cpuclock,		PSPCLOCK_333,	PSPCLOCK_333	},
	{ CFG_INT,	"PSPRefreshRate",	&psp_refreshrate,	0,				60000000		},
	{ CFG_INT,	"PSPSampleRate",	&psp_samplerate,	0,				44100			},
#if (EMU_SYSTEM == MVS)
	{ CFG_NONE,	"[Emulation Settings]", },
	{ CFG_INT,	"NeogeoBIOS",			&neogeo_bios, -1, BIOS_MAX-1 },
#endif
	{ CFG_NONE, NULL, }
};

static cfg2_type default_options2[] =
{
	{ CFG_NONE,	"[Directory Settings]", 				},
	{ CFG_STR,	"StartupDir", startupDir,	MAX_PATH	},
	{ CFG_NONE, NULL, }
};

#define INCLUDE_CONFIG_STRUCT

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "config/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "config/mvs.c"
#endif

#undef INCLUDE_CONFIG_STRUCT

typedef struct padname_t
{
	int code;
	const char name[16];
} PADNAME;

static const PADNAME pad_name[13] =
{
	{ 0,					"PAD_NONE"		},
	{ PSP_CTRL_UP,			"PAD_UP"		},
	{ PSP_CTRL_DOWN,		"PAD_DOWN"		},
	{ PSP_CTRL_LEFT,		"PAD_LEFT"		},
	{ PSP_CTRL_RIGHT,		"PAD_RIGHT"		},
	{ PSP_CTRL_CROSS,		"PAD_CROSS"		},
	{ PSP_CTRL_CIRCLE,		"PAD_CIRCLE"	},
	{ PSP_CTRL_SQUARE,		"PAD_SQUARE"	},
	{ PSP_CTRL_TRIANGLE,	"PAD_TRIANGLE"	},
	{ PSP_CTRL_START,		"PAD_START"		},
	{ PSP_CTRL_SELECT,		"PAD_SELECT"	},
	{ PSP_CTRL_LTRIGGER,	"PAD_LTRIGGER"	},
	{ PSP_CTRL_RTRIGGER,	"PAD_RTRIGGER"	}
};


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*------------------------------------------------------
	CFG_BOOL�̒l��ǂݍ���
------------------------------------------------------*/

static int get_config_bool(char *str)
{
	if (!stricmp(str, "yes"))
		return 1;
	else
		return 0;
}


/*------------------------------------------------------
	CFG_INT�̒l��ǂݍ���
------------------------------------------------------*/

static int get_config_int(char *str, int maxval)
{
	int value = atoi(str);

	if (value < 0) value = 0;
	if (value > maxval) value = maxval;
	return value;
}


/*------------------------------------------------------
	CFG_PAD�̒l��ǂݍ���
------------------------------------------------------*/

static int get_config_pad(char *str)
{
	int i;

	for (i = 0; i < PAD_MAX; i++)
	{
		if (strcmp(str, pad_name[i].name) == 0)
			return pad_name[i].code;
	}

	return pad_name[PAD_NONE].code;
}


/*------------------------------------------------------
	CFG_BOOL�̒l��ۑ�����
------------------------------------------------------*/

static const char *set_config_bool(int value)
{
	if (value)
		return "yes";
	else
		return "no";
}


/*------------------------------------------------------
	CFG_INT�̒l��ۑ�����
------------------------------------------------------*/

static char *set_config_int(int value, int maxval)
{
	static char buf[16];

	if (value < 0) value = 0;
	if (value > maxval) value = maxval;

	sprintf(buf, "%d", value);

	return buf;
}


/*------------------------------------------------------
	CFG_PAD�̒l��ۑ�����
------------------------------------------------------*/

static const char *set_config_pad(int value)
{
	int i;

	for (i = 0; i < PAD_MAX; i++)
	{
		if (value == pad_name[i].code)
			return pad_name[i].name;
	}

	return pad_name[PAD_NONE].name;
}


/*------------------------------------------------------
	.ini�t�@�C������ݒ��ǂݍ���
------------------------------------------------------*/

static int load_inifile(const char *path, cfg_type *cfg, cfg2_type *cfg2)
{
	FILE *fp;

	if ((fp = fopen(path, "r")) != NULL)
	{
		int i;
		char linebuf[LINEBUF_SIZE];

		while (1)
		{
			char *name, *value;

			memset(linebuf, LINEBUF_SIZE, 0);
			if (fgets(linebuf, LINEBUF_SIZE - 1, fp) == NULL)
				break;

			if (linebuf[0] == ';' || linebuf[0] == '[')
				continue;

			name = strtok(linebuf, " =\r\n");
			if (name == NULL)
				continue;

			value = strtok(NULL, " =\r\n");
			if (value == NULL)
				continue;

			/* check name and value */
			for (i = 0; cfg[i].name; i++)
			{
				if (!strcmp(name, cfg[i].name))
				{
					switch (cfg[i].type)
					{
					case CFG_INT:  *cfg[i].value = get_config_int(value, cfg[i].max); break;
					case CFG_BOOL: *cfg[i].value = get_config_bool(value); break;
					case CFG_PAD:  *cfg[i].value = get_config_pad(value); break;
					}
				}
			}
		}

		if (cfg2)
		{
			fseek(fp, 0, SEEK_SET);

			while (1)
			{
				char *name, *value;
				char *p1, *p2, temp[LINEBUF_SIZE];

				memset(linebuf, LINEBUF_SIZE, 0);
				if (fgets(linebuf, LINEBUF_SIZE - 1, fp) == NULL)
					break;

				strcpy(temp, linebuf);

				if (linebuf[0] == ';' || linebuf[0] == '[')
					continue;

				name = strtok(linebuf, " =\r\n");
				if (name == NULL)
					continue;

				value = strtok(NULL, " =\r\n");
				if (value == NULL)
					continue;

				p1 = strchr(temp, '\"');
				if (p1)
				{
					p2 = strchr(p1 + 1, '\"');
					if (p2)
					{
						value = p1 + 1;
						*p2 = '\0';
					}
				}

				/* check name and value */
				for (i = 0; cfg2[i].name; i++)
				{
					if (!strcmp(name, cfg2[i].name))
					{
						if (cfg2[i].type == CFG_STR)
						{
							memset(cfg2[i].value, 0, cfg2[i].max_len);
							strncpy(cfg2[i].value, value, cfg2[i].max_len - 1);
						}
					}
				}
			}
		}

		fclose(fp);

		return 1;
	}

	return 0;
}


/*------------------------------------------------------
	.ini�t�@�C���ɐݒ��ۑ�
------------------------------------------------------*/

static int save_inifile(const char *path, cfg_type *cfg, cfg2_type *cfg2)
{
	FILE *fp;

	if ((fp = fopen(path, "w")) != NULL)
	{
		int i;

		fprintf(fp, ";-------------------------------------------\r\n");
		fprintf(fp, "; " APPNAME_STR " " VERSION_STR "\r\n");
		fprintf(fp, ";-------------------------------------------\r\n");

		for (i = 0; cfg[i].name; i++)
		{
			switch (cfg[i].type)
			{
			case CFG_NONE: if (cfg[i].name) fprintf(fp, "\r\n%s\r\n", cfg[i].name); break;
			case CFG_INT:  fprintf(fp, "%s = %s\r\n", cfg[i].name, set_config_int(*cfg[i].value, cfg[i].max)); break;
			case CFG_BOOL: fprintf(fp, "%s = %s\r\n", cfg[i].name, set_config_bool(*cfg[i].value)); break;
			case CFG_PAD:  fprintf(fp, "%s = %s\r\n", cfg[i].name, set_config_pad(*cfg[i].value)); break;
			}
		}

		if (cfg2)
		{
			for (i = 0; cfg2[i].name; i++)
			{
				switch (cfg2[i].type)
				{
				case CFG_NONE: if (cfg2[i].name) fprintf(fp, "\r\n%s\r\n", cfg2[i].name); break;
				case CFG_STR:  fprintf(fp, "%s = \"%s\"\r\n", cfg2[i].name, cfg2[i].value); break;
				}
			}
		}

		fclose(fp);

		return 1;
	}

	return 0;
}


/******************************************************************************
	�O���[�o���֐�
******************************************************************************/

/*------------------------------------------------------
	�A�v���P�[�V�����̐ݒ��ǂݍ���
------------------------------------------------------*/

void load_settings(void)
{
	int i;
	char path[MAX_PATH];

	for (i = 0; default_options[i].name; i++)
	{
		if (default_options[i].value)
			*default_options[i].value = default_options[i].def;
	}

	sprintf(path, "%s%s", launchDir, inifile_name);

	if (load_inifile(path, default_options, default_options2) == 0)
	{
		save_settings();
	}
	else if (ini_version != INIVERSION)
	{
		for (i = 0; default_options[i].name; i++)
		{
			if (default_options[i].value)
				*default_options[i].value = default_options[i].def;
		}

		sceIoRemove(inifile_name);
		delete_files("nvram", "nv");
		delete_files("config", "ini");

		save_settings();
	}
}


/*------------------------------------------------------
	�A�v���P�[�V�����̐ݒ��ۑ�����
------------------------------------------------------*/

void save_settings(void)
{
	char path[MAX_PATH];

	sprintf(path, "%s%s", launchDir, inifile_name);

	save_inifile(path, default_options, default_options2);
}


/*------------------------------------------------------
	�Q�[���̐ݒ��ǂݍ���
------------------------------------------------------*/

void load_gamecfg(const char *name)
{
	int i;
	char path[MAX_PATH];
	cfg_type *gamecfg;

	sprintf(path, "%sconfig/%s.ini", launchDir, name);

	memset(input_map, 0, sizeof(input_map));

#define INCLUDE_SETUP_CONFIG_STRUCT

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "config/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "config/mvs.c"
#endif

#undef INCLUDE_SETUP_CONFIG_STRUCT

	for (i = 0; gamecfg[i].name; i++)
	{
		if (gamecfg[i].value)
			*gamecfg[i].value = gamecfg[i].def;
	}

#define INCLUDE_SETUP_DIPSWITCH

#if (EMU_SYSTEM == CPS1)
#include "config/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "config/mvs.c"
#endif

#undef INCLUDE_SETUP_DIPSWITCH

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
	if (!machine_screen_type) cps_rotate_screen = 0;
#endif

	if (load_inifile(path, gamecfg, NULL) == 0)
		save_gamecfg(name);
}


/*------------------------------------------------------
	�Q�[���̐ݒ��ۑ�����
------------------------------------------------------*/

void save_gamecfg(const char *name)
{
	char path[MAX_PATH];
	cfg_type *gamecfg;

	sprintf(path, "%sconfig/%s.ini", launchDir, name);

#define INCLUDE_SETUP_CONFIG_STRUCT

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "config/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "config/mvs.c"
#endif

#undef INCLUDE_SETUP_CONFIG_STRUCT

	save_inifile(path, gamecfg, NULL);
}
