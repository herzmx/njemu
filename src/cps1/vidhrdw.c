/******************************************************************************

	vidhrdw.c

	CPS1 ビデオエミュレーション

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

int cps_flip_screen;
int cps_rotate_screen;
int cps_raster_enable;


/******************************************************************************
	ローカル変数
******************************************************************************/

#define cps1_scroll_size		0x4000		/* scroll1, scroll2, scroll3 */
#define cps1_other_size			0x0800
#define cps1_palette_align		0x0800		/* can't be larger than this, breaks ringdest & batcirc otherwise */
#define cps1_palette_size		6*32*16*2	/* Size of palette RAM */

#define cps1_scroll_mask		(((~0x3fff) & 0x3ffff) >> 1)
#define cps1_other_mask			(((~0x07ff) & 0x3ffff) >> 1)
#define cps1_palette_mask		(((~0x07ff) & 0x3ffff) >> 1)

static u16 *cps_scroll1;
static u16 *cps_scroll2;
static u16 *cps_scroll3;
static u16 *cps_other;
static u16 *cps1_palette;
static u16 ALIGN_DATA cps1_old_palette[cps1_palette_size >> 1];

static int cps_layer_enabled[4];			/* Layer enabled [Y/N] */
static int cps_scroll1x, cps_scroll1y;
static int cps_scroll2x, cps_scroll2y;
static int cps_scroll3x, cps_scroll3y;
static u32 cps1_object_num;					/* object total sprites num */
static u8  *cps1_object_pen_usage;			/* object sprites pen usage */
static u16 ALIGN_DATA cps1_pen_usage[4][0x10000];		/* scroll sprites pen usage */

#define cps1_obj_size			0x800
#define cps1_max_obj			(cps1_obj_size >> 3)
#define cps1_obj_mask			(((~0x07ff) & 0x3ffff) >> 1)

struct cps1_object_t
{
	u16 sx;
	u16 sy;
	u16 code;
	u16 attr;
};

static struct cps1_object_t ALIGN_DATA cps1_object[cps1_max_obj + 1];
static struct cps1_object_t *cps1_last_object;

static int cps1_transparency_scroll[4];	/* Transparency pens of scroll layers */
static int cps1_has_stars;
static int cps1_stars1x, cps1_stars1y;
static int cps1_stars2x, cps1_stars2y;
static int cps1_high_layer;
static int cps1_kludge;

static u16 ALIGN_DATA video_clut16[65536];
u16 ALIGN_DATA video_palette[cps1_palette_size >> 1];


/* CPS1 output port */
#define CPS1_OBJ_BASE			0x00    /* Base address of objects */
#define CPS1_SCROLL1_BASE		0x02    /* Base address of scroll 1 */
#define CPS1_SCROLL2_BASE		0x04    /* Base address of scroll 2 */
#define CPS1_SCROLL3_BASE		0x06    /* Base address of scroll 3 */
#define CPS1_OTHER_BASE			0x08    /* Base address of other video */
#define CPS1_PALETTE_BASE		0x0a    /* Base address of palette */
#define CPS1_SCROLL1_SCROLLX	0x0c    /* Scroll 1 X */
#define CPS1_SCROLL1_SCROLLY	0x0e    /* Scroll 1 Y */
#define CPS1_SCROLL2_SCROLLX	0x10    /* Scroll 2 X */
#define CPS1_SCROLL2_SCROLLY	0x12    /* Scroll 2 Y */
#define CPS1_SCROLL3_SCROLLX	0x14    /* Scroll 3 X */
#define CPS1_SCROLL3_SCROLLY	0x16    /* Scroll 3 Y */
#define CPS1_STARS1_SCROLLX		0x18    /* Stars 1 X */
#define CPS1_STARS1_SCROLLY		0x1a    /* Stars 1 Y */
#define CPS1_STARS2_SCROLLX		0x1c    /* Stars 2 X */
#define CPS1_STARS2_SCROLLY		0x1e    /* Stars 2 Y */
#define CPS1_VIDEO_CONTROL		0x22    /* Video control */

/* CPS1 other RAM */
#define CPS1_ROWSCROLL_OFFS		0x20    /* base of row scroll offsets in other RAM */
#define CPS1_SCROLL2_WIDTH		0x40
#define CPS1_SCROLL2_HEIGHT		0x40


/*------------------------------------------------------
	CPSポート読み込み
------------------------------------------------------*/

#define cps1_port(offset)	cps1_output[(offset) >> 1]


/******************************************************************************
	CPS1 メモリハンドラ
******************************************************************************/

READ16_HANDLER( cps1_output_r )
{
	offset &= 0x7f;

	if (offset)
	{
		/* Some games interrogate a couple of registers on bootup. */
		/* These are CPS1 board B self test checks. They wander from game to */
		/* game. */
		if (offset == driver->cpsb_addr >> 1)
			return driver->cpsb_value;

		/* some games use as a protection check the ability to do 16-bit multiplies */
		/* with a 32-bit result, by writing the factors to two ports and reading the */
		/* result from two other ports. */
		if (offset == driver->mult_result_lo >> 1)
			return (cps1_output[driver->mult_factor1 >> 1] * cps1_output[driver->mult_factor2 >> 1]) & 0xffff;
		if (offset == driver->mult_result_hi >> 1)
			return (cps1_output[driver->mult_factor1 >> 1] * cps1_output[driver->mult_factor2 >> 1]) >> 16;

		/* Pang 3 EEPROM interface */
		if (cps1_kludge == CPS1_KLUDGE_PANG3 && offset == 0x7a/2)
			return cps1_eeprom_port_r(0, mem_mask);
	}

	return cps1_output[offset];
}

WRITE16_HANDLER( cps1_output_w )
{
	offset &= 0x7f;

	/* Pang 3 EEPROM interface */
	if (cps1_kludge == CPS1_KLUDGE_PANG3 && offset == 0x7a/2)
	{
		cps1_eeprom_port_w(0, data, mem_mask);
		return;
	}

	COMBINE_DATA(&cps1_output[offset]);
}


/******************************************************************************
	CPS1 ビデオ描画処理
******************************************************************************/

/*------------------------------------------------------
	カラーテーブル作成
------------------------------------------------------*/

