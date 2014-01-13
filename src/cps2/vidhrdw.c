/******************************************************************************

	vidhrdw.c

	CPS2 �r�f�I�G�~�����[�V����

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	�O���[�o���ϐ�
******************************************************************************/

int cps_flip_screen;
int cps_rotate_screen;
int cps_raster_enable;

int cps2_objram_bank;
int scanline1;
int scanline2;


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

#define cps2_scroll_size		0x4000		/* scroll1, scroll2, scroll3 */
#define cps2_other_size			0x0800		/* line scroll offset etc. */
#define cps2_palette_align		0x0800		/* can't be larger than this, breaks ringdest & batcirc otherwise */
#define cps2_palette_size		4*32*16*2	/* Size of palette RAM */

#define cps2_scroll_mask		(((~0x3fff) & 0x3ffff) >> 1)
#define cps2_other_mask			(((~0x07ff) & 0x3ffff) >> 1)
#define cps2_palette_mask		(((~0x07ff) & 0x3ffff) >> 1)

static u16 *cps_scroll1;
static u16 *cps_scroll2;
static u16 *cps_scroll3;
static u16 *cps_other;
static u16 *cps2_palette;
static u16 ALIGN_DATA cps2_old_palette[cps2_palette_size >> 1];

static int cps_layer_enabled[4];		/* Layer enabled [Y/N] */
static int cps_scroll1x, cps_scroll1y;
static int cps_scroll2x, cps_scroll2y;
static int cps_scroll3x, cps_scroll3y;
static u32 cps_total_elements[3];		/* total sprites num */
static u8  *cps_pen_usage[3];			/* sprites pen usage */

#define cps2_obj_size			0x2000
#define cps2_max_obj			(cps2_obj_size >> 3)
static u16 ALIGN_DATA cps2_buffered_palette[1*32*16];

struct cps2_object_t
{
	u16 sx;
	u16 sy;
	u16 attr;
	u16 pri;
	u32 code;
};

static struct cps2_object_t ALIGN_DATA cps2_object[cps2_max_obj + 1];
static struct cps2_object_t *cps2_last_object;

static u16 cps2_object_xoffs;
static u16 cps2_object_yoffs;

struct cps_scroll2_t
{
	int value;
	int start;
	int end;
};

static struct cps_scroll2_t ALIGN_DATA scroll2[224];
static int cps_scroll2_blocks;

static int cps2_scanline_start;
static int cps2_scanline_end;
static int cps2_scroll3_base;
static int cps2_kludge;

static int cps2_mask_max;

static u16 ALIGN_DATA video_clut16[65536];
u16 ALIGN_DATA video_palette[cps2_palette_size >> 1];


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
#define CPS1_VIDEO_CONTROL		0x22    /* Video control */

#define CPS2_PROT_FACTOR1		0x40	/* Multiply protection (factor1) */
#define CPS2_PROT_FACTOR2		0x42	/* Multiply protection (factor2) */
#define CPS2_PROT_RESULT_LO		0x44	/* Multiply protection (result low) */
#define CPS2_PROT_RESULT_HI		0x46	/* Multiply protection (result high) */
#define CPS2_SCANLINE1			0x50	/* Scanline interrupt 1 */
#define CPS2_SCANLINE2			0x52	/* Scanline interrupt 2 */
#define CPS2_LAYER_CONTROL		0x66	/* Layer control */

/* CPS1 other RAM */
#define CPS1_ROWSCROLL_OFFS		0x20    /* base of row scroll offsets in other RAM */
#define CPS1_SCROLL2_WIDTH		0x40
#define CPS1_SCROLL2_HEIGHT		0x40

/* CPS2 object RAM */
#define CPS2_OBJ_BASE			0x00	/* Unknown (not base address of objects). Could be bass address of bank used when object swap bit set? */
#define CPS2_OBJ_UK1			0x02	/* Unknown (nearly always 0x807d, or 0x808e when screen flipped) */
#define CPS2_OBJ_PRI			0x04	/* Layers priorities */
#define CPS2_OBJ_UK2			0x06	/* Unknown (usually 0x0000, 0x1101 in ssf2, 0x0001 in 19XX) */
#define CPS2_OBJ_XOFFS			0x08	/* X offset (usually 0x0040) */
#define CPS2_OBJ_YOFFS			0x0a	/* Y offset (always 0x0010) */

