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
#define CACHE_SAFETY		0x20000

#if (EMU_SYSTEM == CPS2)
#define GFX_MEMORY		memory_region_gfx1
#define GFX_SIZE		memory_length_gfx1
#define CHECK_FNAME		"block_empty"
#elif (EMU_SYSTEM == MVS)
#define GFX_MEMORY		memory_region_gfx3
#define GFX_SIZE		memory_length_gfx3
#endif


/******************************************************************************
	�O���[�o���\����/�ϐ�
******************************************************************************/

u32 (*read_cache)(u32 offset);
void (*update_cache)(u32 offset);
#if (EMU_SYSTEM != MVS)
u32 block_offset[MAX_CACHE_BLOCKS];
u8  *block_empty = (u8 *)block_offset;
#endif
int cache_type;


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

static int num_cache;
static s16 ALIGN_DATA blocks[MAX_CACHE_BLOCKS];
static char spr_cache_name[MAX_PATH];
static int cache_fd;

#if (EMU_SYSTEM == MVS)
int pcm_cache_enable;
static SceUID pcm_fd;
static char pcm_cache_name[MAX_PATH];
static u8 *pcm_cache[7];
static u16 pcm_cache_block[7];
#endif


/******************************************************************************
	���[�J���֐�
******************************************************************************/

#if (EMU_SYSTEM == MVS)

/*------------------------------------------------------
	PCM�L���b�V�����擾
------------------------------------------------------*/

u8 *pcm_get_cache(int ch)
{
	return pcm_cache[ch];
}


/*------------------------------------------------------
	PCM�L���b�V����ǂݍ���
------------------------------------------------------*/

u8 pcm_cache_read(int ch, u32 offset)
{
	u16 block = (offset >> 16);

	if (pcm_cache_block[ch] != block)
	{
		sceIoLseek(pcm_fd, offset & ~0xffff, PSP_SEEK_SET);
		sceIoRead(pcm_fd, pcm_cache[ch], PCM_CACHE_SIZE);
		pcm_cache_block[ch] = block;
	}
	return pcm_cache[ch][offset & 0xffff];
}


#else

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


#endif

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

#if (EMU_SYSTEM == MVS)
	while (i < num_cache)
	{
		p = head;
		p->block = block;
		blocks[block] = p->idx;

		sceIoLseek(cache_fd, block << 16, SEEK_SET);
		sceIoRead(cache_fd, &GFX_MEMORY[p->idx << 16], 0x10000);

		head = p->next;
		head->prev = NULL;

		p->prev = tail;
		p->next = NULL;

		tail->next = p;
		tail = p;
		i++;

		if (++block >= MAX_CACHE_BLOCKS)
			break;
	}