static void cps1_init_tables(void)
{
	int r, g, b, bright;

	for (bright = 0; bright < 16; bright++)
	{
		for (r = 0; r < 16; r++)
		{
			for (g = 0; g < 16; g++)
			{
				for (b = 0; b < 16; b++)
				{
					u16 pen;
					float fr, fg, fb, fbright;

					pen = (bright << 12) | (r << 8) | (g << 4) | b;

					if (bright)
						fbright = (float)(bright + 1);
					else
						fbright = 0;

					fr = ((float)r * fbright) / (16.0 * 16.0);
					fg = ((float)g * fbright) / (16.0 * 16.0);
					fb = ((float)b * fbright) / (16.0 * 16.0);

					fr *= 255.0;
					fg *= 255.0;
					fb *= 255.0;

					if (fr > 255.0) fr = 255.0;
					if (fg > 255.0) fg = 255.0;
					if (fb > 255.0) fb = 255.0;
					if (fr < 16.0) fr = 0.0;
					if (fg < 16.0) fg = 0.0;
					if (fb < 16.0) fb = 0.0;

					video_clut16[pen] = MAKECOL15((int)fr, (int)fg, (int)fb);
				}
			}
		}
	}
}


/*------------------------------------------------------
	スプライトデコード
 -----------------------------------------------------*/

static int cps1_gfx_decode(void)
{
	int i, j;
	u8 *gfx = memory_region_gfx1;
	int size = memory_length_gfx1;

	if (driver->has_stars)
		i = 0x10000 >> 2;
	else
		i = 0;

	for (; i < size >> 2; i++)
	{
		u32 src = gfx[4 * i] + (gfx[4 * i + 1] << 8) + (gfx[4 * i + 2] << 16) + (gfx[4 * i + 3] << 24);
		u32 dwval = 0;

		for (j = 0; j < 8; j++)
		{
			int n = 0;
			u32 mask = (0x80808080 >> j) & src;

			if (mask & 0x000000ff) n |= 1;
			if (mask & 0x0000ff00) n |= 2;
			if (mask & 0x00ff0000) n |= 4;
			if (mask & 0xff000000) n |= 8;

			dwval |= n << (j * 4);
		}
		gfx[4 * i + 0] = dwval >>  0;
		gfx[4 * i + 1] = dwval >>  8;
		gfx[4 * i + 2] = dwval >> 16;
		gfx[4 * i + 3] = dwval >> 24;
	}

	if (driver->gfx_limit)
	{
		for (i = driver->gfx_limit + 1; i < size; i++)
			memory_region_gfx1[i] = 0xff;
		size = driver->gfx_limit + 1;
	}

	cps1_object_pen_usage = NULL;
	memset(cps1_pen_usage, 0, sizeof(cps1_pen_usage));

	//---------------------------------------------------------
	// object (16x16)
	//---------------------------------------------------------
	cps1_object_num = size >> 7;
	if ((cps1_object_pen_usage = calloc(1, cps1_object_num)) != NULL)
	{
		int count;
		u32 *gfx;
		u32 base, start, end;

		if (driver->has_stars)
			start = 0x10000 >> 7;
		else
			start = 0;

		for (i = start; i < cps1_object_num; i++)
		{
			count = 0;
			gfx = (u32 *)&memory_region_gfx1[i << 7];

			for (j = 0; j < 2*16; j++)
			{
				if (*gfx++ == 0xffffffff) count++;
			}
			if (count != 2*16) cps1_object_pen_usage[i] = 1;
		}

		if (cps1_kludge != CPS1_KLUDGE_PANG3)
		{
			base  = driver->bank_scroll1 << 15;
			start = driver->scroll1.start;
			end   = driver->scroll1.end;

			start = (base + start) << 6;
			end   = ((base + end) << 6) | 0x3ff;

			for (i = start >> 7; i <= end >> 7; i++)
				cps1_object_pen_usage[i] = 0;

			base  = driver->bank_scroll3 << 12;
			start = driver->scroll3.start;
			end   = driver->scroll3.end;

			start = (base + start) << 9;
			end   = ((base + end) << 9) | 0x1ff;

			for (i = start >> 7; i <= end >> 7; i++)
				cps1_object_pen_usage[i] = 0;

			if (cps1_kludge == CPS1_KLUDGE_MERCS)
			{
				start = (base + 0x1700) << 9;
				end   = ((base + 0x17ff) << 9) | 0x1ff;

				for (i = start >> 7; i <= end >> 7; i++)
					cps1_object_pen_usage[i] = 0;
			}
		}
	}
	else return 0;

	//---------------------------------------------------------
	// scroll1 (8x8)
	//---------------------------------------------------------
	{
		int k;
		u32 base  = driver->bank_scroll1 << 15;
		u32 start = driver->scroll1.start;
		u32 end   = driver->scroll1.end;
		u32 limit = (size >> 6) - base;
		u32 *gfx, tile;

		for (i = start; i <= end; i++)
		{
			if (i >= limit || i == 0x20) continue;

			gfx = (u32 *)&memory_region_gfx1[(base + i) << 6];

			for (j = 0; j < 8; j++)
			{
				tile = *gfx++;
				for (k = 0; k < 8; k++)
				{
					cps1_pen_usage[0][i] |= 1 << (tile & 0x0f);
					tile >>= 4;
				}
				tile = *gfx++;
				for (k = 0; k < 8; k++)
				{
					cps1_pen_usage[1][i] |= 1 << (tile & 0x0f);
					tile >>= 4;
				}
			}
			cps1_pen_usage[0][i] &= 0x7fff;
			cps1_pen_usage[1][i] &= 0x7fff;
		}
	}

	//---------------------------------------------------------
	// scroll2 (16x16)
	//---------------------------------------------------------
	{
		int k;
		u32 base  = driver->bank_scroll2 << 14;
		u32 start = driver->scroll2.start;
		u32 end   = driver->scroll2.end;
		u32 limit = (size >> 7) - base;
		u32 *gfx, tile;

scroll2_check:
		for (i = start; i <= end; i++)
		{
			if (i >= limit) continue;

			gfx = (u32 *)&memory_region_gfx1[(base + i) << 7];

			for (j = 0; j < 2*16; j++)
			{
				tile = *gfx++;
				for (k = 0; k < 8; k++)
				{
					cps1_pen_usage[2][i] |= 1 << (tile & 0x0f);
					tile >>= 4;
				}
			}
			cps1_pen_usage[2][i] &= 0x7fff;
		}
		if (cps1_kludge == CPS1_KLUDGE_MERCS)
		{
			if (start != 0x5400)
			{
				start = 0x5400;
				end   = 0x5bff;
				goto scroll2_check;
			}
		}
		else if (cps1_kludge == CPS1_KLUDGE_PANG3)
		{
			for (i = 0x0790; i < 0x079f; i++)
				cps1_pen_usage[2][i] = 1 << (i & 0x0f);
		}
	}

	//---------------------------------------------------------
	// scroll3 (32x32)
	//---------------------------------------------------------
	{
		int k;
		u32 base  = driver->bank_scroll3 << 12;
		u32 start = driver->scroll3.start;
		u32 end   = driver->scroll3.end;
		u32 limit = (size >> 9) - base;
		u32 *gfx, tile;

scroll3_check:
		for (i = start; i <= end; i++)
		{
			if (i >= limit) continue;

			gfx = (u32 *)&memory_region_gfx1[(base + i) << 9];

			for (j = 0; j < 4*32; j++)
			{
				tile = *gfx++;
				for (k = 0; k < 8; k++)
				{
					cps1_pen_usage[3][i] |= 1 << (tile & 0x0f);
					tile >>= 4;
				}
			}
			cps1_pen_usage[3][i] &= 0x7fff;
		}
		if (cps1_kludge == CPS1_KLUDGE_MERCS)
		{
			if (start != 0x1700)
			{
				start = 0x1700;
				end   = 0x17ff;
				goto scroll3_check;
			}
		}
	}

	return 1;
}