/* pen usage flag */
#define SPR_MASK				0x0f
#define SPR_MASK1				0x01
#define SPR_MASK2				0x02
#define SPR_MASK3				0x04
#define SPR_MASK4				0x08
#define SPR_MASKED_OBJ			0x40
#define SPR_NOT_EMPTY			0x80

/*------------------------------------------------------
	CPS�|�[�g�ǂݍ���
------------------------------------------------------*/

#define cps1_port(offset)	cps1_output[(offset) >> 1]
#define cps2_port(offset)	cps2_output[(offset) >> 1]


void (*cps2_build_palette)(void);
static void cps2_build_palette_normal(void);
static void cps2_build_palette_delay(void);


/******************************************************************************
	CPS2 �������n���h��
******************************************************************************/

READ16_HANDLER( cps1_output_r )
{
	offset &= 0x7f;

	switch (offset)
	{
	case CPS2_PROT_RESULT_LO/2:
		return (cps1_port(CPS2_PROT_FACTOR1) * cps1_port(CPS2_PROT_FACTOR2)) & 0xffff;

	case CPS2_PROT_RESULT_HI/2:
		return (cps1_port(CPS2_PROT_FACTOR1) * cps1_port(CPS2_PROT_FACTOR2)) >> 16;
	}

	return cps1_output[offset];
}

WRITE16_HANDLER( cps1_output_w )
{
	offset &= 0x7f;
	data = COMBINE_DATA(&cps1_output[offset]);

	switch (offset)
	{
	case CPS2_SCANLINE1/2:
		cps1_port(CPS2_SCANLINE1) &= 0x1ff;
		scanline1 = data & 0x1ff;
		break;

	case CPS2_SCANLINE2/2:
		cps1_port(CPS2_SCANLINE2) &= 0x1ff;
		scanline2 = data & 0x1ff;
		break;
	}
}


/******************************************************************************
	CPS2 �r�f�I�`�揈��
******************************************************************************/

/*------------------------------------------------------
	�J���[�e�[�u���쐬
------------------------------------------------------*/

static void cps2_init_tables(void)
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
					int r2, g2, b2, bright2;
					float fr, fg, fb;

					pen = (bright << 12) | (r << 8) | (g << 4) | b;

					bright2 = bright + 16;

					fr = (float)(r * bright2) / (15.0 * 31.0);
					fg = (float)(g * bright2) / (15.0 * 31.0);
					fb = (float)(b * bright2) / (15.0 * 31.0);

					r2 = (int)(fr * 255.0) - 15;
					g2 = (int)(fg * 255.0) - 15;
					b2 = (int)(fb * 255.0) - 15;

					if (r2 < 0) r2 = 0;
					if (g2 < 0) g2 = 0;
					if (b2 < 0) b2 = 0;

					video_clut16[pen] = MAKECOL15(r2, g2, b2);
				}
			}
		}
	}
}


/*------------------------------------------------------
	CPS1�x�[�X�I�t�Z�b�g�擾
------------------------------------------------------*/

static u16 *cps1_base(int offset, int address_mask)
{
	int base = (cps1_port(offset) << 7) & address_mask;

	return &cps1_gfxram[base];
}


/*------------------------------------------------------
	CPS2�r�f�I������
------------------------------------------------------*/

int cps2_video_init(void)
{
	int i;

	cps2_mask_max = 0;

	for (i = 0; i < 4; i++)
	{
		if (driver->mask[i].mask_pri >= 0) cps2_mask_max++;
	}
	cps2_kludge = driver->kludge;
	cps2_scroll3_base = (driver->kludge & (CPS2_KLUDGE_SSF2 | CPS2_KLUDGE_SSF2T)) ? 0x0000 : 0x4000;

	if (driver->flags & 2)
		cps2_build_palette = cps2_build_palette_delay;
	else
		cps2_build_palette = cps2_build_palette_normal;

	cps_total_elements[TILE08] = gfx_total_elements[TILE08] + 0x20000;
	cps_total_elements[TILE16] = gfx_total_elements[TILE16];
	cps_total_elements[TILE32] = gfx_total_elements[TILE32] + 0x4000;

	cps_pen_usage[TILE08] = gfx_pen_usage[TILE08] - 0x20000;
	cps_pen_usage[TILE16] = gfx_pen_usage[TILE16];
	cps_pen_usage[TILE32] = gfx_pen_usage[TILE32] - 0x4000;

	cps2_init_tables();

	return 1;
}


