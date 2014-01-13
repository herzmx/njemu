/******************************************************************************

	sprite.c

	CPS2 �X�v���C�g�}�l�[�W��

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	�萔/�}�N����
******************************************************************************/

#define TEXTURE_HEIGHT	512

#define MAKE_KEY(code, attr)	(code | ((attr & 0x0f) << 28))
#define PSP_UNCACHE_PTR(p)		(((UINT32)(p)) | 0x40000000)


/******************************************************************************
	�v���g�^�C�v
******************************************************************************/

void (*blit_finish_object)(int start_pri, int end_pri);
static void blit_render_object(int start_pri, int end_pri);
static void blit_render_object_zb(int start_pri, int end_pri);

void (*blit_draw_scroll2)(INT16 x, INT16 y, UINT32 code, UINT16 attr);
static void blit_draw_scroll2_software(INT16 x, INT16 y, UINT32 code, UINT16 attr);
static void blit_draw_scroll2_hardware(INT16 x, INT16 y, UINT32 code, UINT16 attr);


/******************************************************************************
	���[�J���ϐ�/�\����
******************************************************************************/

typedef struct sprite_t SPRITE;
typedef struct object_t OBJECT;

struct sprite_t
{
	UINT32 key;
	UINT32 used;
	INT32 index;
	SPRITE *next;
};

struct object_t
{
	UINT32 clut;
	struct Vertex vertices[2];
	OBJECT *next;
};

static RECT cps_src_clip = { 64, 16, 64 + 384, 16 + 224 };

static RECT cps_clip[5] =
{
	{ 48, 24, 48 + 384, 24 + 224 },	// option_stretch = 0  (384x224)
	{ 60,  1, 60 + 360,  1 + 270 },	// option_stretch = 1  (360x270  4:3)
	{ 48,  1, 48 + 384,  1 + 270 },	// option_stretch = 2  (384x270 24:17)
	{ 0,   1, 480,       1 + 270 },	// option_stretch = 3  (480x270 16:9)
	{ 138, 0, 138 + 204,     272 }	// option_stretch = 4  (204x272 3:4 vertical)
};

static INT16 clip_min_y;
static INT16 clip_max_y;

static INT16 object_min_y;
static INT16 scroll2_min_y;
static INT16 scroll2_max_y;
static INT16 scroll2_sy;
static INT16 scroll2_ey;

static UINT8 *pen_usage;
static UINT16 *scrbitmap;


/*------------------------------------------------------------------------
	OBJECT: �L�����N�^��
------------------------------------------------------------------------*/

#define OBJECT_HASH_SIZE		0x200
#define OBJECT_HASH_MASK		0x1ff
#define OBJECT_TEXTURE_SIZE		((BUF_WIDTH/16)*(TEXTURE_HEIGHT/16))
#define OBJECT_MAX_SPRITES		0x1400

static SPRITE ALIGN_DATA *object_head[OBJECT_HASH_SIZE];
static SPRITE ALIGN_DATA object_data[OBJECT_TEXTURE_SIZE];
static SPRITE ALIGN_DATA *object_free_head;

static UINT8 *tex_object;
static UINT16 object_texture_num;


/*------------------------------------------------------------------------
	SCROLL1: �X�N���[����1(�e�L�X�g��)
------------------------------------------------------------------------*/

#define SCROLL1_HASH_SIZE		0x200
#define SCROLL1_HASH_MASK		0x1ff
#define SCROLL1_TEXTURE_SIZE	((BUF_WIDTH/8)*(TEXTURE_HEIGHT/8))
#define SCROLL1_MAX_SPRITES		((384/8 + 2) * (224/8 + 2))

static SPRITE ALIGN_DATA *scroll1_head[SCROLL1_HASH_SIZE];
static SPRITE ALIGN_DATA scroll1_data[SCROLL1_TEXTURE_SIZE];
static SPRITE ALIGN_DATA *scroll1_free_head;

static UINT8 *tex_scroll1;
static UINT16 scroll1_texture_num;


/*------------------------------------------------------------------------
	SCROLL2: �X�N���[����2
------------------------------------------------------------------------*/

#define SCROLL2_HASH_SIZE		0x100
#define SCROLL2_HASH_MASK		0xff
#define SCROLL2_TEXTURE_SIZE	((BUF_WIDTH/16)*(TEXTURE_HEIGHT/16))
#define SCROLL2_MAX_SPRITES		((384/16 + 2) * (224/16 + 2))

static SPRITE ALIGN_DATA *scroll2_head[SCROLL2_HASH_SIZE];
static SPRITE ALIGN_DATA scroll2_data[SCROLL2_TEXTURE_SIZE];
static SPRITE ALIGN_DATA *scroll2_free_head;

static UINT8 *tex_scroll2;
static UINT16 scroll2_texture_num;


/*------------------------------------------------------------------------
	SCROLL3: �X�N���[����3
------------------------------------------------------------------------*/

#define SCROLL3_HASH_SIZE		0x40
#define SCROLL3_HASH_MASK		0x3f
#define SCROLL3_TEXTURE_SIZE	((BUF_WIDTH/32)*(TEXTURE_HEIGHT/32))
#define SCROLL3_MAX_SPRITES		((384/32 + 2) * (224/32 + 2))

static SPRITE ALIGN_DATA *scroll3_head[SCROLL3_HASH_SIZE];
static SPRITE ALIGN_DATA scroll3_data[SCROLL3_TEXTURE_SIZE];
static SPRITE ALIGN_DATA *scroll3_free_head;

static UINT8 *tex_scroll3;
static UINT16 scroll3_texture_num;


/*------------------------------------------------------------------------
	���_�f�[�^
------------------------------------------------------------------------*/

