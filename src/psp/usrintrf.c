/******************************************************************************

	usrintrf.c

	PSP ユーザインタフェース描画関数

******************************************************************************/

#include "psp.h"
#include "psp/font/jpnfont.h"


/******************************************************************************
	定数/マクロ
******************************************************************************/

#define vram_pos(x, y)		video_frame_addr(draw_frame, x, y)
#define vram_pixel(x, y)	*vram_pos(x, y)

#define CODE_NOTFOUND	0xffff

#define MAKECOL(r, g, b)	((b << 10) | (g << 5) | r)
#define GETR(color)			((color >>  0) & 0x1f)
#define GETG(color)			((color >>  5) & 0x1f)
#define GETB(color)			((color >> 10) & 0x1f)

#define isascii(c)	((c) >= 0x20 && (c) <= 0x7e)
#define iskana(c)	((c) >= 0xa0 && (c) <= 0xdf)
#define issjis1(c)	(((c) >= 0x81 && (c) <= 0x9f) | ((c) >= 0xe0 && (c) <= 0xfc))
#define issjis2(c)	((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)


enum
{
	FONT_TYPE_CONTROL = 0,
	FONT_TYPE_ASCII,
	FONT_TYPE_GRAPHIC,
	FONT_TYPE_JPNHAN,
	FONT_TYPE_JPNZEN,
	FONT_TYPE_MAX
};


/******************************************************************************
	グローバル構造体
******************************************************************************/

UI_PALETTE ui_palette[UI_PAL_MAX] = 
{
	{ 255, 255, 255 },	// UI_PAL_TITLE
	{ 255, 255, 255 },	// UI_PAL_SELECT
	{ 180, 180, 180 },	// UI_PAL_NORMAL
	{ 255, 255,  64 },	// UI_PAL_INFO
	{ 255,  64,  64 },	// UI_PAL_WARNING
	{  48,  48,  48 },	// UI_PAL_BG1
	{   0,   0, 160 },	// UI_PAL_BG2
	{   0,   0,   0 },	// UI_PAL_FRAME
	{  40,  40,  40 }	// UI_PAL_FILESEL
};


/******************************************************************************
	フォントコード取得
******************************************************************************/

/*------------------------------------------------------
	フォントコード取得 (ユーザインタフェース)
------------------------------------------------------*/

INLINE u16 uifont_get_code(const u8 *s, int *type)
{
	u8 c1 = s[0];
	u8 c2 = s[1];

	if (issjis1(c1) && issjis2(c2))
	{
		if (sjis_table)
		{
			*type = FONT_TYPE_JPNZEN;
			return sjis_table[(c2 | (c1 << 8)) - 0x8140];
		}
		*type = FONT_TYPE_CONTROL;
		return c1;
	}
	else if (c1 == 0xa5)
	{
		*type = FONT_TYPE_ASCII;
		return 0x7f - 0x20;
	}
	else if (c1 == 0x5c)
	{
		if (jpnfont)
		{
			*type = FONT_TYPE_JPNHAN;
			return 0;
		}
		*type = FONT_TYPE_CONTROL;
		return c1;
	}
	else if (iskana(c1))
	{
		if (jpnfont)
		{
			*type = FONT_TYPE_JPNHAN;
			return c1 - 0xa0;
		}
		*type = FONT_TYPE_CONTROL;
		return c1;
	}
	else if (isascii(c1))
	{
		*type = FONT_TYPE_ASCII;
		return c1 - 0x20;
	}
	else if ((c1 >= 0x10 && c1 <= 0x1e) && c1 != 0x1a)
	{
		*type = FONT_TYPE_GRAPHIC;
		if (c1 < 0x1a)
			return c1 - 0x10;
		else
			return c1 - 0x11;
	}
	*type = FONT_TYPE_CONTROL;
	return c1;
}


/******************************************************************************
	フォント文字幅取得
******************************************************************************/

/*------------------------------------------------------
	フォント描画幅取得 (ユーザインタフェース)
------------------------------------------------------*/

