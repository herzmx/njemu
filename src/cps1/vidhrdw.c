/******************************************************************************

	vidhrdw.c

	CPS1 �r�f�I�G�~�����[�V����

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	�O���[�o���ϐ�
******************************************************************************/

int cps_flip_screen;
int cps_rotate_screen;
int cps_raster_enable;


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

#define cps1_scroll_size		0x4000		/* scroll1, scroll2, scroll3 */
#define cps1_other_size			0x0800
#define cps1_palette_align		0x0800		/* can't be larger than this, breaks ringdest & batcirc otherwise */
#define cps1_palette_size		6*32*16*2	/* Size of palette RAM */

#define cps1_scroll_mask		(((~0x3fff) & 0x3ffff) >> 1)
#define cps1_other_mask			(((~0x07ff) & 0x3ffff) >> 1)
#define cps1_palette_mask		(((~0x07ff) & 0x3ffff) >> 1)

static UINT16 *cps_scroll1;
static UINT16 *cps_scroll2;
static UINT16 *cps_scroll3;
static UINT16 *cps_other;
static UINT16 *cps1_palette;
static UINT16 ALIGN_DATA cps1_old_palette[cps1_palette_size >> 1];

static UINT8 cps_layer_enabled[4];								/* Layer enabled [Y/N] */
static INT16 cps_scroll1x, cps_scroll1y;
static INT16 cps_scroll2x, cps_scroll2y;
static INT16 cps_scroll3x, cps_scroll3y;
static UINT32 cps1_object_num;									/* object total sprites num */
static UINT8  *cps1_object_pen_usage;							/* object sprites pen usage */
static UINT8  ALIGN_DATA cps1_scroll2_pen_usage[0x10000];		/* scroll2 sprites pen usage */
static UINT16 ALIGN_DATA cps1_scroll_pen_usage[4][0x10000];	/* scroll sprites pen usage */

#define cps1_obj_size			0x800
#define cps1_max_obj			(cps1_obj_size >> 3)
#define cps1_obj_mask			(((~0x07ff) & 0x3ffff) >> 1)

struct cps1_object_t
{
	UINT16 sx;
	UINT16 sy;
	UINT32 code;
	UINT16 attr;
};

static struct cps1_object_t ALIGN_DATA cps1_object[cps1_max_obj + 1];
static struct cps1_object_t *cps1_last_object;

static UINT16 cps1_transparency_scroll[4];	/* Transparency pens of scroll layers */
static UINT16 cps1_has_stars;
static INT16 cps1_stars1x, cps1_stars1y;
static INT16 cps1_stars2x, cps1_stars2y;
static UINT16 cps1_high_layer;
static UINT16 cps1_kludge;

struct cps_scroll2_t
{
	UINT16 value;
	INT16 start;
	INT16 end;
};

static struct cps_scroll2_t ALIGN_DATA scroll2[224];
static UINT16 cps_scroll2_blocks;