/*------------------------------------------------------
	CPS1ベースオフセット取得
 -----------------------------------------------------*/

static u16 *cps1_base(int offset, int address_mask)
{
	int base = (cps1_port(offset) << 7) & address_mask;

	return &cps1_gfxram[base];
}


/*------------------------------------------------------
	CPS1ビデオ初期化
------------------------------------------------------*/

int cps1_video_init(void)
{
	cps1_has_stars = driver->has_stars;
	cps1_kludge    = driver->kludge;

	cps1_init_tables();

	return cps1_gfx_decode();
}


/*------------------------------------------------------
	CPS1ビデオ終了
------------------------------------------------------*/

void cps1_video_exit(void)
{
	if (cps1_object_pen_usage)
	{
		free(cps1_object_pen_usage);
	}
}


/*------------------------------------------------------
	CPS1ビデオリセット
------------------------------------------------------*/

void cps1_video_reset(void)
{
	int i;

	memset(cps1_gfxram, 0, sizeof(cps1_gfxram));
	memset(cps1_output, 0, sizeof(cps1_output));

	memset(cps1_object, 0, sizeof(cps1_object));

	memset(cps1_old_palette, 0, sizeof(cps1_old_palette));
	memset(video_palette, 0, sizeof(video_palette));

	for (i = 0; i < cps1_palette_size >> 1; i += 16)
		video_palette[i + 15] = 0x8000;

	cps1_port(CPS1_OBJ_BASE)     = 0x9200;
	cps1_port(CPS1_SCROLL1_BASE) = 0x9000;
	cps1_port(CPS1_SCROLL2_BASE) = 0x9040;
	cps1_port(CPS1_SCROLL3_BASE) = 0x9080;
	cps1_port(CPS1_OTHER_BASE)   = 0x9100;
	cps1_port(CPS1_PALETTE_BASE) = 0x90c0;

	cps_layer_enabled[0] = 1;

	cps1_high_layer = 0;
	cps1_transparency_scroll[0] = 0x0000;
	cps1_transparency_scroll[1] = 0x0000;
	cps1_transparency_scroll[2] = 0x0000;
	cps1_transparency_scroll[3] = 0x0000;

	blit_reset(driver->bank_scroll1, driver->bank_scroll2, driver->bank_scroll3);
}


/*------------------------------------------------------
	パレット
------------------------------------------------------*/

void cps1_build_palette(void)
{
	int offset;
	u16 palette;

	cps1_palette = cps1_base(CPS1_PALETTE_BASE, cps1_palette_mask);

	for (offset = 0; offset < 4*32*16; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps1_palette[offset];

			if (palette != cps1_old_palette[offset])
			{
				cps1_old_palette[offset] = palette;
				video_palette[offset] = video_clut16[palette];
				blit_palette_mark_dirty(offset >> 4);
			}
		}
	}

	for (; offset < 6*32*16; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps1_palette[offset];

			if (palette != cps1_old_palette[offset])
			{
				cps1_old_palette[offset] = palette;
				video_palette[offset] = video_clut16[palette];
			}
		}
	}
}


/******************************************************************************
  Object (16x16)
******************************************************************************/

/*------------------------------------------------------
	object描画
------------------------------------------------------*/

#define SCAN_OBJECT(blit_func)													\
	int x, y, sx, sy, code, attr;												\
	int nx, ny, nsx, nsy;														\
	u16 ncode;																	\
	struct cps1_object_t *object = cps1_object;									\
																				\
	while (object < cps1_last_object)											\
	{																			\
		sx   = object->sx;														\
		sy   = object->sy;														\
		code = object->code;													\
		attr = object->attr;													\
		object++;																\
																				\
		switch (cps1_kludge)													\
		{																		\
		case CPS1_KLUDGE_FORGOTTN: code += 0x4000; break;						\
		case CPS1_KLUDGE_GHOULS:   if (code >= 0x1000) code += 0x4000; break;	\
		case CPS1_KLUDGE_3WONDERS: if (code >= 0x2a00) code += 0x4000; break;	\
		}																		\
																				\
		if (code >= cps1_object_num) continue;									\
																				\
		if (!(attr & 0xff00))													\
		{																		\
			if (cps1_object_pen_usage[code])									\
				blit_func(sx & 0x1ff, sy & 0x1ff, code, attr);					\
			continue;															\
		}																		\
																				\
		nx = (attr >>  8) & 0x0f;												\
		ny = (attr >> 12) & 0x0f;												\
																				\
		for (y = 0; y <= ny; y++)												\
		{																		\
			for (x = 0; x <= nx; x++)											\
			{																	\
				ncode = (code & ~0xf) + ((code + x) & 0xf) + (y << 4);			\
																				\
				if (cps1_object_pen_usage[ncode])								\
				{																\
					if (attr & 0x20)											\
						nsx = sx + ((nx - x) << 4);								\
					else														\
						nsx = sx + (x << 4);									\
																				\
					if (attr & 0x40)											\
						nsy = sy + ((ny - y) << 4);								\
					else														\
						nsy = sy + (y << 4);									\
																				\
					blit_func(nsx & 0x1ff, nsy & 0x1ff, ncode, attr);			\
				}																\
			}																	\
		}																		\
	}

/*------------------------------------------------------
	描画
------------------------------------------------------*/

void cps1_render_object(void)
{
	SCAN_OBJECT(blit_draw_object)

	blit_finish_object();
}


/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

void cps1_scan_object(void)
{
	SCAN_OBJECT(blit_update_object)
}


/******************************************************************************
  Scroll 1 (8x8 layer)
******************************************************************************/

/*------------------------------------------------------
	スプライトのオフセット取得
------------------------------------------------------*/

INLINE u32 scroll1_offset(u32 col, u32 row)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((col & 0x3f) << 5) + ((row & 0x20) << 6);
}

/*------------------------------------------------------
	描画
------------------------------------------------------*/

