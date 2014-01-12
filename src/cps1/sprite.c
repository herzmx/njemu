/******************************************************************************

	sprite.c

	CPS1 スプライトマネージャ

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	定数/マクロ等
******************************************************************************/

#define OBJECT_MAX_HEIGHT	512
#define SCROLL1_MAX_HEIGHT	208
#define SCROLL2_MAX_HEIGHT	384
#define SCROLL3_MAX_HEIGHT	448
#define SCROLLH_MAX_HEIGHT	192

#define MAKE_KEY(code, attr)			(code | ((attr & 0x1f) << 16))
#define MAKE_KEY_SCROLL3(code, attr)	((code << 8) | (attr & 0x1f))
#define MAKE_HIGH_KEY(code, attr)		(code | ((attr & 0x19f) << 15))


/******************************************************************************
	ローカル変数/構造体
******************************************************************************/

typedef struct sprite_t SPRITE ALIGN_DATA;

struct sprite_t
{
	u32 key;
	u32 used;
	u16 pal;
	u16 index;
	SPRITE *next;
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

static int scroll2_min_y;
static int scroll2_max_y;

static const int ALIGN_DATA swizzle_table[32] =
{
	   0, 8, 8, 8, 8, 8, 8, 8,
	4040, 8, 8, 8, 8, 8, 8, 8,
	4040, 8, 8, 8, 8, 8, 8, 8,
	4040, 8, 8, 8, 8, 8, 8, 8
};


/*------------------------------------------------------------------------
	パレットは0～255番まであり、各16色となっている。
	パレットが更新された場合、テクスチャも登録し直す必要がある。
------------------------------------------------------------------------*/

static u8 ALIGN_DATA palette_dirty_marks[256];


/*------------------------------------------------------------------------
	OBJECT: キャラクタ等

	size:    16x16 pixels
	palette: 0～31番を使用
------------------------------------------------------------------------*/

#define OBJECT_HASH_SIZE		0x200
#define OBJECT_HASH_MASK		0x1ff
#define OBJECT_TEXTURE_SIZE		((BUF_WIDTH/16)*(OBJECT_MAX_HEIGHT/16))
#define OBJECT_MAX_SPRITES		0x1000

static SPRITE *object_head[OBJECT_HASH_SIZE];
static SPRITE object_data[OBJECT_TEXTURE_SIZE];
static SPRITE *object_free_head;

static u8  *gfx_object;
static u16 *tex_object;
static struct Vertex ALIGN_DATA vertices_object[OBJECT_MAX_SPRITES * 2];
static u16 object_num;
static u16 object_texture_num;
static u16 object_max;
static u8 object_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLL1: スクロール面1(テキスト等)

	size:    8x8 pixels
	palette: 32～63番を使用
------------------------------------------------------------------------*/

#define SCROLL1_HASH_SIZE		0x200
#define SCROLL1_HASH_MASK		0x1ff
#define SCROLL1_TEXTURE_SIZE	((BUF_WIDTH/8)*(SCROLL1_MAX_HEIGHT/8))
#define SCROLL1_MAX_SPRITES		((384/8 + 2) * (224/8 + 2))

static SPRITE *scroll1_head[SCROLL1_HASH_SIZE];
static SPRITE scroll1_data[SCROLL1_TEXTURE_SIZE];
static SPRITE *scroll1_free_head;

static u8  *gfx_scroll1;
static u16 *tex_scroll1;
static struct Vertex ALIGN_DATA vertices_scroll1[SCROLL1_MAX_SPRITES * 2];
static u16 scroll1_num;
static u16 scroll1_texture_num;
static u16 scroll1_max;
static u8 scroll1_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLL2: スクロール面2

	size:    16x16 pixels
	palette: 64～95番を使用
------------------------------------------------------------------------*/

#define SCROLL2_HASH_SIZE		0x100
#define SCROLL2_HASH_MASK		0xff
#define SCROLL2_TEXTURE_SIZE	((BUF_WIDTH/16)*(SCROLL2_MAX_HEIGHT/16))
#define SCROLL2_MAX_SPRITES		((384/16 + 2) * (224/16 + 2))

static SPRITE *scroll2_head[SCROLL2_HASH_SIZE];
static SPRITE scroll2_data[SCROLL2_TEXTURE_SIZE];
static SPRITE *scroll2_free_head;

static u8  *gfx_scroll2;
static u16 *tex_scroll2;
static struct Vertex ALIGN_DATA vertices_scroll2[SCROLL2_MAX_SPRITES * 2];
static u16 scroll2_num;
static u16 scroll2_texture_num;
static u16 scroll2_max;
static u8 scroll2_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLL3: スクロール面3