static UINT16 ALIGN_DATA video_clut16[65536];
UINT16 ALIGN_PSPDATA video_palette[cps1_palette_size >> 1];


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
	CPS�|�[�g�ǂݍ���
------------------------------------------------------*/

#define cps1_port(offset)	cps1_output[(offset) >> 1]


/******************************************************************************
	CPS1 �������n���h��
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
	CPS1 �r�f�I�`�揈��
******************************************************************************/

/*------------------------------------------------------
	�J���[�e�[�u���쐬
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
					UINT16 pen;
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
	�X�v���C�g�f�R�[�h
------------------------------------------------------*/

static int cps1_gfx_decode(void)
{
	UINT8 *gfx = memory_region_gfx1;
	UINT32 size = memory_length_gfx1;
	UINT32 i, j, k, count;
	UINT32 base, start, end, limit;
	UINT32 *tile, data;

	if (driver->has_stars)
		i = 0x10000 >> 2;
	else
		i = 0;

	for (; i < size >> 2; i++)
	{
		UINT32 src = gfx[4 * i] + (gfx[4 * i + 1] << 8) + (gfx[4 * i + 2] << 16) + (gfx[4 * i + 3] << 24);
		UINT32 dw = 0;

		for (j = 0; j < 8; j++)
		{
			int n = 0;
			UINT32 mask = (0x80808080 >> j) & src;

			if (mask & 0x000000ff) n |= 1;
			if (mask & 0x0000ff00) n |= 2;
			if (mask & 0x00ff0000) n |= 4;
			if (mask & 0xff000000) n |= 8;

			dw |= n << (j * 4);
		}

		data = ((dw & 0x0000000f) >>  0) | ((dw & 0x000000f0) <<  4)
			 | ((dw & 0x00000f00) <<  8) | ((dw & 0x0000f000) << 12)
			 | ((dw & 0x000f0000) >> 12) | ((dw & 0x00f00000) >>  8)
			 | ((dw & 0x0f000000) >>  4) | ((dw & 0xf0000000) >>  0);

		gfx[4 * i + 0] = data >>  0;
		gfx[4 * i + 1] = data >>  8;
		gfx[4 * i + 2] = data >> 16;
		gfx[4 * i + 3] = data >> 24;
	}

	if (driver->gfx_limit)
	{
		for (i = driver->gfx_limit + 1; i < size; i++)
			memory_region_gfx1[i] = 0xff;
	}

	cps1_object_num = memory_length_gfx1 >> 7;
	if ((cps1_object_pen_usage = memalign(MEM_ALIGN, cps1_object_num)) == NULL)
		return 0;

	memset(cps1_object_pen_usage, 0, cps1_object_num);
	memset(cps1_scroll2_pen_usage, 0, sizeof(cps1_scroll2_pen_usage));
	memset(cps1_scroll_pen_usage, 0, sizeof(cps1_scroll_pen_usage));

	//---------------------------------------------------------
	// object (16x16)
	//---------------------------------------------------------
	if (driver->has_stars)
		start = 0x10000 >> 7;
	else
		start = 0;

	for (i = start; i < cps1_object_num; i++)
	{
		count = 0;

		tile = (UINT32 *)&memory_region_gfx1[i << 7];

		for (j = 0; j < 2*16; j++)
		{
			data = *tile++;
			for (k = 0; k < 8; k++)
			{
				if ((data & 0x0f) == 0x0f)
					count++;
				data >>= 4;
			}
		}
		if (count == 0)
			cps1_object_pen_usage[i] = SPRITE_OPAQUE;
		else if (count != 2*16*8)
			cps1_object_pen_usage[i] = SPRITE_TRANSPARENT;
	}

	//---------------------------------------------------------
	// scroll1 (8x8)
	//---------------------------------------------------------
	base  = driver->bank_scroll1 << 15;
	start = driver->scroll1.start;
	end   = driver->scroll1.end;
	limit = (size >> 6) - base;

	for (i = start; i <= end; i++)
	{
		if (i >= limit || i == 0x20) continue;

		tile = (UINT32 *)&memory_region_gfx1[(base + i) << 6];

		for (j = 0; j < 8; j++)
		{
			data = *tile++;
			for (k = 0; k < 8; k++)
			{
				cps1_scroll_pen_usage[0][i] |= 1 << (data & 0x0f);
				data >>= 4;
			}
			data = *tile++;
			for (k = 0; k < 8; k++)
			{
				cps1_scroll_pen_usage[1][i] |= 1 << (data & 0x0f);
				data >>= 4;
			}
		}
		cps1_scroll_pen_usage[0][i] &= 0x7fff;
		cps1_scroll_pen_usage[1][i] &= 0x7fff;
	}
	if (cps1_kludge != CPS1_KLUDGE_PANG3)
	{
		start = (base + start) << 6;
		end   = ((base + end) << 6) | 0x3ff;

		for (i = start >> 7; i <= end >> 7; i++)
			cps1_object_pen_usage[i] = SPRITE_BLANK;
	}

	//---------------------------------------------------------
	// scroll2 (16x16)
	//---------------------------------------------------------
	base  = driver->bank_scroll2 << 14;
	start = driver->scroll2.start;
	end   = driver->scroll2.end;
	limit = (size >> 7) - base;

scroll2_check:
	for (i = start; i <= end; i++)
	{
		if (i >= limit) continue;

		count = 0;

		tile = (UINT32 *)&memory_region_gfx1[(base + i) << 7];

		for (j = 0; j < 2*16; j++)
		{
			data = *tile++;
			for (k = 0; k < 8; k++)
			{
				if ((data & 0x0f) == 0x0f) count++;
				cps1_scroll_pen_usage[2][i] |= 1 << (data & 0x0f);
				data >>= 4;
			}
		}

		if (count == 0)
			cps1_scroll2_pen_usage[i] = SPRITE_OPAQUE;
//		else if (count != 2*16*8)
//			cps1_scroll2_pen_usage[i] = SPRITE_TRANSPARENT;

		cps1_scroll_pen_usage[2][i] &= 0x7fff;
	}
	if (cps1_kludge == CPS1_KLUDGE_PANG3)
	{
		for (i = 0x0790; i < 0x079f; i++)
		{
			cps1_scroll_pen_usage[2][i] = 1 << (i & 0x0f);
		}
	}
	else if (cps1_kludge == CPS1_KLUDGE_MERCS)
	{
		if (start != 0x5400)
		{
			start = 0x5400;
			end   = 0x5bff;
			goto scroll2_check;
		}
	}

	//---------------------------------------------------------
	// scroll3 (32x32)
	//---------------------------------------------------------
	base  = driver->bank_scroll3 << 12;
	start = driver->scroll3.start;
	end   = driver->scroll3.end;
	limit = (size >> 9) - base;

scroll3_check:
	for (i = start; i <= end; i++)
	{
		if (i >= limit) continue;

		tile = (UINT32 *)&memory_region_gfx1[(base + i) << 9];

		for (j = 0; j < 4*32; j++)
		{
			data = *tile++;
			for (k = 0; k < 8; k++)
			{
				cps1_scroll_pen_usage[3][i] |= 1 << (data & 0x0f);
				data >>= 4;
			}
		}
		cps1_scroll_pen_usage[3][i] &= 0x7fff;
	}
	if (cps1_kludge != CPS1_KLUDGE_PANG3)
	{
		UINT32 start2 = (base + start) << 9;
		UINT32 end2   = ((base + end) << 9) | 0x1ff;

		for (i = start2 >> 7; i <= end2 >> 7; i++)
			cps1_object_pen_usage[i] = SPRITE_BLANK;
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

	return 1;
}


/*------------------------------------------------------
	CPS1�x�[�X�I�t�Z�b�g�擾
------------------------------------------------------*/

static UINT16 *cps1_base(int offset, int address_mask)
{
	UINT32 base = (cps1_port(offset) << 7) & address_mask;

	return &cps1_gfxram[base];
}


/*------------------------------------------------------
	CPS1�r�f�I������
------------------------------------------------------*/

int cps1_video_init(void)
{
	cps1_has_stars = driver->has_stars;
	cps1_kludge    = driver->kludge;

#if !RELEASE
	if (strcmp(game_name, "sf2rb") == 0)
	{
		/* Patch out protection check */
		UINT16 *rom = (UINT16 *)memory_region_cpu1;
		rom[0xe5464 >> 1] = 0x6012;
	}
	if (strcmp(game_name, "sf2rb2") == 0)
	{
		/* Patch out protection check */
		UINT16 *rom = (UINT16 *)memory_region_cpu1;
		rom[0xe5332 >> 1] = 0x6014;
	}
	if (strcmp(game_name, "sf2m2") == 0)
	{
		/* Patch out protection check */
		UINT16 *rom = (UINT16 *)memory_region_cpu1;
		rom[0xc0670 >> 1] = 0x4e71;
	}
	if (strcmp(game_name, "dinoh") == 0)
	{
		/* Patch out Q-Sound test */
		UINT16 *rom = (UINT16 *)memory_region_cpu1;
		rom[0xaacf4 >> 1] = 0x4e71;
	}
	if (strcmp(game_name, "dinoha") == 0)
	{
		/* Patch out Q-Sound test */
		UINT16 *rom = (UINT16 *)memory_region_cpu1;
		rom[0xaacf4 >> 1] = 0x4e71;
	}
#endif

	cps1_init_tables();

	return cps1_gfx_decode();
}


/*------------------------------------------------------
	CPS1�r�f�I�I��
------------------------------------------------------*/

void cps1_video_exit(void)
{
	if (cps1_object_pen_usage)
	{
		free(cps1_object_pen_usage);
	}
}


/*------------------------------------------------------
	CPS1�r�f�I���Z�b�g
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

#if !RELEASE
	if (cps1_kludge == CPS1_KLUDGE_SF2CEB)
		cps1_port(CPS1_OBJ_BASE) = 0x9100;
#endif

	cps_layer_enabled[0] = 1;

	cps1_high_layer = 0;
	cps1_transparency_scroll[0] = 0x0000;
	cps1_transparency_scroll[1] = 0x0000;
	cps1_transparency_scroll[2] = 0x0000;
	cps1_transparency_scroll[3] = 0x0000;

	blit_reset(driver->bank_scroll1, driver->bank_scroll2, driver->bank_scroll3, cps1_scroll2_pen_usage);
}


/*------------------------------------------------------
	�p���b�g
------------------------------------------------------*/

static void cps1_build_palette(void)
{
	UINT32 offset;
	UINT16 palette;

	cps1_palette = cps1_base(CPS1_PALETTE_BASE, cps1_palette_mask);

	for (offset = 0; offset < 1*32*16; offset++)
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

	for (; offset < 4*32*16; offset++)
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
	object�`��
------------------------------------------------------*/

#define SCAN_OBJECT(blit_func)												\
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
	}

/*------------------------------------------------------
	�`��
------------------------------------------------------*/

static struct cps1_object_t *object1;

static void cps1_render_object(void)
{
	INT16 x, y, sx, sy, nx, ny, nsx, nsy;
	UINT16 attr;
	UINT32 code, ncode;

	object1 = cps1_last_object;

	while (object1 >= cps1_object)
	{
		sx   = object1->sx;
		sy   = object1->sy;
		code = object1->code;
		attr = object1->attr;
		object1--;

		SCAN_OBJECT(blit_draw_object)
	}

	blit_finish_object();
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���`�F�b�N
------------------------------------------------------*/

void cps1_scan_object(void)
{
	INT16 x, y, sx, sy, nx, ny, nsx, nsy;
	UINT16 attr;
	UINT32 code, ncode;
	struct cps1_object_t *object = object1;

	while (object >= cps1_object)
	{
		sx   = object->sx;
		sy   = object->sy;
		code = object->code;
		attr = object->attr;
		object--;

		SCAN_OBJECT(blit_update_object)
	}
}


/******************************************************************************
  Scroll 1 (8x8 layer)
******************************************************************************/

#define scroll1_offset(col, row) (((row) & 0x1f) + (((col) & 0x3f) << 5) + (((row) & 0x20) << 6)) << 1

#define SCAN_SCROLL1()													\
	UINT32 code;															\
	UINT16 offs, attr, gfxset;												\
	INT16 x, y, sx, sy, min_x, max_x, min_y, max_y;						\
	INT16 logical_col = cps_scroll1x >> 3;								\
	INT16 logical_row = cps_scroll1y >> 3;								\
	INT16 scroll_col  = cps_scroll1x & 0x07;								\
	INT16 scroll_row  = cps_scroll1y & 0x07;								\
																		\
	min_x = ( 64 + scroll_col) >> 3;									\
	max_x = (447 + scroll_col) >> 3;									\
																		\
	min_y = ( 16 + scroll_row) >> 3;									\
	max_y = (239 + scroll_row) >> 3;									\
																		\
	sy = (min_y << 3) - scroll_row;										\
																		\
	for (y = min_y; y <= max_y; y++, sy += 8)							\
	{																	\
		sx = (min_x << 3) - scroll_col;									\
																		\
		for (x = min_x; x <= max_x; x++, sx += 8)						\
		{																\
			offs   = scroll1_offset(logical_col + x, logical_row + y);	\
			gfxset = (offs & 0x40) >> 6;								\
			code   = cps_scroll1[offs];									\
			DRAW_SCROLL1												\
		}																\
	}

/*------------------------------------------------------
	�`��
------------------------------------------------------*/

#define DRAW_SCROLL1													\
	if (cps1_scroll_pen_usage[gfxset][code])							\
	{																	\
		attr = cps_scroll1[offs + 1];									\
		blit_draw_scroll1(sx, sy, code, attr, gfxset);					\
	}

static void cps1_render_scroll1_normal(void)
{
	SCAN_SCROLL1()
}

#undef DRAW_SCROLL1

#define DRAW_SCROLL1													\
	attr  = cps_scroll1[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (cps1_scroll_pen_usage[gfxset][code] & (tpens ^ 0x7fff))			\
	{																	\
		blit_draw_scroll1(sx, sy, code, attr, gfxset);					\
	}																	\
	if (cps1_scroll_pen_usage[gfxset][code] & tpens)					\
	{																	\
		blit_draw_scroll1h(sx, sy, code, attr, tpens, gfxset);			\
	}

static void cps1_render_scroll1_separate(void)
{
	UINT16 tpens;
	SCAN_SCROLL1()
}

#undef DRAW_SCROLL1

static void cps1_render_scroll1(void)
{
	if (cps1_high_layer == LAYER_SCROLL1)
		cps1_render_scroll1_separate();
	else
		cps1_render_scroll1_normal();

	blit_finish_scroll1();
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���X�L����
------------------------------------------------------*/

#define DRAW_SCROLL1													\
	if (cps1_scroll_pen_usage[gfxset][code])							\
	{																	\
		attr = cps_scroll1[offs + 1];									\
		blit_update_scroll1(sx, sy, code, attr);						\
	}

static void cps1_scan_scroll1_normal(void)
{
	SCAN_SCROLL1()
}

#undef DRAW_SCROLL1

#define DRAW_SCROLL1													\
	attr  = cps_scroll1[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (cps1_scroll_pen_usage[gfxset][code] & (tpens ^ 0x7fff))			\
	{																	\
		blit_update_scroll1(sx, sy, code, attr);						\
	}

static void cps1_scan_scroll1_background(void)
{
	UINT16 tpens;
	SCAN_SCROLL1()
}

#undef DRAW_SCROLL1

#define DRAW_SCROLL1													\
	attr  = cps_scroll1[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (cps1_scroll_pen_usage[gfxset][code] & tpens)					\
	{																	\
		blit_update_scrollh(sx, sy, code, attr);						\
	}

void cps1_scan_scroll1_foreground(void)
{
	UINT16 tpens;
	SCAN_SCROLL1()
}

#undef DRAW_SCROLL1

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

#define scroll2_offset(col, row) (((row) & 0x0f) + (((col) & 0x3f) << 4) + (((row) & 0x30) << 6)) << 1

#define SCAN_SCROLL2()														\
	UINT32 code;																\
	UINT16 block, offs, attr;													\
	INT16 x, y, sx, sy, min_x, max_x, min_y, max_y;							\
	INT16 logical_col, scroll_col;											\
	INT16 logical_row = cps_scroll2y >> 4;									\
	INT16 scroll_row = cps_scroll2y & 0x0f;									\
	UINT16 *pen_usage = cps1_scroll_pen_usage[2];								\
																			\
	for (block = 0; block < cps_scroll2_blocks; block++)					\
	{																		\
		BLIT_SET_CLIP_FUNC													\
																			\
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
			BLIT_CHECK_CLIP_FUNC											\
																			\
			sx = (min_x << 4) - scroll_col;									\
																			\
			for (x = min_x; x <= max_x; x++, sx += 16)						\
			{																\
				offs = scroll2_offset(logical_col + x, logical_row + y);	\
				code = cps_scroll2[offs];									\
																			\
				DRAW_SCROLL2												\
			}																\
		}																	\
																			\
		BLIT_FINISH_FUNC													\
	}


/*------------------------------------------------------
	�`��
------------------------------------------------------*/

#define BLIT_SET_CLIP_FUNC		blit_set_clip_scroll2(scroll2[block].start, scroll2[block].end);
#define BLIT_CHECK_CLIP_FUNC	if (!blit_check_clip_scroll2(sy)) continue;
#define BLIT_FINISH_FUNC		blit_finish_scroll2();

#define DRAW_SCROLL2													\
	if (pen_usage[code])												\
	{																	\
		attr = cps_scroll2[offs + 1];									\
		blit_draw_scroll2(sx, sy, code, attr);							\
	}

static void cps1_render_scroll2_normal(void)
{
	SCAN_SCROLL2()
}

#undef DRAW_SCROLL2

#define DRAW_SCROLL2													\
	attr  = cps_scroll2[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & (tpens ^ 0x7fff))								\
	{																	\
		blit_draw_scroll2(sx, sy, code, attr);							\
	}																	\
	if (pen_usage[code] & tpens)										\
	{																	\
		blit_draw_scroll2h(sx, sy, code, attr, tpens);					\
	}

static void cps1_render_scroll2_separate(void)
{
	UINT16 tpens;
	SCAN_SCROLL2()
}

#undef DRAW_SCROLL2

#define DRAW_SCROLL2													\
	attr  = cps_scroll2[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & (tpens ^ 0x7fff))								\
	{																	\
		blit_draw_scroll2(sx, sy, code, attr);							\
	}

static void cps1_render_scroll2_background(void)
{
	UINT16 tpens;
	SCAN_SCROLL2()
}

#undef DRAW_SCROLL2

#undef BLIT_FINISH_FUNC
#define BLIT_FINISH_FUNC	blit_finish_scroll2h();

#define DRAW_SCROLL2													\
	attr  = cps_scroll2[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & tpens)										\
	{																	\
		blit_draw_scroll2h(sx, sy, code, attr, tpens);					\
	}

static void cps1_render_scroll2_foreground(void)
{
	if (cps_scroll2_blocks == 1)
	{
		blit_finish_scroll2h();
	}
	else
	{
		UINT16 tpens;
		SCAN_SCROLL2()
	}
}

#undef DRAW_SCROLL2

static void cps1_render_scroll2(void)
{
	if (cps1_high_layer == LAYER_SCROLL2)
	{
		if (cps_scroll2_blocks == 1)
			cps1_render_scroll2_separate();
		else
			cps1_render_scroll2_background();
	}
	else
	{
		cps1_render_scroll2_normal();
	}
}

#undef BLIT_FINISH_FUNC
#undef BLIT_CHECK_CLIP_FUNC
#undef BLIT_SET_CLIP_FUNC


/*------------------------------------------------------
	�g�p���̃X�v���C�g���`�F�b�N
------------------------------------------------------*/

#define BLIT_SET_CLIP_FUNC
#define BLIT_CHECK_CLIP_FUNC
#define BLIT_FINISH_FUNC

#define DRAW_SCROLL2													\
	if (pen_usage[code])												\
	{																	\
		attr = cps_scroll2[offs + 1];									\
		blit_update_scroll2(sx, sy, code, attr);						\
	}

static void cps1_scan_scroll2_normal(void)
{
	SCAN_SCROLL2()
}

#undef DRAW_SCROLL2

#define DRAW_SCROLL2													\
	attr  = cps_scroll2[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & (tpens ^ 0x7fff))								\
	{																	\
		blit_update_scroll2(sx, sy, code, attr);						\
	}

static void cps1_scan_scroll2_background(void)
{
	UINT16 tpens;
	SCAN_SCROLL2()
}

#undef DRAW_SCROLL2

#define DRAW_SCROLL2													\
	attr  = cps_scroll2[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & tpens)										\
	{																	\
		blit_update_scroll2h(sx, sy, code, attr);						\
	}

void cps1_scan_scroll2_foreground(void)
{
	UINT16 tpens;
	SCAN_SCROLL2()
}

#undef DRAW_SCROLL2

void cps1_scan_scroll2(void)
{
	if (cps1_high_layer == LAYER_SCROLL2)
		cps1_scan_scroll2_background();
	else
		cps1_scan_scroll2_normal();
}

#undef BLIT_FINISH_FUNC
#undef BLIT_CHECK_CLIP_FUNC
#undef BLIT_SET_CLIP_FUNC


/*------------------------------------------------------
	���C���X�N���[���v�Z
------------------------------------------------------*/

static void cps1_check_scroll2_distort(int distort)
{
	UINT16 line, block = 0;

	line = 16;
	scroll2[0].start = 16;
	scroll2[0].end   = 16;

	if (distort)
	{
		UINT16 otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);

		scroll2[0].value = cps_scroll2x + cps_other[(16 + otheroffs) & 0x3ff];

		if (!cps_raster_enable)
		{
			scroll2[0].value = cps_scroll2x + cps_other[(128 + otheroffs) & 0x3ff];
		}
		else
		{
			UINT16 value, prev_value;

			prev_value = scroll2[0].value;

			for (; line < 240; line++)
			{
				value = cps_scroll2x + cps_other[(line + otheroffs) & 0x3ff];

				if (prev_value == value)
				{
					scroll2[block].end = line;
				}
				else
				{
					block++;
					scroll2[block].value = prev_value = value;
					scroll2[block].start = line;
					scroll2[block].end   = line;
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
		scroll2[0].end = 239;
	}

	cps_scroll2_blocks = block + 1;
}


/******************************************************************************
  Scroll 3 (32x32 layer)
******************************************************************************/

#define scroll3_offset(col, row) (((row) & 0x07) + (((col) & 0x3f) << 3) + (((row) & 0x38) << 6)) << 1

#define SCAN_SCROLL3(blit_func)											\
	UINT32 code;															\
	UINT16 offs, attr;														\
	INT16 x, y, sx, sy, min_x, max_x, min_y, max_y;						\
	INT16 logical_col = cps_scroll3x >> 5;								\
	INT16 logical_row = cps_scroll3y >> 5;								\
	INT16 scroll_col  = cps_scroll3x & 0x1f;								\
	INT16 scroll_row  = cps_scroll3y & 0x1f;								\
	UINT16 *pen_usage = cps1_scroll_pen_usage[3];							\
																		\
	min_x = ( 64 + scroll_col) >> 5;									\
	max_x = (447 + scroll_col) >> 5;									\
																		\
	min_y = ( 16 + scroll_row) >> 5;									\
	max_y = (239 + scroll_row) >> 5;									\
																		\
	sy = (min_y << 5) - scroll_row;										\
																		\
	for (y = min_y; y <= max_y; y++, sy += 32)							\
	{																	\
		sx = (min_x << 5) - scroll_col;									\
																		\
		for (x = min_x; x <= max_x; x++, sx += 32)						\
		{																\
			offs = scroll3_offset(logical_col + x, logical_row + y);	\
			code = cps_scroll3[offs];									\
																		\
			if (cps1_kludge == CPS1_KLUDGE_3WONDERS)					\
				if (code < 0x0e00) code += 0x1000;						\
																		\
			DRAW_SCROLL3												\
		}																\
	}

/*------------------------------------------------------
	�`��
------------------------------------------------------*/

#define DRAW_SCROLL3													\
	if (pen_usage[code])												\
	{																	\
		attr = cps_scroll3[offs + 1];									\
		blit_draw_scroll3(sx, sy, code, attr);							\
	}

static void cps1_render_scroll3_normal(void)
{
	SCAN_SCROLL3()
}

#undef DRAW_SCROLL3

#define DRAW_SCROLL3													\
	attr  = cps_scroll3[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & (tpens ^ 0x7fff))								\
	{																	\
		blit_draw_scroll3(sx, sy, code, attr);							\
	}																	\
	if (pen_usage[code] & tpens)										\
	{																	\
		blit_draw_scroll3h(sx, sy, code, attr, tpens);					\
	}

static void cps1_render_scroll3_separate(void)
{
	UINT16 tpens;
	SCAN_SCROLL3()
}

#undef DRAW_SCROLL3

static void cps1_render_scroll3(void)
{
	if (cps1_high_layer == LAYER_SCROLL3)
		cps1_render_scroll3_separate();
	else
		cps1_render_scroll3_normal();

	blit_finish_scroll3();
}


/*------------------------------------------------------
	�g�p���̃X�v���C�g���X�L����
------------------------------------------------------*/

#define DRAW_SCROLL3													\
	if (pen_usage[code])												\
	{																	\
		attr = cps_scroll3[offs + 1];									\
		blit_update_scroll3(sx, sy, code, attr);						\
	}

static void cps1_scan_scroll3_normal(void)
{
	SCAN_SCROLL3()
}

#undef DRAW_SCROLL3

#define DRAW_SCROLL3													\
	attr  = cps_scroll3[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & (tpens ^ 0x7fff))								\
	{																	\
		blit_update_scroll3(sx, sy, code, attr);						\
	}

static void cps1_scan_scroll3_background(void)
{
	UINT16 tpens;
	SCAN_SCROLL3()
}

#undef DRAW_SCROLL3

#define DRAW_SCROLL3													\
	attr  = cps_scroll3[offs + 1];										\
	tpens = cps1_transparency_scroll[(attr & 0x0180) >> 7];				\
	if (pen_usage[code] & tpens)										\
	{																	\
		blit_update_scrollh(sx, sy, code, attr);						\
	}

void cps1_scan_scroll3_foreground(void)
{
	UINT16 tpens;
	SCAN_SCROLL3()
}

#undef DRAW_SCROLL3

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

static void cps1_render_stars(void)
{
	UINT16 offs, *pal;
	UINT16 layer_ctrl = cps1_port(driver->layer_control);
	UINT32 current_frame = timer_getcurrentframe();
	UINT16 *video_buf = (UINT16 *)video_frame_addr(work_frame, 0, 0);

	if (layer_ctrl & driver->layer_enable_mask[3])
	{
		cps1_stars1x = cps1_port(CPS1_STARS1_SCROLLX);
		cps1_stars1y = cps1_port(CPS1_STARS1_SCROLLY);
		pal = &video_palette[0xa00];

		for (offs = 0; offs < 0x2000 >> 1; offs++)
		{
			UINT8 col = memory_region_gfx1[(offs << 3) + 4];

			if (col != 0x0f)
			{
				INT16 sx = (offs >> 8) << 5;
				INT16 sy = offs & 0xff;

				sx = (sx - cps1_stars2x + (col & 0x1f)) & 0x1ff;
				sy = (sy - cps1_stars2y) & 0xff;
				col = ((col & 0xe0) >> 1) + ((current_frame >> 4) & 0x0f);

				if ((sx >= 64 && sx <= 448) && (sy >= 16 && sy <= 248))
					video_buf[(sy << 9) + sx] = pal[col];
			}
		}
	}

	if (layer_ctrl & driver->layer_enable_mask[4])
	{
		cps1_stars2x = cps1_port(CPS1_STARS2_SCROLLX);
		cps1_stars2y = cps1_port(CPS1_STARS2_SCROLLY);
		pal = &video_palette[0x800];

		for (offs = 0; offs < 0x2000 >> 1; offs++)
		{
			UINT8 col = memory_region_gfx1[offs << 3];

			if (col != 0x0f)
			{
				int sx = (offs >> 8) << 5;
				int sy = offs & 0xff;

				sx = (sx - cps1_stars1x + (col & 0x1f)) & 0x1ff;
				sy = (sy - cps1_stars1y) & 0xff;
				col = ((col & 0xe0) >> 1) + ((current_frame >> 4) & 0x0f);

				if ((sx >= 64 && sx <= 448) && (sy >= 16 && sy <= 248))
					video_buf[(sy << 9) + sx] = pal[col];
			}
		}
	}
}


/******************************************************************************
	��ʍX�V����
******************************************************************************/

/*------------------------------------------------------
	���C���[��`��
------------------------------------------------------*/

static void cps1_render_layer(int layer)
{
	switch (layer)
	{
	case 0:
		cps1_render_object();
		switch (cps1_high_layer)
		{
		case 2: cps1_render_scroll2_foreground(); break;

		case 1:
		case 3: blit_finish_scrollh(); break;
		}
		break;

	case 1: cps1_render_scroll1(); break;
	case 2: cps1_render_scroll2(); break;
	case 3: cps1_render_scroll3(); break;
	}
}

/*------------------------------------------------------
	��ʍX�V
------------------------------------------------------*/

void cps1_screenrefresh(void)
{
	int i, l0, l1, l2, l3;
	UINT16 video_ctrl = cps1_port(CPS1_VIDEO_CONTROL);
	UINT16 layer_ctrl = cps1_port(driver->layer_control);
	UINT16 mask = 0, prio_mask;

	cps_flip_screen = video_ctrl & 0x8000;

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

#if !RELEASE
	switch (cps1_kludge)
	{
	case CPS1_KLUDGE_SF2CEB:
		cps_scroll1x -= 0x0c;
		cps_scroll2x -= 0x0e;
		cps_scroll3x -= 0x10;
		break;

	case CPS1_KLUDGE_WOFB:
		cps_scroll1x += 0xffc0;
		cps_scroll2x += 0xffc0;
		cps_scroll3x += 0xffc0;
		break;
#if 0
	case CPS1_KLUDGE_CAWINGB:
		cps_scroll1x += 0xffc0;
		break;
#endif
	}
#endif

	l0 = (layer_ctrl >> 0x06) & 3;
	l1 = (layer_ctrl >> 0x08) & 3;
	l2 = (layer_ctrl >> 0x0a) & 3;
	l3 = (layer_ctrl >> 0x0c) & 3;

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

	cps1_check_scroll2_distort(video_ctrl & 1);

	blit_start(cps1_high_layer);

	if (cps1_has_stars) cps1_render_stars();
	cps1_render_layer(l0);
	cps1_render_layer(l1);
	cps1_render_layer(l2);
	cps1_render_layer(l3);
}


/*------------------------------------------------------
	object RAM�X�V
------------------------------------------------------*/

void cps1_objram_latch(void)
{
	UINT16 *base = cps1_base(CPS1_OBJ_BASE, cps1_obj_mask);
	UINT16 *end  = base + (cps1_obj_size >> 1);
	struct cps1_object_t *object = cps1_object;

#if !RELEASE
	if (cps1_kludge == CPS1_KLUDGE_SF2CEB
	||	cps1_kludge == CPS1_KLUDGE_KNIGHTSB)
	{
		UINT16 *end2;

		cps1_port(CPS1_OBJ_BASE) = 0x9100;
		base = cps1_base(CPS1_OBJ_BASE, cps1_obj_mask);
		end  = base + (cps1_obj_size >> 1);
		end2 = base;

		while (end2 < end)
		{
			if ((end2[3] & 0xff00) == 0xff00) break;
			end2 += 4;
		}
		end2 -= 4;

		while (end2 >= base)
		{
			object->sx   = end2[0];
			object->sy   = end2[1];
			object->code = end2[2];
			object->attr = end2[3];
			object++;

			end2 -= 4;
		}
	}
	else
#endif
	{
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
	}

	cps1_last_object = --object;

	cps1_build_palette();
}


/******************************************************************************
	�Z�[�u/���[�h �X�e�[�g
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