int uifont_get_string_width(const char *s)
{
	int width, type;
	u16 code;
	u8 *p = (u8 *)s;

	width = 0;

	while (*p)
	{
		if ((code = uifont_get_code(p, &type)) != CODE_NOTFOUND)
		{
			switch (type)
			{
			case FONT_TYPE_ASCII:
				width += ascii_14p_get_pitch(code);
				p++;
				break;

			case FONT_TYPE_GRAPHIC:
				width += graphic_font_get_pitch(code);
				p++;
				break;

			case FONT_TYPE_CONTROL:
				width += ascii_14p_get_pitch(0);
				p++;
				break;

			}
		}
		else break;
	}

	return width;
}


/*------------------------------------------------------
	内部フォント描画関数
------------------------------------------------------*/

static int internal_font_putc(struct font_t *font, int sx, int sy, int src_r, int src_g, int src_b)
{
	int x, y, p;
	int dst_r, dst_g, dst_b, alpha;
	u16 *dst, *vptr, color;
	u8 data;

	if (sx + font->pitch < 0 || sx >= SCR_WIDTH)
		return 0;

	vptr = vram_pos(sx, sy);
	dst = &vptr[font->skipx + (font->skipy << 9)];	// font_skipy << 9 = font_skipy * BUF_WIDTH

	sy += font->skipy;

	p = 0;

	for (y = 0; y < font->height; y++)
	{
		if (sy + y >= 0 && sy + y < SCR_HEIGHT)
		{
			for (x = 0; x < font->width;)
			{
				data = font->data[p++];

				alpha = data & 0x0f;
				if (alpha)
				{
					color = dst[x];

					dst_r = GETR(color);
					dst_g = GETG(color);
					dst_b = GETB(color);

					dst_r = alpha_blend[alpha][src_r][dst_r];
					dst_g = alpha_blend[alpha][src_g][dst_g];
					dst_b = alpha_blend[alpha][src_b][dst_b];

					dst[x] = MAKECOL(dst_r, dst_g, dst_b);
				}
				x++;

				alpha = data >> 4;
				if (alpha)
				{
					color = dst[x];

					dst_r = GETR(color);
					dst_g = GETG(color);
					dst_b = GETB(color);

					dst_r = alpha_blend[alpha][src_r][dst_r];
					dst_g = alpha_blend[alpha][src_g][dst_g];
					dst_b = alpha_blend[alpha][src_b][dst_b];

					dst[x] = MAKECOL(dst_r, dst_g, dst_b);
				}
				x++;
			}
			dst += BUF_WIDTH;
		}
	}

	return 1;
}


/******************************************************************************
	ユーザインタフェース用フォント描画
******************************************************************************/

/*------------------------------------------------------
	文字を描画 (ユーザインタフェース用)
------------------------------------------------------*/

INLINE void uifont_draw(int sx, int sy, int r, int g, int b, const char *s)
{
	int type, res = 1;
	u16 code;
	u8 *p = (u8 *)s;
	struct font_t font;

	r >>= 3;
	g >>= 3;
	b >>= 3;

	while (*p && res)
	{
		code = uifont_get_code(p, &type);
		res = 1;

		switch (type)
		{
		case FONT_TYPE_ASCII:
			if ((res = ascii_14p_get_gryph(&font, code)) != 0)
			{
				res = internal_font_putc(&font, sx, sy, r, g, b);
				sx += font.pitch;
				p++;
			}
			break;

		case FONT_TYPE_GRAPHIC:
			if ((res = graphic_font_get_gryph(&font, code)) != 0)
			{
				res = internal_font_putc(&font, sx, sy, r, g, b);
				sx += font.pitch;
				p++;
			}
			break;

		case FONT_TYPE_JPNHAN:
			if ((res = jpn_h14p_get_gryph(&font, code)) != 0)
			{
				res = internal_font_putc(&font, sx, sy, r, g, b);
				sx += font.pitch;
				p++;
			}
			break;

		case FONT_TYPE_JPNZEN:
			if ((res = jpn_z14p_get_gryph(&font, code)) != 0)
			{
				res = internal_font_putc(&font, sx, sy, r, g, b);
				sx += font.pitch;
				p += 2;
			}
			break;

		default:
			res = 0;
			break;
		}
	}
}


/*------------------------------------------------------
	文字列を描画
------------------------------------------------------*/

void uifont_print(int sx, int sy, int r, int g, int b, const char *s)
{
	uifont_draw(sx, sy, r, g, b, s);
}


/*------------------------------------------------------
	文字列を描画 / センタリング処理
------------------------------------------------------*/