static OBJECT *vertices_object_head[8];
static OBJECT *vertices_object_tail[8];
static OBJECT ALIGN_DATA vertices_object[OBJECT_MAX_SPRITES];

static UINT16 object_num[8];
static UINT16 object_index;

static struct Vertex ALIGN_DATA vertices_scroll[2][SCROLL1_MAX_SPRITES * 2];


/*------------------------------------------------------------------------
	�J���[���b�N�A�b�v�e�[�u��
------------------------------------------------------------------------*/

static UINT16 *clut;
static UINT16 clut0_num;
static UINT16 clut1_num;

static const UINT32 ALIGN_DATA color_table[16] =
{
	0x00000000, 0x10101010, 0x20202020, 0x30303030,
	0x40404040, 0x50505050, 0x60606060, 0x70707070,
	0x80808080, 0x90909090, 0xa0a0a0a0, 0xb0b0b0b0,
	0xc0c0c0c0, 0xd0d0d0d0, 0xe0e0e0e0, 0xf0f0f0f0
};


/*------------------------------------------------------------------------
	'swizzle'�e�N�X�`���A�h���X�v�Z�e�[�u�� (8bit�J���[)
------------------------------------------------------------------------*/

static const int ALIGN_DATA swizzle_table_8bit[32] =
{
	   0, 16, 16, 16, 16, 16, 16, 16,
	3984, 16, 16, 16, 16, 16, 16, 16,
	3984, 16, 16, 16, 16, 16, 16, 16,
	3984, 16, 16, 16, 16, 16, 16, 16
};


/******************************************************************************
	SCROLL2 �\�t�g�E�F�A�`��
******************************************************************************/

static void drawgfx16_16x16(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);
static void drawgfx16_16x16_flipx(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);
static void drawgfx16_16x16_flipy(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);
static void drawgfx16_16x16_flipxy(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);

static void drawgfx16_16x16_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);
static void drawgfx16_16x16_flipx_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);
static void drawgfx16_16x16_flipy_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);
static void drawgfx16_16x16_flipxy_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines);

static void ALIGN_DATA (*drawgfx16[8])(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines) =
{
	drawgfx16_16x16,
	drawgfx16_16x16_opaque,
	drawgfx16_16x16_flipx,
	drawgfx16_16x16_flipx_opaque,
	drawgfx16_16x16_flipy,
	drawgfx16_16x16_flipy_opaque,
	drawgfx16_16x16_flipxy,
	drawgfx16_16x16_flipxy_opaque
};


/*------------------------------------------------------------------------
	16bpp 16x16
------------------------------------------------------------------------*/

static void drawgfx16_16x16(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile, mask;

	while (lines--)
	{
		tile = src[0];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[ 0] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[ 4] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[ 1] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[ 5] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[ 2] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[ 6] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[ 3] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[ 7] = pal[(tile >> 28) & 0x0f];
		}
		tile = src[1];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[ 8] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[12] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[ 9] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[13] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[10] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[14] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[11] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[15] = pal[(tile >> 28) & 0x0f];
		}
		src += 2;
		dst += BUF_WIDTH;
	}
}

static void drawgfx16_16x16_flipx(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile, mask;

	while (lines--)
	{
		tile = src[0];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[15] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[11] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[14] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[10] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[13] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[ 9] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[12] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[ 8] = pal[(tile >> 28) & 0x0f];
		}
		tile = src[1];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[ 7] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[ 3] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[ 6] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[ 2] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[ 5] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[ 1] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[ 4] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[ 0] = pal[(tile >> 28) & 0x0f];
		}
		src += 2;
		dst += BUF_WIDTH;
	}
}

static void drawgfx16_16x16_flipy(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile, mask;

	while (lines--)
	{
		tile = src[0];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[ 0] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[ 4] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[ 1] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[ 5] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[ 2] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[ 6] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[ 3] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[ 7] = pal[(tile >> 28) & 0x0f];
		}
		tile = src[1];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[ 8] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[12] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[ 9] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[13] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[10] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[14] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[11] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[15] = pal[(tile >> 28) & 0x0f];
		}
		src += 2;
		dst -= BUF_WIDTH;
	}
}

static void drawgfx16_16x16_flipxy(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile, mask;

	while (lines--)
	{
		tile = src[0];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[15] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[11] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[14] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[10] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[13] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[ 9] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[12] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[ 8] = pal[(tile >> 28) & 0x0f];
		}
		tile = src[1];
		mask = ~tile;
		if (mask)
		{
			if (mask & 0x000f) dst[ 7] = pal[(tile >>  0) & 0x0f];
			if (mask & 0x00f0) dst[ 3] = pal[(tile >>  4) & 0x0f];
			if (mask & 0x0f00) dst[ 6] = pal[(tile >>  8) & 0x0f];
			if (mask & 0xf000) dst[ 2] = pal[(tile >> 12) & 0x0f];
			mask >>= 16;
			if (mask & 0x000f) dst[ 5] = pal[(tile >> 16) & 0x0f];
			if (mask & 0x00f0) dst[ 1] = pal[(tile >> 20) & 0x0f];
			if (mask & 0x0f00) dst[ 4] = pal[(tile >> 24) & 0x0f];
			if (mask & 0xf000) dst[ 0] = pal[(tile >> 28) & 0x0f];
		}
		src += 2;
		dst -= BUF_WIDTH;
	}
}


/*------------------------------------------------------------------------
	16bpp 16x16 (���߂Ȃ�)
------------------------------------------------------------------------*/

