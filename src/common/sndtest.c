/******************************************************************************

	sndtest.c

	サウンドテスト

******************************************************************************/

#include "emumain.h"
#include "psp/font/jpnfont.h"


#if (EMU_SYSTEM == MVS)
#define TICKS_PER_FRAME		16896		// 1000000 / 15625 * RASTER_LINES
#else
#define TICKS_PER_FRAME		(int)(1000000 / FPS)
#endif


/******************************************************************************
	グローバル変数
******************************************************************************/

int sound_test = 0;


/******************************************************************************
	ローカル変数/構造体
******************************************************************************/

typedef struct sndlist_t
{
	int  lines;
	char **line;
	u8 *command;
} SNDLIST;

static SNDLIST sndlist;

static char *sndlistbuf;
static int top, rows, sel, prev_sel, update;
static char title[80];
static char game_title[80];

static u8 pre_code_exist;
static u8 pre_code;
static u8 stop_code_exist;
static u8 stop_code;


/******************************************************************************
	サウンドドライバ (MVS)
******************************************************************************/

#if (EMU_SYSTEM == MVS)

#define MAX_COMMAND 32

static int command_count = 0;
static u16 command_que[MAX_COMMAND];


/*--------------------------------------------------------
	サウンドコマンド送信
--------------------------------------------------------*/

static void mvssnd_send_command(u16 data)
{
	if (command_count < MAX_COMMAND)
		command_que[command_count++] = data;
}


/*--------------------------------------------------------
	サウンドドライバ初期化
--------------------------------------------------------*/

static void mvssnd_driver_reset(void)
{
	memcpy(memory_region_cpu2, &memory_region_cpu2[0x10000], 0x10000);

	z80_init();
	neogeo_z80_port_w(0x000c, 0);
	neogeo_z80_port_r(0x0000);
	z80_reset();

	command_count = 0;
	memset(command_que, 0, sizeof(command_que));

	mvssnd_send_command(0x01);
	mvssnd_send_command(0x03);
}


/*--------------------------------------------------------
	サウンド更新
--------------------------------------------------------*/

static void mvssnd_update_sound(void)
{
	static int command_wait = 0;

	if (!command_wait && command_count)
	{
		int i;

		neogeo_z80_w(0, (command_que[0] << 8), 0);
		z80_set_irq_line(IRQ_LINE_NMI, PULSE_LINE);

		for (i = 0; i < command_count - 1; i++)
			command_que[i] = command_que[i + 1];
		command_count--;
		command_wait = 1;
	}
	else if (command_wait)
	{
		command_wait = 0;
	}
}


/*--------------------------------------------------------
	サウンド停止コマンド送信
--------------------------------------------------------*/

static void mvssnd_stop_sound(void)
{
	if (stop_code != 0x03)
	{
		if (pre_code_exist)
		{
			mvssnd_send_command(pre_code);
		}
		if (stop_code_exist) mvssnd_send_command(stop_code);
	}
	else
	{
		mvssnd_send_command(stop_code);
	}
}

/*--------------------------------------------------------
	サウンド再生コマンド送信
--------------------------------------------------------*/

static void mvssnd_play_sound(void)
{
	mvssnd_stop_sound();

	if (sndlist.command[sel] == 0x02)
	{
		mvssnd_send_command(0x03);
		mvssnd_send_command(0x02);
	}
	else
	{
		if (pre_code_exist)
		{
			mvssnd_send_command(0x03);
			mvssnd_send_command(pre_code);
			mvssnd_send_command(sndlist.command[sel]);
		}
		else
		{
			mvssnd_send_command(sndlist.command[sel]);
		}
	}
}

#endif

/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	16進数文字列→数値変換
--------------------------------------------------------*/

static int atoh(const char *s)
{
	int len = strlen(s);
	int i, res = 0;

	for (i = 0; i < len; i++)
	{
		res *= 16;

		if (s[i] >= '0' && s[i] <= '9')
			res += s[i] - '0';
		else if (s[i] >= 'a' && s[i] <= 'f')
			res += s[i] - 'a' + 10;
		else if (s[i] >= 'A' && s[i] <= 'F')
			res += s[i] - 'A' + 10;
	}
	return res;
}


/*--------------------------------------------------------
	10進数文字列→数値変換
--------------------------------------------------------*/

#if 0
static int atoi(const char *s)
{
	int len = strlen(s);
	int i, res = 0;

	for (i = 0; i < len; i++)
	{
		res *= 10;

		if (s[i] >= '0' && s[i] <= '9')
			res += s[i] - '0';
	}
	return res;
}
#endif


/*--------------------------------------------------------
	ラインバッファから値を取得
--------------------------------------------------------*/

static char *sndlist_get_value(char *buf)
{
	if (strtok(buf, "=") != NULL)
		return strtok(NULL, "\r\n");

	return NULL;
}