static void cps1_render_scroll1_normal(void)
{
	u32 offs, code, attr, gfxset;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll1x >> 3;
	int logical_row = cps_scroll1y >> 3;
	int scroll_col  = cps_scroll1x & 0x07;
	int scroll_row  = cps_scroll1y & 0x07;

	min_x = ( 64 + scroll_col) >> 3;
	max_x = (448 + scroll_col) >> 3;

	min_y = ( 16 + scroll_row) >> 3;
	max_y = (248 + scroll_row) >> 3;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 3) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs   = scroll1_offset(logical_col + x, logical_row + y) << 1;
			gfxset = (offs & 0x40) >> 6;
			code   = cps_scroll1[offs];

			if (cps1_pen_usage[gfxset][code])
			{
				attr = cps_scroll1[offs + 1];
				sx = (x << 3) - scroll_col;
				blit_draw_scroll1(sx, sy, code, attr, gfxset);
			}
		}
	}
}

static void cps1_render_scroll1_separate(void)
{
	u16 tpens;
	u32 offs, code, attr, gfxset;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll1x >> 3;
	int logical_row = cps_scroll1y >> 3;
	int scroll_col  = cps_scroll1x & 0x07;
	int scroll_row  = cps_scroll1y & 0x07;

	min_x = ( 64 + scroll_col) >> 3;
	max_x = (448 + scroll_col) >> 3;

	min_y = ( 16 + scroll_row) >> 3;
	max_y = (248 + scroll_row) >> 3;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 3) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs   = scroll1_offset(logical_col + x, logical_row + y) << 1;
			gfxset = (offs & 0x40) >> 6;
			code   = cps_scroll1[offs];
			attr   = cps_scroll1[offs + 1];
			tpens  = cps1_transparency_scroll[(attr & 0x0180) >> 7];
			sx     = (x << 3) - scroll_col;

			if (cps1_pen_usage[gfxset][code] & ~tpens)
			{
				blit_draw_scroll1(sx, sy, code, attr, gfxset);
			}
			if (cps1_pen_usage[gfxset][code] & tpens)
			{
				blit_draw_scroll1h(sx, sy, code, attr, tpens, gfxset);
			}
		}
	}
}

static void cps1_render_scroll1(void)
{
	if (cps1_high_layer == LAYER_SCROLL1)
		cps1_render_scroll1_separate();
	else
		cps1_render_scroll1_normal();

	blit_finish_scroll1();
}

/*------------------------------------------------------
	使用中のスプライトをスキャン
------------------------------------------------------*/

static void cps1_scan_scroll1_normal(void)
{
	u32 offs, code, attr, gfxset;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll1x >> 3;
	int logical_row = cps_scroll1y >> 3;
	int scroll_col  = cps_scroll1x & 0x07;
	int scroll_row  = cps_scroll1y & 0x07;

	min_x = ( 64 + scroll_col) >> 3;
	max_x = (448 + scroll_col) >> 3;

	min_y = ( 16 + scroll_row) >> 3;
	max_y = (248 + scroll_row) >> 3;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 3) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs   = scroll1_offset(logical_col + x, logical_row + y) << 1;
			gfxset = (offs & 0x40) >> 6;
			code   = cps_scroll1[offs];

			if (cps1_pen_usage[gfxset][code])
			{
				attr = cps_scroll1[offs + 1];
				sx = (x << 3) - scroll_col;
				blit_update_scroll1(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_scan_scroll1_background(void)
{
	u16 tpens;
	u32 offs, code, attr, gfxset;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll1x >> 3;
	int logical_row = cps_scroll1y >> 3;
	int scroll_col  = cps_scroll1x & 0x07;
	int scroll_row  = cps_scroll1y & 0x07;

	min_x = ( 64 + scroll_col) >> 3;
	max_x = (448 + scroll_col) >> 3;

	min_y = ( 16 + scroll_row) >> 3;
	max_y = (248 + scroll_row) >> 3;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 3) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs   = scroll1_offset(logical_col + x, logical_row + y) << 1;
			gfxset = (offs & 0x40) >> 6;
			code   = cps_scroll1[offs];
			attr   = cps_scroll1[offs + 1];
			tpens  = cps1_transparency_scroll[(attr & 0x0180) >> 7];

			if (cps1_pen_usage[gfxset][code] & ~tpens)
			{
				sx = (x << 3) - scroll_col;
				blit_update_scroll1(sx, sy, code, attr);
			}
		}
	}
}

void cps1_scan_scroll1_foreground(void)
{
	u16 tpens;
	u32 offs, code, attr, gfxset;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll1x >> 3;
	int logical_row = cps_scroll1y >> 3;
	int scroll_col  = cps_scroll1x & 0x07;
	int scroll_row  = cps_scroll1y & 0x07;

	min_x = ( 64 + scroll_col) >> 3;
	max_x = (448 + scroll_col) >> 3;

	min_y = ( 16 + scroll_row) >> 3;
	max_y = (248 + scroll_row) >> 3;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 3) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs   = scroll1_offset(logical_col + x, logical_row + y) << 1;
			gfxset = (offs & 0x40) >> 6;
			code   = cps_scroll1[offs];
			attr   = cps_scroll1[offs + 1];
			tpens  = cps1_transparency_scroll[(attr & 0x0180) >> 7];

			if (cps1_pen_usage[gfxset][code] & tpens)
			{
				sx = (x << 3) - scroll_col;
				blit_update_scroll1h(sx, sy, code, attr);
			}
		}
	}
}

void cps1_scan_scroll1(void)
{
	if (cps1_high_layer == LAYER_SCROLL1)
		cps1_scan_scroll1_background();
	else
		cps1_scan_scroll1_normal();
}


/******************************************************************************
  Scroll 2 (16x16 layer)
******************************************************************************/

/*------------------------------------------------------
	スプライトのオフセット取得
------------------------------------------------------*/

INLINE u32 scroll2_offset(u32 col, u32 row)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x30) << 6);
}

/*------------------------------------------------------
	描画
------------------------------------------------------*/

