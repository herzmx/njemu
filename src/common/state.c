/******************************************************************************

	state.c

	ステートセーブ/ロード

******************************************************************************/

#ifdef SAVE_STATE

#include "emumain.h"
#include <time.h>


/******************************************************************************
	グローバル変数
******************************************************************************/

char date_str[16];
char time_str[16];
char stver_str[16];
int  state_version;
int  current_state_version;
#if (EMU_SYSTEM == MVS)
int  state_reload_bios;
#endif


/******************************************************************************
	ローカル変数
******************************************************************************/

#if (EMU_SYSTEM == CPS1)
static const char *current_version_str = "CPS1SV04";
#elif (EMU_SYSTEM == CPS2)
static const char *current_version_str = "CPS2SV07";
#elif (EMU_SYSTEM == MVS)
static const char *current_version_str = "MVSSV003";
#endif


/******************************************************************************
	ローカル関数
******************************************************************************/

/*------------------------------------------------------
	サムネイルをワーク領域からファイルに保存
------------------------------------------------------*/

static void save_thumbnail(FILE *fp)
{
	int x, y, w, h;
	u16 *src = video_frame_addr(tex_frame, 152, 0);

	if (machine_screen_type)
	{
		w = 112;
		h = 152;
	}
	else
	{
		w = 152;
		h = 112;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			fwrite(&src[x], 1, 2, fp);
		}
		src += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	サムネイルをファイルからワーク領域に読み込み
------------------------------------------------------*/

static void load_thumbnail(FILE *fp)
{
	int x, y, w, h;
	u16 *dst = video_frame_addr(tex_frame, 0, 0);

	if (machine_screen_type)
	{
		w = 112;
		h = 152;
	}
	else
	{
		w = 152;
		h = 112;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			fread(&dst[x], 1, 2, fp);
		}
		dst += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	ワーク領域のサムネイルをクリア
------------------------------------------------------*/

static void clear_thumbnail(void)
{
	int x, y, w, h;
	u16 *dst = video_frame_addr(tex_frame, 0, 0);

	if (machine_screen_type)
	{
		w = 112;
		h = 152;
	}
	else
	{
		w = 152;
		h = 112;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			dst[x] = 0;
		}
		dst += BUF_WIDTH;
	}
}


/******************************************************************************
	ステートセーブ/ロード関数
******************************************************************************/

/*------------------------------------------------------
	ステートセーブ
------------------------------------------------------*/

int state_save(int slot)
{
	FILE *fp = NULL;
	char path[MAX_PATH];

	sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, slot);
	remove(path);

	if ((fp = fopen(path, "wb")) != NULL)
	{
		pspTime nowtime;

		sceRtcGetCurrentClockLocalTime(&nowtime);

		// ヘッダ (0x20 bytes)
		state_draw_progress(2);
		state_save_byte(current_version_str, 8);
		state_save_byte(&nowtime, 16);

		// サムネイル (152 * 112 * 2 bytes)
		state_draw_progress(3);
		save_thumbnail(fp);

		// ステートデータ
		state_draw_progress(4);
		state_save_memory(fp);
		state_save_m68000(fp);
		state_save_z80(fp);
		state_save_input(fp);
		state_save_timer(fp);
		state_save_driver(fp);
		state_save_video(fp);
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
		state_save_coin(fp);
#endif
#if (EMU_SYSTEM == CPS1)
		switch (machine_driver_type)
		{
		case MACHINE_qsound:
			state_save_qsound(fp);
			state_save_eeprom(fp);
			break;

		case MACHINE_pang3:
			state_save_eeprom(fp);

		default:
			state_save_ym2151(fp);
			break;
		}
#elif (EMU_SYSTEM == CPS2)
		state_save_qsound(fp);
		state_save_eeprom(fp);
#elif (EMU_SYSTEM == MVS)
		state_save_ym2610(fp);
		state_save_pd4990a(fp);
#endif
		fclose(fp);

		state_draw_progress(5);
		return 1;
	}

	ui_popup("Error: Could not open file \"%s.sv%d\"", game_name, slot);

	return 0;
}


/*------------------------------------------------------
	ステートロード
------------------------------------------------------*/

