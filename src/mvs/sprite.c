/******************************************************************************

	sprite.c

	MVS スプライトマネージャ

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	定数/マクロ等
******************************************************************************/

#define MAKE_FIX_KEY(code, color)	(code | (color << 24))
#define MAKE_SPR_KEY(code, attr)	(code | ((attr & 0xff00) << 16) | ((attr & 0x0c) << 20))


/******************************************************************************
	ローカル変数/構造体
******************************************************************************/

typedef struct sprite_t SPRITE ALIGN_DATA;

struct sprite_t
{
	u32 key;
	int used;
	u16 pal;
	u16 index;
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

static const int swizzle_table[16] =
{
	   0, 8, 8, 8, 8, 8, 8, 8,
	4040, 8, 8, 8, 8, 8, 8, 8
};


/*------------------------------------------------------------------------
	パレットは0～255番まであり、各16色となっている。
	パレットが更新された場合、テクスチャも登録し直す必要がある。
------------------------------------------------------------------------*/

static u8 palette_is_dirty[256];


/*------------------------------------------------------------------------
	FIX: サイズ/位置8pixel固定のスプライト

	size:    8x8 pixels
	palette: 0～15番を使用
------------------------------------------------------------------------*/

#define FIX_TEXTURE_SIZE	((BUF_WIDTH/8)*(208/8))
#define FIX_HASH_MASK		0xff
#define FIX_HASH_SIZE		0x100
#define FIX_MAX_SPRITES		((320/8) * (240/8))

static SPRITE *fix_head[FIX_HASH_SIZE];
static SPRITE fix_data[FIX_TEXTURE_SIZE];
static SPRITE *fix_free_head;

static u16 *tex_fix;
static struct Vertex ALIGN_DATA vertices_fix[FIX_MAX_SPRITES * 2];
static u16 fix_num;
static u16 fix_texture_num;
static u8 fix_texture_clear;
static u8 fix_palette_is_dirty;


/*------------------------------------------------------------------------
	SPR: 可変サイズのスプライト
	     縮小、拡大、反転、ライン毎に描画が可能
	     特殊な機能としてチェーンスプライト、オートアニメーション機能あり

	size:    2x1 pixels ～ 16x16 pixels (可変)
	palette: 0～255番を使用
------------------------------------------------------------------------*/

#define SPR_TEXTURE_SIZE	((BUF_WIDTH/16)*((512*2)/16))
#define SPR_HASH_MASK		0x1ff
#define SPR_HASH_SIZE		0x200
#define SPR_MAX_SPRITES		0x2000
#define SPR_AUTOANIME		(3 << 20)
#define SPR_AUTOANIME4		(1 << 20)
#define SPR_AUTOANIME8		(2 << 20)

static SPRITE *spr_head[SPR_HASH_SIZE];
static SPRITE spr_data[SPR_TEXTURE_SIZE];
static SPRITE *spr_free_head;

static u16 *tex_spr1;
static u16 *tex_spr2;
static struct Vertex ALIGN_DATA vertices_spr[SPR_MAX_SPRITES * 2];
static u16 spr_num;
static u16 spr_texture_num;
static u8 spr_texture_clear;
static u8 spr_palette_is_dirty;
static u8 spr_disable_this_frame;


/******************************************************************************
	FIXスプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	FIXテクスチャをリセットする
------------------------------------------------------------------------*/

static void fix_reset_sprite(void)
{
	int i;

	for (i = 0; i < FIX_TEXTURE_SIZE - 1; i++)
		fix_data[i].next = &fix_data[i + 1];

	fix_data[i].next = NULL;
	fix_free_head = &fix_data[0];

	memset(fix_head, 0, sizeof(SPRITE *) * FIX_HASH_SIZE);

	fix_texture_num = 0;
	fix_texture_clear = 0;
	fix_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	FIXテクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int fix_get_sprite(int key)
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
	FIXテクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int fix_insert_sprite(u32 key)
{
	u16 hash = key & FIX_HASH_MASK;
	SPRITE *p = fix_head[hash];
	SPRITE *q = fix_free_head;

	if (!q) return -1;

	fix_free_head = fix_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 24;
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
	FIXテクスチャから一定時間を経過したスプライトを削除
------------------------------------------------------------------------*/

static void fix_delete_sprite(int delay)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < FIX_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = fix_head[i];

		while (p)
		{
			if (frames_displayed - p->used > delay)
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


/*------------------------------------------------------------------------
	FIXテクスチャからパレットが変更されたスプライトを削除
------------------------------------------------------------------------*/

static void fix_delete_dirty_palette(void)
{
	SPRITE *p, *prev_p;
	int i, palno;

	for (palno = 0; palno < 0x10; palno++)
	{
		if (!palette_is_dirty[palno]) continue;

		for (i = 0; i < FIX_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = fix_head[i];

			while (p)
			{
				if (p->pal == palno)
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

	fix_palette_is_dirty = 0;
}


/******************************************************************************
	SPRスプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	SPRテクスチャをリセットする
------------------------------------------------------------------------*/

static void spr_reset_sprite(void)
{
	int i;

	for (i = 0; i < SPR_TEXTURE_SIZE - 1; i++)
		spr_data[i].next = &spr_data[i + 1];

	spr_data[i].next = NULL;
	spr_free_head = &spr_data[0];

	memset(spr_head, 0, sizeof(SPRITE *) * SPR_HASH_SIZE);

	spr_texture_num = 0;
	spr_texture_clear = 0;
	spr_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	SPRテクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int spr_get_sprite(int key)
{
	SPRITE *p = spr_head[key & SPR_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			if (p->used != frames_displayed)
			{
				update_cache((key << 7) & 0x3ffffff);
				p->used = frames_displayed;
			}
			return p->index;
		}
		p = p->next;
	}
	return -1;
}


/*------------------------------------------------------------------------
	SPRテクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int spr_insert_sprite(u32 key)
{
	u16 hash = key & SPR_HASH_MASK;
	SPRITE *p = spr_head[hash];
	SPRITE *q = spr_free_head;

	if (!q) return -1;

	spr_free_head = spr_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 24;
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
	SPRテクスチャから一定時間を経過したスプライトを削除
------------------------------------------------------------------------*/

static void spr_delete_sprite(int delay, int clear_all)
{
	int i;
	SPRITE *p, *prev_p;

	if (clear_all)
	{
		for (i = 0; i < SPR_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = spr_head[i];

			while (p)
			{
				if (frames_displayed - p->used > delay)
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
	else
	{
		int unused_frames;

		for (i = 0; i < SPR_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = spr_head[i];

			while (p)
			{
				unused_frames = frames_displayed - p->used;

				if (unused_frames > delay)
				{
					int clear = 1;

					switch (p->key & SPR_AUTOANIME)
					{
					case SPR_AUTOANIME4:
						if (unused_frames <= (neogeo_frame_counter << 2))
							clear = 0;
						break;

					case SPR_AUTOANIME8:
						if (unused_frames <= (neogeo_frame_counter << 3))
							clear = 0;
						break;
					}

					if (clear)
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
				else
				{
					prev_p = p;
					p = p->next;
				}
			}
		}
	}
}


/*------------------------------------------------------------------------
	SPRテクスチャからパレットが変更されたスプライトを削除
------------------------------------------------------------------------*/

static void spr_delete_dirty_palette(void)
{
	SPRITE *p, *prev_p;
	int i, palno;

	for (palno = 0; palno < 0x100; palno++)
	{
		if (!palette_is_dirty[palno]) continue;

		for (i = 0; i < SPR_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = spr_head[i];

			while (p)
			{
				if (p->pal == palno)
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

	spr_palette_is_dirty = 0;
}


/******************************************************************************
	スプライト描画インタフェース関数
******************************************************************************/

/*------------------------------------------------------------------------
	全てのスプライトを即座にクリアする
------------------------------------------------------------------------*/

void blit_clear_all_sprite(void)
{
	fix_reset_sprite();
	spr_reset_sprite();
	memset(palette_is_dirty, 0, sizeof(palette_is_dirty));
	spr_disable_this_frame = 0;
}


/*------------------------------------------------------------------------
	FIXスプライトのクリアフラグを立てる
------------------------------------------------------------------------*/

void blit_clear_fix_sprite(void)
{
	fix_texture_clear = 1;
	fix_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	SPRスプライトのクリアフラグを立てる
------------------------------------------------------------------------*/

void blit_clear_spr_sprite(void)
{
	spr_texture_clear = 1;
	spr_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	パレット変更フラグを立てる
------------------------------------------------------------------------*/

void blit_palette_mark_dirty(int palno)
{
	palette_is_dirty[palno] = 1;

	if (palno < 0x10) fix_palette_is_dirty = 1;
	spr_palette_is_dirty = 1;
}


/*------------------------------------------------------------------------
	スプライト処理のリセット
------------------------------------------------------------------------*/

void blit_reset(void)
{
	int i;
	u16 *work_buffer;

	work_buffer = video_frame_addr(work_frame, 0, 0);
	tex_spr1 = work_buffer + BUF_WIDTH * 256;
	tex_spr2 = tex_spr1 + BUF_WIDTH * 512;
	tex_fix  = tex_spr2 + BUF_WIDTH * 512;

	for (i = 0; i < FIX_TEXTURE_SIZE - 1; i++) fix_data[i].index = i;
	for (i = 0; i < SPR_TEXTURE_SIZE - 1; i++) spr_data[i].index = i;

	blit_clear_all_sprite();

	if (neogeo_bios != ASIA_AES) spr_disable_this_frame = 1;
}


/*------------------------------------------------------------------------
	スプライト描画開始
------------------------------------------------------------------------*/

void blit_start(void)
{
	if (fix_texture_clear) fix_reset_sprite();
	if (spr_texture_clear) spr_reset_sprite();

	if (fix_palette_is_dirty) fix_delete_dirty_palette();
	if (spr_palette_is_dirty) spr_delete_dirty_palette();
	memset(palette_is_dirty, 0, sizeof(palette_is_dirty));

	if (neogeo_selected_vectors) spr_disable_this_frame = 0;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, draw_frame, BUF_WIDTH);
	sceGuScissor(0, 0, 512, 272);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(0, 0, 512, 256);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuScissor(8, 16, 312, 240);
	sceGuClearColor(CNVCOL15TO32(video_palette16[4095]));
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuClearColor(0);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuTexMode(GU_PSM_5551, 0, 0, GU_TRUE);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	画面の一部更新開始
------------------------------------------------------------------------*/

void blit_partial_start(int start, int end)
{
	clip_min_y = start;
	clip_max_y = end + 1;

	fix_num = 0;
	spr_num = 0;

	if (start == FIRST_VISIBLE_LINE)
		blit_start();
}


/*------------------------------------------------------------------------
	スプライト描画終了
------------------------------------------------------------------------*/

void blit_finish(void)
{
	video_copy_rect(work_frame, draw_frame, &mvs_src_clip, &mvs_clip[option_stretch]);
}


/*------------------------------------------------------------------------
	FIXを描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_fix(int x, int y, u32 code, u32 attr)
{
	int idx;
	struct Vertex *vertices;
	u32 key;

	key = MAKE_FIX_KEY(code, attr);

	if ((idx = fix_get_sprite(key)) < 0)
	{
		int lines = 8;
		u32 tile, *gfx;
		u16 *dst, *pal;

		if (fix_texture_num == FIX_TEXTURE_SIZE - 1)
		{
			fix_delete_sprite(0);
		}

		idx = fix_insert_sprite(key);
		gfx = (u32 *)&fix_memory[code << 5];
		pal = &video_palette16[attr << 4];
		dst = SWIZZLED_8x8(tex_fix, idx);

		while (lines--)
		{
			tile = *gfx++;
			dst[0] = pal[tile & 0x0f]; tile >>= 4;
			dst[1] = pal[tile & 0x0f]; tile >>= 4;
			dst[2] = pal[tile & 0x0f]; tile >>= 4;
			dst[3] = pal[tile & 0x0f]; tile >>= 4;
			dst[4] = pal[tile & 0x0f]; tile >>= 4;
			dst[5] = pal[tile & 0x0f]; tile >>= 4;
			dst[6] = pal[tile & 0x0f]; tile >>= 4;
			dst[7] = pal[tile & 0x0f];
			dst += 8;
		}
	}

	vertices = &vertices_fix[fix_num];

	vertices[0].u = vertices[1].u = (idx & 63) << 3;
	vertices[0].v = vertices[1].v = (idx >> 6) << 3;

	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].u += 8;
	vertices[1].v += 8;
	vertices[1].x = x + 8;
	vertices[1].y = y + 8;

	fix_num += 2;
}


/*------------------------------------------------------------------------
	FIX描画終了
------------------------------------------------------------------------*/

void blit_finish_fix(void)
{
	struct Vertex *vertices;

	if (!fix_num) return;

	sceGuStart(GU_DIRECT, gulist);

	vertices = (struct Vertex *)sceGuGetMemory(fix_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_fix, fix_num * sizeof(struct Vertex));

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(8, 16, 312, 240);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_fix);

	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, fix_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	SPRを描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_spr(int x, int y, int w, int h, u32 code, u32 attr)
{
	int idx;
	struct Vertex *vertices;
	u32 key;

	if (spr_disable_this_frame) return;
	if (spr_num == SPR_MAX_SPRITES) return;

	key = MAKE_SPR_KEY(code, attr);

	if ((idx = spr_get_sprite(key)) < 0)
	{
		int lines = 16;
		u16 *dst, *pal;
		u32 *gfx, tile, offs;

		if (spr_texture_num == SPR_TEXTURE_SIZE - 1)
		{
			spr_delete_sprite(0, 0);

			if (spr_texture_num == SPR_TEXTURE_SIZE - 1)
			{
				spr_disable_this_frame = 1;
				return;
			}
		}

		idx  = spr_insert_sprite(key);
		offs = (*read_cache)(code << 7);
		gfx  = (u32 *)&memory_region_gfx3[offs];
		pal  = &video_palette16[(attr & 0xff00) >> 4];
		dst  = SWIZZLED_16x16(tex_spr1, idx);

		while (lines--)
		{
			tile = *gfx++;
			dst[ 0] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 1] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 2] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 3] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 4] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 5] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 6] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 7] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[64] = pal[tile & 0x0f]; tile >>= 4;
			dst[65] = pal[tile & 0x0f]; tile >>= 4;
			dst[66] = pal[tile & 0x0f]; tile >>= 4;
			dst[67] = pal[tile & 0x0f]; tile >>= 4;
			dst[68] = pal[tile & 0x0f]; tile >>= 4;
			dst[69] = pal[tile & 0x0f]; tile >>= 4;
			dst[70] = pal[tile & 0x0f]; tile >>= 4;
			dst[71] = pal[tile & 0x0f];
			dst += swizzle_table[lines];
		}
	}

	vertices = &vertices_spr[spr_num];

	vertices[0].u = vertices[1].u = (idx & 31) << 4;
	vertices[0].v = vertices[1].v = (idx >> 5) << 4;

	if (vertices[0].v & 512)
	{
		vertices[0].v = vertices[1].v &= 511;
		vertices[0].z = vertices[1].z = 1;
	}
	else
	{
		vertices[0].z = vertices[1].z = 0;
	}

	attr ^= 0x03;
	vertices[(attr & 0x01) >> 0].u += 16;
	vertices[(attr & 0x02) >> 1].v += 16;

	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].x = x + w;
	vertices[1].y = y + h;

	spr_num += 2;
}


/*------------------------------------------------------------------------
	SPR描画終了
------------------------------------------------------------------------*/

void blit_finish_spr(void)
{
	int i, total_spr = 0, cur_tex = 0;
	struct Vertex *vertices, *vertices_tmp;

	if (!spr_num) return;

	sceGuStart(GU_DIRECT, gulist);

	vertices_tmp = vertices = (struct Vertex *)sceGuGetMemory(spr_num * sizeof(struct Vertex));

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(8, clip_min_y, 312, clip_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_spr1);

	for (i = 0; i < spr_num; i += 2)
	{
		if (vertices_spr[i].z != cur_tex)
		{
			sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_spr, 0, vertices);

			cur_tex ^= 1;
			total_spr = 0;
			vertices = vertices_tmp;
			sceGuTexImage(0, 512, 512, BUF_WIDTH, (cur_tex ? tex_spr2 : tex_spr1));
		}

		vertices_tmp[0] = vertices_spr[i + 0];
		vertices_tmp[1] = vertices_spr[i + 1];

		total_spr += 2;
		vertices_tmp += 2;
	}

	if (total_spr)
		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, total_spr, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);

	spr_num = 0;
}