static void cps1_render_scroll2_normal_noraster(void)
{
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll2x >> 4;
	int logical_row = cps_scroll2y >> 4;
	int scroll_col  = cps_scroll2x & 0x0f;
	int scroll_row  = cps_scroll2y & 0x0f;

	min_x = ( 64 + scroll_col) >> 4;
	max_x = (448 + scroll_col) >> 4;

	min_y = ( 16 + scroll_row) >> 4;
	max_y = (248 + scroll_row) >> 4;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 4) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
			code = cps_scroll2[offs];

			if (cps1_pen_usage[2][code])
			{
				attr = cps_scroll2[offs + 1];
				sx = (x << 4) - scroll_col;
				blit_draw_scroll2(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_render_scroll2_separate_noraster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll2x >> 4;
	int logical_row = cps_scroll2y >> 4;
	int scroll_col  = cps_scroll2x & 0x0f;
	int scroll_row  = cps_scroll2y & 0x0f;

	min_x = ( 64 + scroll_col) >> 4;
	max_x = (448 + scroll_col) >> 4;

	min_y = ( 16 + scroll_row) >> 4;
	max_y = (248 + scroll_row) >> 4;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 4) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs  = scroll2_offset(logical_col + x, logical_row + y) << 1;
			code  = cps_scroll2[offs];
			attr  = cps_scroll2[offs + 1];
			tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];
			sx    = (x << 4) - scroll_col;

			if (cps1_pen_usage[2][code] & ~tpens)
			{
				blit_draw_scroll2(sx, sy, code, attr);
			}
			if (cps1_pen_usage[2][code] & tpens)
			{
				blit_draw_scroll2h(sx, sy, code, attr, tpens);
			}
		}
	}
}

