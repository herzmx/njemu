/******************************************************************************

	cache.c

	�������L���b�V���C���^�t�F�[�X�֐�

******************************************************************************/

#include "emumain.h"

#if USE_CACHE

#define MIN_CACHE_SIZE		0x40		// ����  4MB
#define MAX_CACHE_SIZE		0x140		// ��� 20MB
#define BLOCK_SIZE			0x10000		// 1�u���b�N�̃T�C�Y = 64KB
#define BLOCK_NOT_CACHED	-1
#define CACHE_SAFETY		0x18000

#if (EMU_SYSTEM == CPS2)
#define GFX_MEMORY	memory_region_gfx1
#define GFX_SIZE	memory_length_gfx1
#elif (EMU_SYSTEM == MVS)
#define GFX_MEMORY	memory_region_gfx3
#define GFX_SIZE	memory_length_gfx3
#endif


/******************************************************************************
	�O���[�o���\����/�ϐ�
******************************************************************************/

u32 (*read_cache)(u32 offset);
void (*update_cache)(u32 offset);
u8 ALIGN_DATA block_empty[MAX_CACHE_BLOCKS];


/******************************************************************************
	���[�J���\����/�ϐ�
******************************************************************************/

typedef struct cache_s
{
	int idx;
	int block;
	struct cache_s *prev;
	struct cache_s *next;
} cache_t;


static cache_t ALIGN_DATA cache_data[MAX_CACHE_SIZE];
static cache_t *head;
static cache_t *tail;