#else
	if (cache_type == CACHE_RAWFILE)
	{
		while (i < num_cache)
		{
			if (block_offset[block] != 0xffffffff)
			{
				p = head;
				p->block = block;
				blocks[block] = p->idx;

				sceIoLseek(cache_fd, block_offset[block], SEEK_SET);
				sceIoRead(cache_fd, &GFX_MEMORY[p->idx << 16], 0x10000);

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
	}
	else
	{
		while (i < num_cache)
		{
			if (!block_empty[block])
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
	}
#endif

	return 1;
}


/*------------------------------------------------------
	�L���b�V�����g�p���Ȃ�

	�f�[�^���S�ă������Ɋi�[����Ă���ꍇ
------------------------------------------------------*/

#if (EMU_SYSTEM == MVS)
static u32 read_cache_disable(u32 offset)
{
	return offset;
}
#endif


/*------------------------------------------------------
	�A�h���X�ϊ��̂ݍs��

	�󂫗̈���폜������Ԃł��A�S�ă������Ɋi�[�����
	����ꍇ
------------------------------------------------------*/

#if (EMU_SYSTEM == CPS2)
static u32 read_cache_static(u32 offset)
{
	int idx = blocks[offset >> 16];

	return ((idx << 16) | (offset & 0xffff));
}
#endif


/*------------------------------------------------------
	�����k�L���b�V�����g�p

	�����k�̃L���b�V���t�@�C������f�[�^��ǂݍ���
------------------------------------------------------*/

static u32 read_cache_rawfile(u32 offset)
{
	s16 new_block = offset >> 16;
	int idx = blocks[new_block];
	cache_t *p;

	if (idx == BLOCK_NOT_CACHED)
	{
		p = head;
		blocks[p->block] = BLOCK_NOT_CACHED;

		p->block = new_block;
		blocks[new_block] = p->idx;

#if (EMU_SYSTEM == MVS)
		sceIoLseek(cache_fd, new_block << 16, SEEK_SET);
#else
		sceIoLseek(cache_fd, block_offset[new_block], SEEK_SET);
#endif
		sceIoRead(cache_fd, &GFX_MEMORY[p->idx << 16], 0x10000);
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
	ZIP���k�L���b�V�����g�p

	ZIP���k�̃L���b�V���t�@�C������f�[�^��ǂݍ���
------------------------------------------------------*/

#if (EMU_SYSTEM == CPS2)
static u32 read_cache_zipfile(u32 offset)
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
#endif


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
	cache_fd   = -1;
	cache_type = CACHE_NOTFOUND;

#if (EMU_SYSTEM == MVS)
	pcm_cache_enable = 0;
	pcm_fd = -1;
	memset(pcm_cache, 0, sizeof(pcm_cache));
	memset(pcm_cache_block, 0xff, sizeof(pcm_cache_block));
#endif

#if (EMU_SYSTEM == MVS)
	read_cache   = read_cache_disable;
#else
	read_cache   = read_cache_static;
#endif
	update_cache = update_cache_disable;

	for (i = 0; i < MAX_CACHE_BLOCKS; i++)
		blocks[i] = BLOCK_NOT_CACHED;
}


/*------------------------------------------------------
	�L���b�V�������J�n
------------------------------------------------------*/

int cache_start(void)
{
	int i;
	u32 size = 0;
#if (EMU_SYSTEM == MVS)
	extern int disable_sound;
#else
	int found = 0;
#endif

	zip_close();

#if (EMU_SYSTEM == MVS)
	if (option_sound_enable && disable_sound)
	{
		sprintf(pcm_cache_name, "%s/%s_cache/vrom", cache_dir, game_name);
		if ((pcm_fd = sceIoOpen(pcm_cache_name, PSP_O_RDONLY, 0777)) >= 0)
		{
			pcm_cache_enable = 1;

			for (i = 0; i < 7; i++)
			{
				if ((pcm_cache[i] = memalign(64, 0x10000)) == NULL)
				{
					int j;

					for (j = 0; j < i; j++)
					{
						free(pcm_cache[j]);
						pcm_cache[j] = NULL;
					}
					pcm_cache_enable = 0;
					sceIoClose(pcm_fd);
					pcm_fd = -1;
					break;
				}
				memset(pcm_cache[i], 0, 0x10000);
			}
			if (pcm_cache_enable)
			{
				disable_sound = 0;
				msg_printf("PCM cache enabled.\n");
			}
		}
		if (!pcm_cache_enable) memory_length_sound1 = 0;
	}

	sprintf(spr_cache_name, "%s/%s_cache/crom", cache_dir, game_name);
	if ((cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777)) < 0)
	{
		msg_printf("ERROR: Could not open cache file.\n");
		return 0;
	}
#else
	if (cache_type == CACHE_RAWFILE)
	{
		sprintf(spr_cache_name, "%s/%s.cache", cache_dir, game_name);
		found = (cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777)) >= 0;
	}
	else
	{
		sprintf(spr_cache_name, "%s/%s_cache.zip", cache_dir, game_name);
		found = zip_open(spr_cache_name) != -1;
	}
	if (!found && cache_parent_name[0])
	{
		if (cache_type == CACHE_RAWFILE)
		{
			sprintf(spr_cache_name, "%s/%s.cache", cache_dir, cache_parent_name);
			found = (cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777)) >= 0;
		}
		else
		{
			sprintf(spr_cache_name, "%s/%s_cache.zip", cache_dir, cache_parent_name);
			found = zip_open(spr_cache_name) != -1;
		}
	}
	if (!found)
	{
		msg_printf("ERROR: Could not open cache file.\n");
		return 0;
	}
#endif

#if (EMU_SYSTEM == CPS2)
	if ((GFX_MEMORY = (u8 *)memalign(64, GFX_SIZE + CACHE_SAFETY)) != NULL)
	{
		free(GFX_MEMORY);
		GFX_MEMORY = (u8 *)memalign(64, GFX_SIZE);
		memset(GFX_MEMORY, 0, GFX_SIZE);

		num_cache = GFX_SIZE >> 16;
	}
	else
#endif
	{
#if (EMU_SYSTEM == MVS)
		read_cache = read_cache_rawfile;
#else
		if (cache_type == CACHE_RAWFILE)
			read_cache = read_cache_rawfile;
		else
			read_cache = read_cache_zipfile;
#endif
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

	if (size == 0) cache_shutdown();

	msg_printf("Complete.\n");

	return 1;
}


/*------------------------------------------------------
	�L���b�V�������I��
------------------------------------------------------*/

void cache_shutdown(void)
{
#if (EMU_SYSTEM == MVS)
	if (pcm_cache_enable)
	{
		int i;

		for (i = 0; i < 7; i++)
		{
			if (pcm_cache[i]) free(pcm_cache[i]);
		}
		if (pcm_fd != -1)
		{
			sceIoClose(pcm_fd);
		}
		pcm_cache_enable = 0;
	}
	if (cache_fd != -1)
	{
		sceIoClose(cache_fd);
		cache_fd = -1;
	}
#else
	if (cache_type == CACHE_RAWFILE)
	{
		if (cache_fd != -1)
		{
			sceIoClose(cache_fd);
			cache_fd = -1;
		}
	}
	else
	{
		zip_close();
	}
#endif
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
#if (EMU_SYSTEM == MVS)
			sceIoClose(cache_fd);
			if (pcm_cache_enable) sceIoClose(pcm_fd);
#else
			if (cache_type == CACHE_RAWFILE)
				sceIoClose(cache_fd);
			else
				zip_close();
#endif
		}
		else
		{
#if (EMU_SYSTEM == MVS)
			cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777);
			if (pcm_cache_enable)
				pcm_fd = sceIoOpen(pcm_cache_name, PSP_O_RDONLY, 0777);
#else
			if (cache_type == CACHE_RAWFILE)
				cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777);
			else
				zip_open(spr_cache_name);
#endif
		}
	}
}

#endif /* USE_CACHE */