static void cps1_render_scroll2_normal_raster(void)
{
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col, scroll_col;
	int logical_row = cps_scroll2y >> 4;
	int scroll_row  = cps_scroll2y & 0x0f;
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
	int line, scrollx[256];

	for (line = 16; line < 248; line++)
		scrollx[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

	for (line = 16; line < 248; )
	{
		int start_line = line;
		int end_line = line + 1;

		while (scrollx[end_line] == scrollx[start_line])
		{
			end_line++;
			if (end_line == 248) break;
		}

		logical_col = scrollx[start_line] >> 4;
		scroll_col  = scrollx[start_line] & 0x0f;

		min_x = ( 64 + scroll_col) >> 4;
		max_x = (448 + scroll_col) >> 4;

		start_line &= ~0x0f;
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;

		for (y = min_y; y <= max_y; y++)
		{
			sy = (y << 4) - scroll_row;

			for (x = min_x; x <= max_x; x++)
			{
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
				code = cps_scroll2[offs];

				if (cps1_pen_usage[2][code])
				{
					attr = cps_scroll2[offs + 1];
					sx = (x << 4) - scroll_col;
					blit_draw_scroll2(sx, sy, code, attr);
				}
			}
		}
		blit_finish_scroll2(line, end_line);
		line = end_line;
	}
}

static void cps1_render_scroll2_background_raster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col, scroll_col;
	int logical_row = cps_scroll2y >> 4;
	int scroll_row  = cps_scroll2y & 0x0f;
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
	int line, scrollx[256];

	for (line = 16; line < 248; line++)
		scrollx[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

	for (line = 16; line < 248; )
	{
		int start_line = line;
		int end_line = line + 1;

		while (scrollx[end_line] == scrollx[start_line])
		{
			end_line++;
			if (end_line == 248) break;
		}

		logical_col = scrollx[start_line] >> 4;
		scroll_col  = scrollx[start_line] & 0x0f;

		min_x = ( 64 + scroll_col) >> 4;
		max_x = (448 + scroll_col) >> 4;

		start_line &= ~0x0f;
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;

		for (y = min_y; y <= max_y; y++)
		{
			sy = (y << 4) - scroll_row;

			for (x = min_x; x <= max_x; x++)
			{
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
				code = cps_scroll2[offs];
				attr = cps_scroll2[offs + 1];
				tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

				if (cps1_pen_usage[2][code] & ~tpens)
				{
					sx = (x << 4) - scroll_col;
					blit_draw_scroll2(sx, sy, code, attr);
				}
			}
		}
		blit_finish_scroll2(line, end_line);
		line = end_line;
	}
}

static void cps1_render_scroll2_foreground_raster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col, scroll_col;
	int logical_row = cps_scroll2y >> 4;
	int scroll_row  = cps_scroll2y & 0x0f;
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
	int line, scrollx[256];

	for (line = 16; line < 248; line++)
		scrollx[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

	for (line = 16; line < 248; )
	{
		int start_line = line;
		int end_line = line + 1;

		while (scrollx[end_line] == scrollx[start_line])
		{
			end_line++;
			if (end_line == 248) break;
		}

		logical_col = scrollx[start_line] >> 4;
		scroll_col  = scrollx[start_line] & 0x0f;

		min_x = ( 64 + scroll_col) >> 4;
		max_x = (448 + scroll_col) >> 4;

		start_line &= ~0x0f;
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;

		for (y = min_y; y <= max_y; y++)
		{
			sy = (y << 4) - scroll_row;

			for (x = min_x; x <= max_x; x++)
			{
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
				code = cps_scroll2[offs];
				attr = cps_scroll2[offs + 1];
				tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

				if (cps1_pen_usage[2][code] & tpens)
				{
					sx = (x << 4) - scroll_col;
					blit_draw_scroll2h(sx, sy, code, attr, tpens);
				}
			}
		}
		blit_finish_scroll2h(line, end_line);
		line = end_line;
	}
}

static void cps1_render_scroll2(void)
{
	int distort = cps1_port(CPS1_VIDEO_CONTROL) & 0x0001;

	if (cps_raster_enable && distort)
	{
		if (cps1_high_layer == LAYER_SCROLL2)
			cps1_render_scroll2_background_raster();
		else
			cps1_render_scroll2_normal_raster();
	}
	else
	{
		if (distort)
		{
			int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
			cps_scroll2x += cps_other[(128 + otheroffs) & 0x3ff];
		}

		if (cps1_high_layer == LAYER_SCROLL2)
			cps1_render_scroll2_separate_noraster();
		else
			cps1_render_scroll2_normal_noraster();

		blit_finish_scroll2(16, 248);
	}
}

static void cps1_render_scroll2_foreground(void)
{
	int distort = cps1_port(CPS1_VIDEO_CONTROL) & 0x0001;

	if (cps_raster_enable && distort)
	{
		cps1_render_scroll2_foreground_raster();
	}
	else
	{
		blit_finish_scroll2h(16, 248);
	}
}

/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

static void cps1_scan_scroll2_normal_noraster(void)
{
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll2x >> 4;
	int logical_row = cps_scroll2y >> 4;
	int scroll_col  = cps_scroll2x & 0x0f;
	int scroll_row  = cps_scroll2y & 0x0f;

	min_x = ( 64 + scroll_col) >> 4;
	max_x = (448 + scroll_col) >> 4;

	min_y = ( 16 + scroll_row) >> 4;
	max_y = (248 + scroll_row) >> 4;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 4) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
			code = cps_scroll2[offs];

			if (cps1_pen_usage[2][code])
			{
				attr = cps_scroll2[offs + 1];
				sx = (x << 4) - scroll_col;
				blit_update_scroll2(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_scan_scroll2_background_noraster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll2x >> 4;
	int logical_row = cps_scroll2y >> 4;
	int scroll_col  = cps_scroll2x & 0x0f;
	int scroll_row  = cps_scroll2y & 0x0f;

	min_x = ( 64 + scroll_col) >> 4;
	max_x = (448 + scroll_col) >> 4;

	min_y = ( 16 + scroll_row) >> 4;
	max_y = (248 + scroll_row) >> 4;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 4) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
			code = cps_scroll2[offs];
			attr = cps_scroll2[offs + 1];
			tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

			if (cps1_pen_usage[2][code] & ~tpens)
			{
				sx = (x << 4) - scroll_col;
				blit_update_scroll2(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_scan_scroll2_foreground_noraster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll2x >> 4;
	int logical_row = cps_scroll2y >> 4;
	int scroll_col  = cps_scroll2x & 0x0f;
	int scroll_row  = cps_scroll2y & 0x0f;

	min_x = ( 64 + scroll_col) >> 4;
	max_x = (448 + scroll_col) >> 4;

	min_y = ( 16 + scroll_row) >> 4;
	max_y = (248 + scroll_row) >> 4;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 4) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
			code = cps_scroll2[offs];
			attr = cps_scroll2[offs + 1];
			tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

			if (cps1_pen_usage[2][code] & tpens)
			{
				sx = (x << 4) - scroll_col;
				blit_update_scroll2h(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_scan_scroll2_normal_raster(void)
{
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col, scroll_col;
	int logical_row = cps_scroll2y >> 4;
	int scroll_row  = cps_scroll2y & 0x0f;
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
	int line, scrollx[256];

	for (line = 16; line < 248; line++)
		scrollx[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

	for (line = 16; line < 248; )
	{
		int start_line = line;
		int end_line = line + 1;

		while (scrollx[end_line] == scrollx[start_line])
		{
			end_line++;
			if (end_line == 248) break;
		}

		logical_col = scrollx[start_line] >> 4;
		scroll_col  = scrollx[start_line] & 0x0f;

		min_x = ( 64 + scroll_col) >> 4;
		max_x = (448 + scroll_col) >> 4;

		start_line &= ~0x0f;
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;

		for (y = min_y; y <= max_y; y++)
		{
			sy = (y << 4) - scroll_row;

			for (x = min_x; x <= max_x; x++)
			{
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
				code = cps_scroll2[offs];

				if (cps1_pen_usage[2][code])
				{
					attr = cps_scroll2[offs + 1];
					sx = (x << 4) - scroll_col;
					blit_update_scroll2(sx, sy, code, attr);
				}
			}
		}
		line = end_line;
	}
}

static void cps1_scan_scroll2_background_raster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col, scroll_col;
	int logical_row = cps_scroll2y >> 4;
	int scroll_row  = cps_scroll2y & 0x0f;
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
	int line, scrollx[256];

	for (line = 16; line < 248; line++)
		scrollx[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

	for (line = 16; line < 248; )
	{
		int start_line = line;
		int end_line = line + 1;

		while (scrollx[end_line] == scrollx[start_line])
		{
			end_line++;
			if (end_line == 248) break;
		}

		logical_col = scrollx[start_line] >> 4;
		scroll_col  = scrollx[start_line] & 0x0f;

		min_x = ( 64 + scroll_col) >> 4;
		max_x = (448 + scroll_col) >> 4;

		start_line &= ~0x0f;
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;

		for (y = min_y; y <= max_y; y++)
		{
			sy = (y << 4) - scroll_row;

			for (x = min_x; x <= max_x; x++)
			{
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
				code = cps_scroll2[offs];
				attr = cps_scroll2[offs + 1];
				tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

				if (cps1_pen_usage[2][code] & ~tpens)
				{
					sx = (x << 4) - scroll_col;
					blit_update_scroll2(sx, sy, code, attr);
				}
			}
		}
		line = end_line;
	}
}

static void cps1_scan_scroll2_foreground_raster(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col, scroll_col;
	int logical_row = cps_scroll2y >> 4;
	int scroll_row  = cps_scroll2y & 0x0f;
	int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);
	int line, scrollx[256];

	for (line = 16; line < 248; line++)
		scrollx[line] = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

	for (line = 16; line < 248; )
	{
		int start_line = line;
		int end_line = line + 1;

		while (scrollx[end_line] == scrollx[start_line])
		{
			end_line++;
			if (end_line == 248) break;
		}

		logical_col = scrollx[start_line] >> 4;
		scroll_col  = scrollx[start_line] & 0x0f;

		min_x = ( 64 + scroll_col) >> 4;
		max_x = (448 + scroll_col) >> 4;

		start_line &= ~0x0f;
		min_y = ((start_line + scroll_row) & 0x3ff) >> 4;
		max_y = ((end_line + scroll_row) & 0x3ff) >> 4;

		for (y = min_y; y <= max_y; y++)
		{
			sy = (y << 4) - scroll_row;

			for (x = min_x; x <= max_x; x++)
			{
				offs = scroll2_offset(logical_col + x, logical_row + y) << 1;
				code = cps_scroll2[offs];
				attr = cps_scroll2[offs + 1];
				tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

				if (cps1_pen_usage[2][code] & tpens)
				{
					sx = (x << 4) - scroll_col;
					blit_update_scroll2h(sx, sy, code, attr);
				}
			}
		}
		line = end_line;
	}
}

void cps1_scan_scroll2(void)
{
	int distort = cps1_port(CPS1_VIDEO_CONTROL) & 0x0001;

	if (cps_raster_enable && distort)
	{
		if (cps1_high_layer == LAYER_SCROLL2)
			cps1_scan_scroll2_background_raster();
		else
			cps1_scan_scroll2_normal_raster();
	}
	else
	{
		if (cps1_high_layer == LAYER_SCROLL2)
			cps1_scan_scroll2_background_noraster();
		else
			cps1_scan_scroll2_normal_noraster();
	}
}

void cps1_scan_scroll2_foreground(void)
{
	int distort = cps1_port(CPS1_VIDEO_CONTROL) & 0x0001;

	if (cps_raster_enable && distort)
	{
		cps1_scan_scroll2_foreground_raster();
	}
	else
	{
		cps1_scan_scroll2_foreground_noraster();
	}
}


/******************************************************************************
  Scroll 3 (32x32 layer)
******************************************************************************/

/*------------------------------------------------------
	スプライトのオフセット取得
------------------------------------------------------*/

INLINE u32 scroll3_offset(u32 col, u32 row)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x07) + ((col & 0x3f) << 3) + ((row & 0x38) << 6);
}

/*------------------------------------------------------
	描画
------------------------------------------------------*/

static void cps1_render_scroll3_normal(void)
{
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll3x >> 5;
	int logical_row = cps_scroll3y >> 5;
	int scroll_col  = cps_scroll3x & 0x1f;
	int scroll_row  = cps_scroll3y & 0x1f;

	min_x = ( 64 + scroll_col) >> 5;
	max_x = (448 + scroll_col) >> 5;

	min_y = ( 16 + scroll_row) >> 5;
	max_y = (248 + scroll_row) >> 5;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 5) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs = scroll3_offset(logical_col + x, logical_row + y) << 1;
			code = cps_scroll3[offs];

			if (cps1_kludge == CPS1_KLUDGE_3WONDERS)
				if (code < 0x0e00) code += 0x1000;

			if (cps1_pen_usage[3][code])
			{
				attr = cps_scroll3[offs + 1];
				sx = (x << 5) - scroll_col;
				blit_draw_scroll3(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_render_scroll3_separate(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll3x >> 5;
	int logical_row = cps_scroll3y >> 5;
	int scroll_col  = cps_scroll3x & 0x1f;
	int scroll_row  = cps_scroll3y & 0x1f;

	min_x = ( 64 + scroll_col) >> 5;
	max_x = (448 + scroll_col) >> 5;

	min_y = ( 16 + scroll_row) >> 5;
	max_y = (248 + scroll_row) >> 5;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 5) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs  = scroll3_offset(logical_col + x, logical_row + y) << 1;
			code  = cps_scroll3[offs];
			attr  = cps_scroll3[offs + 1];
			tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];
			sx    = (x << 5) - scroll_col;

			if (cps1_kludge == CPS1_KLUDGE_3WONDERS)
				if (code < 0x0e00) code += 0x1000;

			if (cps1_pen_usage[3][code] & ~tpens)
			{
				blit_draw_scroll3(sx, sy, code, attr);
			}
			if (cps1_pen_usage[3][code] & tpens)
			{
				blit_draw_scroll3h(sx, sy, code, attr, tpens);
			}
		}
	}
}

static void cps1_render_scroll3(void)
{
	if (cps1_high_layer == LAYER_SCROLL3)
		cps1_render_scroll3_separate();
	else
		cps1_render_scroll3_normal();

	blit_finish_scroll3();
}

/*------------------------------------------------------
	使用中のスプライトをチェック
------------------------------------------------------*/

static void cps1_scan_scroll3_normal(void)
{
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll3x >> 5;
	int logical_row = cps_scroll3y >> 5;
	int scroll_col  = cps_scroll3x & 0x1f;
	int scroll_row  = cps_scroll3y & 0x1f;

	min_x = ( 64 + scroll_col) >> 5;
	max_x = (448 + scroll_col) >> 5;

	min_y = ( 16 + scroll_row) >> 5;
	max_y = (248 + scroll_row) >> 5;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 5) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs = scroll3_offset(logical_col + x, logical_row + y) << 1;
			code = cps_scroll3[offs];

			if (cps1_kludge == CPS1_KLUDGE_3WONDERS)
				if (code < 0x0e00) code += 0x1000;

			if (cps1_pen_usage[3][code])
			{
				attr = cps_scroll3[offs + 1];
				sx = (x << 5) - scroll_col;
				blit_update_scroll3(sx, sy, code, attr);
			}
		}
	}
}