static void drawgfx16_16x16_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile;

	while (lines--)
	{
		tile = src[0];
		dst[ 0] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 4] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 1] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 5] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 2] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 6] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 3] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 7] = pal[tile & 0x0f];
		tile = src[1];
		dst[ 8] = pal[tile & 0x0f]; tile >>= 4;
		dst[12] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 9] = pal[tile & 0x0f]; tile >>= 4;
		dst[13] = pal[tile & 0x0f]; tile >>= 4;
		dst[10] = pal[tile & 0x0f]; tile >>= 4;
		dst[14] = pal[tile & 0x0f]; tile >>= 4;
		dst[11] = pal[tile & 0x0f]; tile >>= 4;
		dst[15] = pal[tile & 0x0f];
		src += 2;
		dst += BUF_WIDTH;
	}
}

static void drawgfx16_16x16_flipx_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile;

	while (lines--)
	{
		tile = src[0];
		dst[15] = pal[tile & 0x0f]; tile >>= 4;
		dst[11] = pal[tile & 0x0f]; tile >>= 4;
		dst[14] = pal[tile & 0x0f]; tile >>= 4;
		dst[10] = pal[tile & 0x0f]; tile >>= 4;
		dst[13] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 9] = pal[tile & 0x0f]; tile >>= 4;
		dst[12] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 8] = pal[tile & 0x0f];
		tile = src[1];
		dst[ 7] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 3] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 6] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 2] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 5] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 1] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 4] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 0] = pal[tile & 0x0f];
		src += 2;
		dst += BUF_WIDTH;
	}
}

static void drawgfx16_16x16_flipy_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile;

	while (lines--)
	{
		tile = src[0];
		dst[ 0] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 4] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 1] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 5] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 2] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 6] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 3] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 7] = pal[tile & 0x0f];
		tile = src[1];
		dst[ 8] = pal[tile & 0x0f]; tile >>= 4;
		dst[12] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 9] = pal[tile & 0x0f]; tile >>= 4;
		dst[13] = pal[tile & 0x0f]; tile >>= 4;
		dst[10] = pal[tile & 0x0f]; tile >>= 4;
		dst[14] = pal[tile & 0x0f]; tile >>= 4;
		dst[11] = pal[tile & 0x0f]; tile >>= 4;
		dst[15] = pal[tile & 0x0f];
		src += 2;
		dst -= BUF_WIDTH;
	}
}

static void drawgfx16_16x16_flipxy_opaque(UINT32 *src, UINT16 *dst, UINT16 *pal, int lines)
{
	UINT32 tile;

	while (lines--)
	{
		tile = src[0];
		dst[15] = pal[tile & 0x0f]; tile >>= 4;
		dst[11] = pal[tile & 0x0f]; tile >>= 4;
		dst[14] = pal[tile & 0x0f]; tile >>= 4;
		dst[10] = pal[tile & 0x0f]; tile >>= 4;
		dst[13] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 9] = pal[tile & 0x0f]; tile >>= 4;
		dst[12] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 8] = pal[tile & 0x0f];
		tile = src[1];
		dst[ 7] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 3] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 6] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 2] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 5] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 1] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 4] = pal[tile & 0x0f]; tile >>= 4;
		dst[ 0] = pal[tile & 0x0f];
		src += 2;
		dst -= BUF_WIDTH;
	}
}


/******************************************************************************
	OBJECT�X�v���C�g�Ǘ�
******************************************************************************/

/*------------------------------------------------------------------------
	OBJECT�e�N�X�`������X�v���C�g�ԍ����擾
------------------------------------------------------------------------*/

static INT32 object_get_sprite(UINT32 key)
{
	SPRITE *p = object_head[key & OBJECT_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
#if USE_CACHE
				update_cache(key << 7);
#endif
				p->used = frames_displayed;
			}
			return p->index;
	 	}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	OBJECT�e�N�X�`���ɃX�v���C�g��o�^
------------------------------------------------------------------------*/

static INT32 object_insert_sprite(UINT32 key)
{
	UINT16 hash = key & OBJECT_HASH_MASK;
	SPRITE *p = object_head[hash];
	SPRITE *q = object_free_head;

	if (!q) return -1;

	object_free_head = object_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->used = frames_displayed;

	if (!p)
	{
		object_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	object_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	OBJECT�e�N�X�`�������莞�Ԃ��o�߂����X�v���C�g���폜
------------------------------------------------------------------------*/