void uifont_print_center(int sy, int r, int g, int b, const char *s)
{
	int width = uifont_get_string_width(s);
	int sx = (SCR_WIDTH - width) / 2;

	uifont_print(sx, sy, r, g, b, s);
}


/******************************************************************************
	アイコン描画
******************************************************************************/

/*------------------------------------------------------
	アイコン(小)を描画
------------------------------------------------------*/

void small_icon(int sx, int sy, int r, int g, int b, int no)
{
	struct font_t font;

	r >>= 3;
	g >>= 3;
	b >>= 3;

	if (icon_s_get_gryph(&font, no))
		internal_font_putc(&font, sx, sy, r, g, b);
}


/*------------------------------------------------------
	アイコン(大)を描画
------------------------------------------------------*/

void large_icon(int sx, int sy, int r, int g, int b, int no)
{
	struct font_t font;

	r >>= 3;
	g >>= 3;
	b >>= 3;

	if (icon_l_get_gryph(&font, no))
		internal_font_putc(&font, sx, sy, r, g, b);
}


/******************************************************************************
	図形描画
******************************************************************************/

/*------------------------------------------------------
	水平線描画
------------------------------------------------------*/

void hline(int sx, int ex, int y, int r, int g, int b)
{
	int x;
	int width  = (ex - sx) + 1;
	u16 *dst = vram_pos(sx, y);
	u16 color;

	r >>= 3;
	g >>= 3;
	b >>= 3;

	color = MAKECOL(r, g, b);

	for (x = 0; x < width; x++)
		*dst++ = color;
}


/*------------------------------------------------------
	水平線描画 / アルファブレンドあり
------------------------------------------------------*/

void hline_alpha(int sx, int ex, int y, int r, int g, int b, int alpha)
{
	int x, dst_r, dst_g, dst_b;
	int width  = (ex - sx) + 1;
	u16 *dst = vram_pos(sx, y);

	r >>= 3;
	g >>= 3;
	b >>= 3;

	for (x = 0; x < width; x++)
	{
		dst_r = GETR(dst[x]);
		dst_g = GETG(dst[x]);
		dst_b = GETB(dst[x]);

		dst_r = alpha_blend[alpha][r][dst_r];
		dst_g = alpha_blend[alpha][g][dst_g];
		dst_b = alpha_blend[alpha][b][dst_b];

		dst[x] = MAKECOL(dst_r, dst_g, dst_b);
	}
}


/*------------------------------------------------------
	垂直線描画
------------------------------------------------------*/