	size:    32x32 pixels
	palette: 96～127番を使用
------------------------------------------------------------------------*/

#define SCROLL3_HASH_SIZE		0x20
#define SCROLL3_HASH_MASK		0x1f
#define SCROLL3_TEXTURE_SIZE	((BUF_WIDTH/32)*(SCROLL3_MAX_HEIGHT/32))
#define SCROLL3_MAX_SPRITES		((384/32 + 2) * (224/32 + 2))

static SPRITE *scroll3_head[SCROLL3_HASH_SIZE];
static SPRITE scroll3_data[SCROLL3_TEXTURE_SIZE];
static SPRITE *scroll3_free_head;

static u8  *gfx_scroll3;
static u16 *tex_scroll3;
static struct Vertex ALIGN_DATA vertices_scroll3[SCROLL3_MAX_SPRITES * 2];
static u16 scroll3_num;
static u16 scroll3_texture_num;
static u16 scroll3_max;
static u8 scroll3_palette_is_dirty;


/*------------------------------------------------------------------------
	SCROLLH: スクロール面 (ハイプライオリティ)

	size:    8x8～32x32 pixels
	palette: 32～127番を使用
------------------------------------------------------------------------*/

#define SCROLLH_HASH_SIZE		0x200
#define SCROLLH_HASH_MASK		0x1ff
#define SCROLLH_TEXTURE_SIZE	((BUF_WIDTH/8)*(SCROLLH_MAX_HEIGHT/8))
#define SCROLLH_MAX_SPRITES		SCROLL1_MAX_SPRITES

#define SCROLL1H_TEXTURE_SIZE	SCROLLH_TEXTURE_SIZE
#define SCROLL1H_MAX_SPRITES	SCROLL1_MAX_SPRITES
#define SCROLL2H_TEXTURE_SIZE	((BUF_WIDTH/16)*(SCROLLH_MAX_HEIGHT/16))
#define SCROLL2H_MAX_SPRITES	SCROLL2_MAX_SPRITES
#define SCROLL3H_TEXTURE_SIZE	((BUF_WIDTH/32)*(SCROLLH_MAX_HEIGHT/32))
#define SCROLL3H_MAX_SPRITES	SCROLL3_MAX_SPRITES

static SPRITE *scrollh_head[SCROLLH_HASH_SIZE];
static SPRITE scrollh_data[SCROLLH_TEXTURE_SIZE];
static SPRITE *scrollh_free_head;

static u16 *tex_scrollh;
static struct Vertex ALIGN_DATA vertices_scrollh[SCROLLH_MAX_SPRITES * 2];
static u16 scrollh_num;
static u16 scrollh_texture_num;
static u16 scroll1h_max;
static u16 scroll2h_max;
static u16 scroll3h_max;
static u8  scrollh_texture_clear;
static u8  scrollh_layer_number;


/******************************************************************************
	OBJECTスプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	OBJECTテクスチャをリセットする
------------------------------------------------------------------------*/

static void object_reset_sprite(void)
{
	int i;

	for (i = 0; i < object_max - 1; i++)
		object_data[i].next = &object_data[i + 1];

	object_data[i].next = NULL;
	object_free_head = &object_data[0];

	memset(object_head, 0, sizeof(SPRITE *) * OBJECT_HASH_SIZE);

	object_texture_num = 0;
	object_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	OBJECTテクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int object_get_sprite(u32 key)
{
	SPRITE *p = object_head[key & OBJECT_HASH_MASK];

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
	OBJECTテクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int object_insert_sprite(u32 key)
{
	u16 hash = key & OBJECT_HASH_MASK;
	SPRITE *p = object_head[hash];
	SPRITE *q = object_free_head;

	if (!q) return -1;

	object_free_head = object_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 16;
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
	OBJECTテクスチャから一定時間を経過したスプライトを削除
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


/*------------------------------------------------------------------------
	OBJECTテクスチャからパレットが変更されたスプライトを削除
------------------------------------------------------------------------*/

static void object_delete_dirty_palette(void)
{
	int i, palno;
	SPRITE *p, *prev_p;

	for (palno = 0; palno < 32; palno++)
	{
		if (!palette_dirty_marks[palno]) continue;

		for (i = 0; i < OBJECT_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = object_head[i];

			while (p)
			{
				if (p->pal == palno)
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

	object_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLL1スプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLL1テクスチャをリセットする
------------------------------------------------------------------------*/

static void scroll1_reset_sprite(void)
{
	int i;

	for (i = 0; i < scroll1_max - 1; i++)
		scroll1_data[i].next = &scroll1_data[i + 1];

	scroll1_data[i].next = NULL;
	scroll1_free_head = &scroll1_data[0];

	memset(scroll1_head, 0, sizeof(SPRITE *) * SCROLL1_HASH_SIZE);

	scroll1_texture_num = 0;
	scroll1_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	SCROLL1テクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int scroll1_get_sprite(u32 key)
{
	SPRITE *p = scroll1_head[key & SCROLL1_HASH_MASK];

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
	SCROLL1テクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int scroll1_insert_sprite(u32 key)
{
	u16 hash = key & SCROLL1_HASH_MASK;
	SPRITE *p = scroll1_head[hash];
	SPRITE *q = scroll1_free_head;

	if (!q) return -1;

	scroll1_free_head = scroll1_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 16;
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
	SCROLL1テクスチャから一定時間を経過したスプライトを削除
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


/*------------------------------------------------------------------------
	SCROLL1テクスチャからパレットが変更されたスプライトを削除
------------------------------------------------------------------------*/

static void scroll1_delete_dirty_palette(void)
{
	int i, palno;
	u8 *dirty_flags = &palette_dirty_marks[32];
	SPRITE *p, *prev_p;

	for (palno = 0; palno < 32; palno++)
	{
		if (!dirty_flags[palno]) continue;

		for (i = 0; i < SCROLL1_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = scroll1_head[i];

			while (p)
			{
				if (p->pal == palno)
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

		if (scrollh_layer_number == 1)
		{
			for (i = 0; i < SCROLLH_HASH_SIZE; i++)
			{
				prev_p = NULL;
				p = scrollh_head[i];

				while (p)
				{
					if (p->pal == palno)
					{
						scrollh_texture_num--;

						if (!prev_p)
						{
							scrollh_head[i] = p->next;
							p->next = scrollh_free_head;
							scrollh_free_head = p;
							p = scrollh_head[i];
						}
						else
						{
							prev_p->next = p->next;
							p->next = scrollh_free_head;
							scrollh_free_head = p;
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
	}

	scroll1_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLL2スプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLL2テクスチャをリセットする
------------------------------------------------------------------------*/

static void scroll2_reset_sprite(void)
{
	int i;

	for (i = 0; i < scroll2_max - 1; i++)
		scroll2_data[i].next = &scroll2_data[i + 1];

	scroll2_data[i].next = NULL;
	scroll2_free_head = &scroll2_data[0];

	memset(scroll2_head, 0, sizeof(SPRITE *) * SCROLL2_HASH_SIZE);

	scroll2_texture_num = 0;
	scroll2_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	SCROLL2テクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int scroll2_get_sprite(u32 key)
{
	SPRITE *p = scroll2_head[key & SCROLL2_HASH_MASK];

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
	SCROLL2テクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int scroll2_insert_sprite(u32 key)
{
	u16 hash = key & SCROLL2_HASH_MASK;
	SPRITE *p = scroll2_head[hash];
	SPRITE *q = scroll2_free_head;

	if (!q) return -1;

	scroll2_free_head = scroll2_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = key >> 16;
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
	SCROLL2テクスチャから一定時間を経過したスプライトを削除
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


/*------------------------------------------------------------------------
	SCROLL2テクスチャからパレットが変更されたスプライトを削除
------------------------------------------------------------------------*/

static void scroll2_delete_dirty_palette(void)
{
	int i, palno;
	u8 *dirty_flags = &palette_dirty_marks[64];
	SPRITE *p, *prev_p;

	for (palno = 0; palno < 32; palno++)
	{
		if (!dirty_flags[palno]) continue;

		for (i = 0; i < SCROLL2_HASH_SIZE; i++)
		{
			prev_p = NULL;
			p = scroll2_head[i];

			while (p)
			{
				if (p->pal == palno)
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

		if (scrollh_layer_number == 2)
		{
			for (i = 0; i < SCROLLH_HASH_SIZE; i++)
			{
				prev_p = NULL;
				p = scrollh_head[i];

				while (p)
				{
					if (p->pal == palno)
					{
						scrollh_texture_num--;

						if (!prev_p)
						{
							scrollh_head[i] = p->next;
							p->next = scrollh_free_head;
							scrollh_free_head = p;
							p = scrollh_head[i];
						}
						else
						{
							prev_p->next = p->next;
							p->next = scrollh_free_head;
							scrollh_free_head = p;
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
	}

	scroll2_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLL3スプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLL3テクスチャをリセットする
------------------------------------------------------------------------*/

static void scroll3_reset_sprite(void)
{
	int i;

	for (i = 0; i < scroll3_max - 1; i++)
		scroll3_data[i].next = &scroll3_data[i + 1];

	scroll3_data[i].next = NULL;
	scroll3_free_head = &scroll3_data[0];

	memset(scroll3_head, 0, sizeof(SPRITE *) * SCROLL3_HASH_SIZE);

	scroll3_texture_num = 0;
	scroll3_palette_is_dirty = 0;
}


/*------------------------------------------------------------------------
	SCROLL3テクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int scroll3_get_sprite(u32 key)
{
	SPRITE *p = scroll3_head[key & SCROLL3_HASH_MASK];

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
	SCROLL3テクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int scroll3_insert_sprite(u32 key)
{
	u16 hash = key & SCROLL3_HASH_MASK;
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
	SCROLL3テクスチャから一定時間を経過したスプライトを削除
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


/*------------------------------------------------------------------------
	SCROLL3テクスチャからパレットが変更されたスプライトを削除
------------------------------------------------------------------------*/

static void scroll3_delete_dirty_palette(void)
{
	int i, palno;
	u8 *dirty_flags = &palette_dirty_marks[96];
	SPRITE *p, *prev_p;

	for (palno = 0; palno < 32; palno++)
	{
		if (!dirty_flags[palno]) continue;

		p = scroll3_head[palno];

		while (p)
		{
			scroll3_texture_num--;

			scroll3_head[palno] = p->next;
			p->next = scroll3_free_head;
			scroll3_free_head = p;
			p = scroll3_head[palno];
		}

		if (scrollh_layer_number == LAYER_SCROLL3)
		{
			for (i = 0; i < SCROLLH_HASH_SIZE; i++)
			{
				prev_p = NULL;
				p = scrollh_head[i];

				while (p)
				{
					if (p->pal == palno)
					{
						scrollh_texture_num--;

						if (!prev_p)
						{
							scrollh_head[i] = p->next;
							p->next = scrollh_free_head;
							scrollh_free_head = p;
							p = scrollh_head[i];
						}
						else
						{
							prev_p->next = p->next;
							p->next = scrollh_free_head;
							scrollh_free_head = p;
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
	}

	scroll3_palette_is_dirty = 0;
}


/******************************************************************************
	SCROLLHスプライト管理
******************************************************************************/

/*------------------------------------------------------------------------
	SCROLLHテクスチャをリセットする
------------------------------------------------------------------------*/

static void scrollh_reset_sprite(void)
{
	int i;

	for (i = 0; i < scroll1h_max - 1; i++)
		scrollh_data[i].next = &scrollh_data[i + 1];

	scrollh_data[i].next = NULL;
	scrollh_free_head = &scrollh_data[0];

	memset(scrollh_head, 0, sizeof(SPRITE *) * SCROLLH_HASH_SIZE);

	scrollh_texture_num = 0;
	scrollh_texture_clear = 0;
	scrollh_layer_number = 0;
}


/*------------------------------------------------------------------------
	SCROLLHテクスチャからスプライト番号を取得
------------------------------------------------------------------------*/

static int scrollh_get_sprite(u32 key)
{
	SPRITE *p = scrollh_head[key & SCROLLH_HASH_MASK];

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
	SCROLLHテクスチャにスプライトを登録
------------------------------------------------------------------------*/

static int scrollh_insert_sprite(u32 key)
{
	u16 hash = key & SCROLLH_HASH_MASK;
	SPRITE *p = scrollh_head[hash];
	SPRITE *q = scrollh_free_head;

	if (!q) return -1;

	scrollh_free_head = scrollh_free_head->next;

	q->next = NULL;
	q->key  = key;
	q->pal  = (key >> 16) & 0x1f;
	q->used = frames_displayed;

	if (!p)
	{
		scrollh_head[hash] = q;
	}
	else
	{
		while (p->next) p = p->next;
		p->next = q;
	}

	scrollh_texture_num++;

	return q->index;
}


/*------------------------------------------------------------------------
	SCROLLHテクスチャから一定時間を経過したスプライトを削除
------------------------------------------------------------------------*/

static void scrollh_delete_sprite(void)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLLH_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scrollh_head[i];

		while (p)
		{
			if (frames_displayed != p->used)
			{
				scrollh_texture_num--;

				if (!prev_p)
				{
					scrollh_head[i] = p->next;
					p->next = scrollh_free_head;
					scrollh_free_head = p;
					p = scrollh_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scrollh_free_head;
					scrollh_free_head = p;
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
	SCROLLHキャッシュから指定した透過ペンのテクスチャを削除
------------------------------------------------------------------------*/

static void scrollh_delete_sprite_tpens(u16 tpens)
{
	int i;
	SPRITE *p, *prev_p;

	for (i = 0; i < SCROLLH_HASH_SIZE; i++)
	{
		prev_p = NULL;
		p = scrollh_head[i];

		while (p)
		{
			if ((p->key >> 23) == tpens)
			{
				scrollh_texture_num--;

				if (!prev_p)
				{
					scrollh_head[i] = p->next;
					p->next = scrollh_free_head;
					scrollh_free_head = p;
					p = scrollh_head[i];
				}
				else
				{
					prev_p->next = p->next;
					p->next = scrollh_free_head;
					scrollh_free_head = p;
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
	スプライト描画インタフェース関数
******************************************************************************/

/*------------------------------------------------------------------------
	全てのスプライトを即座にクリアする
------------------------------------------------------------------------*/

void blit_clear_all_sprite(void)
{
	object_reset_sprite();
	scroll1_reset_sprite();
	scroll2_reset_sprite();
	scroll3_reset_sprite();
	scrollh_reset_sprite();
	memset(palette_dirty_marks, 0, sizeof(palette_dirty_marks));
}


/*------------------------------------------------------------------------
	ハイレイヤーをクリアする
------------------------------------------------------------------------*/

void blit_scrollh_clear_sprite(int tpens)
{
	scrollh_delete_sprite_tpens(tpens);
}


/*------------------------------------------------------------------------
	パレット変更フラグを立てる
------------------------------------------------------------------------*/

void blit_palette_mark_dirty(int palno)
{
	palette_dirty_marks[palno] = 1;

	if (palno < 32) object_palette_is_dirty = 1;
	else if (palno < 64) scroll1_palette_is_dirty = 1;
	else if (palno < 96) scroll2_palette_is_dirty = 1;
	else if (palno < 128) scroll3_palette_is_dirty = 1;
}


/*------------------------------------------------------------------------
	スプライト処理のリセット
------------------------------------------------------------------------*/

void blit_reset(int bank_scroll1, int bank_scroll2, int bank_scroll3)
{
	int i;
	int object_height;
	int scroll1_height;
	int scroll2_height;
	int scroll3_height;
	int scrollh_height;
	u16 *work_buffer;

	// 合計が1232pixelになるように割り当てる
	object_height  = driver->object_tex_height;
	scroll1_height = driver->scroll1_tex_height;
	scroll2_height = driver->scroll2_tex_height;
	scroll3_height = driver->scroll3_tex_height;
	scrollh_height = driver->scrollh_tex_height;

	object_max   = (BUF_WIDTH/16) * (object_height /16);
	scroll1_max  = (BUF_WIDTH/ 8) * (scroll1_height/ 8);
	scroll2_max  = (BUF_WIDTH/16) * (scroll2_height/16);
	scroll3_max  = (BUF_WIDTH/32) * (scroll3_height/32);
	scroll1h_max = (BUF_WIDTH/ 8) * (scrollh_height/ 8);
	scroll2h_max = (BUF_WIDTH/16) * (scrollh_height/16);
	scroll3h_max = (BUF_WIDTH/32) * (scrollh_height/32);

	work_buffer = video_frame_addr(work_frame, 0, 0);
	tex_object  = work_buffer + BUF_WIDTH * 256;
	tex_scroll1 = tex_object  + BUF_WIDTH * object_height;
	tex_scroll2 = tex_scroll1 + BUF_WIDTH * scroll1_height;
	tex_scroll3 = tex_scroll2 + BUF_WIDTH * scroll2_height;
	tex_scrollh = tex_scroll3 + BUF_WIDTH * scroll3_height;

	for (i = 0; i < object_max; i++) object_data[i].index = i;
	for (i = 0; i < scroll1_max; i++) scroll1_data[i].index = i;
	for (i = 0; i < scroll2_max; i++) scroll2_data[i].index = i;
	for (i = 0; i < scroll3_max; i++) scroll3_data[i].index = i;
	for (i = 0; i < scroll1h_max; i++) scrollh_data[i].index = i;

	blit_clear_all_sprite();

	gfx_object  = memory_region_gfx1;
	gfx_scroll1 = &memory_region_gfx1[bank_scroll1 << 21];
	gfx_scroll2 = &memory_region_gfx1[bank_scroll2 << 21];
	gfx_scroll3 = &memory_region_gfx1[bank_scroll3 << 21];
}


/*------------------------------------------------------------------------
	スプライト描画開始
------------------------------------------------------------------------*/

void blit_start(int high_layer)
{
	if (scrollh_texture_clear || high_layer != scrollh_layer_number)
	{
		scrollh_reset_sprite();
		scrollh_layer_number = high_layer;
	}

	if (object_palette_is_dirty) object_delete_dirty_palette();
	if (scroll1_palette_is_dirty) scroll1_delete_dirty_palette();
	if (scroll2_palette_is_dirty) scroll2_delete_dirty_palette();
	if (scroll3_palette_is_dirty) scroll3_delete_dirty_palette();
	memset(palette_dirty_marks, 0, sizeof(palette_dirty_marks));

	object_num  = 0;
	scroll1_num = 0;
	scroll2_num = 0;
	scroll3_num = 0;
	scrollh_num = 0;

	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(0, 0, 512, 256);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuTexMode(GU_PSM_5551, 0, 0, GU_TRUE);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	スプライト描画終了
------------------------------------------------------------------------*/

void blit_finish(void)
{
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
	OBJECTテクスチャを更新
------------------------------------------------------------------------*/

void blit_update_object(int x, int y, u32 code, u32 attr)
{
	if ((x > 47 && x < 448) && (y > 0 && y < 239))
	{
		u32 key = MAKE_KEY(code, attr);
		SPRITE *p = object_head[key & OBJECT_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				p->used = frames_displayed;
				return;
		 	}
			p = p->next;
		}
	}
}


/*------------------------------------------------------------------------
	OBJECTを描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_object(int x, int y, u32 code, u32 attr)
{
	if ((x > 47 && x < 448) && (y > 0 && y < 239))
	{
		int idx;
		struct Vertex *vertices;
		u32 key = MAKE_KEY(code, attr);

		if ((idx = object_get_sprite(key)) < 0)
		{
			int lines = 16;
			u16 *dst, *pal;
			u32 *gfx, tile;
			u16 color = attr & 0x1f;

			if (object_texture_num == object_max - 1)
			{
				cps1_scan_object();
				object_delete_sprite();
			}

			idx = object_insert_sprite(key);
			gfx = (u32 *)&gfx_object[code << 7];
			pal = &video_palette[color << 4];
			dst = SWIZZLED_16x16(tex_object, idx);

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

		vertices = &vertices_object[object_num];

		vertices[0].u = vertices[1].u = (idx & 31) << 4;
		vertices[0].v = vertices[1].v = (idx >> 5) << 4;

		attr ^= 0x60;
		vertices[(attr & 0x20) >> 5].u += 16;
		vertices[(attr & 0x40) >> 6].v += 16;

		vertices[0].x = x;
		vertices[1].x = x + 16;

		vertices[0].y = y;
		vertices[1].y = y + 16;

		object_num += 2;
	}
}


/*------------------------------------------------------------------------
	OBJECT描画終了
------------------------------------------------------------------------*/

void blit_finish_object(void)
{
	struct Vertex *vertices;

	if (!object_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, 16, 448, 240);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_object);

	vertices = (struct Vertex *)sceGuGetMemory(object_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_object, object_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, object_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	SCROLL1テクスチャを更新
------------------------------------------------------------------------*/

void blit_update_scroll1(int x, int y, u32 code, u32 attr)
{
	u32 key = MAKE_KEY(code, attr);
	SPRITE *p = scroll1_head[key & SCROLL1_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			p->used = frames_displayed;
			return;
		}
		p = p->next;
	}
}


/*------------------------------------------------------------------------
	SCROLL1を描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_scroll1(int x, int y, u32 code, u32 attr, int gfxset)
{
	int idx;
	struct Vertex *vertices;
	u32 key = MAKE_KEY(code, attr);

	if ((idx = scroll1_get_sprite(key)) < 0)
	{
		int lines = 8;
		u16 *dst, *pal;
		u32 *gfx, tile;
		u16 color = (attr & 0x1f) + 32;

		if (scroll1_texture_num == scroll1_max - 1)
		{
			cps1_scan_scroll1();
			scroll1_delete_sprite();
		}

		idx = scroll1_insert_sprite(key);
		gfx = (u32 *)&gfx_scroll1[(code << 6) + (gfxset << 2)];
		pal = &video_palette[color << 4];
		dst = SWIZZLED_8x8(tex_scroll1, idx);

		while (lines--)
		{
			tile = *gfx;
			dst[0] = pal[tile & 0x0f]; tile >>= 4;
			dst[1] = pal[tile & 0x0f]; tile >>= 4;
			dst[2] = pal[tile & 0x0f]; tile >>= 4;
			dst[3] = pal[tile & 0x0f]; tile >>= 4;
			dst[4] = pal[tile & 0x0f]; tile >>= 4;
			dst[5] = pal[tile & 0x0f]; tile >>= 4;
			dst[6] = pal[tile & 0x0f]; tile >>= 4;
			dst[7] = pal[tile & 0x0f];
			gfx += 2;
			dst += 8;
		}
	}

	vertices = &vertices_scroll1[scroll1_num];

	vertices[0].u = vertices[1].u = (idx & 63) << 3;
	vertices[0].v = vertices[1].v = (idx >> 6) << 3;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 8;
	vertices[(attr & 0x40) >> 6].v += 8;

	vertices[0].x = x;
	vertices[1].x = x + 8;

	vertices[0].y = y;
	vertices[1].y = y + 8;

	scroll1_num += 2;
}


/*------------------------------------------------------------------------
	SCROLL1描画終了
------------------------------------------------------------------------*/

void blit_finish_scroll1(void)
{
	struct Vertex *vertices;

	if (!scroll1_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, 16, 448, 240);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scroll1);

	vertices = (struct Vertex *)sceGuGetMemory(scroll1_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_scroll1, scroll1_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, scroll1_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	SCROLL1(ハイレイヤー)を描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_scroll1h(int x, int y, u32 code, u32 attr, u16 tpens, int gfxset)
{
	int idx;
	struct Vertex *vertices;
	u32 key = MAKE_HIGH_KEY(code, attr);

	if ((idx = scrollh_get_sprite(key)) < 0)
	{
		int lines = 8;
		u16 *dst, *pal, pal2[16];
		u32 *gfx, tile;
		u16 color = (attr & 0x1f) + 32;

		if (scrollh_texture_num == scroll1h_max - 1)
		{
			cps1_scan_scroll1_foreground();
			scrollh_delete_sprite();
		}

		idx = scrollh_insert_sprite(key);
		gfx = (u32 *)&gfx_scroll1[(code << 6) + (gfxset << 2)];
		pal = &video_palette[color << 4];
		dst = SWIZZLED_8x8(tex_scrollh, idx);

		if (tpens != 0x7fff)
		{
			int i;

			for (i = 0; i < 15; i++)
				pal2[i] = (tpens & (1 << i)) ? pal[i] : 0x8000;
			pal2[15] = 0x8000;
			pal = pal2;
		}

		while (lines--)
		{
			tile = *gfx;
			dst[0] = pal[tile & 0x0f]; tile >>= 4;
			dst[1] = pal[tile & 0x0f]; tile >>= 4;
			dst[2] = pal[tile & 0x0f]; tile >>= 4;
			dst[3] = pal[tile & 0x0f]; tile >>= 4;
			dst[4] = pal[tile & 0x0f]; tile >>= 4;
			dst[5] = pal[tile & 0x0f]; tile >>= 4;
			dst[6] = pal[tile & 0x0f]; tile >>= 4;
			dst[7] = pal[tile & 0x0f];
			gfx += 2;
			dst += 8;
		}
	}

	vertices = &vertices_scrollh[scrollh_num];

	vertices[0].u = vertices[1].u = (idx & 63) << 3;
	vertices[0].v = vertices[1].v = (idx >> 6) << 3;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 8;
	vertices[(attr & 0x40) >> 6].v += 8;

	vertices[0].x = x;
	vertices[1].x = x + 8;

	vertices[0].y = y;
	vertices[1].y = y + 8;

	scrollh_num += 2;
}


/*------------------------------------------------------------------------
	SCROLL2 Y軸描画範囲設定
------------------------------------------------------------------------*/

void blit_set_clip_scroll2(int min_y, int max_y)
{
	scroll2_min_y = min_y;
	scroll2_max_y = max_y + 1;
}


/*------------------------------------------------------------------------
	SCROLL2テクスチャを更新
------------------------------------------------------------------------*/

void blit_update_scroll2(int x, int y, u32 code, u32 attr)
{
	if (y + 16 > scroll2_min_y && y < scroll2_max_y)
	{
		u32 key = MAKE_KEY(code, attr);
		SPRITE *p = scroll2_head[key & SCROLL2_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				p->used = frames_displayed;
				return;
			}
			p = p->next;
		}
	}
}


/*------------------------------------------------------------------------
	SCROLL2を描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_scroll2(int x, int y, u32 code, u32 attr)
{
	if (y + 16 > scroll2_min_y && y < scroll2_max_y)
	{
		int idx;
		struct Vertex *vertices;
		u32 key = MAKE_KEY(code, attr);

		if ((idx = scroll2_get_sprite(key)) < 0)
		{
			int lines = 16;
			u16 *dst, *pal;
			u32 *gfx, tile;
			u16 color = (attr & 0x1f) + 64;

			if (scroll2_texture_num == scroll2_max - 1)
			{
				cps1_scan_scroll2();
				scroll2_delete_sprite();
			}

			idx = scroll2_insert_sprite(key);
			gfx = (u32 *)&gfx_scroll2[code << 7];
			pal = &video_palette[color << 4];
			dst = SWIZZLED_16x16(tex_scroll2, idx);

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

		vertices = &vertices_scroll2[scroll2_num];

		vertices[0].u = vertices[1].u = (idx & 31) << 4;
		vertices[0].v = vertices[1].v = (idx >> 5) << 4;

		attr ^= 0x60;
		vertices[(attr & 0x20) >> 5].u += 16;
		vertices[(attr & 0x40) >> 6].v += 16;

		vertices[0].x = x;
		vertices[1].x = x + 16;

		vertices[0].y = y;
		vertices[1].y = y + 16;

		scroll2_num += 2;
	}
}


/*------------------------------------------------------------------------
	SCROLL2描画終了
------------------------------------------------------------------------*/

void blit_finish_scroll2(void)
{
	struct Vertex *vertices;

	if (!scroll2_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, scroll2_min_y, 448, scroll2_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scroll2);

	vertices = (struct Vertex *)sceGuGetMemory(scroll2_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_scroll2, scroll2_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, scroll2_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);

	scroll2_num = 0;
}


/*------------------------------------------------------------------------
	SCROLL2(ハイレイヤー)テクスチャを更新
------------------------------------------------------------------------*/

void blit_update_scroll2h(int x, int y, u32 code, u32 attr)
{
	if (y + 16 > scroll2_min_y && y < scroll2_max_y)
	{
		u32 key = MAKE_HIGH_KEY(code, attr);
		SPRITE *p = scrollh_head[key & SCROLLH_HASH_MASK];

		while (p)
		{
			if (p->key == key)
			{
				p->used = frames_displayed;
				return;
			}
			p = p->next;
		}
	}
}


/*------------------------------------------------------------------------
	SCROLL2(ハイレイヤー)を描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_scroll2h(int x, int y, u32 code, u32 attr, u16 tpens)
{
	if (y + 16 > scroll2_min_y && y < scroll2_max_y)
	{
		int idx;
		struct Vertex *vertices;
		u32 key = MAKE_HIGH_KEY(code, attr);

		if ((idx = scrollh_get_sprite(key)) < 0)
		{
			int lines = 16;
			u16 *dst, *pal, pal2[16];
			u32 *gfx, tile;
			u16 color = (attr & 0x1f) + 64;

			if (scrollh_texture_num == scroll2h_max - 1)
			{
				cps1_scan_scroll2_foreground();
				scrollh_delete_sprite();
			}

			idx = scrollh_insert_sprite(key);
			gfx = (u32 *)&gfx_scroll2[code << 7];
			pal = &video_palette[color << 4];
			dst = SWIZZLED_16x16(tex_scrollh, idx);

			if (tpens != 0x7fff)
			{
				int i;

				for (i = 0; i < 15; i++)
					pal2[i] = (tpens & (1 << i)) ? pal[i] : 0x8000;
				pal2[15] = 0x8000;
				pal = pal2;
			}

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

		vertices = &vertices_scrollh[scrollh_num];

		vertices[0].u = vertices[1].u = (idx & 31) << 4;
		vertices[0].v = vertices[1].v = (idx >> 5) << 4;

		attr ^= 0x60;
		vertices[(attr & 0x20) >> 5].u += 16;
		vertices[(attr & 0x40) >> 6].v += 16;

		vertices[0].x = x;
		vertices[1].x = x + 16;

		vertices[0].y = y;
		vertices[1].y = y + 16;

		scrollh_num += 2;
	}
}


/*------------------------------------------------------------------------
	SCROLL2(ハイレイヤー)描画終了
------------------------------------------------------------------------*/

void blit_finish_scroll2h(void)
{
	struct Vertex *vertices;

	if (!scrollh_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, scroll2_min_y, 448, scroll2_max_y);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scrollh);

	vertices = (struct Vertex *)sceGuGetMemory(scrollh_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_scrollh, scrollh_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, scrollh_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);

	scrollh_num = 0;
}


/*------------------------------------------------------------------------
	SCROLL3テクスチャを更新
------------------------------------------------------------------------*/

void blit_update_scroll3(int x, int y, u32 code, u32 attr)
{
	u32 key = MAKE_KEY_SCROLL3(code, attr);
	SPRITE *p = scroll3_head[key & SCROLL3_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			p->used = frames_displayed;
			return;
		}
		p = p->next;
	}
}


/*------------------------------------------------------------------------
	SCROLL3を描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_scroll3(int x, int y, u32 code, u32 attr)
{
	int idx;
	struct Vertex *vertices;
	u32 key = MAKE_KEY_SCROLL3(code, attr);

	if ((idx = scroll3_get_sprite(key)) < 0)
	{
		int lines = 32;
		u16 *dst, *pal;
		u32 *gfx, tile;
		u16 color = (attr & 0x1f) + 96;

		if (scroll3_texture_num == scroll3_max - 1)
		{
			cps1_scan_scroll3();
			scroll3_delete_sprite();
		}

		idx = scroll3_insert_sprite(key);
		gfx = (u32 *)&gfx_scroll3[code << 9];
		pal = &video_palette[color << 4];
		dst = SWIZZLED_32x32(tex_scroll3, idx);

		while (lines--)
		{
			tile = *gfx++;
			dst[  0] = pal[tile & 0x0f]; tile >>= 4;
			dst[  1] = pal[tile & 0x0f]; tile >>= 4;
			dst[  2] = pal[tile & 0x0f]; tile >>= 4;
			dst[  3] = pal[tile & 0x0f]; tile >>= 4;
			dst[  4] = pal[tile & 0x0f]; tile >>= 4;
			dst[  5] = pal[tile & 0x0f]; tile >>= 4;
			dst[  6] = pal[tile & 0x0f]; tile >>= 4;
			dst[  7] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[ 64] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 65] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 66] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 67] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 68] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 69] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 70] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 71] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[128] = pal[tile & 0x0f]; tile >>= 4;
			dst[129] = pal[tile & 0x0f]; tile >>= 4;
			dst[130] = pal[tile & 0x0f]; tile >>= 4;
			dst[131] = pal[tile & 0x0f]; tile >>= 4;
			dst[132] = pal[tile & 0x0f]; tile >>= 4;
			dst[133] = pal[tile & 0x0f]; tile >>= 4;
			dst[134] = pal[tile & 0x0f]; tile >>= 4;
			dst[135] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[192] = pal[tile & 0x0f]; tile >>= 4;
			dst[193] = pal[tile & 0x0f]; tile >>= 4;
			dst[194] = pal[tile & 0x0f]; tile >>= 4;
			dst[195] = pal[tile & 0x0f]; tile >>= 4;
			dst[196] = pal[tile & 0x0f]; tile >>= 4;
			dst[197] = pal[tile & 0x0f]; tile >>= 4;
			dst[198] = pal[tile & 0x0f]; tile >>= 4;
			dst[199] = pal[tile & 0x0f];
			dst += swizzle_table[lines];
		}
	}

	vertices = &vertices_scroll3[scroll3_num];

	vertices[0].u = vertices[1].u = (idx & 15) << 5;
	vertices[0].v = vertices[1].v = (idx >> 4) << 5;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 32;
	vertices[(attr & 0x40) >> 6].v += 32;

	vertices[0].x = x;
	vertices[1].x = x + 32;

	vertices[0].y = y;
	vertices[1].y = y + 32;

	scroll3_num += 2;
}


/*------------------------------------------------------------------------
	SCROLL3描画終了
------------------------------------------------------------------------*/

void blit_finish_scroll3(void)
{
	struct Vertex *vertices;

	if (!scroll3_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, 16, 448, 240);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scroll3);

	vertices = (struct Vertex *)sceGuGetMemory(scroll3_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_scroll3, scroll3_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, scroll3_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}


/*------------------------------------------------------------------------
	SCROLL3(ハイレイヤー)を描画リストに登録
------------------------------------------------------------------------*/

void blit_draw_scroll3h(int x, int y, u32 code, u32 attr, u16 tpens)
{
	int idx;
	struct Vertex *vertices;
	u32 key = MAKE_HIGH_KEY(code, attr);

	if ((idx = scrollh_get_sprite(key)) < 0)
	{
		int lines = 32;
		u16 *dst, *pal, pal2[16];
		u32 *gfx, tile;
		u16 color = (attr & 0x1f) + 96;

		if (scrollh_texture_num == scroll3h_max - 1)
		{
			cps1_scan_scroll3_foreground();
			scrollh_delete_sprite();
		}

		idx = scrollh_insert_sprite(key);
		gfx = (u32 *)&gfx_scroll3[code << 9];
		pal = &video_palette[color << 4];
		dst = SWIZZLED_32x32(tex_scrollh, idx);

		if (tpens != 0x7fff)
		{
			int i;

			for (i = 0; i < 15; i++)
				pal2[i] = (tpens & (1 << i)) ? pal[i] : 0x8000;
			pal2[15] = 0x8000;
			pal = pal2;
		}

		while (lines--)
		{
			tile = *gfx++;
			dst[  0] = pal[tile & 0x0f]; tile >>= 4;
			dst[  1] = pal[tile & 0x0f]; tile >>= 4;
			dst[  2] = pal[tile & 0x0f]; tile >>= 4;
			dst[  3] = pal[tile & 0x0f]; tile >>= 4;
			dst[  4] = pal[tile & 0x0f]; tile >>= 4;
			dst[  5] = pal[tile & 0x0f]; tile >>= 4;
			dst[  6] = pal[tile & 0x0f]; tile >>= 4;
			dst[  7] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[ 64] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 65] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 66] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 67] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 68] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 69] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 70] = pal[tile & 0x0f]; tile >>= 4;
			dst[ 71] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[128] = pal[tile & 0x0f]; tile >>= 4;
			dst[129] = pal[tile & 0x0f]; tile >>= 4;
			dst[130] = pal[tile & 0x0f]; tile >>= 4;
			dst[131] = pal[tile & 0x0f]; tile >>= 4;
			dst[132] = pal[tile & 0x0f]; tile >>= 4;
			dst[133] = pal[tile & 0x0f]; tile >>= 4;
			dst[134] = pal[tile & 0x0f]; tile >>= 4;
			dst[135] = pal[tile & 0x0f];
			tile = *gfx++;
			dst[192] = pal[tile & 0x0f]; tile >>= 4;
			dst[193] = pal[tile & 0x0f]; tile >>= 4;
			dst[194] = pal[tile & 0x0f]; tile >>= 4;
			dst[195] = pal[tile & 0x0f]; tile >>= 4;
			dst[196] = pal[tile & 0x0f]; tile >>= 4;
			dst[197] = pal[tile & 0x0f]; tile >>= 4;
			dst[198] = pal[tile & 0x0f]; tile >>= 4;
			dst[199] = pal[tile & 0x0f];
			dst += swizzle_table[lines];
		}
	}

	vertices = &vertices_scrollh[scrollh_num];

	vertices[0].u = vertices[1].u = (idx & 15) << 5;
	vertices[0].v = vertices[1].v = (idx >> 4) << 5;

	attr ^= 0x60;
	vertices[(attr & 0x20) >> 5].u += 32;
	vertices[(attr & 0x40) >> 6].v += 32;

	vertices[0].x = x;
	vertices[1].x = x + 32;

	vertices[0].y = y;
	vertices[1].y = y + 32;

	scrollh_num += 2;
}


/*------------------------------------------------------------------------
	SCROLL1,3(ハイレイヤー)テクスチャを更新
------------------------------------------------------------------------*/

void blit_update_scrollh(int x, int y, u32 code, u32 attr)
{
	u32 key = MAKE_HIGH_KEY(code, attr);
	SPRITE *p = scrollh_head[key & SCROLLH_HASH_MASK];

	while (p)
	{
		if (p->key == key)
		{
			p->used = frames_displayed;
			return;
		}
		p = p->next;
	}
}


/*------------------------------------------------------------------------
	SCROLL1,3(ハイレイヤー)描画終了
------------------------------------------------------------------------*/

void blit_finish_scrollh(void)
{
	struct Vertex *vertices;

	if (!scrollh_num) return;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, work_frame, BUF_WIDTH);
	sceGuScissor(64, 16, 448, 240);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, tex_scrollh);

	vertices = (struct Vertex *)sceGuGetMemory(scrollh_num * sizeof(struct Vertex));
	memcpy(vertices, vertices_scrollh, scrollh_num * sizeof(struct Vertex));
	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, scrollh_num, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}