static s16 ALIGN_DATA blocks[MAX_CACHE_BLOCKS];
static int num_cache;
static int cache_fd;
static char cache_name[16];


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*------------------------------------------------------
	�L���b�V���t�@�C�����̃f�[�^�t�@�C�����J��
------------------------------------------------------*/

static int cache_open(int number)
{
	static const char cnv_table[16] =
	{
		'0','1','2','3','4','5','6','7',
		'8','9','a','b','c','d','e','f'
	};
	char fname[4];

	fname[0] = cnv_table[(number >> 8) & 0x0f];
	fname[1] = cnv_table[(number >> 4) & 0x0f];
	fname[2] = cnv_table[ number       & 0x0f];
	fname[3] = '\0';

	cache_fd = zopen(fname);

	return cache_fd != -1;
}


/*------------------------------------------------------
	�L���b�V���t�@�C�����̃f�[�^�t�@�C���ǂݍ���
------------------------------------------------------*/

#define cache_load(offs)	zread(cache_fd, &GFX_MEMORY[offs << 16], 0x10000)


/*------------------------------------------------------
	�L���b�V���t�@�C�����̃f�[�^�t�@�C�������
------------------------------------------------------*/

#define cache_close()		zclose(cache_fd)


/*------------------------------------------------------
	�L���b�V�����f�[�^�Ŗ��߂�

	3��ނ̃f�[�^�����݂��Ă��邽�߁A�̈����؂���
	���ꂼ��K���ȃT�C�Y��ǂݍ��ނׂ��ł����A���
	�����Đ擪����ǂݍ���ł��邾���ɂȂ��Ă��܂��B
------------------------------------------------------*/

static int fill_cache(void)
{
	int i, block;
	cache_t *p;

	i = 0;
	block = 0;

	while (i < num_cache)
	{
#if (EMU_SYSTEM != MVS)
		if (!block_empty[block])
#endif
		{
			p = head;
			p->block = block;
			blocks[block] = p->idx;

			if (!cache_open(p->block))
			{
				msg_printf("ERROR: Could not open sprite block %03x\n", p->block);
				return 0;
			}
			cache_load(p->idx);
			cache_close();

			head = p->next;
			head->prev = NULL;

			p->prev = tail;
			p->next = NULL;

			tail->next = p;
			tail = p;
			i++;
		}

		if (++block >= MAX_CACHE_BLOCKS)
			break;
	}

	return 1;
}


/*------------------------------------------------------
	�L���b�V������f�[�^��ǂݍ���

	�L���b�V�����g�p���Ȃ��ꍇ�B
	���̂܂܃A�h���X��Ԃ��B
------------------------------------------------------*/

static u32 read_cache_disable(u32 offset)
{
	return offset;
}


/*------------------------------------------------------
	�L���b�V������f�[�^��ǂݍ���

	�T�C�Y���傫���ă������Ɏ��܂�Ȃ��ꍇ�͂�����
------------------------------------------------------*/

static u32 read_cache_dynamic(u32 offset)
{
	s16 new_block = offset >> 16;
	int idx = blocks[new_block];
	cache_t *p;

	if (idx == BLOCK_NOT_CACHED)
	{
		if (!cache_open(new_block))
			return 0;

		p = head;
		blocks[p->block] = BLOCK_NOT_CACHED;

		p->block = new_block;
		blocks[new_block] = p->idx;

		cache_load(p->idx);
		cache_close();
	}
	else p = &cache_data[idx];

	if (p->next)
	{
		if (p->prev)
		{
			p->prev->next = p->next;
			p->next->prev = p->prev;
		}
		else
		{
			head = p->next;
			head->prev = NULL;
		}

		p->prev = tail;
		p->next = NULL;

		tail->next = p;
		tail = p;
	}

	return ((tail->idx << 16) | (offset & 0xffff));
}


/*------------------------------------------------------
	�L���b�V������f�[�^��ǂݍ���

	�S�ēǂݍ��񂾏ꍇ�͂�����B�L���b�V���̊Ǘ���
	�s���܂���B
------------------------------------------------------*/

static u32 read_cache_static(u32 offset)
{
	int idx = blocks[offset >> 16];

	return ((idx << 16) | (offset & 0xffff));
}


/*------------------------------------------------------
	�L���b�V���̃f�[�^���X�V����

	�L���b�V�����g�p���Ȃ��A�܂��͑S�ēǂݍ��񂾏ꍇ�B
------------------------------------------------------*/

static void update_cache_disable(u32 offset)
{
}


/*------------------------------------------------------
	�L���b�V���̃f�[�^���X�V����

	�w�肳�ꂽ�f�[�^���L���b�V���̍Ō���ɉ񂵂܂��B
	�L���b�V�����Ǘ����Ȃ��ꍇ�͕s�v�B
------------------------------------------------------*/

static void update_cache_dynamic(u32 offset)
{
	s16 new_block = offset >> 16;
	int idx = blocks[new_block];

	if (idx != BLOCK_NOT_CACHED)
	{
		cache_t *p = &cache_data[idx];

		if (p->next)
		{
			if (p->prev)
			{
				p->prev->next = p->next;
				p->next->prev = p->prev;
			}
			else
			{
				head = p->next;
				head->prev = NULL;
			}

			p->prev = tail;
			p->next = NULL;

			tail->next = p;
			tail = p;
		}
	}
}


/*------------------------------------------------------
	�L���b�V���̃f�[�^���X�V����

	�w�肳�ꂽ�f�[�^���L���b�V���̍Ō���ɉ񂵁A
	�K�v�ɉ����ăt�@�C�����ǂݍ��݂܂��B
	�L���b�V�����Ǘ����Ȃ��ꍇ�͕s�v�B
------------------------------------------------------*/

static void update_cache_reload(u32 offset)
{
	s16 new_block = offset >> 16;
	int idx = blocks[new_block];
	cache_t *p;

	if (idx == BLOCK_NOT_CACHED)
	{
		if (!cache_open(new_block))
			return;

		p = head;
		blocks[p->block] = BLOCK_NOT_CACHED;

		p->block = new_block;
		blocks[new_block] = p->idx;

		cache_load(p->idx);
		cache_close();
	}
	else p = &cache_data[idx];

	if (p->next)
	{
		if (p->prev)
		{
			p->prev->next = p->next;
			p->next->prev = p->prev;
		}
		else
		{
			head = p->next;
			head->prev = NULL;
		}

		p->prev = tail;
		p->next = NULL;

		tail->next = p;
		tail = p;
	}
}


/******************************************************************************
	�L���b�V���C���^�t�F�[�X�֐�
******************************************************************************/

/*------------------------------------------------------
	�L���b�V��������������
------------------------------------------------------*/

void cache_init(void)
{
	int i;

	num_cache  = 0;
	cache_fd = -1;

	read_cache   = read_cache_disable;
	update_cache = update_cache_disable;

	for (i = 0; i < MAX_CACHE_BLOCKS; i++)
		blocks[i] = BLOCK_NOT_CACHED;
}


/*------------------------------------------------------
	�L���b�V�������J�n
------------------------------------------------------*/

int cache_start(void)
{
	int i, found = 0;
	u32 size = 0;
	char path[MAX_PATH];

	strcpy(cache_name, game_name);
	sprintf(path, "%s/%s_cache.zip", cache_dir, cache_name);
	if (zip_open(path) != -1)
	{
		found = 1;
	}

	if (!found && cache_parent_name[0])
	{
		strcpy(cache_name, cache_parent_name);
		sprintf(path, "%s/%s_cache.zip", cache_dir, cache_name);
		if (zip_open(path) != -1)
		{
			found = 1;
		}
	}

	if (!found)
	{
		msg_printf("ERROR: Could not open cache file.\n");
		return 0;
	}

	if ((GFX_MEMORY = (u8 *)memalign(64, GFX_SIZE + CACHE_SAFETY)) != NULL)
	{
		free(GFX_MEMORY);
		GFX_MEMORY = (u8 *)memalign(64, GFX_SIZE);
		memset(GFX_MEMORY, 0, GFX_SIZE);

		read_cache   = read_cache_static;
		update_cache = update_cache_disable;
		num_cache = GFX_SIZE >> 16;
	}
	else
	{
		read_cache   = read_cache_dynamic;
		update_cache = update_cache_dynamic;

		// �m�ۉ\�ȃT�C�Y���`�F�b�N
		for (i = GFX_SIZE >> 16; i >= MIN_CACHE_SIZE; i--)
		{
			if ((GFX_MEMORY = (u8 *)memalign(64, (i << 16) + CACHE_SAFETY)) != NULL)
			{
				size = i << 16;
				free(GFX_MEMORY);
				GFX_MEMORY = NULL;
				break;
			}
		}

		if (i < MIN_CACHE_SIZE)
		{
			msg_printf("ERROR: memory not enough.\n");
			return 0;
		}

		if ((GFX_MEMORY = (u8 *)memalign(64, size)) == NULL)
		{
			msg_printf("ERROR: Could not allocate cache memory.\n");
			return 0;
		}
		memset(GFX_MEMORY, 0, size);

		num_cache = i;
	}

	msg_printf("%dKB cache allocated.\n", (num_cache << 16) / 1024);

	for (i = 0; i < num_cache; i++)
		cache_data[i].idx = i;

	for (i = 1; i < num_cache; i++)
		cache_data[i].prev = &cache_data[i - 1];

	for (i = 0; i < num_cache - 1; i++)
		cache_data[i].next = &cache_data[i + 1];

	cache_data[0].prev = NULL;
	cache_data[num_cache - 1].next = NULL;

	head = &cache_data[0];
	tail = &cache_data[num_cache - 1];

	if (!fill_cache())
	{
		msg_printf("Cache load error!!!\n");
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (size == 0)
	{
		zip_close();
		num_cache = 0;
	}

	msg_printf("Complete.\n");

	return 1;
}


/*------------------------------------------------------
	�L���b�V�������I��
------------------------------------------------------*/

void cache_shutdown(void)
{
	zip_close();
	num_cache = 0;
}


/*------------------------------------------------------
	�L���b�V�����ꎞ�I�ɒ�~/�ĊJ����
------------------------------------------------------*/

void cache_sleep(int flag)
{
	if (num_cache)
	{
		if (flag)
		{
			zip_close();
		}
		else
		{
			char path[MAX_PATH];

			sprintf(path, "%s/%s_cache.zip", cache_dir, cache_name);
			zip_open(path);
		}
	}
}


/*------------------------------------------------------
	�L���b�V���X�V�֐���؂�ւ���

	�X�e�[�g���[�h�p�ł�
------------------------------------------------------*/

void cache_set_update_func(int flag)
{
	if (num_cache)
	{
		if (flag)
			update_cache = update_cache_reload;
		else
			update_cache = update_cache_dynamic;
	}
}

#endif /* USE_CACHE */