/*------------------------------------------------------
	CPS2�r�f�I�I��
------------------------------------------------------*/

void cps2_video_exit(void)
{
}


/*------------------------------------------------------
	CPS2�r�f�I���Z�b�g
------------------------------------------------------*/

void  cps2_video_reset(void)
{
	int i;

	memset(cps1_gfxram, 0, sizeof(cps1_gfxram));
	memset(cps1_output, 0, sizeof(cps1_output));

	memset(cps2_object, 0, sizeof(cps2_object));
	memset(cps2_objram[0], 0, cps2_obj_size);
	memset(cps2_objram[1], 0, cps2_obj_size);

	memset(cps2_old_palette, 0, sizeof(cps2_old_palette));
	memset(video_palette, 0, sizeof(video_palette));
	memset(cps2_buffered_palette, 0, sizeof(cps2_buffered_palette));

	for (i = 0; i < cps2_palette_size >> 1; i += 16)
		video_palette[i + 15] = 0x8000;

	cps1_port(CPS1_OBJ_BASE)     = 0x9200;
	cps1_port(CPS1_SCROLL1_BASE) = 0x9000;
	cps1_port(CPS1_SCROLL2_BASE) = 0x9040;
	cps1_port(CPS1_SCROLL3_BASE) = 0x9080;
	cps1_port(CPS1_OTHER_BASE)   = 0x9100;
	cps1_port(CPS1_PALETTE_BASE) = 0x90c0;

	scanline1 = 262;
	scanline2 = 262;

	blit_reset();
}


/*------------------------------------------------------
	�p���b�g
------------------------------------------------------*/

static void cps2_build_palette_normal(void)
{
	int offset;
	u16 palette;

	cps2_palette = cps1_base(CPS1_PALETTE_BASE, cps2_palette_mask);

	for (offset = 0; offset < cps2_palette_size >> 1; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps2_palette[offset];

			if (palette != cps2_old_palette[offset])
			{
				cps2_old_palette[offset] = palette;
				video_palette[offset] = video_clut16[palette];
				blit_palette_mark_dirty(offset >> 4);
			}
		}
	}
}


/*------------------------------------------------------
	�p���b�g(xmcota�p object�̂�1�t���[���x�点�Ĕ��f)
------------------------------------------------------*/

static void cps2_build_palette_delay(void)
{
	int offset;
	u16 palette;

	cps2_palette = cps1_base(CPS1_PALETTE_BASE, cps2_palette_mask);

	for (offset = 0; offset < cps2_palette_size >> 3; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps2_buffered_palette[offset];

			if (palette != video_palette[offset])
			{
				video_palette[offset] = palette;
				blit_palette_mark_dirty(offset >> 4);
			}

			palette = cps2_palette[offset];

			if (palette != cps2_old_palette[offset])
			{
				cps2_old_palette[offset] = palette;
				cps2_buffered_palette[offset] = video_clut16[palette];
			}
		}
	}

	for (; offset < cps2_palette_size >> 1; offset++)
	{
		if (!(video_palette[offset] & 0x8000))
		{
			palette = cps2_palette[offset];

			if (palette != cps2_old_palette[offset])
			{
				cps2_old_palette[offset] = palette;
				video_palette[offset] = video_clut16[palette];
				blit_palette_mark_dirty(offset >> 4);
			}
		}
	}
}


/******************************************************************************
  Object (16x16)
******************************************************************************/

/*------------------------------------------------------
	object�`��
------------------------------------------------------*/

