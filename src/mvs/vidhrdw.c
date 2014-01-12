/******************************************************************************

	vidhrdw.c

	MVS ビデオエミュレーション

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

u16 ALIGN_DATA neogeo_vidram16[0x20000 / 2];
u16 neogeo_vidram16_offset;
u16 neogeo_vidram16_modulo;

u16 *neogeo_paletteram16;
u16 ALIGN_DATA neogeo_palettebank16[2][0x2000 / 2];
u32 neogeo_palette_index;

u16 *video_palette16;
u16 ALIGN_DATA video_palettebank16[2][0x2000 / 2];
u16 ALIGN_DATA video_clut16[0x8000];

u8 *video_fix_usage[2];
u8 *video_spr_usage;

int fix_bank;
u8  *fix_usage;
u8  *fix_memory;
int neogeo_fix_bank_type;


/******************************************************************************
	ローカル変数
******************************************************************************/

static u32 no_of_tiles;
static int next_update_first_line;

static u16 *sprite_scale = &neogeo_vidram16[0x10000 >> 1];
static u16 *sprite_yctrl = &neogeo_vidram16[0x10400 >> 1];
static u16 *sprite_xctrl = &neogeo_vidram16[0x10800 >> 1];

#include "zoom.c"


/******************************************************************************
	ローカル関数
******************************************************************************/

/*------------------------------------------------------
	FIXスプライト描画
------------------------------------------------------*/

static void draw_fix(void)
{
	u16 x, y, code, attr;

	if (fix_bank && neogeo_fix_bank_type)
	{
		int garouoffsets[32];

		if (neogeo_fix_bank_type == 1)
		{
			int garoubank = 0;
			int k = 0;

			y = 0;
			while (y < 32)
			{
				if (neogeo_vidram16[(0xea00 >> 1) + k] == 0x0200
				&& (neogeo_vidram16[(0xeb00 >> 1) + k] & 0xff00) == 0xff00)
				{
					garoubank = neogeo_vidram16[(0xeb00 >> 1) + k] & 3;
					garouoffsets[y++] = garoubank;
				}
				garouoffsets[y++] = garoubank;
				k += 2;
			}
		}

		for (y = 16/8; y < 240/8; y++)
		{
			for (x = 1/8; x < 312/8; x++)
			{
				code = neogeo_vidram16[(0xe000 >> 1) + y + (x << 5)];
				attr = (code & 0xf000) >> 12;
				code &= 0x0fff;

				switch (neogeo_fix_bank_type)
				{
				case 1:
					/* Garou, MSlug 3 */
					code += 0x1000 * (garouoffsets[(y - 2) & 31] ^ 3);
					break;

				case 2:
					code += 0x1000 * (((neogeo_vidram16[(0xea00 >> 1) + ((y - 1) & 31) + 32 * (x / 6)] >> (5 - (x % 6)) * 2) & 3) ^ 3);
					break;
				}

				if (fix_usage[code])
				{
					blit_draw_fix(x << 3, y << 3, code, attr);
				}
			}
		}
	}
	else
	{
		for (y = 16/8; y < 240/8; y++)
		{
			for (x = 1/8; x < 312/8; x++)
			{
				code  = neogeo_vidram16[(0xe000 >> 1) + y + (x << 5)];
				attr = (code & 0xf000) >> 12;
				code &= 0x0fff;

				if (fix_usage[code])
				{
					blit_draw_fix(x << 3, y << 3, code, attr);
				}
			}
		}
	}

	blit_finish_fix();
}


/*------------------------------------------------------
	SPRスプライト描画
------------------------------------------------------*/

#define DRAW_SPRITE()											\
	if (sy2 + zy >= min_y && sy2 <= max_y)						\
	{															\
		code = sprite_base[0];									\
		attr = sprite_base[1];									\
		code |= (attr & 0x70) << 12;							\
																\
		if (attr & 0x08)										\
			code = (code & ~7) | (neogeo_frame_counter & 7);	\
		else if (attr & 0x04)									\
			code = (code & ~3) | (neogeo_frame_counter & 3);	\
																\
		code %= no_of_tiles;									\
																\
		if (video_spr_usage[code])								\
			blit_draw_spr(sx, sy2, zx + 1, zy, code, attr);		\
	}