static void cps1_scan_scroll3_background(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll3x >> 5;
	int logical_row = cps_scroll3y >> 5;
	int scroll_col  = cps_scroll3x & 0x1f;
	int scroll_row  = cps_scroll3y & 0x1f;

	min_x = ( 64 + scroll_col) >> 5;
	max_x = (448 + scroll_col) >> 5;

	min_y = ( 16 + scroll_row) >> 5;
	max_y = (248 + scroll_row) >> 5;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 5) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs  = scroll3_offset(logical_col + x, logical_row + y) << 1;
			code  = cps_scroll3[offs];
			attr  = cps_scroll3[offs + 1];
			tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

			if (cps1_kludge == CPS1_KLUDGE_3WONDERS)
				if (code < 0x0e00) code += 0x1000;

			if (cps1_pen_usage[3][code] & ~tpens)
			{
				sx = (x << 5) - scroll_col;
				blit_update_scroll3(sx, sy, code, attr);
			}
		}
	}
}

void cps1_scan_scroll3_foreground(void)
{
	u16 tpens;
	u32 offs, code, attr;
	int x, y, sx, sy, min_x, max_x, min_y, max_y;
	int logical_col = cps_scroll3x >> 5;
	int logical_row = cps_scroll3y >> 5;
	int scroll_col  = cps_scroll3x & 0x1f;
	int scroll_row  = cps_scroll3y & 0x1f;

	min_x = ( 64 + scroll_col) >> 5;
	max_x = (448 + scroll_col) >> 5;

	min_y = ( 16 + scroll_row) >> 5;
	max_y = (248 + scroll_row) >> 5;

	for (y = min_y; y <= max_y; y++)
	{
		sy = (y << 5) - scroll_row;

		for (x = min_x; x <= max_x; x++)
		{
			offs  = scroll3_offset(logical_col + x, logical_row + y) << 1;
			code  = cps_scroll3[offs];
			attr  = cps_scroll3[offs + 1];
			tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];

			if (cps1_kludge == CPS1_KLUDGE_3WONDERS)
				if (code < 0x0e00) code += 0x1000;

			if (cps1_pen_usage[3][code] & tpens)
			{
				sx = (x << 5) - scroll_col;
				blit_update_scroll3h(sx, sy, code, attr);
			}
		}
	}
}

void cps1_scan_scroll3(void)
{
	if (cps1_high_layer == LAYER_SCROLL3)
		cps1_scan_scroll3_background();
	else
		cps1_scan_scroll3_normal();
}


/******************************************************************************
	Stars
******************************************************************************/