static void object_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < OBJECT_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = object_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				object_texture_num--;

				if (!prev_p)
				{
					object_head[i] = p->next;
					p->next = object_free_head;
					object_free_head = p;
					p = object_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = object_free_head;
					object_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/******************************************************************************
	SCROLL1�X�v���C�g�Ǘ�
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLL1�e�N�X�`������X�v���C�g�ԍ����擾
------------------------------------------------------------------------*/

static INT32 scroll1_get_sprite(UINT32 key)
{
	SPRITE *p = scroll1_head[key & SCROLL1_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
#if USE_CACHE
				update_cache(key << 6);
#endif
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	SCROLL1�e�N�X�`���ɃX�v���C�g��o�^
------------------------------------------------------------------------*/

static INT32 scroll1_insert_sprite(UINT32 key)
{
	UINT16 hash = key & SCROLL1_HASH_MASK;
	SPRITE *p = scroll1_head[hash];
	SPRITE *q = scroll1_free_head;

	if (!q) return -1;

	scroll1_free_head = scroll1_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->used = frames_displayed;

	if (!p)
	{
		scroll1_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scroll1_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	SCROLL1�e�N�X�`�������莞�Ԃ��o�߂����X�v���C�g���폜
------------------------------------------------------------------------*/

static void scroll1_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLL1_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scroll1_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				scroll1_texture_num--;

				if (!prev_p)
				{
					scroll1_head[i] = p->next;
					p->next = scroll1_free_head;
					scroll1_free_head = p;
					p = scroll1_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scroll1_free_head;
					scroll1_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/******************************************************************************
	SCROLL2�X�v���C�g�Ǘ�
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLL2�e�N�X�`������X�v���C�g�ԍ����擾
------------------------------------------------------------------------*/

static INT32 scroll2_get_sprite(UINT32 key)
{
	SPRITE *p = scroll2_head[key & SCROLL2_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
#if USE_CACHE
				update_cache(key << 7);
#endif
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	SCROLL2�e�N�X�`���ɃX�v���C�g��o�^
------------------------------------------------------------------------*/

static INT32 scroll2_insert_sprite(UINT32 key)
{
	UINT16 hash = key & SCROLL2_HASH_MASK;
	SPRITE *p = scroll2_head[hash];
	SPRITE *q = scroll2_free_head;

	if (!q) return -1;

	scroll2_free_head = scroll2_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->used = frames_displayed;

	if (!p)
	{
		scroll2_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scroll2_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	SCROLL2�e�N�X�`�������莞�Ԃ��o�߂����X�v���C�g���폜
------------------------------------------------------------------------*/

static void scroll2_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLL2_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scroll2_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				scroll2_texture_num--;

				if (!prev_p)
				{
					scroll2_head[i] = p->next;
					p->next = scroll2_free_head;
					scroll2_free_head = p;
					p = scroll2_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scroll2_free_head;
					scroll2_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/******************************************************************************
	SCROLL3�X�v���C�g�Ǘ�
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLL3�e�N�X�`������X�v���C�g�ԍ����擾
------------------------------------------------------------------------*/

static INT32 scroll3_get_sprite(UINT32 key)
{
	SPRITE *p = scroll3_head[key & SCROLL3_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
#if USE_CACHE
				update_cache(key << 9);
#endif
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	SCROLL3�e�N�X�`���ɃX�v���C�g��o�^
------------------------------------------------------------------------*/

static INT32 scroll3_insert_sprite(UINT32 key)
{
	UINT16 hash = key & SCROLL3_HASH_MASK;
	SPRITE *p = scroll3_head[hash];
	SPRITE *q = scroll3_free_head;

	if (!q) return -1;

	scroll3_free_head = scroll3_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->used = frames_displayed;

	if (!p)
	{
		scroll3_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scroll3_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	SCROLL3�e�N�X�`�������莞�Ԃ��o�߂����X�v���C�g���폜
------------------------------------------------------------------------*/

static void scroll3_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLL3_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scroll3_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				scroll3_texture_num--;

				if (!prev_p)
				{
					scroll3_head[i] = p->next;
					p->next = scroll3_free_head;
					scroll3_free_head = p;
					p = scroll3_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scroll3_free_head;
					scroll3_free_head = p;
					p = prev_p->next;
				}
			}
			else
			{
				prev_p = p;
				p = p->next;
			}
		}
	}
}


/******************************************************************************
	�X�v���C�g�`��C���^�t�F�[�X�֐�
******************************************************************************/

/*------------------------------------------------------------------------
	�S�ẴX�v���C�g�𑦍��ɃN���A����
------------------------------------------------------------------------*/

void blit_clear_all_sprite(void)
{
	int i;

	for (i = 0; i < OBJECT_TEXTURE_SIZE - 1; i++)
		object_data[i].next = &object_data[i + 1];

	object_data[i].next = NULL;
	object_free_head = &object_data[0];

	for (i = 0; i < SCROLL1_TEXTURE_SIZE - 1; i++)
		scroll1_data[i].next = &scroll1_data[i + 1];

	scroll1_data[i].next = NULL;
	scroll1_free_head = &scroll1_data[0];

	for (i = 0; i < SCROLL2_TEXTURE_SIZE - 1; i++)
		scroll2_data[i].next = &scroll2_data[i + 1];

	scroll2_data[i].next = NULL;
	scroll2_free_head = &scroll2_data[0];

	for (i = 0; i < SCROLL3_TEXTURE_SIZE - 1; i++)
		scroll3_data[i].next = &scroll3_data[i + 1];

	scroll3_data[i].next = NULL;
	scroll3_free_head = &scroll3_data[0];

	memset(object_head, 0, sizeof(SPRITE *) * OBJECT_HASH_SIZE);
	memset(scroll1_head, 0, sizeof(SPRITE *) * SCROLL1_HASH_SIZE);
	memset(scroll2_head, 0, sizeof(SPRITE *) * SCROLL2_HASH_SIZE);
	memset(scroll3_head, 0, sizeof(SPRITE *) * SCROLL3_HASH_SIZE);

	object_texture_num = 0;
	scroll1_texture_num = 0;
	scroll2_texture_num = 0;
	scroll3_texture_num = 0;
}


/*------------------------------------------------------------------------
	�X�v���C�g�����̃��Z�b�g
------------------------------------------------------------------------*/

void blit_reset(void)
{
	int i;

	scrbitmap   = (UINT16 *)video_frame_addr(work_frame, 0, 0);
	tex_object  = (UINT8 *)(scrbitmap + BUF_WIDTH * SCR_HEIGHT);
	tex_scroll1 = tex_object  + BUF_WIDTH * TEXTURE_HEIGHT;
	tex_scroll2 = tex_scroll1 + BUF_WIDTH * TEXTURE_HEIGHT;
	tex_scroll3 = tex_scroll2 + BUF_WIDTH * TEXTURE_HEIGHT;

	for (i = 0; i < OBJECT_TEXTURE_SIZE; i++) object_data[i].index = i;
	for (i = 0; i < SCROLL1_TEXTURE_SIZE; i++) scroll1_data[i].index = i;
	for (i = 0; i < SCROLL2_TEXTURE_SIZE; i++) scroll2_data[i].index = i;
	for (i = 0; i < SCROLL3_TEXTURE_SIZE; i++) scroll3_data[i].index = i;

	clip_min_y = FIRST_VISIBLE_LINE;
	clip_max_y = LAST_VISIBLE_LINE;

	pen_usage = gfx_pen_usage[TILE16];
	clut = (UINT16 *)PSP_UNCACHE_PTR(&video_palette);

	blit_finish_object = blit_render_object;

	blit_clear_all_sprite();
}


/*------------------------------------------------------------------------
	��ʂ̍X�V�J�n
------------------------------------------------------------------------*/

void blit_start(int start, int end)
{
	int i;

	clip_min_y = start;
	clip_max_y = end + 1;

	object_min_y = start - 16;

	clut0_num = 0;
	clut1_num = 0;

	object_index = 0;
	for (i = 0; i < 8; i++)
	{
		object_num[i] = 0;
		vertices_object_head[i] = NULL;
	}

	if (start == FIRST_VISIBLE_LINE)
	{
		if (cps2_has_mask)
			blit_finish_object = blit_render_object_zb;
		else
			blit_finish_object = blit_render_object;

		sceGuStart(GU_DIRECT, gulist);
		sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
		sceGuDepthBuffer(draw_frame, BUF_WIDTH);
		sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
		sceGuEnable(GU_ALPHA_TEST);
		sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT | GU_FAST_CLEAR_BIT);
		sceGuTexMode(GU_PSM_T8, 0, 0, GU_TRUE);
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
		sceGuFinish();
		sceGuSync(0, GU_SYNC_FINISH);
	}
}


/*------------------------------------------------------------------------
	��ʂ̍X�V�I��
------------------------------------------------------------------------*/

void blit_finish(void)
{
	if (cps2_has_mask) video_clear_frame(draw_frame);

	if (cps_rotate_screen)
	{
		if (cps_flip_screen)
		{
			video_copy_rect_flip(work_frame, draw_frame, &cps_src_clip, &cps_src_clip);
			video_copy_rect(draw_frame, work_frame, &cps_src_clip, &cps_src_clip);
			video_clear_frame(draw_frame);
		}
		video_copy_rect_rotate(work_frame, draw_frame, &cps_src_clip, &cps_clip[4]);
	}
	else
	{
		if (cps_flip_screen)
			video_copy_rect_flip(work_frame, draw_frame, &cps_src_clip, &cps_clip[option_stretch]);
		else
			video_copy_rect(work_frame, draw_frame, &cps_src_clip, &cps_clip[option_stretch]);
	}
}


/*------------------------------------------------------------------------
	OBJECT�e�N�X�`���ƃL���b�V�����X�V
------------------------------------------------------------------------*/

void blit_update_object(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	if ((x > 48 && x < 448) && (y > object_min_y && y < clip_max_y))
	{
		UINT32 key = MAKE_KEY(code, attr);
		SPRITE *p = object_head[key & OBJECT_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				if (p->used != frames_displayed)
				{
#if USE_CACHE
					update_cache(key << 7);
#endif
					p->used = frames_displayed;
				}
				return;
		 	}
			p = p->next;
		}
	}
}


/*------------------------------------------------------------------------
	OBJECT��`�惊�X�g�ɓo�^
------------------------------------------------------------------------*/

void blit_draw_object(INT16 x, INT16 y, UINT16 z, INT16 pri, UINT32 code, UINT16 attr)
{
	if ((x > 48 && x < 448) && (y > object_min_y && y < clip_max_y))
	{
		INT16 idx;
		OBJECT *object;
		struct Vertex *vertices;
		UINT32 key = MAKE_KEY(code, attr);

		if ((idx = object_get_sprite(key)) < 0)
		{
			UINT32 col, tile;
			UINT8 *src, *dst, lines = 16;

			if (object_texture_num == OBJECT_TEXTURE_SIZE - 1)
			{
				cps2_scan_object_callback();
				object_delete_sprite();
			}

			idx = object_insert_sprite(key);
			dst = SWIZZLED8_16x16(tex_object, idx);
#if USE_CACHE
			src = &memory_region_gfx1[(*read_cache)(code << 7)];
#else
			src = &memory_region_gfx1[code << 7];
#endif
			col = color_table[attr & 0x0f];

			while (lines--)
			{
				tile = *(UINT32 *)(src + 0);
				*(UINT32 *)(dst +  0) = ((tile >> 0) & 0x0f0f0f0f) | col;
				*(UINT32 *)(dst +  4) = ((tile >> 4) & 0x0f0f0f0f) | col;
				tile = *(UINT32 *)(src + 4);
				*(UINT32 *)(dst +  8) = ((tile >> 0) & 0x0f0f0f0f) | col;
				*(UINT32 *)(dst + 12) = ((tile >> 4) & 0x0f0f0f0f) | col;
				src += 8;
				dst += swizzle_table_8bit[lines];
			}
		}

		object = &vertices_object[object_index++];
		object->clut = attr & 0x10;
		object->next = NULL;

		if (!vertices_object_head[pri])
			vertices_object_head[pri] = object;
		else
			vertices_object_tail[pri]->next = object;

		vertices_object_tail[pri] = object;

		vertices = object->vertices;

		vertices[0].x = vertices[1].x = x;
		vertices[0].y = vertices[1].y = y;
		vertices[0].z = vertices[1].z = z;
		vertices[0].u = vertices[1].u = (idx & 0x001f) << 4;
		vertices[0].v = vertices[1].v = (idx & 0x03e0) >> 1;

		attr ^= 0x60;
		vertices[(attr & 0x20) >> 5].u += 16;
		vertices[(attr & 0x40) >> 6].v += 16;

		vertices[1].x += 16;
		vertices[1].y += 16;

		object_num[pri] += 2;
	}
}


/*------------------------------------------------------------------------
	OBJECT�`��
------------------------------------------------------------------------*/

static void blit_render_object(int start_pri, int end_pri)
{
	int i, size = 0, total_sprites = 0;
	UINT8 color = 0;
	struct Vertex *vertices, *vertices_tmp;
	OBJECT *object;

	for (i = start_pri; i <= end_pri; i++)
		size += object_num[i];

	if (!size) return;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, clip_min_y, 448, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_object);
	sceGuClutLoad(256/8, clut);

	vertices_tmp = vertices = (struct Vertex *)sceGuGetMemory(size * sizeof(struct Vertex));

	for (i = start_pri; i <= end_pri; i++)
	{
		object = vertices_object_head[i];

		while (object)
		{
			if (color != object->clut)
			{
				color = object->clut;

				if (total_sprites)
				{
					sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, NULL, vertices);
					total_sprites = 0;
					vertices = vertices_tmp;
				}

				sceGuClutLoad(256/8, &clut[color << 4]);
			}

			vertices_tmp[0] = object->vertices[0];
			vertices_tmp[1] = object->vertices[1];

			total_sprites += 2;
			vertices_tmp += 2;
			object = object->next;
		}
	}

	if (total_sprites)
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, NULL, vertices);

	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}