void vline(int x, int sy, int ey, int r, int g, int b)
{
	int y;
	int height = (ey - sy) + 1;
	u16 *dst = vram_pos(x, sy);
	u16 color;

	r >>= 3;
	g >>= 3;
	b >>= 3;

	color = MAKECOL(r, g, b);

	for (y = 0; y < height; y++)
	{
		*dst = color;
		dst += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	垂直線描画 / アルファブレンドあり
------------------------------------------------------*/

void vline_alpha(int x, int sy, int ey, int r, int g, int b, int alpha)
{
	int y, dst_r, dst_g, dst_b;
	int height = (ey - sy) + 1;
	u16 *dst = vram_pos(x, sy);

	r >>= 3;
	g >>= 3;
	b >>= 3;

	for (y = 0; y < height; y++)
	{
		dst_r = GETR(*dst);
		dst_g = GETG(*dst);
		dst_b = GETB(*dst);

		dst_r = alpha_blend[alpha][r][dst_r];
		dst_g = alpha_blend[alpha][g][dst_g];
		dst_b = alpha_blend[alpha][b][dst_b];

		*dst = MAKECOL(dst_r, dst_g, dst_b);

		dst += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	四角形描画
------------------------------------------------------*/

void box(int sx, int sy, int ex, int ey, int r, int g, int b)
{
	hline(sx, ex - 1, sy, r, g, b);
	vline(ex, sy, ey - 1, r, g, b);
	hline(sx + 1, ex, ey, r, g, b);
	vline(sx, sy + 1, ey, r, g, b);
}

/*------------------------------------------------------
	四角形塗りつぶし
------------------------------------------------------*/

void boxfill(int sx, int sy, int ex, int ey, int r, int g, int b)
{
	int x, y;
	int width  = (ex - sx) + 1;
	int height = (ey - sy) + 1;
	u16 color, *dst = vram_pos(sx, sy);

	color = MAKECOL15(r, g, b);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			dst[x] = color;
		}
		dst += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	四角形塗りつぶし / アルファブレンドあり
------------------------------------------------------*/

void boxfill_alpha(int sx, int sy, int ex, int ey, int r, int g, int b, int alpha)
{
	int x, y;
	int dst_r, dst_g, dst_b;
	int width  = (ex - sx) + 1;
	int height = (ey - sy) + 1;
	u16 color, *dst = vram_pos(sx, sy);

	r >>= 3;
	g >>= 3;
	b >>= 3;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			color = dst[x];

			dst_r = GETR(color);
			dst_g = GETG(color);
			dst_b = GETB(color);

			dst_r = alpha_blend[alpha][r][dst_r];
			dst_g = alpha_blend[alpha][g][dst_g];
			dst_b = alpha_blend[alpha][b][dst_b];

			dst[x] = MAKECOL(dst_r, dst_g, dst_b);
		}
		dst += BUF_WIDTH;
	}
}


/******************************************************************************
	ユーザインタフェースのパーツを描画
******************************************************************************/

/*------------------------------------------------------
	四角形の影を描画 (ダイアログボックス等)
------------------------------------------------------*/

static void draw_boxshadow(u16 *vptr, int w, int h, int no)
{
	int x, y;
	int dst_r, dst_g, dst_b, a;
	u16 *dst = vptr;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			if (x & 1)
				a = shadow[no][y][x >> 1] >> 4;
			else
				a = shadow[no][y][x >> 1] & 0x0f;

			if (a)
			{
				dst_r = GETR(dst[x]);
				dst_g = GETG(dst[x]);
				dst_b = GETB(dst[x]);

				dst_r = alpha_blend[a][dst_r >> 2][dst_r];
				dst_g = alpha_blend[a][dst_g >> 2][dst_g];
				dst_b = alpha_blend[a][dst_b >> 2][dst_b];

				dst[x] = MAKECOL(dst_r, dst_g, dst_b);
			}
		}
		dst += BUF_WIDTH;
	}
}


static void fill_boxshadow(u16 *vptr, int w, int h)
{
	int x, y;
	int dst_r, dst_g, dst_b;
	u16 *dst = vptr;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			dst_r = GETR(dst[x]);
			dst_g = GETG(dst[x]);
			dst_b = GETB(dst[x]);

			dst_r = alpha_blend[15][dst_r >> 2][dst_r];
			dst_g = alpha_blend[15][dst_g >> 2][dst_g];
			dst_b = alpha_blend[15][dst_b >> 2][dst_b];

			dst[x] = MAKECOL(dst_r, dst_g, dst_b);
		}
		dst += BUF_WIDTH;
	}
}


void draw_box_shadow(int sx, int sy, int ex, int ey)
{
	int i, width, height;
	int w, h, nw, nh, sx2, sy2;
	u16 *vptr;

	width = (ex - sx) + 1;
	height = (ey - sy) + 1;

	width  -= 14;
	height -= 14;

	nw = width / 8;
	nh = height / 8;

	w = width % 8;
	h = height % 8;

	/*----------------------------------*/
	// top
	/*----------------------------------*/
	sx2 = sx + 2;
	sy2 = sy + 2;
	vptr = vram_pos(sx2, sy2);

	// left top
	draw_boxshadow(vptr, 8, 8, 0);
	vptr += 8;

	// top
	for (i = 0; i < nw; i++)
	{
		draw_boxshadow(vptr, 8, 8, 1);
		vptr += 8;
	}
	draw_boxshadow(vptr, w, 8, 1);
	vptr += w;

	// right top
	draw_boxshadow(vptr, 8, 8, 2);

	/*----------------------------------*/
	// left
	/*----------------------------------*/
	sx2 = sx + 2;
	sy2 = sy + 10;
	vptr = vram_pos(sx2, sy2);

	for (i = 0; i < nh; i++)
	{
		draw_boxshadow(vptr, 8, 8, 3);
		vptr += 8 * BUF_WIDTH;
	}
	draw_boxshadow(vptr, 8, h, 3);

	/*----------------------------------*/
	// right
	/*----------------------------------*/
	sx2 = sx + 10 + width;
	sy2 = sy + 10;
	vptr = vram_pos(sx2, sy2);

	for (i = 0; i < nh; i++)
	{
		draw_boxshadow(vptr, 8, 8, 5);
		vptr += 8 * BUF_WIDTH;
	}
	draw_boxshadow(vptr, 8, h, 5);

	/*----------------------------------*/
	// bottom
	/*----------------------------------*/
	sx2 = sx + 2;
	sy2 = sy + 10 + height;
	vptr = vram_pos(sx2, sy2);

	// left bottom
	draw_boxshadow(vptr, 8, 8, 6);
	vptr += 8;

	// bottom
	for (i = 0; i < nw; i++)
	{
		draw_boxshadow(vptr, 8, 8, 7);
		vptr += 8;
	}
	draw_boxshadow(vptr, w, 8, 7);
	vptr += w;

	// right bottom
	draw_boxshadow(vptr, 8, 8, 8);

	vptr = vram_pos(sx + 10, sy + 10);
	fill_boxshadow(vptr, width, height);
}