static void draw_spr(int min_y, int max_y)
{
	int sx = 0, sy = 0, my = 0, zx = 0, zy = 0, rzy = 0;
	u32 offset, code, attr;
	int fullmode = 0, y, sy2;
	u16 scale, yctrl;
	u16 *sprite_base;

	for (offset = 0; offset < 0x300 >> 1; offset++)
	{
		scale = sprite_scale[offset];
		yctrl = sprite_yctrl[offset];

		if (yctrl & 0x40)
		{
			sx += zx + 1;
		}
		else
		{
			rzy = scale & 0xff;

			sy = 0x200 - (yctrl >> 7);

			my = yctrl & 0x3f;

			if (my > 0x20)
			{
				fullmode = 1;
				my = 0x20;

				if (sy > 0x100) sy -= 0x200;
				while (sy < -16) sy += (rzy + 1) << 1;
			}
			else
				fullmode = 0;

			sx = sprite_xctrl[offset] >> 7;
		}

		if (sx >= 0x1f0) sx -= 0x200;

		zx = (scale >> 8) & 0x0f;

		if (sx + zx < 8 || sx > 311) continue;
		if (!my || !rzy) continue;

		sprite_base = &neogeo_vidram16[offset << 6];

		if (fullmode)
		{
			sy2 = sy;

			for (y = 0; y < my; y++)
			{
				if ((zy = yskip[rzy][y]))
				{
					if (sy2 >= 248) sy2 -= (rzy + 1) << 1;
					DRAW_SPRITE()
					sy2 += zy;
				}

				sprite_base += 2;
			}
		}
		else
		{
			int drawn_lines = 0;

			y = 0;

			while (drawn_lines < my << 4)
			{
				if ((zy = yskip[rzy][y]))
				{
					sy2 = (sy + drawn_lines) & 0x1ff;
					DRAW_SPRITE()
					drawn_lines += zy;
				}

				y++;
				if (y == 0x10 || y == 0x20)
					drawn_lines += (0xff - rzy) << 1;

				sprite_base += 2;
			}
		}
	}

	blit_finish_spr();
}


/******************************************************************************
	MVS ビデオ描画処理
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

				u16 color = ((r & 1) << 14) | ((r & 0x1e) << 7)
						  | ((g & 1) << 13) | ((g & 0x1e) << 3)
						  | ((b & 1) << 12) | ((b & 0x1e) >> 1);

				video_clut16[color] = MAKECOL15(r1, g1, b1);
			}
		}
	}

	no_of_tiles = memory_length_gfx3 / 128;

	memset(neogeo_vidram16, 0, sizeof(neogeo_vidram16));
	memset(neogeo_palettebank16, 0, sizeof(neogeo_palettebank16));
	memset(video_palettebank16, 0, sizeof(video_palettebank16));

	for (i = 0; i < 0x1000; i += 16)
	{
		video_palettebank16[0][i] = 0x8000;
		video_palettebank16[1][i] = 0x8000;
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
	neogeo_paletteram16 = neogeo_palettebank16[0];
	video_palette16 = video_palettebank16[0];
	neogeo_palette_index = 0;
	neogeo_vidram16_modulo = 1;
	neogeo_vidram16_offset = 0;

	next_update_first_line = FIRST_VISIBLE_LINE;

	if (neogeo_bios == ASIA_AES)
	{
		fix_bank   = 1;
		fix_usage  = video_fix_usage[1];
		fix_memory = memory_region_gfx2;
	}
	else
	{
		fix_bank   = 0;
		fix_usage  = video_fix_usage[0];
		fix_memory = memory_region_gfx1;
	}

	blit_reset();
}


/******************************************************************************
	画面更新処理
******************************************************************************/

/*------------------------------------------------------
	スクリーン更新
------------------------------------------------------*/

void neogeo_screenrefresh(void)
{
	blit_partial_start(FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE);
	draw_spr(FIRST_VISIBLE_LINE, LAST_VISIBLE_LINE);
	draw_fix();
	blit_finish();
}


/*------------------------------------------------------
	スクリーン更新 (Raster driver)
------------------------------------------------------*/

void neogeo_partial_screenrefresh(int current_line)
{
	if (current_line >= next_update_first_line)
	{
		blit_partial_start(next_update_first_line, current_line);
		draw_spr(next_update_first_line, current_line);
	}

	next_update_first_line = current_line + 1;
}


void neogeo_raster_screenrefresh(void)
{
	neogeo_partial_screenrefresh(LAST_VISIBLE_LINE);
	draw_fix();
	blit_finish();
	next_update_first_line = FIRST_VISIBLE_LINE;
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( video )
{
	state_save_word(neogeo_vidram16, 0x10000);
	state_save_word(neogeo_palettebank16[0], 0x1000);
	state_save_word(neogeo_palettebank16[1], 0x1000);
	state_save_word(&neogeo_vidram16_offset, 1);
	state_save_word(&neogeo_vidram16_modulo, 1);
	state_save_long(&neogeo_palette_index, 1);

	state_save_long(&fix_bank, 1);
}

STATE_LOAD( video )
{
	int i;

	state_load_word(neogeo_vidram16, 0x10000);
	state_load_word(neogeo_palettebank16[0], 0x1000);
	state_load_word(neogeo_palettebank16[1], 0x1000);
	state_load_word(&neogeo_vidram16_offset, 1);
	state_load_word(&neogeo_vidram16_modulo, 1);
	state_load_long(&neogeo_palette_index, 1);

	state_load_long(&fix_bank, 1);

	for (i = 0; i < 0x1000; i++)
	{
		if (i & 0x0f)
		{
			video_palettebank16[0][i] = video_clut16[neogeo_palettebank16[0][i] & 0x7fff];
			video_palettebank16[1][i] = video_clut16[neogeo_palettebank16[1][i] & 0x7fff];
		}
	}

	neogeo_paletteram16 = neogeo_palettebank16[neogeo_palette_index];
	video_palette16 = video_palettebank16[neogeo_palette_index];
}

#endif /* SAVE_STATE */
