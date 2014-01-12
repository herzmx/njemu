/******************************************************************************

	vidhrdw.c

	NEOGEO CDZ ビデオエミュレーション

******************************************************************************/

#include "ncdz.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

UINT16 ALIGN_DATA neogeo_videoram[0x20000 / 2];
UINT16 videoram_read_buffer;
UINT16 videoram_offset;
UINT16 videoram_modulo;

UINT16 ALIGN_DATA palettes[2][0x2000 / 2];
UINT32 palette_bank;

UINT16 *video_palette;
UINT16 ALIGN_PSPDATA video_palettebank[2][0x2000 / 2];
UINT16 ALIGN_DATA video_clut16[0x8000];

UINT8 video_fix_usage[0x20000 / 32];
UINT8 video_spr_usage[0x400000 / 128];

int video_enable;
int spr_disable;
int fix_disable;


/******************************************************************************
	ローカル変数
******************************************************************************/

static int next_update_first_line;

static UINT16 *sprite_zoom_control = &neogeo_videoram[0x8000];
static UINT16 *sprite_y_control = &neogeo_videoram[0x8200];
static UINT16 *sprite_x_control = &neogeo_videoram[0x8400];

#include "zoom.c"


/******************************************************************************
	プロトタイプ
******************************************************************************/

static void patch_vram_rbff2(void);
static void patch_vram_adkworld(void);
static void patch_vram_crsword2(void);


/******************************************************************************
	ローカル関数
******************************************************************************/

/*------------------------------------------------------
	FIXスプライト描画
------------------------------------------------------*/

static void draw_fix(void)
{
	UINT16 x, y, code, attr;

	for (x = 1/8; x < 312/8; x++)
	{
		UINT16 *vram = &neogeo_videoram[0x7000 + (16 / 8) + (x << 5)];

		for (y = 16/8; y < 240/8; y++)
		{
			code = *vram++;
			attr = code >> 12;
			code &= 0x0fff;

			if (video_fix_usage[code])
				blit_draw_fix(x << 3, y << 3, code, attr);
		}
	}

	blit_finish_fix();
}


/*------------------------------------------------------
	SPRスプライト描画
------------------------------------------------------*/

#define MAX_SPRITES_PER_SCREEN	(381)

#define DRAW_SPRITE()															\
	if (sy + yskip > min_y && sy <= max_y)										\
	{																			\
		attr = sprite_base[1];													\
		code = sprite_base[0];													\
																				\
		if (!auto_animation_disabled)											\
		{																		\
			if (attr & 0x0008)													\
				code = (code & ~0x0007) | (auto_animation_counter & 0x0007);	\
			else if (attr & 0x0004)												\
				code = (code & ~0x0003) | (auto_animation_counter & 0x0003);	\
		}																		\
																				\
		code &= 0x7fff;															\
																				\
		if (video_spr_usage[code])												\
			blit_draw_spr(x, sy, zoom_x + 1, yskip, code, attr);				\
	}

static void draw_spr(UINT32 start, UINT32 end, int min_y, int max_y)
{
	int y = 0;
	int x = 0;
	int rows = 0;
	int zoom_x = 0;
	int zoom_y = 0;

	UINT16 sprite_number;
	UINT16 attr;
	UINT32 code;
	UINT16 *sprite_base;

	int sy;
	int yskip = 0;
	int fullmode = 0;

	for (sprite_number = start; sprite_number < end; sprite_number++)
	{
		UINT16 zoom_control = sprite_zoom_control[sprite_number];
		UINT16 y_control = sprite_y_control[sprite_number];

		if (y_control & 0x40)
		{
			x += zoom_x + 1;
		}
		else
		{
			y = 0x200 - (y_control >> 7);
			x = sprite_x_control[sprite_number] >> 7;
			zoom_y = zoom_control & 0xff;
			rows = y_control & 0x3f;

			if (rows > 0x20)
			{
				fullmode = 1;
				rows = 0x20;

				if (y > 0x100) y -= 0x200;
				while (y < -16) y += (zoom_y + 1) << 1;
			}
			else
				fullmode = 0;
		}

		if (x >= 0x1f0) x -= 0x200;

		zoom_x = (zoom_control >> 8) & 0x0f;

		if (rows == 0) continue;
		if (zoom_y == 0) continue;
		if (x > 311) continue;
		if (x + zoom_x < 8) continue;

		sprite_base = &neogeo_videoram[sprite_number << 6];

		if (fullmode)
		{
			int row = 0;

			sy = y;

			while (row < rows)
			{
				if ((yskip = sprite_y_skip[zoom_y][row]))
				{
					if (sy >= 248) sy -= (zoom_y + 1) << 1;
					DRAW_SPRITE()
					sy += yskip;
				}

				row++;

				sprite_base += 2;
			}
		}
		else
		{
			int row = 0;
			int sprite_line = 0;

			while (sprite_line < rows << 4)
			{
				if ((yskip = sprite_y_skip[zoom_y][row]))
				{
					sy = (y + sprite_line) & 0x1ff;
					DRAW_SPRITE()
					sprite_line += yskip;
				}

				row++;
				if (row == 0x10 || row == 0x20)
					sprite_line += (0xff - zoom_y) << 1;

				sprite_base += 2;
			}
		}
	}

	blit_finish_spr();
}