/*------------------------------------------------------
	上部バーの影を描画
------------------------------------------------------*/

void draw_bar_shadow(void)
{
	int i;
	u16 *vptr = vram_pos(0, 20);

	for (i = 0; i < SCR_WIDTH / 8; i++)
	{
		draw_boxshadow(vptr, 8, 8, 7);
		vptr += 8;
	}

	vptr = vram_pos(0, 0);
	fill_boxshadow(vptr, SCR_WIDTH, 20);
}


/*------------------------------------------------------
	ダイアログボックス描画
------------------------------------------------------*/

void draw_dialog(int sx, int sy, int ex, int ey)
{
	draw_box_shadow(sx, sy, ex, ey);

	hline_alpha(sx, ex - 1, sy, UI_COLOR(UI_PAL_FRAME), 10);
	vline_alpha(ex, sy, ey - 1, UI_COLOR(UI_PAL_FRAME), 10);
	hline_alpha(sx + 1, ex, ey, UI_COLOR(UI_PAL_FRAME), 10);
	vline_alpha(sx, sy + 1, ey, UI_COLOR(UI_PAL_FRAME), 10);

	sx++;
	ex--;
	sy++;
	ey--;

	hline_alpha(sx, ex - 1, sy, UI_COLOR(UI_PAL_FRAME), 12);
	vline_alpha(ex, sy, ey - 1, UI_COLOR(UI_PAL_FRAME), 12);
	hline_alpha(sx + 1, ex, ey, UI_COLOR(UI_PAL_FRAME), 12);
	vline_alpha(sx, sy + 1, ey, UI_COLOR(UI_PAL_FRAME), 12);

	sx++;
	ex--;
	sy++;
	ey--;

	boxfill_alpha(sx, sy, ex, ey, UI_COLOR(UI_PAL_BG1), 10);
}


/*------------------------------------------------------
	スクロールバー描画
------------------------------------------------------*/

void draw_scrollbar(int sx, int sy, int ex, int ey, int disp_lines, int total_lines, int current_line)
{
	int height = (ey - sy) + 1;

	if (total_lines > disp_lines)
	{
		int line_height;
		int bar_size, bar_blank, bar_top;

		boxfill_alpha(sx, sy, ex, sy + 9, UI_COLOR(UI_PAL_FRAME), 14);
		boxfill_alpha(sx, sy + 10, ex, ey - 10, UI_COLOR(UI_PAL_FRAME), 6);
		boxfill_alpha(sx, ey - 9, ex, ey, UI_COLOR(UI_PAL_FRAME), 14);

		uifont_print(sx - 2, sy - 2, UI_COLOR(UI_PAL_SELECT), FONT_UPTRIANGLE);
		uifont_print(sx - 2, ey - 11, UI_COLOR(UI_PAL_SELECT), FONT_DOWNTRIANGLE);

		height -= 23;
		sy += 11;

		bar_size = height >> 2;	// 最低限必要なサイズ
		line_height = (height - bar_size) / (total_lines - 1);
		bar_blank = (total_lines - 1) * line_height;
		bar_size  = height - bar_blank;
		bar_top = line_height * current_line;

		sy = sy + bar_top;
		ey = sy + bar_size;

		hline_alpha(sx, ex - 1, sy, UI_COLOR(UI_PAL_FRAME), 12);
		vline_alpha(ex, sy, ey - 1, UI_COLOR(UI_PAL_FRAME), 12);
		hline_alpha(sx + 1, ex, ey, UI_COLOR(UI_PAL_FRAME), 12);
		vline_alpha(sx, sy + 1, ey, UI_COLOR(UI_PAL_FRAME), 12);

		sx++;
		ex--;
		sy++;
		ey--;

		boxfill_alpha(sx, sy, ex, ey, UI_COLOR(UI_PAL_BG1), 6);
	}
}