void cps1_render_stars(void)
{
	u32 offs;
	int layer_ctrl = cps1_port(driver->layer_control);
	u32 current_frame = timer_getcurrentframe();
	u16 *video_buf = video_frame_addr(work_frame, 0, 0);

	if (layer_ctrl & driver->layer_enable_mask[3])
	{
		cps1_stars1x = cps1_port(CPS1_STARS1_SCROLLX);
		cps1_stars1y = cps1_port(CPS1_STARS1_SCROLLY);

		for (offs = 0; offs < 0x2000 >> 1; offs++)
		{
			int col = memory_region_gfx1[(offs << 3) + 4];

			if (col != 0x0f)
			{
				int sx = (offs >> 8) << 5;
				int sy = offs & 0xff;

				sx = (sx - cps1_stars2x + (col & 0x1f)) & 0x1ff;
				sy = (sy - cps1_stars2y) & 0xff;
				col = ((col & 0xe0) >> 1) + ((current_frame >> 4) & 0x0f);

				if ((sx >= 64 && sx <= 448) && (sy >= 16 && sy <= 248))
					video_buf[(sy << 9) + sx] = video_palette[0xa00 + col];
			}
		}
	}

	if (layer_ctrl & driver->layer_enable_mask[4])
	{
		cps1_stars2x = cps1_port(CPS1_STARS2_SCROLLX);
		cps1_stars2y = cps1_port(CPS1_STARS2_SCROLLY);

		for (offs = 0; offs < 0x2000 >> 1; offs++)
		{
			int col = memory_region_gfx1[offs << 3];

			if (col != 0x0f)
			{
				int sx = (offs >> 8) << 5;
				int sy = offs & 0xff;

				sx = (sx - cps1_stars1x + (col & 0x1f)) & 0x1ff;
				sy = (sy - cps1_stars1y) & 0xff;
				col = ((col & 0xe0) >> 1) + ((current_frame >> 4) & 0x0f);

				if ((sx >= 64 && sx <= 448) && (sy >= 16 && sy <= 248))
					video_buf[(sy << 9) + sx] = video_palette[0x800 + col];
			}
		}
	}
}


/******************************************************************************
	画面更新処理
******************************************************************************/

/*------------------------------------------------------
	レイヤーを描画
------------------------------------------------------*/

static void cps1_render_layer(int layer)
{
	switch (layer)
	{
	case 0:
		cps1_render_object();
		switch (cps1_high_layer)
		{
		case 1: blit_finish_scroll1h(); break;
		case 2: cps1_render_scroll2_foreground(); break;
		case 3: blit_finish_scroll3h(); break;
		}
		break;

	case 1: cps1_render_scroll1(); break;
	case 2: cps1_render_scroll2(); break;
	case 3: cps1_render_scroll3(); break;
	}
}

/*------------------------------------------------------
	画面更新
------------------------------------------------------*/

void cps1_screenrefresh(void)
{
	int i, layer_ctrl, l0, l1, l2, l3;
	u16 mask, prio_mask;

	cps_flip_screen = cps1_port(CPS1_VIDEO_CONTROL) & 0x8000;

	cps_scroll1 = cps1_base(CPS1_SCROLL1_BASE, cps1_scroll_mask);
	cps_scroll2 = cps1_base(CPS1_SCROLL2_BASE, cps1_scroll_mask);
	cps_scroll3 = cps1_base(CPS1_SCROLL3_BASE, cps1_scroll_mask);
	cps_other   = cps1_base(CPS1_OTHER_BASE, cps1_other_mask);

	cps_scroll1x = cps1_port(CPS1_SCROLL1_SCROLLX);
	cps_scroll1y = cps1_port(CPS1_SCROLL1_SCROLLY);
	cps_scroll2x = cps1_port(CPS1_SCROLL2_SCROLLX);
	cps_scroll2y = cps1_port(CPS1_SCROLL2_SCROLLY);
	cps_scroll3x = cps1_port(CPS1_SCROLL3_SCROLLX);
	cps_scroll3y = cps1_port(CPS1_SCROLL3_SCROLLY);

	layer_ctrl = cps1_port(driver->layer_control);
	l0 = (layer_ctrl >> 0x06) & 03;
	l1 = (layer_ctrl >> 0x08) & 03;
	l2 = (layer_ctrl >> 0x0a) & 03;
	l3 = (layer_ctrl >> 0x0c) & 03;

	cps_layer_enabled[1] = layer_ctrl & driver->layer_enable_mask[0];
	cps_layer_enabled[2] = layer_ctrl & driver->layer_enable_mask[1];
	cps_layer_enabled[3] = layer_ctrl & driver->layer_enable_mask[2];

	cps1_high_layer = 0;

	for (i = 0; i < 4; i++)
	{
		prio_mask = cps1_port(driver->priority[i]) & 0x7fff;
		if (prio_mask != cps1_transparency_scroll[i])
		{
			cps1_transparency_scroll[i] = prio_mask;
			if (prio_mask) blit_scrollh_clear_sprite(i);
		}
		mask |= prio_mask;
	}

	if (mask)
	{
		if (l3 == 0)
			cps1_high_layer = l2;
		else if (l2 == 0)
			cps1_high_layer = l1;
		else if (l1 == 0)
			cps1_high_layer = l0;

		if (!cps_layer_enabled[cps1_high_layer])
			cps1_high_layer = 0;
	}

	if (!cps_layer_enabled[l0]) l0 = LAYER_SKIP;
	if (!cps_layer_enabled[l1]) l1 = LAYER_SKIP;
	if (!cps_layer_enabled[l2]) l2 = LAYER_SKIP;
	if (!cps_layer_enabled[l3]) l3 = LAYER_SKIP;

	if (l0 == l1) l0 = LAYER_SKIP;
	if (l0 == l2) l0 = LAYER_SKIP;
	if (l0 == l3) l0 = LAYER_SKIP;
	if (l1 == l2) l1 = LAYER_SKIP;
	if (l1 == l3) l1 = LAYER_SKIP;
	if (l2 == l3) l2 = LAYER_SKIP;

	blit_start(cps1_high_layer);

	if (cps1_has_stars) cps1_render_stars();
	cps1_render_layer(l0);
	cps1_render_layer(l1);
	cps1_render_layer(l2);
	cps1_render_layer(l3);
}


/*------------------------------------------------------
	object RAM更新
------------------------------------------------------*/

void cps1_objram_latch(void)
{
	u16 *base = cps1_base(CPS1_OBJ_BASE, cps1_obj_mask);
	u16 *end  = base + (cps1_obj_size >> 1);
	struct cps1_object_t *object = cps1_object;

	while (base < end)
	{
		if ((base[3] & 0xff00) == 0xff00)
			break;

		object->sx   = base[0];
		object->sy   = base[1];
		object->code = base[2];
		object->attr = base[3];
		object++;

		base += 4;
	}
	cps1_last_object = object;

	cps1_build_palette();
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( video )
{
	state_save_byte(cps1_old_palette, sizeof(cps1_old_palette));
	state_save_byte(video_palette, sizeof(video_palette));
}

STATE_LOAD( video )
{
	state_load_byte(cps1_old_palette, sizeof(cps1_old_palette));
	state_load_byte(video_palette, sizeof(video_palette));
	cps1_objram_latch();

	cps1_high_layer = 0;
	cps1_transparency_scroll[0] = 0;
	cps1_transparency_scroll[1] = 0;
	cps1_transparency_scroll[2] = 0;
	cps1_transparency_scroll[3] = 0;
}

#endif /* SAVE_STATE */