/*------------------------------------------------------
	SPRスプライト描画 (プライオリティあり/ssrpg専用)
------------------------------------------------------*/

static void draw_spr_prio(int min_y, int max_y)
{
	int start = 0, end = MAX_SPRITES_PER_SCREEN;

	if ((neogeo_videoram[(0x10400 + 4) >> 1] & 0x40) == 0 &&
		(neogeo_videoram[(0x10400 + 6) >> 1] & 0x40) != 0)
	{
		start = 6 >> 1;

		while ((neogeo_videoram[(0x10400 >> 1) + start] & 0x40) != 0)
		    start++;

		if (start == 6 >> 1) start = 0;
	}

    do
	{
		draw_spr(start, end, min_y, max_y);
		end = start;
		start = 0;
	} while (end != 0);
}


/******************************************************************************
	NEOGEO CDZ ビデオ描画処理
******************************************************************************/

/*------------------------------------------------------
	ビデオエミュレーション初期化
------------------------------------------------------*/

void neogeo_video_init(void)
{
	int i, r, g, b;

	for (r = 0; r < 32; r++)
	{
		for (g = 0; g < 32; g++)
		{
			for (b = 0; b < 32; b++)
			{
				int r1 = (r << 3) | (r >> 2);
				int g1 = (g << 3) | (g >> 2);
				int b1 = (b << 3) | (b >> 2);

				UINT16 color = ((r & 1) << 14) | ((r & 0x1e) << 7)
							 | ((g & 1) << 13) | ((g & 0x1e) << 3)
							 | ((b & 1) << 12) | ((b & 0x1e) >> 1);

				video_clut16[color] = MAKECOL15(r1, g1, b1);
			}
		}
	}

	memset(neogeo_videoram, 0, sizeof(neogeo_videoram));
	memset(palettes, 0, sizeof(palettes));
	memset(video_palettebank, 0, sizeof(video_palettebank));

	for (i = 0; i < 0x1000; i += 16)
	{
		video_palettebank[0][i] = 0x8000;
		video_palettebank[1][i] = 0x8000;
	}

	neogeo_video_reset();
}


/*------------------------------------------------------
	ビデオエミュレーション終了
------------------------------------------------------*/

void neogeo_video_exit(void)
{
}


/*------------------------------------------------------
	ビデオエミュレーションリセット
------------------------------------------------------*/

void neogeo_video_reset(void)
{
	video_palette = video_palettebank[0];
	palette_bank = 0;

	videoram_read_buffer = 0;
	videoram_modulo = 1;
	videoram_offset = 0;

	next_update_first_line = FIRST_VISIBLE_LINE;

	video_enable = 1;
	spr_disable = 0;
	fix_disable = 0;

	blit_reset();
}


/******************************************************************************
	画面更新処理
******************************************************************************/

/*------------------------------------------------------
	スクリーン更新
------------------------------------------------------*/

void neogeo_partial_screenrefresh(int current_line)
{
	if (current_line >= FIRST_VISIBLE_LINE && current_line <= LAST_VISIBLE_LINE)
	{
		if (video_enable)
		{
			if (!spr_disable)
			{
				if (current_line >= next_update_first_line)
				{
					blit_start(next_update_first_line, current_line);

					if (NGH_NUMBER(NGH_ssrpg))
					{
						draw_spr_prio(next_update_first_line, current_line);
					}
					else
					{
						switch (neogeo_ngh)
						{
						case NGH_crsword2: patch_vram_crsword2(); break;
						case NGH_adkworld: patch_vram_adkworld(); break;
						case NGH_rbff2: patch_vram_rbff2(); break;
						}

						draw_spr(0, MAX_SPRITES_PER_SCREEN, next_update_first_line, current_line);
					}
				}
			}
		}

		next_update_first_line = current_line + 1;
	}
}


void neogeo_screenrefresh(void)
{
	if (video_enable)
	{
		if (!spr_disable)
		{
			neogeo_partial_screenrefresh(LAST_VISIBLE_LINE);
		}
		else
		{
			blit_start(FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE);
		}

		if (!fix_disable)
		{
			draw_fix();
		}

		blit_finish();
	}
	else
	{
		video_clear_frame(draw_frame);
	}

	next_update_first_line = FIRST_VISIBLE_LINE;
}