/*------------------------------------------------------------------------
	OBJECT�`��(Z�o�b�t�@/�v���C�I���e�B0)
------------------------------------------------------------------------*/

static void blit_render_object_zb0(void)
{
	int size = object_num[0], total_sprites = 0;
	struct Vertex *vertices, *vertices_tmp;
	OBJECT *object;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, clip_min_y, 448, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_object);
	sceGuClutLoad(256/8, clut);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);

	vertices_tmp = vertices = (struct Vertex *)sceGuGetMemory(size * sizeof(struct Vertex));

	object = vertices_object_head[0];

	while (object)
	{
		vertices_tmp[0] = object->vertices[0];
		vertices_tmp[1] = object->vertices[1];

		total_sprites += 2;
		vertices_tmp += 2;
		object = object->next;
	}

	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, NULL, vertices);

	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_TRUE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_FAST_CLEAR_BIT);
	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}


/*------------------------------------------------------------------------
	OBJECT�`��(Z�o�b�t�@)
------------------------------------------------------------------------*/

static void blit_render_object_zb(int start_pri, int end_pri)
{
	int i, size = 0, total_sprites = 0;
	UINT8 color = 0;
	struct Vertex *vertices, *vertices_tmp;
	OBJECT *object;

	if (start_pri == 0 && object_num[0] != 0)
	{
		blit_render_object_zb0();
		start_pri = 1;
	}

	for (i = start_pri; i <= end_pri; i++)
		size += object_num[i];

	if (!size) return;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, clip_min_y, 448, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_object);
	sceGuClutLoad(256/8, clut);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);

	vertices_tmp = vertices = (struct Vertex *)sceGuGetMemory(size * sizeof(struct Vertex));

	for (i = start_pri; i <= end_pri; i++)
	{
		object = vertices_object_head[i];

		while (object)
		{
			if (color != object->clut)
			{
				color = object->clut;

				if (total_sprites)
				{
					sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, NULL, vertices);
					total_sprites = 0;
					vertices = vertices_tmp;
				}

				sceGuClutLoad(256/8, &clut[color << 4]);
			}

			vertices_tmp[0] = object->vertices[0];
			vertices_tmp[1] = object->vertices[1];

			total_sprites += 2;
			vertices_tmp += 2;
			object = object->next;
		}
	}

	if (total_sprites)
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, NULL, vertices);

	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_TRUE);
	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}