#define SCAN_OBJECT()												\
	if (attr & 0x80)												\
	{																\
		sx += cps2_object_xoffs;									\
		sy += cps2_object_yoffs;									\
	}																\
	sx += xoffs;													\
	sy += yoffs;													\
																	\
	if (!(attr & 0xff00))											\
	{																\
		if (pen_usage[code])										\
			BLIT_FUNC(sx, sy, code);								\
		continue;													\
	}																\
																	\
	nx = (attr & 0x0f00) >> 8;										\
	ny = (attr & 0xf000) >> 12;										\
																	\
	for (y = 0; y <= ny; y++)										\
	{																\
		for (x = 0; x <= nx; x++)									\
		{															\
			ncode = (code & ~0xf) + ((code + x) & 0xf) + (y << 4);	\
			ncode &= 0x3ffff;										\
																	\
			if (pen_usage[ncode])									\
			{														\
				if (attr & 0x20)									\
					nsx = sx + ((nx - x) << 4);						\
				else												\
					nsx = sx + (x << 4);							\
																	\
				if (attr & 0x40)									\
					nsy = sy + ((ny - y) << 4);						\
				else												\
					nsy = sy + (y << 4);							\
																	\
				BLIT_FUNC(nsx, nsy, ncode);							\
			}														\
		}															\
	}

/*------------------------------------------------------
	�ʏ�`��
------------------------------------------------------*/

static void cps2_render_object(void)
{
	int x, y, sx, sy, code, attr, pri;
	int nx, ny, nsx, nsy, ncode;
	int xoffs = 64 - cps2_object_xoffs;
	int yoffs = 16 - cps2_object_yoffs;
	struct cps2_object_t *object = cps2_object;
	u8 *pen_usage = cps_pen_usage[TILE16];

	while (object < cps2_last_object)
	{
		sx   = object->sx;
		sy   = object->sy;
		code = object->code;
		attr = object->attr;
		pri  = object->pri;
		object++;

#define BLIT_FUNC(x, y, c)	blit_draw_object(x & 0x3ff, y & 0x3ff, pri, c, attr)
		SCAN_OBJECT()
#undef BLIT_FUNC
	}
}


/*------------------------------------------------------
	�}�X�N�`��
------------------------------------------------------*/