/*------------------------------------------------------
	スクリーン更新 (CD-ROMロード画面)
------------------------------------------------------*/

int neogeo_loading_screenrefresh(int flag, int draw)
{
	static TICKER prev;
	static int limit;

	if (flag)
	{
		prev = ticker() - TICKS_PER_SEC / FPS;
		limit = neogeo_cdspeed_limit;
	}

	if (limit)
	{
		TICKER target = prev + TICKS_PER_SEC / FPS;
		TICKER curr = ticker();

		while (curr < target) curr = ticker();

		prev = curr;
	}

#ifdef ADHOC
	if (!adhoc_enable)
#endif
	{
		pad_update();

		if (limit && pad_pressed(PSP_CTRL_LTRIGGER))
		{
			limit = 0;
			ui_popup(TEXT(CDROM_SPEED_LIMIT_OFF));
		}
		else if (!limit && pad_pressed(PSP_CTRL_RTRIGGER))
		{
			limit = 1;
			ui_popup(TEXT(CDROM_SPEED_LIMIT_ON));
		}
	}

	if (limit || draw)
	{
		blit_start(FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE);
		if (video_enable && !fix_disable) draw_fix();
		blit_finish();
		draw = ui_show_popup(1);
		video_flip_screen(0);
	}
	else draw = ui_show_popup(0);

#ifdef ADHOC
	if (adhoc_enable)
		update_inputport();
#endif

	return draw;
}


/*------------------------------------------------------
	VRAMパッチ (Realbout Fatal Fury 2)
------------------------------------------------------*/

static void patch_vram_rbff2(void)
{
	UINT16 offs;

	for (offs = 0; offs < ((0x300 >> 1) << 6) ; offs += 2)
	{
		UINT16 tileno  = neogeo_videoram[offs];
		UINT16 tileatr = neogeo_videoram[offs + 1];

		if (tileno == 0x7a00 && (tileatr == 0x4b00 || tileatr == 0x1400))
		{
			neogeo_videoram[offs] = 0x7ae9;
			return;
		}
	}
}


/*------------------------------------------------------
	VRAMパッチ (ADK World)
------------------------------------------------------*/

static void patch_vram_adkworld(void)
{
	UINT16 offs;

	for (offs = 0; offs < ((0x300 >> 1) << 6) ; offs += 2)
	{
		UINT16 tileno  = neogeo_videoram[offs];
		UINT16 tileatr = neogeo_videoram[offs + 1];

		if ((tileno == 0x14c0 || tileno == 0x1520) && tileatr == 0x0000)
			neogeo_videoram[offs] = 0x0000;
	}
}


/*------------------------------------------------------
	VRAMパッチ (Crossed Swords 2)
------------------------------------------------------*/

static void patch_vram_crsword2(void)
{
	UINT16 offs;

	for (offs = 0; offs < ((0x300 >> 1) << 6) ; offs += 2)
	{
		UINT16 tileno  = neogeo_videoram[offs];
		UINT16 tileatr = neogeo_videoram[offs + 1];

		if (tileno == 0x52a0 && tileatr == 0x0000)
			neogeo_videoram[offs] = 0x0000;
	}
}


/*------------------------------------------------------
	セーブ/ロード ステート
------------------------------------------------------*/

#ifdef SAVE_STATE

STATE_SAVE( video )
{
	state_save_word(neogeo_videoram, 0x10000);
	state_save_word(palettes[0], 0x1000);
	state_save_word(palettes[1], 0x1000);
	state_save_word(&videoram_read_buffer, 1);
	state_save_word(&videoram_offset, 1);
	state_save_word(&videoram_modulo, 1);
	state_save_long(&palette_bank, 1);

	state_save_long(&video_enable, 1);
	state_save_long(&fix_disable, 1);
	state_save_long(&spr_disable, 1);
}

STATE_LOAD( video )
{
	int i;

	state_load_word(neogeo_videoram, 0x10000);
	state_load_word(palettes[0], 0x1000);
	state_load_word(palettes[1], 0x1000);
	state_load_word(&videoram_read_buffer, 1);
	state_load_word(&videoram_offset, 1);
	state_load_word(&videoram_modulo, 1);
	state_load_long(&palette_bank, 1);

	state_load_long(&video_enable, 1);
	state_load_long(&fix_disable, 1);
	state_load_long(&spr_disable, 1);

	for (i = 0; i < 0x1000; i++)
	{
		if (i & 0x0f)
		{
			video_palettebank[0][i] = video_clut16[palettes[0][i] & 0x7fff];
			video_palettebank[1][i] = video_clut16[palettes[1][i] & 0x7fff];
		}
	}

	video_palette = video_palettebank[palette_bank];
}

#endif /* SAVE_STATE */