/*------------------------------------------------------------------------
	SCROLL1�e�N�X�`�����X�V
------------------------------------------------------------------------*/

void blit_update_scroll1(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	UINT32 key = MAKE_KEY(code, attr);
	SPRITE *p = scroll1_head[key & SCROLL1_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
#if USE_CACHE
				update_cache(key << 6);
#endif
				p->used = frames_displayed;
			}
			return;
		}
		p = p->next;
	}
}


/*------------------------------------------------------------------------
	SCROLL1��`�惊�X�g�ɓo�^
------------------------------------------------------------------------*/

void blit_draw_scroll1(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	INT16 idx;
	struct Vertex *vertices;
	UINT32 key = MAKE_KEY(code, attr);

	if ((idx = scroll1_get_sprite(key)) < 0)
	{
		UINT32 col, tile;
		UINT8 *src, *dst, lines = 8;

		if (scroll1_texture_num == SCROLL1_TEXTURE_SIZE - 1)
		{
			cps2_scan_scroll1_callback();
			scroll1_delete_sprite();
		}

		idx = scroll1_insert_sprite(key);
		dst = SWIZZLED8_8x8(tex_scroll1, idx);
#if USE_CACHE
		src = &memory_region_gfx1[(*read_cache)(code << 6)];
#else
		src = &memory_region_gfx1[code << 6];
#endif
		col = color_table[attr & 0x0f];

		while (lines--)
		{
			tile = *(UINT32 *)(src + 4);
			*(UINT32 *)(dst +  0) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst +  4) = ((tile >> 4) & 0x0f0f0f0f) | col;
			src += 8;
			dst += 16;
		}
	}

	if (attr & 0x10)
	{
		vertices = &vertices_scroll[1][clut1_num];
		clut1_num += 2;
	}
	else
	{
		vertices = &vertices_scroll[0][clut0_num];
		clut0_num += 2;
	}

	vertices[0].x = vertices[1].x = x;
	vertices[0].y = vertices[1].y = y;
	vertices[0].u = vertices[1].u = (idx & 0x003f) << 3;
	vertices[0].v = vertices[1].v = (idx & 0x0fc0) >> 3;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 8;
	vertices[(attr & 0x40) >> 6].v += 8;

	vertices[1].x += 8;
	vertices[1].y += 8;
}