/*--------------------------------------------------------
	プレイリストの解放
--------------------------------------------------------*/

static void free_playlist(void)
{
	if (sndlist.command)
	{
		free(sndlist.command);
		sndlist.command = NULL;
	}
	if (sndlist.line)
	{
		free(sndlist.line);
		sndlist.line = NULL;
	}
	if (sndlistbuf)
	{
		free(sndlistbuf);
		sndlistbuf = NULL;
	}
	free_jpnfont();
}


/*--------------------------------------------------------
	コマンドリストのロード
--------------------------------------------------------*/

static int load_playlist(const char *game_name)
{
	FILE *fp;
	char path[MAX_PATH];
	char *p, linebuf[256];
	int found, line;
	int size, start, end, pos;
	int num_lines;

	sndlistbuf = NULL;
	sndlist.lines = 0;
	sndlist.line = NULL;
	sndlist.command = NULL;

	pre_code_exist = 0;
	pre_code = 0x00;
	stop_code_exist = 0;
	stop_code = 0x00;

	memset(game_title, 0, 80);

	found = 0;

	sprintf(path, "%smusic/jp/%s.lst", launchDir, game_name);
	if (file_exist(path))
		if (load_jpnfont())
			found = 1;

	if (!found)
	{
		sprintf(path, "%smusic/en/%s.lst", launchDir, game_name);
		found = file_exist(path);
	}

	if (!found)
	{
		sprintf(path, "%smusic/default.lst", launchDir);
		if (!file_exist(path))
		{
			fatalerror("Could not open playlist.");
			return 0;
		}
	}

	if ((fp = fopen(path, "rb")) == NULL)
	{
		fatalerror("Could not open playlist.");
		return 0;
	}

	start = 0;
	end   = 0;
	pos   = 0;
	num_lines = 0;
	while (fgets(linebuf, 255, fp) != NULL)
	{
		if (linebuf[0] == '$')
		{
			if (strnicmp(linebuf, "$title", 6) == 0)
			{
				if ((p = sndlist_get_value(linebuf)) != NULL)
					strcpy(game_title, p);
			}
			else if (strnicmp(linebuf, "$pre", 4) == 0)
			{
				if ((p = sndlist_get_value(linebuf)) != NULL)
				{
					pre_code_exist = 1;
					pre_code = atoh(p);
				}
			}
			else if (strnicmp(linebuf, "$stop", 5) == 0)
			{
				if ((p = sndlist_get_value(linebuf)) != NULL)
				{
					stop_code_exist = 1;
					stop_code = atoh(p);
				}
			}
			else if (strnicmp(linebuf, "$start", 6) == 0)
			{
				start = ftell(fp);
			}
			else if (start && strnicmp(linebuf, "$end", 4) == 0)
			{
				num_lines--;
				end = pos;
				break;
			}
		}
		if (start) num_lines++;
		pos = ftell(fp);
	}

	if (end == 0)
	{
		fclose(fp);
		fatalerror("playlist data not found.");
		return 0;
	}

	size = end - start;

	if ((sndlistbuf = malloc(size + 8)) == NULL)
	{
		fclose(fp);
		fatalerror("Could not allocate memory. (sndlistbuf)");
		return 0;
	}
	memset(sndlistbuf, 0, size + 8);

	if ((sndlist.line = malloc(sizeof(char *) * num_lines + 1)) == NULL)
	{
		fatalerror("Could not allocate memory. (sndlist.lines)");
		goto error;
	}

	if ((sndlist.command = malloc(sizeof(u16) * num_lines + 1)) == NULL)
	{
		fatalerror("Could not allocate memory. (sndlist.command)");
		goto error;
	}

	fseek(fp, start, SEEK_SET);

	line = 0;
	p = sndlistbuf;
	while (fgets(linebuf, 255, fp) != NULL)
	{
		if (strnicmp(linebuf, "$end", 4) == 0)
			break;

		strcpy(p, strtok(linebuf, "\r\n"));
		sndlist.line[line] = p;
		p += strlen(linebuf) + 1;
		line++;
	}

	fclose(fp);

	sndlist.lines = line;

	for (line = 0; line < sndlist.lines; line++)
	{
		char temp[4];

		memcpy(temp, sndlist.line[line], 2);

		temp[0] = tolower(temp[0]);
		temp[1] = tolower(temp[1]);
		temp[2] = '\0';

		sndlist.command[line] = atoh(temp);
	}

	return 1;

error:
	fclose(fp);
	free_playlist();
	return 0;
}


/*--------------------------------------------------------
	画面の初期化
--------------------------------------------------------*/

