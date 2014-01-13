/******************************************************************************

	sprite.c

	MVS �X�v���C�g�}�l�[�W��

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	�萔/�}�N����
******************************************************************************/

#define TEXTURE_HEIGHT	512

#define MAKE_FIX_KEY(code, attr)	(code | (attr << 28))
#define MAKE_SPR_KEY(code, attr)	(code | ((attr & 0x0f00) << 20))
#define PSP_UNCACHE_PTR(p)			(((UINT32)(p)) | 0x40000000)


/******************************************************************************
	���[�J���ϐ�/�\����
******************************************************************************/

typedef struct sprite_t SPRITE;

struct sprite_t
{
	UINT32 key;
	UINT32 used;
	INT32 index;
	SPRITE *next;
};

static RECT mvs_src_clip = { 8, 16, 8 + 304, 16 + 224 };

static RECT mvs_clip[4] =
{
	{ 88, 24, 88 + 304, 24 + 224 },	// option_stretch = 0  (304x224  4:3)
	{ 60,  1, 60 + 360,  1 + 270 },	// option_stretch = 1  (360x270  4:3)
	{ 30,  1, 30 + 420,  1 + 270 },	// option_stretch = 2  (420x270 14:9)
	{  0,  1,  0 + 480,  1 + 270 }	// option_stretch = 3  (480x270 16:9)
};

static int clip_min_y;
static int clip_max_y;
static int clear_spr_texture;
static int clear_fix_texture;

static UINT16 *scrbitmap;


/*------------------------------------------------------------------------
	FIX: �T�C�Y/�ʒu8pixel�Œ�̃X�v���C�g
------------------------------------------------------------------------*/

#define FIX_TEXTURE_SIZE	((BUF_WIDTH/8)*(TEXTURE_HEIGHT/8))
#define FIX_HASH_MASK		0x1ff
#define FIX_HASH_SIZE		0x200
#define FIX_MAX_SPRITES		((320/8) * (240/8))

static SPRITE ALIGN_DATA *fix_head[FIX_HASH_SIZE];
static SPRITE ALIGN_DATA fix_data[FIX_TEXTURE_SIZE];
static SPRITE *fix_free_head;

static UINT8 *tex_fix;
static struct Vertex ALIGN_DATA vertices_fix[FIX_MAX_SPRITES * 2];
static UINT16 fix_num;
static UINT16 fix_texture_num;


/*------------------------------------------------------------------------
	SPR: �σT�C�Y�̃X�v���C�g
------------------------------------------------------------------------*/

#define SPR_TEXTURE_SIZE	((BUF_WIDTH/16)*((TEXTURE_HEIGHT*3)/16))
#define SPR_HASH_MASK		0x1ff
#define SPR_HASH_SIZE		0x200
#define SPR_MAX_SPRITES		0x3000

static SPRITE ALIGN_DATA *spr_head[SPR_HASH_SIZE];
static SPRITE ALIGN_DATA spr_data[SPR_TEXTURE_SIZE];
static SPRITE *spr_free_head;

static UINT8 *tex_spr[3];
static struct Vertex ALIGN_DATA vertices_spr[SPR_MAX_SPRITES * 2];
static UINT16 ALIGN_DATA spr_flags[SPR_MAX_SPRITES];
static UINT16 spr_num;
static UINT16 spr_texture_num;
static UINT16 spr_index;
static UINT8 spr_disable;


/*------------------------------------------------------------------------
	�J���[���b�N�A�b�v�e�[�u��
------------------------------------------------------------------------*/

static UINT16 *clut;

static const UINT32 ALIGN_DATA color_table[16] =
{
	0x00000000, 0x10101010, 0x20202020, 0x30303030,
	0x40404040, 0x50505050, 0x60606060, 0x70707070,
	0x80808080, 0x90909090, 0xa0a0a0a0, 0xb0b0b0b0,
	0xc0c0c0c0, 0xd0d0d0d0, 0xe0e0e0e0, 0xf0f0f0f0
};


/*------------------------------------------------------------------------
	'swizzle'�e�N�X�`���A�h���X�v�Z�e�[�u��
------------------------------------------------------------------------*/

