/*****************************************************************************

	emumain.c

	エミュレーションコア

******************************************************************************/

#include "emumain.h"
#include <stdarg.h>


#define FRAMESKIP_LEVELS	12

#if (EMU_SYSTEM == MVS)
#define TICKS_PER_FRAME		16896		// 1000000 / 15625 * RASTER_LINES
#endif

/******************************************************************************
	グローバル変数
******************************************************************************/

char game_name[16];
char parent_name[16];

char game_dir[MAX_PATH];
#if USE_CACHE
char cache_dir[MAX_PATH];
char cache_parent_name[16];
#endif

int option_showfps;
int option_speedlimit;
int option_autoframeskip;
int option_frameskip;
int option_vsync;
int option_stretch;

int option_sound_enable;
int option_samplerate;
int option_sound_volume;
int option_sound_filter;

int machine_driver_type;
int machine_init_type;
int machine_input_type;
int machine_screen_type;
int machine_sound_type;

u32 frames_displayed;
int fatal_error;


/******************************************************************************
	ローカル変数
******************************************************************************/

static int frameskip;
static int frameskipadjust;
static int frameskip_counter;

static TICKER last_skipcount0_time;
static TICKER this_frame_base;
static int warming_up;

static int frames_since_last_fps;
static int rendered_frames_since_last_fps;
static int game_speed_percent;
static int frames_per_second;

static int snap_no = -1;

static char fatal_error_message[256];

static const u8 skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};


/******************************************************************************
	プロトタイプ
******************************************************************************/

static void show_fps(void);
static void show_battery_warning(void);


/*--------------------------------------------------------
	エミュレーション開始
--------------------------------------------------------*/

void emu_main(void)
{
	snap_no = -1;
	sound_thread_init();
	machine_main();
	sound_thread_exit();
}


/*--------------------------------------------------------
	フレームスキップを初期化
--------------------------------------------------------*/

void autoframeskip_reset(void)
{
	frameskip = option_autoframeskip ? 0 : option_frameskip;
	frameskipadjust = 0;
	frameskip_counter = FRAMESKIP_LEVELS - 1;

	rendered_frames_since_last_fps = 0;
	frames_since_last_fps = 0;

	game_speed_percent = 100;
	frames_per_second = FPS;

	frames_displayed = 0;

	warming_up = 1;
}


/*--------------------------------------------------------
	フレームスキップテーブル
--------------------------------------------------------*/

u8 skip_this_frame(void)
{
	return skiptable[frameskip][frameskip_counter];
}


/*--------------------------------------------------------
	画面更新
--------------------------------------------------------*/

void update_screen(void)
{
	u8 skipped_it = skiptable[frameskip][frameskip_counter];

	if (!skipped_it)
	{
		if (option_showfps) show_fps();
		show_battery_warning();
		ui_show_popup(1);
	}
	else ui_show_popup(0);

	if (warming_up)
	{
		video_wait_vsync();
#if (EMU_SYSTEM == MVS)
		last_skipcount0_time = ticker() - FRAMESKIP_LEVELS * TICKS_PER_FRAME;
#else
		last_skipcount0_time = ticker() - (int)((float)FRAMESKIP_LEVELS * 1000000.0 / FPS);
#endif
		warming_up = 0;
	}

	if (frameskip_counter == 0)
#if (EMU_SYSTEM == MVS)
		this_frame_base = last_skipcount0_time + FRAMESKIP_LEVELS * TICKS_PER_FRAME;
#else
		this_frame_base = last_skipcount0_time + (int)((float)FRAMESKIP_LEVELS * 1000000.0 / FPS);
#endif

	frames_displayed++;
	frames_since_last_fps++;

	if (!skipped_it)
	{
		TICKER curr = ticker();

		if (option_speedlimit)
		{
#if (EMU_SYSTEM == MVS)
			TICKER target = this_frame_base + frameskip_counter * TICKS_PER_FRAME;
#else
			TICKER target = this_frame_base + (int)((float)frameskip_counter * 1000000.0 / FPS);
#endif

			while (curr < target)
			{
				curr = ticker();
			}
		}

		video_flip_screen(option_vsync);

		rendered_frames_since_last_fps++;

		if (frameskip_counter == 0)
		{
			float seconds_elapsed = (float)(curr - last_skipcount0_time) * (1.0 / 1000000.0);
			float frames_per_sec = (float)frames_since_last_fps / seconds_elapsed;

			game_speed_percent = (int)(100.0 * frames_per_sec / FPS + 0.5);
			frames_per_second = (int)((float)rendered_frames_since_last_fps / seconds_elapsed + 0.5);

			last_skipcount0_time = curr;
			frames_since_last_fps = 0;
			rendered_frames_since_last_fps = 0;

			if (option_autoframeskip)
			{
				if (option_speedlimit && frames_displayed > 2 * FRAMESKIP_LEVELS)
				{
					if (game_speed_percent >= 99)
					{
						frameskipadjust++;

						if (frameskipadjust >= 3)
						{
							frameskipadjust = 0;
							if (frameskip > 0) frameskip--;
						}
					}
					else
					{
						if (game_speed_percent < 80)
						{
							frameskipadjust -= (90 - game_speed_percent) / 5;
						}
						else if (frameskip < 8)
						{
							frameskipadjust--;
						}

						while (frameskipadjust <= -2)
						{
							frameskipadjust += 2;
							if (frameskip < FRAMESKIP_LEVELS - 1)
								frameskip++;
						}
					}
				}
			}
		}
	}

	frameskip_counter = (frameskip_counter + 1) % FRAMESKIP_LEVELS;
}