static void soundtest_initscreen(void)
{
	load_background();

	top      = 0;
	sel      = 0;
	prev_sel = 0;
	rows     = 14;
	update   = 1;

	if (strlen(game_title))
		sprintf(title, "Sound test - %s", game_title);
	else
		sprintf(title, "Sound test - %s", game_name);
}


/*--------------------------------------------------------
	画面の更新
--------------------------------------------------------*/

static void soundtest_updatescreen(void)
{
	if (update)
	{
		char temp[32];
		int y;

		show_background();
		small_icon(8, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
		uifont_print(36, 5, UI_COLOR(UI_PAL_TITLE), title);

		if (top != 0)
			uifont_print(235, 24, UI_COLOR(UI_PAL_SELECT), FONT_UPTRIANGLE);

		for (y = 0; y < rows; y++)
		{
			if (top + y >= sndlist.lines) break;

			sprintf(temp, "%02x", sndlist.command[top + y]);

			if (top + y == sel)
			{
				uifont_print(8, 37 + 16 * y, UI_COLOR(UI_PAL_SELECT), temp);
				uifont_print(32, 37 + 16 * y, UI_COLOR(UI_PAL_SELECT), &sndlist.line[sel][3]);
			}
			else
			{
				uifont_print(8, 37 + 16 * y, UI_COLOR(UI_PAL_NORMAL), temp);
				uifont_print(32, 37 + 16 * y, UI_COLOR(UI_PAL_NORMAL), &sndlist.line[top + y][3]);
			}
		}

		if (top + rows < sndlist.lines)
			uifont_print(235, 260, UI_COLOR(UI_PAL_SELECT), FONT_DOWNTRIANGLE);

		update = draw_battery_status(1);
		update |= ui_show_popup(1);
		video_flip_screen(0);
	}
	else
	{
		update = draw_battery_status(0);
		update |= ui_show_popup(0);
	}

	prev_sel = sel;
	pad_update();
	if (pad_pressed(PSP_CTRL_UP))
	{
		sel--;
	}
	else if (pad_pressed(PSP_CTRL_DOWN))
	{
		sel++;
	}
	else if (pad_pressed(PSP_CTRL_LEFT))
	{
		sel -= rows;
	}
	else if (pad_pressed(PSP_CTRL_RIGHT))
	{
		sel += rows;
	}
	else if (pad_pressed(PSP_CTRL_RTRIGGER))
	{
		help(HELP_SOUNDTEST);
		update = 1;
		pad_wait_clear();
	}
	else if (pad_pressed(PSP_CTRL_CIRCLE))
	{
#if (EMU_SYSTEM == MVS)
		mvssnd_play_sound();
#endif
		pad_wait_clear();
	}
	else if (pad_pressed(PSP_CTRL_CROSS))
	{
#if (EMU_SYSTEM == MVS)
		mvssnd_stop_sound();
#endif
		pad_wait_clear();
	}
	else if (pad_pressed(PSP_CTRL_TRIANGLE))
	{
		Loop = LOOP_BROWSER;
	}

	if (top > sndlist.lines - rows) top = sndlist.lines - rows;
	if (top < 0) top = 0;
	if (sel >= sndlist.lines) sel = 0;
	if (sel < 0) sel = sndlist.lines - 1;
	if (sel >= top + rows) top = sel - rows + 1;
	if (sel < top) top = sel;

	if (sel != prev_sel) update = 1;
}


/*--------------------------------------------------------
	NEOGEOサウンドエミュレーション初期化
--------------------------------------------------------*/

static void soundtest_run(void)
{
	if (load_playlist(game_name))
	{
		if (sound_init())
		{
			soundtest_initscreen();

#if (EMU_SYSTEM == MVS)
			mvssnd_driver_reset();
#endif
			timer_reset();
			sound_reset();

			Loop = LOOP_EXEC;
			while (Loop == LOOP_EXEC)
			{
				if (Sleep)
				{
					do
					{
						sceKernelDelayThread(5000000);
					} while (Sleep);
				}

				soundtest_updatescreen();
				sceKernelDelayThread(TICKS_PER_FRAME);
			}

			video_clear_screen();
			sound_mute(1);
		}
		sound_exit();
		show_exit_screen();
	}
	free_playlist();
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	サウンドテストメイン
--------------------------------------------------------*/

void soundtest_main(void)
{
	ui_popup_reset(POPUP_MENU);
	video_clear_screen();
	fatal_error = 0;

	if (memory_init())
	{
		soundtest_run();
	}
	memory_shutdown();

	show_fatal_error();
	ui_popup_reset(POPUP_MENU);
	pad_wait_clear();

	sound_test = 0;
}


/*--------------------------------------------------------
	サウンドコマンド更新
--------------------------------------------------------*/

void soundtest_sound_update(void)
{
#if (EMU_SYSTEM == MVS)
	mvssnd_update_sound();
#endif
}