/*------------------------------------------------------------------------
	SCROLL1�`��I��
------------------------------------------------------------------------*/

void blit_finish_scroll1(void)
{
	struct Vertex *vertices;

	if (clut0_num + clut1_num == 0) return;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, clip_min_y, 448, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scroll1);

	vertices = (struct Vertex *)sceGuGetMemory((clut0_num + clut1_num) * sizeof(struct Vertex));

	if (clut0_num)
	{
		sceGuClutLoad(256/8, &clut[32 << 4]);

		memcpy(vertices, vertices_scroll[0], clut0_num * sizeof(struct Vertex));
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, clut0_num, NULL, vertices);
		vertices += clut0_num;

		clut0_num = 0;
	}
	if (clut1_num)
	{
		sceGuClutLoad(256/8, &clut[48 << 4]);

		memcpy(vertices, vertices_scroll[1], clut1_num * sizeof(struct Vertex));
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, clut1_num, NULL, vertices);

		clut1_num = 0;
	}

	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}


/*------------------------------------------------------------------------
	SCROLL2�N���b�v�͈͂�ݒ�
------------------------------------------------------------------------*/

void blit_set_clip_scroll2(INT16 min_y, INT16 max_y)
{
	scroll2_min_y = min_y;
	scroll2_max_y = max_y + 1;

	if (scroll2_max_y - scroll2_min_y >= 16)
		blit_draw_scroll2 = blit_draw_scroll2_hardware;
	else
		blit_draw_scroll2 = blit_draw_scroll2_software;
}


/*------------------------------------------------------------------------
	SCROLL2�̕`��͈̓`�F�b�N
------------------------------------------------------------------------*/

int blit_check_clip_scroll2(INT16 sy)
{
	scroll2_sy = sy;
	scroll2_ey = sy + 16;

	if (scroll2_min_y > scroll2_sy) scroll2_sy = scroll2_min_y;
	if (scroll2_max_y < scroll2_ey) scroll2_ey = scroll2_max_y;

	return (scroll2_sy < scroll2_ey);
}


/*------------------------------------------------------------------------
	SCROLL2�e�N�X�`�����X�V
------------------------------------------------------------------------*/

void blit_update_scroll2(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	if (y > clip_min_y - 16 && y < clip_max_y)
	{
		UINT32 key = MAKE_KEY(code, attr);
		SPRITE *p = scroll2_head[key & SCROLL2_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				if (p->used != frames_displayed)
				{
#if USE_CACHE
					update_cache(key << 7);
#endif
					p->used = frames_displayed;
				}
				return;
			}
			p = p->next;
		}
	}
}


/*------------------------------------------------------------------------
	SCROLL2(���C���X�N���[��)�𒼐�VRAM�ɕ`��
------------------------------------------------------------------------*/

static void blit_draw_scroll2_software(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	UINT32 src, dst;
	UINT8 func;

#if USE_CACHE
	src = (*read_cache)(code << 7);
#else
	src = code << 7;
#endif

	if (attr & 0x40)
	{
		src += ((y + 16) - scroll2_ey) << 3;
		dst = ((scroll2_ey - 1) << 9) + x;
	}
	else
	{
		src += (scroll2_sy - y) << 3;
		dst = (scroll2_sy << 9) + x;
	}

	func = (pen_usage[code] & 1) | ((attr & 0x60) >> 4);

	(*drawgfx16[func])((UINT32 *)&memory_region_gfx1[src],
					&scrbitmap[dst],
					&video_palette[((attr & 0x1f) + 64) << 4],
					scroll2_ey - scroll2_sy);
}


/*------------------------------------------------------------------------
	SCROLL2��`�惊�X�g�ɓo�^
------------------------------------------------------------------------*/

static void blit_draw_scroll2_hardware(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	INT16 idx;
	struct Vertex *vertices;
	UINT32 key = MAKE_KEY(code, attr);

	if ((idx = scroll2_get_sprite(key)) < 0)
	{
		UINT32 col, tile;
		UINT8 *src, *dst, lines = 16;

		if (scroll2_texture_num == SCROLL2_TEXTURE_SIZE - 1)
		{
			cps2_scan_scroll2_callback();
			scroll2_delete_sprite();
		}

		idx = scroll2_insert_sprite(key);
		dst = SWIZZLED8_16x16(tex_scroll2, idx);
#if USE_CACHE
		src = &memory_region_gfx1[(*read_cache)(code << 7)];
#else
		src = &memory_region_gfx1[code << 7];
#endif
		col = color_table[attr & 0x0f];

		while (lines--)
		{
			tile = *(UINT32 *)(src + 0);
			*(UINT32 *)(dst +  0) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst +  4) = ((tile >> 4) & 0x0f0f0f0f) | col;
			tile = *(UINT32 *)(src + 4);
			*(UINT32 *)(dst +  8) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst + 12) = ((tile >> 4) & 0x0f0f0f0f) | col;
			src += 8;
			dst += swizzle_table_8bit[lines];
		}
	}

	if (attr & 0x10)
	{
		vertices = &vertices_scroll[1][clut1_num];
		clut1_num += 2;
	}
	else
	{
		vertices = &vertices_scroll[0][clut0_num];
		clut0_num += 2;
	}

	vertices[0].x = vertices[1].x = x;
	vertices[0].y = vertices[1].y = y;
	vertices[0].u = vertices[1].u = (idx & 0x001f) << 4;
	vertices[0].v = vertices[1].v = (idx & 0x03e0) >> 1;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 16;
	vertices[(attr & 0x40) >> 6].v += 16;

	vertices[1].x += 16;
	vertices[1].y += 16;
}