static void cps2_render_object_zb(const struct spr_mask_t *mask)
{
	int x, y, sx, sy, code, attr, pri;
	int nx, ny, nsx, nsy, ncode;
	int xoffs = 64 - cps2_object_xoffs;
	int yoffs = 16 - cps2_object_yoffs;
	struct cps2_object_t *object = cps2_object;
	u8 *pen_usage = cps_pen_usage[TILE16];

	while (object < cps2_last_object)
	{
		sx   = object->sx;
		sy   = object->sy;
		code = object->code;
		attr = object->attr;
		pri  = object->pri;
		object++;

		if (pri == mask->obj_pri)
		{
			if (mask->mask_flag & MASK_CHECK_ATTR)
			{
				if ((attr & 0x1f) == 0x0e)
				{
					pri |= OBJECT_FLAG_ZOBJ;
				}
			}
			else if (!(mask->mask_flag & MASK_CHECK_OBJ) || (cps_pen_usage[TILE16][code] & SPR_MASKED_OBJ))
			{
				pri |= OBJECT_FLAG_ZOBJ;
			}
		}
		else if (pri == mask->mask_pri)
		{
			if (cps_pen_usage[TILE16][code] & SPR_MASK)
				pri |= OBJECT_FLAG_MASK;
		}

#define BLIT_FUNC(x, y, c)	blit_draw_object(x & 0x3ff, y & 0x3ff, pri, c, attr)
		SCAN_OBJECT()
#undef BLIT_FUNC
	}

	blit_set_object_mask(mask);
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���`�F�b�N
------------------------------------------------------*/

void cps2_scan_object_callback(void)
{
	int x, y, sx, sy, code, attr;
	int nx, ny, nsx, nsy, ncode;
	int xoffs = 64 - cps2_object_xoffs;
	int yoffs = 16 - cps2_object_yoffs;
	struct cps2_object_t *object = cps2_object;
	u8 *pen_usage = cps_pen_usage[TILE16];

	while (object < cps2_last_object)
	{
		sx   = object->sx;
		sy   = object->sy;
		code = object->code;
		attr = object->attr;
		object++;

#define BLIT_FUNC(x, y, c)	blit_update_object(x & 0x3ff, y & 0x3ff, c, attr)
		SCAN_OBJECT()
#undef BLIT_FUNC
	}
}


/******************************************************************************
  Scroll 1 (8x8 layer)
******************************************************************************/

#define scroll1_offset(col, row) (((row) & 0x1f) + (((col) & 0x3f) << 5) + (((row) & 0x20) << 6)) << 1

#define SCAN_SCROLL1(blit_func)												\
	int x, y, sx, sy, offs, code;											\
	int logical_col = cps_scroll1x >> 3;									\
	int logical_row = cps_scroll1y >> 3;									\
	int scroll_col  = cps_scroll1x & 0x07;									\
	int scroll_row  = cps_scroll1y & 0x07;									\
	int min_x, max_x, min_y, max_y;											\
	u8  *pen_usage = cps_pen_usage[TILE08];									\
																			\
	min_x = ( 64 + scroll_col) >> 3;										\
	max_x = (447 + scroll_col) >> 3;										\
																			\
	min_y = cps2_scanline_start & ~0x07;									\
	min_y = (min_y + scroll_row) >> 3;										\
	max_y = cps2_scanline_end;												\
	max_y = (max_y + scroll_row) >> 3;										\
																			\
	sy = (min_y << 3) - scroll_row;											\
																			\
	for (y = min_y; y <= max_y; y++, sy += 8)								\
	{																		\
		sx = (min_x << 3) - scroll_col;										\
																			\
		for (x = min_x; x <= max_x; x++, sx += 8)							\
		{																	\
			offs = scroll1_offset(logical_col + x, logical_row + y);		\
			code = 0x20000 + cps_scroll1[offs];								\
																			\
			if (pen_usage[code])											\
				blit_func(sx, sy, code, cps_scroll1[offs + 1]);				\
		}																	\
	}


/*------------------------------------------------------
	�`��
------------------------------------------------------*/

static void cps2_render_scroll1(void)
{
	SCAN_SCROLL1(blit_draw_scroll1)

	blit_finish_scroll1();
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���`�F�b�N
------------------------------------------------------*/

void cps2_scan_scroll1_callback(void)
{
	SCAN_SCROLL1(blit_update_scroll1)
}


/******************************************************************************
  Scroll 2 (16x16 layer)
******************************************************************************/

#define scroll2_offset(col, row) (((row) & 0x0f) + (((col) & 0x3f) << 4) + (((row) & 0x30) << 6)) << 1

#define SCAN_SCROLL2()														\
	int block;																\
	int x, y, sx, sy, offs, code;											\
	int logical_col, scroll_col;											\
	int logical_row = cps_scroll2y >> 4;									\
	int scroll_row = cps_scroll2y & 0x0f;									\
	int min_x, max_x, min_y, max_y;											\
	u8  *pen_usage = cps_pen_usage[TILE16];									\
																			\
	for (block = 0; block < cps_scroll2_blocks; block++)					\
	{																		\
		cps_scroll2x = scroll2[block].value;								\
		logical_col  = cps_scroll2x >> 4;									\
		scroll_col   = cps_scroll2x & 0x0f;									\
																			\
		min_x = ( 64 + scroll_col) >> 4;									\
		max_x = (447 + scroll_col) >> 4;									\
																			\
		min_y = scroll2[block].start & ~0x0f;								\
		min_y = (min_y + scroll_row) >> 4;									\
		max_y = scroll2[block].end;											\
		max_y = (max_y + scroll_row) >> 4;									\
																			\
		sy = (min_y << 4) - scroll_row;										\
																			\
		for (y = min_y; y <= max_y; y++, sy += 16)							\
		{																	\
			sx = (min_x << 4) - scroll_col;									\
																			\
			for (x = min_x; x <= max_x; x++, sx += 16)						\
			{																\
				offs = scroll2_offset(logical_col + x, logical_row + y);	\
				code = 0x10000 + cps_scroll2[offs];							\
																			\
				if (pen_usage[code]) BLIT_FUNC								\
			}																\
		}																	\
																			\
		BLIT_FINISH_FUNC													\
	}


/*------------------------------------------------------
	�`��
------------------------------------------------------*/


static void cps2_render_scroll2(void)
{
#define BLIT_FUNC			blit_draw_scroll2(sx, sy, code, cps_scroll2[offs + 1]);
#define BLIT_FINISH_FUNC	blit_finish_scroll2(scroll2[block].start, scroll2[block].end + 1);
	SCAN_SCROLL2()
#undef BLIT_FINISH_FUNC
#undef BLIT_FUNC
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���`�F�b�N
------------------------------------------------------*/

void cps2_scan_scroll2_callback(void)
{
#define BLIT_FUNC			blit_update_scroll2(sx, sy, code, cps_scroll2[offs + 1]);
#define BLIT_FINISH_FUNC
	SCAN_SCROLL2()
#undef BLIT_FINISH_FUNC
#undef BLIT_FUNC
}


/*------------------------------------------------------
	���C���X�N���[���v�Z
------------------------------------------------------*/

static void cps2_check_scroll2_distort(int distort)
{
	int line, block = 0;

	line = cps2_scanline_start;
	scroll2[0].start = line;
	scroll2[0].end   = line;

	if (distort)
	{
		int otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);

		scroll2[0].value = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

		if (!cps_raster_enable && !(driver->flags & 1))
		{
			line  = ((cps2_scanline_end + 1) - cps2_scanline_start) >> 1;
			line += cps2_scanline_start;
			scroll2[0].value = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];
			distort = 0;
		}

		if (distort)
		{
			u16 value, prev_value;

			prev_value = scroll2[0].value;

			for (; line <= cps2_scanline_end; line++)
			{
				value = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

				if (prev_value == value)
				{
					scroll2[block].end++;
				}
				else
				{
					block++;
					scroll2[block].value = prev_value = value;
					scroll2[block].start = line;
					scroll2[block].end   = line;
				}
			}

			if (!cps_raster_enable)
			{
				if (block >= 1 && scroll2[0].end - cps2_scanline_start > 16)
				{
					line  = ((scroll2[block].end + 1) - scroll2[block].start) >> 1;
					line += cps2_scanline_start;
					scroll2[1].value = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];
					scroll2[1].end = cps2_scanline_end;
					block = 1;
				}
				else
				{
					line  = ((cps2_scanline_end + 1) - cps2_scanline_start) >> 1;
					line += cps2_scanline_start;
					scroll2[0].value = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];
					block = 0;
				}
			}
		}
	}
	else
	{
		scroll2[0].value = cps_scroll2x;
	}

	if (!block)
	{
		cps_scroll2x = scroll2[0].value;
		scroll2[0].end = cps2_scanline_end;
	}

	cps_scroll2_blocks = block + 1;
}


/******************************************************************************
  Scroll 3 (32x32 layer)
******************************************************************************/

#define scroll3_offset(col, row) (((row) & 0x07) + (((col) & 0x3f) << 3) + (((row) & 0x38) << 6)) << 1

#define SCAN_SCROLL3(blit_func)												\
	int x, y, sx, sy, offs, code;											\
	int base = cps2_scroll3_base;											\
	int logical_col = cps_scroll3x >> 5;									\
	int logical_row = cps_scroll3y >> 5;									\
	int scroll_col  = cps_scroll3x & 0x1f;									\
	int scroll_row  = cps_scroll3y & 0x1f;									\
	int min_x, max_x, min_y, max_y;											\
	u8  *pen_usage = cps_pen_usage[TILE32];									\
																			\
	min_x = ( 64 + scroll_col) >> 5;										\
	max_x = (447 + scroll_col) >> 5;										\
																			\
	min_y = cps2_scanline_start & ~0x1f;									\
	min_y = (min_y + scroll_row) >> 5;										\
	max_y = cps2_scanline_end;												\
	max_y = (max_y + scroll_row) >> 5;										\
																			\
	sy = (min_y << 5) - scroll_row;											\
																			\
	for (y = min_y; y <= max_y; y++, sy += 32)								\
	{																		\
		sx = (min_x << 5) - scroll_col;										\
																			\
		for (x = min_x; x <= max_x; x++, sx += 32)							\
		{																	\
			offs = scroll3_offset(logical_col + x, logical_row + y);		\
			code = cps_scroll3[offs];										\
																			\
			switch (cps2_kludge)											\
			{																\
			case CPS2_KLUDGE_SSF2T:											\
				if (code < 0x5600) code += 0x4000;							\
				break;														\
																			\
			case CPS2_KLUDGE_XMCOTA:										\
				if (code >= 0x5800) code -= 0x4000;							\
				break;														\
																			\
			case CPS2_KLUDGE_DIMAHOO:										\
				code &= 0x3fff;												\
				break;														\
			}																\
			code += base;													\
																			\
			if (pen_usage[code])											\
				blit_func(sx, sy, code, cps_scroll3[offs + 1]);				\
		}																	\
	}


/*------------------------------------------------------
	�`��
------------------------------------------------------*/

static void cps2_render_scroll3(void)
{
	SCAN_SCROLL3(blit_draw_scroll3)

	blit_finish_scroll3();
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���`�F�b�N
------------------------------------------------------*/

void cps2_scan_scroll3_callback(void)
{
	SCAN_SCROLL3(blit_update_scroll3)
}


/******************************************************************************
	��ʍX�V����
******************************************************************************/

/*------------------------------------------------------
	���C���[��`��
------------------------------------------------------*/

static void cps2_render_layer(int layer)
{
	if (cps_layer_enabled[layer])
	{
		switch (layer)
		{
		case 1: cps2_render_scroll1(); break;
		case 2: cps2_render_scroll2(); break;
		case 3: cps2_render_scroll3(); break;
		}
	}
}


/*------------------------------------------------------
	�}�X�N�����`�F�b�N
------------------------------------------------------*/

static int cps2_check_mask(int mask_number)
{
	int mask_flag = driver->mask[mask_number].mask_flag;
	int mask_pri = driver->mask[mask_number].mask_pri;
	int obj_pri = driver->mask[mask_number].obj_pri;
	struct cps2_object_t *object;

	if (mask_flag & MASK_CHECK_MASK)
	{
		int found = 0;
		int mask = 1 << mask_number;

		object = cps2_object;

		while (object < cps2_last_object)
		{
			if (object->pri == mask_pri)
			{
				if (cps_pen_usage[TILE16][object->code] & mask)
				{
					found = 1;
					break;
				}
			}
			object++;
		}
		if (!found) return 0;
	}

	if (mask_flag & MASK_CHECK_OBJ)
	{
		object = cps2_object;

		while (object < cps2_last_object)
		{
			if (object->pri == obj_pri)
			{
				if (cps_pen_usage[TILE16][object->code] & SPR_MASKED_OBJ)
					return 1;
			}
			object++;
		}
		return 0;
	}
	else if (mask_flag & MASK_COUNT_OBJ)
	{
		int count = 0;

		object = cps2_object;

		while (object < cps2_last_object)
		{
			if (object->pri == obj_pri)
			{
				if (cps_pen_usage[TILE16][object->code] & SPR_MASKED_OBJ)
					count++;
			}
			object++;
		}
		return ((count == 24) ? 1 : 0);
	}
	else if (mask_flag & MASK_MSH)
	{
		object = cps2_object;

		while (object < cps2_last_object)
		{
			if (object->pri == obj_pri)
				object->pri = 6;
			else if (object->pri == mask_pri)
				object->pri = 7;
			object++;
		}
		return 0;
	}
	return 1;
}


/*------------------------------------------------------
	��ʍX�V
------------------------------------------------------*/

void cps2_screenrefresh(int start, int end)
{
	u16 layer_ctrl = cps1_port(CPS2_LAYER_CONTROL);
	u16 video_ctrl = cps1_port(CPS1_VIDEO_CONTROL);
	u16 pri_ctrl   = cps2_port(CPS2_OBJ_PRI);
	int i, priority, prev_pri;
	u8  layer[4], pri[4] = {0,};
	const struct spr_mask_t *mask = NULL;

	cps2_scanline_start = start;
	cps2_scanline_end   = end;

	blit_partial_start(start, end);

	cps_flip_screen = video_ctrl & 0x8000;

	cps_scroll1 = cps1_base(CPS1_SCROLL1_BASE, cps2_scroll_mask);
	cps_scroll2 = cps1_base(CPS1_SCROLL2_BASE, cps2_scroll_mask);
	cps_scroll3 = cps1_base(CPS1_SCROLL3_BASE, cps2_scroll_mask);
	cps_other   = cps1_base(CPS1_OTHER_BASE, cps2_other_mask);

	cps_scroll1x = cps1_port(CPS1_SCROLL1_SCROLLX);
	cps_scroll1y = cps1_port(CPS1_SCROLL1_SCROLLY);
	cps_scroll2x = cps1_port(CPS1_SCROLL2_SCROLLX);
	cps_scroll2y = cps1_port(CPS1_SCROLL2_SCROLLY);
	cps_scroll3x = cps1_port(CPS1_SCROLL3_SCROLLX);
	cps_scroll3y = cps1_port(CPS1_SCROLL3_SCROLLY);

	cps2_object_xoffs = cps2_port(CPS2_OBJ_XOFFS);
	cps2_object_yoffs = cps2_port(CPS2_OBJ_YOFFS);

	cps_layer_enabled[1] = layer_ctrl & 2;
	cps_layer_enabled[2] = layer_ctrl & 4;
	cps_layer_enabled[3] = layer_ctrl & 8;

	layer[0] = (layer_ctrl >>  6) & 3;
	layer[1] = (layer_ctrl >>  8) & 3;
	layer[2] = (layer_ctrl >> 10) & 3;
	layer[3] = (layer_ctrl >> 12) & 3;

	pri[1] = (pri_ctrl >>  4) & 7;
	pri[2] = (pri_ctrl >>  8) & 7;
	pri[3] = (pri_ctrl >> 12) & 7;

	priority = 8;
	for (i = 3; i >= 0; i--)
	{
		if (layer[i] > 0)
		{
			if (pri[layer[i]] > priority)
				pri[layer[i]] = priority;
			else
				priority = pri[layer[i]];
		}
	}

	for (i = 0; i < cps2_mask_max; i++)
	{
		if (cps2_check_mask(i))
		{
			mask = &driver->mask[i];
			break;
		}
	}

	cps2_check_scroll2_distort(video_ctrl & 1);

	if (mask)
		cps2_render_object_zb(mask);
	else
		cps2_render_object();

	prev_pri = -1;
	for (priority = 0; priority < 8; priority++)
	{
		for (i = 0; i < LAYER_MAX; i++)
		{
			if (pri[layer[i]] == priority)
			{
				if (prev_pri < priority)
				{
					blit_finish_object(prev_pri + 1, priority);
					prev_pri = priority;
				}
				cps2_render_layer(layer[i]);
			}
		}
	}
	if (prev_pri < 7)
	{
		blit_finish_object(prev_pri + 1, 7);
	}
}


/*------------------------------------------------------
	object RAM�X�V
------------------------------------------------------*/

void cps2_objram_latch(void)
{
	u16 *base = (u16 *)cps2_objram[cps2_objram_bank];
	u16 *end  = base + (cps2_obj_size >> 1);
	struct cps2_object_t *object = cps2_object;

	while (base < end)
	{
		if (base[1] >= 0x8000 || base[3] >= 0xff00)
			break;

		if (base[0] | base[3])
		{
			object->sx   = base[0];
			object->sy   = base[1];
			object->code = base[2] + ((base[1] & 0x6000) << 3);
			object->attr = base[3];
			object->pri  = (base[0] >> 13) & 7;
			object++;
		}
		base += 4;
	}
	cps2_last_object = object;
}


/******************************************************************************
	�Z�[�u/���[�h �X�e�[�g
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( video )
{
	state_save_long(&cps2_objram_bank, 1);
	state_save_word(cps2_old_palette, 2048);
	state_save_word(video_palette, 2048);
	state_save_word(cps2_buffered_palette, 512);
}

STATE_LOAD( video )
{
	state_load_long(&cps2_objram_bank, 1);
	state_load_word(cps2_old_palette, 2048);
	state_load_word(video_palette, 2048);
	state_load_word(cps2_buffered_palette, 512);
	cps2_objram_latch();
}

#endif /* SAVE_STATE */