/******************************************************************************
	ゲーム画面用フォント描画 (16bit colorのみ)
******************************************************************************/

/*------------------------------------------------------
	テクスチャ作成
------------------------------------------------------*/

#define NUM_FONTS	0x60
#define MAX_STR_LEN	256

static u16 *tex_font;

void create_small_font(void)
{
	int code, x, y;
	u16 *dst;
	u16 color[8] = {
		MAKECOL15(248,248,248),
		MAKECOL15(240,240,240),
		MAKECOL15(232,232,232),
		MAKECOL15(224,224,224),
		MAKECOL15(216,216,216),
		MAKECOL15(208,208,208),
		MAKECOL15(200,200,200),
		MAKECOL15(192,192,192)
	};

	tex_font = video_frame_addr(0, 0, 2032);
	dst = tex_font;

	for (code = 0; code < NUM_FONTS; code++)
	{
		for (y = 0; y < 8; y++)
		{
			u8 data = font_s[(code << 3) + y];
			u8 mask = 0x80;

			for (x = 0; x < 8; x++)
			{
				*dst++ = (data & mask) ? color[y] : 0x8000;
				mask >>= 1;
			}
		}
	}
}


/*------------------------------------------------------
	文字列描画
------------------------------------------------------*/

void small_font_print(int sx, int sy, const char *s, int bg)
{
	int len = strlen(s);
	struct Vertex *vertices = (struct Vertex *)sceGuGetMemory(len * 2 * sizeof(struct Vertex));

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, draw_frame, BUF_WIDTH);
	sceGuScissor(sx, sy, sx + 8 * len, sy + 8);

	if (bg)
		sceGuDisable(GU_ALPHA_TEST);
	else
		sceGuEnable(GU_ALPHA_TEST);

	sceGuTexMode(GU_PSM_5551, 0, 0, GU_TRUE);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_font);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);

	if (vertices)
	{
		int i;
		struct Vertex *vertices_tmp = vertices;

		for (i = 0; i < len; i++)
		{
			u8 code = isascii((u8)s[i]) ? s[i] - 0x20 : 0x20;
			int u = (code & 63) << 3;
			int v = (code >> 6) << 3;

			vertices_tmp[0].u = u;
			vertices_tmp[0].v = v;
			vertices_tmp[0].x = sx;
			vertices_tmp[0].y = sy;

			vertices_tmp[1].u = u + 8;
			vertices_tmp[1].v = v + 8;
			vertices_tmp[1].x = sx + 8;
			vertices_tmp[1].y = sy + 8;

			vertices_tmp += 2;
			sx += 8;
		}

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2 * len, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------
	書式付文字列描画
------------------------------------------------------*/

void small_font_printf(int x, int y, const char *text, ...)
{
	char buf[256];
	va_list arg;

	va_start(arg, text);
	vsprintf(buf, text, arg);
	va_end(arg);

	small_font_print(x << 3, y << 3, buf, 1);
}


/******************************************************************************
	ポップアップメッセージ
******************************************************************************/

static int ui_popup_mode = POPUP_MENU;
static int ui_popup_updated = 0;
static int ui_popup_counter = 0;
static int ui_popup_prev_counter = 0;
static char ui_popup_message[128];


/*--------------------------------------------------------
	ポップアップメッセージをリセット
--------------------------------------------------------*/

void ui_popup_reset(int mode)
{
	memset(ui_popup_message, 0, sizeof(ui_popup_message));
	ui_popup_mode = mode;
	ui_popup_updated = 0;
	ui_popup_counter = 0;
	ui_popup_prev_counter = 0;
}

/*--------------------------------------------------------
	ポップアップメッセージを登録
--------------------------------------------------------*/

void ui_popup(const char *text, ...)
{
	va_list arg;

	va_start(arg, text);
	vsprintf(ui_popup_message, text, arg);
	va_end(arg);

	ui_popup_counter = 2 * 60;
	ui_popup_updated = 1;
}


/*--------------------------------------------------------
	ポップアップメッセージを表示
--------------------------------------------------------*/

int ui_show_popup(int draw)
{
	int update = ui_popup_updated;

	ui_popup_updated = 0;

	if (ui_popup_counter > 0)
	{
		if (ui_popup_prev_counter == 0)
			update = 1;

		if (draw)
		{
			int sx, sy, ex, ey;
			int width = uifont_get_string_width(ui_popup_message);

			sx = (SCR_WIDTH - width) >> 1;
			if (ui_popup_mode == POPUP_MENU)
				sy = (SCR_HEIGHT - FONTSIZE) >> 1;
			else
				sy = SCR_HEIGHT - (FONTSIZE << 1);
			ex = sx + width;
			ey = sy + (FONTSIZE - 1);

			draw_dialog(sx - FONTSIZE/2, sy - FONTSIZE/2, ex + FONTSIZE/2, ey + FONTSIZE/2);
			uifont_print_center(sy, COLOR_WHITE, ui_popup_message);
		}

		ui_popup_counter--;
		ui_popup_prev_counter = ui_popup_counter;

		if (update || !ui_popup_counter) return 1;
	}
	return update;
}


/******************************************************************************
	簡易書式付文字列表示
******************************************************************************/

#define MAX_LINES	13
#define MIN_X		24
#define MIN_Y		47
#define INC_Y		16

static int cy;
static int linefeed;
static int text_r = 0xff;
static int text_g = 0xff;
static int text_b = 0xff;
static char msg_lines[MAX_LINES][128];
static int msg_r[MAX_LINES];
static int msg_g[MAX_LINES];
static int msg_b[MAX_LINES];


/*--------------------------------------------------------
	メッセージ初期化
--------------------------------------------------------*/

void msg_screen_init(const char *title)
{
	cy = 0;
	linefeed = 1;
	memset(msg_lines, 0, sizeof(msg_lines));

	load_background();
	small_icon(6, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
	uifont_print(32, 5, UI_COLOR(UI_PAL_TITLE), title);
	draw_dialog(14, 37, 465, 259);
	video_copy_rect(draw_frame, work_frame, &full_rect, &full_rect);
}


/*--------------------------------------------------------
	メッセージ消去
--------------------------------------------------------*/

void msg_screen_clear(void)
{
	cy = 0;
	linefeed = 1;
}


/*--------------------------------------------------------
	テキストカラー設定
--------------------------------------------------------*/

void msg_set_text_color(u32 color)
{
	text_r = (color >>  0) & 0xff;
	text_g = (color >>  8) & 0xff;
	text_b = (color >> 16) & 0xff;
}


/*--------------------------------------------------------
	メッセージ表示
--------------------------------------------------------*/

void msg_printf(const char *text, ...)
{
	int y;
	char buf[128];
	va_list arg;

	va_start(arg, text);
	vsprintf(buf, text, arg);
	va_end(arg);

	if (linefeed)
	{
		if (cy == MAX_LINES)
		{
			for (y = 1; y < MAX_LINES; y++)
			{
				strcpy(msg_lines[y - 1], msg_lines[y]);
				msg_r[y - 1] = msg_r[y];
				msg_g[y - 1] = msg_g[y];
				msg_b[y - 1] = msg_b[y];
			}
			cy = MAX_LINES - 1;
		}
		strcpy(msg_lines[cy], buf);
	}
	else
	{
		strcat(msg_lines[cy], buf);
	}

	msg_r[cy] = text_r;
	msg_g[cy] = text_g;
	msg_b[cy] = text_b;

	show_background();
	draw_battery_status(1);

	for (y = 0; y <= cy; y++)
		uifont_print(MIN_X, MIN_Y + y * 16, msg_r[y], msg_g[y], msg_b[y], msg_lines[y]);

	if (buf[strlen(buf) - 1] == '\n')
	{
		linefeed = 1;
		cy++;
	}
	else
	{
		linefeed = 0;
	}

	video_flip_screen(1);
}