int state_load(int slot)
{
	FILE *fp;
	char path[MAX_PATH];

	sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, slot);

#if (EMU_SYSTEM == MVS)
	state_reload_bios = 0;
#endif

	if ((fp = fopen(path, "rb")) != NULL)
	{
		// ヘッダ (スキップ)
		state_draw_progress(2);
		state_load_skip((8+16));

		// サムネイル (スキップ)
		state_draw_progress(3);
		state_load_skip((152*112*2));

		// ステートデータ
		state_draw_progress(4);
		state_load_memory(fp);
		state_load_m68000(fp);
		state_load_z80(fp);
		state_load_input(fp);
		state_load_timer(fp);
		state_load_driver(fp);
		state_load_video(fp);
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
		state_load_coin(fp);
#endif
#if (EMU_SYSTEM == CPS1)
		switch (machine_driver_type)
		{
		case MACHINE_qsound:
			state_load_qsound(fp);
			state_load_eeprom(fp);
			break;

		case MACHINE_pang3:
			state_load_ym2151(fp);
			state_load_eeprom(fp);

		default:
			state_load_ym2151(fp);
			break;
		}
#elif (EMU_SYSTEM == CPS2)
		state_load_qsound(fp);
		state_load_eeprom(fp);
#elif (EMU_SYSTEM == MVS)
		state_load_ym2610(fp);
		state_load_pd4990a(fp);
#endif
		fclose(fp);

#if (EMU_SYSTEM == CPS2)
		blit_update_all_cache();
#elif (EMU_SYSTEM == MVS)
//		blit_update_all_cache();

		if (state_reload_bios)
		{
			state_reload_bios = 0;

			if (!reload_bios())
			{
				fatalerror("Error: Could not reload BIOS.");
				Loop = LOOP_BROWSER;
				return 0;
			}
		}
#endif

		state_draw_progress(5);
		return 1;
	}

	ui_popup("Error: Could not open file \"%s.sv%d\"", game_name, slot);

	return 0;
}


/*------------------------------------------------------
	サムネイル作成
------------------------------------------------------*/

void state_make_thumbnail(void)
{
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
	RECT clip1 = { 64, 16, 64 + 384, 16 + 224 };

	if (machine_screen_type)
	{
		RECT clip2 = { 152, 0, 152 + 112, 152 };
		video_copy_rect_rotate(work_frame, tex_frame, &clip1, &clip2);
	}
	else
	{
		RECT clip2 = { 152, 0, 152 + 152, 112 };
		video_copy_rect(work_frame, tex_frame, &clip1, &clip2);
	}
#elif (EMU_SYSTEM == MVS)
	RECT clip1 = { 8, 16, 8 + 304, 16 + 224 };
	RECT clip2 = { 152, 0, 152 + 152, 112 };

	video_copy_rect(work_frame, tex_frame, &clip1, &clip2);
#endif
}


/*------------------------------------------------------
	サムネイル読み込み
------------------------------------------------------*/

int state_load_thumbnail(int slot)
{
	FILE *fp;
	char path[MAX_PATH];

	clear_thumbnail();

	sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, slot);

	if ((fp = fopen(path, "rb")) != NULL)
	{
		pspTime t;

		memset(stver_str, 0, 16);

		fread(stver_str, 1, 8, fp);
		fread(&t, 1, 16, fp);
		load_thumbnail(fp);
		fclose(fp);

		current_state_version = current_version_str[7] - '0';
		state_version = stver_str[7] - '0';

		sprintf(date_str, "%04d/%02d/%02d", t.year, t.month, t.day);
		sprintf(time_str, "%02d:%02d:%02d", t.hour, t.minutes, t.seconds);

		return 1;
	}

	ui_popup("Error: Could not open file \"%s.sv%d\"", game_name, slot);

	return 0;
}


/*------------------------------------------------------
	サムネイル消去
------------------------------------------------------*/

void state_clear_thumbnail(void)
{
	strcpy(date_str, "----/--/--");
	strcpy(time_str, "--:--:--");
	strcpy(stver_str, "--------");

	state_version = 0;

	clear_thumbnail();
}

#endif /* SAVE_STATE */
