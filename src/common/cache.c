/******************************************************************************

	cache.c

	�������L���b�V���C���^�t�F�[�X�֐�

******************************************************************************/

#include "emumain.h"

#if USE_CACHE

#define MIN_CACHE_SIZE		0x40		// ����  4MB
#define MAX_CACHE_SIZE		0x140		// ��� 20MB
#define CACHE_SAFETY		0x20000		// �L���b�V���m�ی�̋󂫃������T�C�Y
#define BLOCK_SIZE			0x10000		// 1�u���b�N�̃T�C�Y = 64KB
#define BLOCK_MASK			0xffff
#define BLOCK_SHIFT			16
#define BLOCK_NOT_CACHED	0xffff
#define BLOCK_EMPTY			0xffffffff


/******************************************************************************
	�O���[�o���\����/�ϐ�
******************************************************************************/

u32 (*read_cache)(u32 offset);
void (*update_cache)(u32 offset);
#if (EMU_SYSTEM != MVS)
u32 block_offset[MAX_CACHE_BLOCKS];
u8  *block_empty = (u8 *)block_offset;
#endif


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
static u16 ALIGN_DATA blocks[MAX_CACHE_BLOCKS];
static char spr_cache_name[MAX_PATH];
static int cache_fd;

#if (EMU_SYSTEM == MVS)
int pcm_cache_enable;
static SceUID pcm_fd;
static char pcm_cache_name[MAX_PATH];
static u8 *pcm_cache[7];
static u16 ALIGN_DATA pcm_cache_block[7];
#else
static int cache_type;
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
	u16 block = offset >> PCM_CACHE_SHIFT;

	if (pcm_cache_block[ch] != block)
	{
		sceIoLseek(pcm_fd, offset & ~PCM_CACHE_MASK, PSP_SEEK_SET);
		sceIoRead(pcm_fd, pcm_cache[ch], PCM_CACHE_SIZE);
		pcm_cache_block[ch] = block;
	}
	return pcm_cache[ch][offset & PCM_CACHE_MASK];
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

		sceIoLseek(cache_fd, block << BLOCK_SHIFT, PSP_SEEK_SET);
		sceIoRead(cache_fd, &GFX_MEMORY[p->idx << BLOCK_SHIFT], BLOCK_SIZE);

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
			if (block_offset[block] != BLOCK_EMPTY)
			{
				p = head;
				p->block = block;
				blocks[block] = p->idx;

				sceIoLseek(cache_fd, block_offset[block], PSP_SEEK_SET);
				sceIoRead(cache_fd, &GFX_MEMORY[p->idx << BLOCK_SHIFT], BLOCK_SIZE);

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
					msg_printf(TEXT(COULD_NOT_OPEN_SPRITE_BLOCK_x), p->block);
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

	�󂫗̈���폜������ԂŁA�S�ă������Ɋi�[�����
	����ꍇ
------------------------------------------------------*/

#if (EMU_SYSTEM == CPS2)
static u32 read_cache_static(u32 offset)
{
	int idx = blocks[offset >> BLOCK_SHIFT];

	return ((idx << BLOCK_SHIFT) | (offset & BLOCK_MASK));
}
#endif


/*------------------------------------------------------
	�����k�L���b�V�����g�p

	�����k�̃L���b�V���t�@�C������f�[�^��ǂݍ���
------------------------------------------------------*/

static u32 read_cache_rawfile(u32 offset)
{
	s16 new_block = offset >> BLOCK_SHIFT;
	u32 idx = blocks[new_block];
	cache_t *p;

	if (idx == BLOCK_NOT_CACHED)
	{
		p = head;
		blocks[p->block] = BLOCK_NOT_CACHED;

		p->block = new_block;
		blocks[new_block] = p->idx;

#if (EMU_SYSTEM == MVS)
		sceIoLseek(cache_fd, new_block << BLOCK_SHIFT, PSP_SEEK_SET);
#else
		sceIoLseek(cache_fd, block_offset[new_block], PSP_SEEK_SET);
#endif
		sceIoRead(cache_fd, &GFX_MEMORY[p->idx << BLOCK_SHIFT], BLOCK_SIZE);
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

	return ((tail->idx << BLOCK_SHIFT) | (offset & BLOCK_MASK));
}


/*------------------------------------------------------
	ZIP���k�L���b�V�����g�p

	ZIP���k�̃L���b�V���t�@�C������f�[�^��ǂݍ���
------------------------------------------------------*/

#if (EMU_SYSTEM == CPS2)
static u32 read_cache_zipfile(u32 offset)
{
	s16 new_block = offset >> BLOCK_SHIFT;
	u32 idx = blocks[new_block];
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

	return ((tail->idx << BLOCK_SHIFT) | (offset & BLOCK_MASK));
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
	s16 new_block = offset >> BLOCK_SHIFT;
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

	num_cache = 0;
	cache_fd = -1;

#if (EMU_SYSTEM == MVS)
	pcm_cache_enable = 0;
	pcm_fd = -1;
	memset(pcm_cache, 0, sizeof(pcm_cache));
	memset(pcm_cache_block, 0xff, sizeof(pcm_cache_block));
	read_cache = read_cache_disable;
#else
	cache_type = CACHE_NOTFOUND;
	read_cache = read_cache_static;
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
	int i, found;
	u32 size = 0;
	char version_str[8];
#if (EMU_SYSTEM == MVS)
	extern int disable_sound;
#endif

	zip_close();

#if (EMU_SYSTEM == MVS)

	msg_printf(TEXT(LOADING_CACHE_INFORMATION_DATA));

	found = 0;
	if (cachefile_open("cache_info") != -1)
	{
		file_read(version_str, 8);

		if (strcmp(version_str, "MVS_" CACHE_VERSION) == 0)
		{
			file_read(gfx_pen_usage[2], memory_length_gfx3 / 128);
			found = 1;
		}
		file_close();

		if (!found)
		{
			msg_printf(TEXT(UNSUPPORTED_VERSION_OF_CACHE_FILE), version_str[5], version_str[6]);
			msg_printf(TEXT(CURRENT_REQUIRED_VERSION_IS_x));
			msg_printf(TEXT(PLEASE_REBUILD_CACHE_FILE));
			return 0;
		}
	}
	else
	{
		msg_printf(TEXT(COULD_NOT_OPEN_CACHE_FILE));
		return 0;
	}

	if (option_sound_enable && disable_sound)
	{
		sprintf(pcm_cache_name, "%s/%s_cache/vrom", cache_dir, game_name);
		if ((pcm_fd = sceIoOpen(pcm_cache_name, PSP_O_RDONLY, 0777)) >= 0)
		{
			pcm_cache_enable = 1;

			for (i = 0; i < 7; i++)
			{
				if ((pcm_cache[i] = memalign(MEM_ALIGN, BLOCK_SIZE)) == NULL)
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
				memset(pcm_cache[i], 0, BLOCK_SIZE);
			}
			if (pcm_cache_enable)
			{
				disable_sound = 0;
				msg_printf(TEXT(PCM_CACHE_ENABLED));
			}
		}
		if (!pcm_cache_enable) memory_length_sound1 = 0;
	}

	sprintf(spr_cache_name, "%s/%s_cache/crom", cache_dir, game_name);
	if ((cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777)) < 0)
	{
		msg_printf(TEXT(COULD_NOT_OPEN_CACHE_FILE));
		return 0;
	}

#elif (EMU_SYSTEM == CPS2)
	found = 1;
	cache_type = CACHE_RAWFILE;

	sprintf(spr_cache_name, "%s/%s.cache", cache_dir, game_name);
	if ((cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777)) < 0)
	{
		sprintf(spr_cache_name, "%s/%s.cache", cache_dir, cache_parent_name);
		if ((cache_fd = sceIoOpen(spr_cache_name, PSP_O_RDONLY, 0777)) < 0)
		{
			found = 0;
		}
	}
	if (!found)
	{
		found = 1;
		cache_type = CACHE_ZIPFILE;

		sprintf(spr_cache_name, "%s/%s_cache.zip", cache_dir, game_name);
		if (zip_open(spr_cache_name) == -1)
		{
			sprintf(spr_cache_name, "%s/%s_cache.zip", cache_dir, cache_parent_name);
			if (zip_open(spr_cache_name) == -1)
			{
				found = 0;
				zip_close();
			}
		}
	}
	if (!found)
	{
		cache_type = CACHE_NOTFOUND;
		msg_printf(TEXT(COULD_NOT_OPEN_CACHE_FILE));
		return 0;
	}

	msg_printf(TEXT(LOADING_CACHE_INFORMATION_DATA));

	if (cache_type == CACHE_RAWFILE)
	{
		sceIoRead(cache_fd, version_str, 8);

		if (strcmp(version_str, "CPS2" CACHE_VERSION) == 0)
		{
			sceIoRead(cache_fd, gfx_pen_usage[TILE08], gfx_total_elements[TILE08]);
			sceIoRead(cache_fd, gfx_pen_usage[TILE16], gfx_total_elements[TILE16]);
			sceIoRead(cache_fd, gfx_pen_usage[TILE32], gfx_total_elements[TILE32]);
			sceIoRead(cache_fd, block_offset, MAX_CACHE_BLOCKS * sizeof(u32));
		}
		else
		{
			sceIoClose(cache_fd);
			found = 0;
		}
	}
	else
	{
		if ((cache_fd = zopen("cache_info")) != -1)
		{
			zread(cache_fd, version_str, 8);

			if (strcmp(version_str, "CPS2" CACHE_VERSION) == 0)
			{
				zread(cache_fd, gfx_pen_usage[TILE08], gfx_total_elements[TILE08]);
				zread(cache_fd, gfx_pen_usage[TILE16], gfx_total_elements[TILE16]);
				zread(cache_fd, gfx_pen_usage[TILE32], gfx_total_elements[TILE32]);
				zread(cache_fd, block_empty, MAX_CACHE_BLOCKS);
				zclose(cache_fd);
			}
			else
			{
				zclose(cache_fd);
				found = 0;
			}
		}
		else
		{
			found = 0;
		}
		if (!found) zip_close();
	}
	if (!found)
	{
		msg_printf(TEXT(UNSUPPORTED_VERSION_OF_CACHE_FILE), version_str[5], version_str[6]);
		msg_printf(TEXT(CURRENT_REQUIRED_VERSION_IS_x));
		msg_printf(TEXT(PLEASE_REBUILD_CACHE_FILE));
		return 0;
	}

	if ((GFX_MEMORY = (u8 *)memalign(MEM_ALIGN, GFX_SIZE + CACHE_SAFETY)) != NULL)
	{
		free(GFX_MEMORY);
		GFX_MEMORY = (u8 *)memalign(MEM_ALIGN, GFX_SIZE);
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
		for (i = GFX_SIZE >> BLOCK_SHIFT; i >= MIN_CACHE_SIZE; i--)
		{
			if ((GFX_MEMORY = (u8 *)memalign(MEM_ALIGN, (i << BLOCK_SHIFT) + CACHE_SAFETY)) != NULL)
			{
				size = i << BLOCK_SHIFT;
				free(GFX_MEMORY);
				GFX_MEMORY = NULL;
				break;
			}
		}

		if (i < MIN_CACHE_SIZE)
		{
			msg_printf(TEXT(MEMORY_NOT_ENOUGH));
			return 0;
		}

		if ((GFX_MEMORY = (u8 *)memalign(MEM_ALIGN, size)) == NULL)
		{
			msg_printf(TEXT(COULD_NOT_ALLOCATE_CACHE_MEMORY));
			return 0;
		}
		memset(GFX_MEMORY, 0, size);

		num_cache = i;
	}

	msg_printf(TEXT(xKB_CACHE_ALLOCATED), (num_cache << BLOCK_SHIFT) / 1024);

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
		msg_printf(TEXT(CACHE_LOAD_ERROR));
		pad_wait_press(PAD_WAIT_INFINITY);
		Loop = LOOP_BROWSER;
		return 0;
	}

	if (size == 0) cache_shutdown();

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


#ifdef STATE_SAVE

/*------------------------------------------------------
	�X�e�[�g�Z�[�u�̈���ꎞ�I�Ɋm�ۂ���
------------------------------------------------------*/

u8 *cache_alloc_state_buffer(u32 size)
{
	SceUID fd;
	char path[MAX_PATH];

	sprintf(path, "%sstate/cache.tmp", launchDir);

	if ((fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777)) >= 0)
	{
		sceIoWrite(fd, GFX_MEMORY, size);
		sceIoClose(fd);
		return GFX_MEMORY;
	}
	return NULL;
}

/*------------------------------------------------------
	�Z�[�u�̈��������A�ޔ������L���b�V����߂�
------------------------------------------------------*/

void cache_free_state_buffer(u32 size)
{
	SceUID fd;
	char path[MAX_PATH];

	sprintf(path, "%sstate/cache.tmp", launchDir);

	if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0)
	{
		sceIoRead(fd, GFX_MEMORY, size);
		sceIoClose(fd);
	}
	sceIoRemove(path);
}

#endif /* STATE_SAVE */

#endif /* USE_CACHE */