/*--------------------------------------------------------
	FPS表示
--------------------------------------------------------*/

static void show_fps(void)
{
	int sx;
	char buf[32];

	sprintf(buf, "%s%2d %3d%% %2dfps",
		option_autoframeskip ? "auto" : "fskp",
		frameskip,
		game_speed_percent,
		frames_per_second);

	sx = SCR_WIDTH - (strlen(buf) << 3);
	small_font_print(sx, 0, buf, 1);
}


/*--------------------------------------------------------
	バッテリー残量警告表示
--------------------------------------------------------*/

static void show_battery_warning(void)
{
	if (!scePowerIsBatteryCharging())
	{
		int bat = scePowerGetBatteryLifePercent();

		if (bat < 10)
		{
			static u32 counter = 0;

			counter++;
			if ((counter % 120) < 80)
			{
				char warning[128];

				boxfill_alpha(0, 254, SCR_WIDTH-1, SCR_HEIGHT-1, COLOR_BLACK, 12);
				sprintf(warning, "Warning: Battery is low (%d%%). Please charge battery!", bat);
				uifont_print_center(256, UI_COLOR(UI_PAL_WARNING), warning);
			}
		}
	}
}


/*--------------------------------------------------------
	致命的エラーメッセージ
--------------------------------------------------------*/

void fatalerror(const char *text, ...)
{
	va_list arg;

	va_start(arg, text);
	vsprintf(fatal_error_message, text, arg);
	va_end(arg);

	fatal_error = 1;
	Loop = LOOP_BROWSER;
}


/*--------------------------------------------------------
	致命的エラーメッセージ表示
--------------------------------------------------------*/

void show_fatal_error(void)
{
	if (fatal_error)
	{
		int sx, sy, ex, ey;
		int width = uifont_get_string_width(fatal_error_message);
		int update = 1;

		sx = (SCR_WIDTH - width) >> 1;
		sy = (SCR_HEIGHT - FONTSIZE) >> 1;
		ex = sx + width;
		ey = sy + (FONTSIZE - 1);

		load_background();

		while (Loop != LOOP_EXIT)
		{
			if (update)
			{
				show_background();
				small_icon(6, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
				uifont_print(32, 5, UI_COLOR(UI_PAL_TITLE), "Fatal error");
				draw_dialog(sx - FONTSIZE/2, sy - FONTSIZE/2, ex + FONTSIZE/2, ey + FONTSIZE/2);
				uifont_print_center(sy, UI_COLOR(UI_PAL_SELECT), fatal_error_message);

				update = draw_battery_status(1);
				video_flip_screen(1);
			}
			else
			{
				update = draw_battery_status(0);
				video_wait_vsync();
			}

			pad_update();

			if (pad_pressed(PSP_CTRL_ANY))
				break;
		}

		pad_wait_clear();

		fatal_error = 0;
	}
}


/*------------------------------------------------------
	スクリーンショット保存
------------------------------------------------------*/

void save_snapshot(void)
{
	char path[MAX_PATH];

	sound_mute(1);
#if USE_CACHE
	cache_sleep(1);
#endif

	if (snap_no == -1)
	{
		FILE *fp;

		snap_no = 1;

		while (1)
		{
#if (EMU_SYSTEM == CPS1)
			sprintf(path, "%ssnap/%s_%02d.png", launchDir, game_name, snap_no);
#else
			sprintf(path, "%ssnap/%s_%02d.bmp", launchDir, game_name, snap_no);
#endif
			if ((fp = fopen(path, "rb")) == NULL) break;
			fclose(fp);
			snap_no++;
		}
	}

#if (EMU_SYSTEM == CPS1)
	sprintf(path, "%ssnap/%s_%02d.png", launchDir, game_name, snap_no);
	save_png(path);
	ui_popup("Snapshot saved as \"%s_%02d.png\".", game_name, snap_no++);
#else
	sprintf(path, "%ssnap/%s_%02d.bmp", launchDir, game_name, snap_no);
	save_bmp(path);
	ui_popup("Snapshot saved as \"%s_%02d.bmp\".", game_name, snap_no++);
#endif

#if USE_CACHE
	cache_sleep(0);
#endif
	sound_mute(0);
}