/*------------------------------------------------------------------------
	SCROLL2�`��I��
------------------------------------------------------------------------*/

void blit_finish_scroll2(void)
{
	struct Vertex *vertices;

	if (clut0_num + clut1_num == 0) return;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, scroll2_min_y, 448, scroll2_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scroll2);

	vertices = (struct Vertex *)sceGuGetMemory((clut0_num + clut1_num) * sizeof(struct Vertex));

	if (clut0_num)
	{
		sceGuClutLoad(256/8, &clut[64 << 4]);

		memcpy(vertices, vertices_scroll[0], clut0_num * sizeof(struct Vertex));
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, clut0_num, NULL, vertices);
		vertices += clut0_num;

		clut0_num = 0;
	}
	if (clut1_num)
	{
		sceGuClutLoad(256/8, &clut[80 << 4]);

		memcpy(vertices, vertices_scroll[1], clut1_num * sizeof(struct Vertex));
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, clut1_num, NULL, vertices);

		clut1_num = 0;
	}

	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}


/*------------------------------------------------------------------------
	SCROLL3�e�N�X�`�����X�V
------------------------------------------------------------------------*/

void blit_update_scroll3(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	UINT32 key = MAKE_KEY(code, attr);
	SPRITE *p = scroll3_head[key & SCROLL3_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
#if USE_CACHE
				update_cache(key << 9);
#endif
				p->used = frames_displayed;
			}
			return;
		}
		p = p->next;
	}
}


/*------------------------------------------------------------------------
	SCROLL3��`�惊�X�g�ɓo�^
------------------------------------------------------------------------*/

void blit_draw_scroll3(INT16 x, INT16 y, UINT32 code, UINT16 attr)
{
	INT16 idx;
	struct Vertex *vertices;
	UINT32 key = MAKE_KEY(code, attr);

	if ((idx = scroll3_get_sprite(key)) < 0)
	{
		UINT32 col, tile;
		UINT8 *src, *dst, lines = 32;

		if (scroll3_texture_num == SCROLL3_TEXTURE_SIZE - 1)
		{
			cps2_scan_scroll3_callback();
			scroll3_delete_sprite();
		}

		idx = scroll3_insert_sprite(key);
		dst = SWIZZLED8_32x32(tex_scroll3, idx);
#if USE_CACHE
		src = &memory_region_gfx1[(*read_cache)(code << 9)];
#else
		src = &memory_region_gfx1[code << 9];
#endif
		col = color_table[attr & 0x0f];

		while (lines--)
		{
			tile = *(UINT32 *)(src + 0);
			*(UINT32 *)(dst +  0) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst +  4) = ((tile >> 4) & 0x0f0f0f0f) | col;
			tile = *(UINT32 *)(src + 4);
			*(UINT32 *)(dst +  8) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst + 12) = ((tile >> 4) & 0x0f0f0f0f) | col;
			tile = *(UINT32 *)(src + 8);
			*(UINT32 *)(dst + 128) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst + 132) = ((tile >> 4) & 0x0f0f0f0f) | col;
			tile = *(UINT32 *)(src + 12);
			*(UINT32 *)(dst + 136) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst + 140) = ((tile >> 4) & 0x0f0f0f0f) | col;
			src += 16;
			dst += swizzle_table_8bit[lines];
		}
	}

	if (attr & 0x10)
	{
		vertices = &vertices_scroll[1][clut1_num];
		clut1_num += 2;
	}
	else
	{
		vertices = &vertices_scroll[0][clut0_num];
		clut0_num += 2;
	}

	vertices[0].x = vertices[1].x = x;
	vertices[0].y = vertices[1].y = y;
	vertices[0].u = vertices[1].u = (idx & 0x000f) << 5;
	vertices[0].v = vertices[1].v = (idx & 0x00f0) << 1;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 32;
	vertices[(attr & 0x40) >> 6].v += 32;

	vertices[1].x += 32;
	vertices[1].y += 32;
}


/*------------------------------------------------------------------------
	SCROLL3�`��I��
------------------------------------------------------------------------*/

void blit_finish_scroll3(void)
{
	struct Vertex *vertices;

	if (clut0_num + clut1_num == 0) return;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, clip_min_y, 448, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scroll3);

	vertices = (struct Vertex *)sceGuGetMemory((clut0_num + clut1_num) * sizeof(struct Vertex));

	if (clut0_num)
	{
		sceGuClutLoad(256/8, &clut[96 << 4]);

		memcpy(vertices, vertices_scroll[0], clut0_num * sizeof(struct Vertex));
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, clut0_num, NULL, vertices);
		vertices += clut0_num;

		clut0_num = 0;
	}
	if (clut1_num)
	{
		sceGuClutLoad(256/8, &clut[112 << 4]);

		memcpy(vertices, vertices_scroll[1], clut1_num * sizeof(struct Vertex));
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, clut1_num, NULL, vertices);

		clut1_num = 0;
	}

	sceGuFinish();
	sceGuSync(0, GU_SYNC_FINISH);
}