static const int ALIGN_DATA swizzle_table_8bit[16] =
{
	   0, 16, 16, 16, 16, 16, 16, 16,
	3984, 16, 16, 16, 16, 16, 16, 16
};


/******************************************************************************
	FIX�X�v���C�g�Ǘ�
******************************************************************************/

/*------------------------------------------------------------------------
	FIX�e�N�X�`������X�v���C�g�ԍ����擾
------------------------------------------------------------------------*/

static int fix_get_sprite(UINT32 key)
{
	SPRITE *p = fix_head[key & FIX_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			p->used = frames_displayed;
			return p->index;
	 	}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	FIX�e�N�X�`���ɃX�v���C�g��o�^
------------------------------------------------------------------------*/

static int fix_insert_sprite(UINT32 key)
{
	UINT16 hash = key & FIX_HASH_MASK;
	SPRITE *p = fix_head[hash];
	SPRITE *q = fix_free_head;

	if (!q) return -1;

	fix_free_head = fix_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->used = frames_displayed;

	if (!p)
	{
		fix_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	fix_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	FIX�e�N�X�`�������莞�Ԃ��o�߂����X�v���C�g���폜
------------------------------------------------------------------------*/

static void fix_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < FIX_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = fix_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				fix_texture_num--;

				if (!prev_p)
				{
					fix_head[i] = p->next;
					p->next = fix_free_head;
					fix_free_head = p;
					p = fix_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = fix_free_head;
					fix_free_head = p;
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
	SPR�X�v���C�g�Ǘ�
******************************************************************************/

/*------------------------------------------------------------------------
	SPR�e�N�X�`������X�v���C�g�ԍ����擾
------------------------------------------------------------------------*/

static int spr_get_sprite(UINT32 key)
{
	SPRITE *p = spr_head[key & SPR_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
				update_cache(key << 7);
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	SPR�e�N�X�`���ɃX�v���C�g��o�^
------------------------------------------------------------------------*/

static int spr_insert_sprite(UINT32 key)
{
	UINT16 hash = key & SPR_HASH_MASK;
	SPRITE *p = spr_head[hash];
	SPRITE *q = spr_free_head;

	if (!q) return -1;

	spr_free_head = spr_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->used = frames_displayed;

	if (!p)
	{
		spr_head[key & SPR_HASH_MASK] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	spr_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	SPR�e�N�X�`�������莞�Ԃ��o�߂����X�v���C�g���폜
------------------------------------------------------------------------*/

static void spr_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SPR_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = spr_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				spr_texture_num--;

				if (!prev_p)
				{
					spr_head[i] = p->next;
					p->next = spr_free_head;
					spr_free_head = p;
					p = spr_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = spr_free_head;
					spr_free_head = p;
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
	SPR�X�v���C�g�𑦍��ɃN���A����
------------------------------------------------------------------------*/

static void blit_clear_spr_sprite(void)
{
	int i;

	for (i = 0; i < SPR_TEXTURE_SIZE - 1; i++)
		spr_data[i].next = &spr_data[i + 1];

	spr_data[i].next = NULL;
	spr_free_head = &spr_data[0];

	memset(spr_head, 0, sizeof(SPRITE *) * SPR_HASH_SIZE);

	spr_texture_num = 0;
	clear_spr_texture = 0;
}


/*------------------------------------------------------------------------
	FIX�X�v���C�g�𑦍��ɃN���A����
------------------------------------------------------------------------*/

static void blit_clear_fix_sprite(void)
{
	int i;

	for (i = 0; i < FIX_TEXTURE_SIZE - 1; i++)
		fix_data[i].next = &fix_data[i + 1];

	fix_data[i].next = NULL;
	fix_free_head = &fix_data[0];

	memset(fix_head, 0, sizeof(SPRITE *) * FIX_HASH_SIZE);

	fix_texture_num = 0;
	clear_fix_texture = 0;
}


/*------------------------------------------------------------------------
	�S�ẴX�v���C�g�𑦍��ɃN���A����
------------------------------------------------------------------------*/

void blit_clear_all_sprite(void)
{
	blit_clear_spr_sprite();
	blit_clear_fix_sprite();
}


/*------------------------------------------------------------------------
	�X�v���C�g�̃N���A�t���O�𗧂Ă�
------------------------------------------------------------------------*/

void blit_set_spr_clear_flag(void)
{
	clear_spr_texture = 1;
}


/*------------------------------------------------------------------------
	�X�v���C�g�̃N���A�t���O�𗧂Ă�
------------------------------------------------------------------------*/

void blit_set_fix_clear_flag(void)
{
	clear_fix_texture = 1;
}


/*------------------------------------------------------------------------
	�X�v���C�g�����̃��Z�b�g
------------------------------------------------------------------------*/

void blit_reset(void)
{
	int i;

	scrbitmap  = (UINT16 *)video_frame_addr(work_frame, 0, 0);
	tex_spr[0] = (UINT8 *)(scrbitmap + BUF_WIDTH * SCR_HEIGHT);
	tex_spr[1] = tex_spr[0] + BUF_WIDTH * TEXTURE_HEIGHT;
	tex_spr[2] = tex_spr[1] + BUF_WIDTH * TEXTURE_HEIGHT;
	tex_fix    = tex_spr[2] + BUF_WIDTH * TEXTURE_HEIGHT;

	for (i = 0; i < FIX_TEXTURE_SIZE; i++) fix_data[i].index = i;
	for (i = 0; i < SPR_TEXTURE_SIZE; i++) spr_data[i].index = i;

	clip_min_y = FIRST_VISIBLE_LINE;
	clip_max_y = LAST_VISIBLE_LINE;

	clut = (UINT16 *)PSP_UNCACHE_PTR(&video_palettebank[neogeo_palette_index]);

	blit_clear_all_sprite();
}


/*------------------------------------------------------------------------
	��ʂ̍X�V�J�n
------------------------------------------------------------------------*/

void blit_start(int start, int end)
{
	clip_min_y = start;
	clip_max_y = end + 1;

	spr_num   = 0;
	spr_index = 0;

	if (start == FIRST_VISIBLE_LINE)
	{
		clut = (UINT16 *)PSP_UNCACHE_PTR(&video_palettebank[neogeo_palette_index]);

		fix_num = 0;

		if (clear_spr_texture) blit_clear_spr_sprite();
		if (clear_fix_texture) blit_clear_fix_sprite();
		if (neogeo_selected_vectors) spr_disable = 0;

		sceGuStart(GU_DIRECT, gulist);
		sceGuDrawBufferList(GU_PSM_5551, draw_frame, BUF_WIDTH);
		sceGuScissor(0, 0, BUF_WIDTH, SCR_WIDTH);
		sceGuClear(GU_COLOR_BUFFER_BIT);
		sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
		sceGuClear(GU_COLOR_BUFFER_BIT);
		sceGuScissor(8, 16, 312, 240);
		sceGuClearColor(CNVCOL15TO32(video_palette[4095]));
		sceGuClear(GU_COLOR_BUFFER_BIT);
		sceGuClearColor(0);
		sceGuEnable(GU_ALPHA_TEST);
		sceGuTexMode(GU_PSM_T8, 0, 0, GU_TRUE);
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
		sceGuFinish();
		sceGuSync(0, 0);
	}
}


/*------------------------------------------------------------------------
	��ʂ̍X�V�I��
------------------------------------------------------------------------*/

void blit_finish(void)
{
	video_copy_rect(work_frame, draw_frame, &mvs_src_clip, &mvs_clip[option_stretch]);
}


/*------------------------------------------------------------------------
	FIX��`�惊�X�g�ɓo�^
------------------------------------------------------------------------*/

void blit_draw_fix(int x, int y, UINT32 code, UINT32 attr)
{
	s16 idx;
	struct Vertex *vertices;
	UINT32 key = MAKE_FIX_KEY(code, attr);

	if ((idx = fix_get_sprite(key)) < 0)
	{
		UINT32 col, tile;
		UINT8 *src, *dst, lines = 8;

		if (fix_texture_num == FIX_TEXTURE_SIZE - 1)
			fix_delete_sprite();

		idx = fix_insert_sprite(key);
		dst = SWIZZLED8_8x8(tex_fix, idx);
		src = &fix_memory[code << 5];
		col = color_table[attr];

		while (lines--)
		{
			tile = *(UINT32 *)(src + 0);
			*(UINT32 *)(dst +  0) = ((tile >> 0) & 0x0f0f0f0f) | col;
			*(UINT32 *)(dst +  4) = ((tile >> 4) & 0x0f0f0f0f) | col;
			src += 4;
			dst += 16;
		}
	}

	vertices = &vertices_fix[fix_num];
	fix_num += 2;

	vertices[0].x = vertices[1].x = x;
	vertices[0].y = vertices[1].y = y;
	vertices[0].u = vertices[1].u = (idx & 0x003f) << 3;
	vertices[0].v = vertices[1].v = (idx & 0x0fc0) >> 3;

	vertices[1].x += 8;
	vertices[1].y += 8;
	vertices[1].u += 8;
	vertices[1].v += 8;
}


/*------------------------------------------------------------------------
	FIX�`��I��
------------------------------------------------------------------------*/

void blit_finish_fix(void)
{
	struct Vertex *vertices;

	if (!fix_num) return;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(8, 16, 312, 240);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_fix);
	sceGuClutLoad(256/8, clut);

	vertices = (struct Vertex *)sceGuGetMemory(fix_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_fix, fix_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, fix_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	SPR��`�惊�X�g�ɓo�^
------------------------------------------------------------------------*/

void blit_draw_spr(int x, int y, int w, int h, UINT32 code, UINT32 attr)
{
	s16 idx;
	struct Vertex *vertices;
	UINT32 key;

	if (spr_disable) return;

	key = MAKE_SPR_KEY(code, attr);

	if ((idx = spr_get_sprite(key)) < 0)
	{
		UINT32 col, tile;
		UINT8 *src, *dst, lines = 16;

		if (spr_texture_num == SPR_TEXTURE_SIZE - 1)
		{
			spr_delete_sprite();

			if (spr_texture_num == SPR_TEXTURE_SIZE - 1)
			{
				spr_disable = 1;
				return;
			}
		}

		idx = spr_insert_sprite(key);
		dst = SWIZZLED8_16x16(tex_spr[0], idx);
		src = &memory_region_gfx3[(*read_cache)(code << 7)];
		col = color_table[(attr >> 8) & 0x0f];

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

	vertices = &vertices_spr[spr_num];
	spr_num += 2;

	spr_flags[spr_index] = (idx >> 10) | ((attr & 0xf000) >> 4);
	spr_index++;

	vertices[0].x = vertices[1].x = x;
	vertices[0].y = vertices[1].y = y;
	vertices[0].u = vertices[1].u = (idx & 0x001f) << 4;
	vertices[0].v = vertices[1].v = (idx & 0x03e0) >> 1;

	attr ^= 0x03;
	vertices[(attr & 0x01) >> 0].u += 16;
	vertices[(attr & 0x02) >> 1].v += 16;

	vertices[1].x += w;
	vertices[1].y += h;
}


/*------------------------------------------------------------------------
	SPR�`��I��
------------------------------------------------------------------------*/

void blit_finish_spr(void)
{
	int i, total_sprites = 0;
	UINT16 flags, *pflags = spr_flags;
	struct Vertex *vertices, *vertices_tmp;

	if (!spr_index) return;

	flags = *pflags;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(8, clip_min_y, 312, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_spr[flags & 3]);
	sceGuClutLoad(256/8, &clut[flags & 0xf00]);

	vertices_tmp = vertices = (struct Vertex *)sceGuGetMemory(spr_num * sizeof(struct Vertex));

	for (i = 0; i < spr_num; i += 2)
	{
		if (flags != *pflags)
		{
			if (total_sprites)
			{
				sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, 0, vertices);
				total_sprites = 0;
				vertices = vertices_tmp;
			}

			flags = *pflags;
			sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_spr[flags & 3]);
			sceGuClutLoad(256/8, &clut[flags & 0xf00]);
		}

		vertices_tmp[0] = vertices_spr[i + 0];
		vertices_tmp[1] = vertices_spr[i + 1];

		vertices_tmp += 2;
		total_sprites += 2;
		pflags++;
	}

	if (total_sprites)
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_sprites, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}
