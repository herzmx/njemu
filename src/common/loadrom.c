/******************************************************************************

	loadrom.c

	ROM�C���[�W�t�@�C�����[�h�֐�

******************************************************************************/

#include "emumain.h"
#include <sys/unistd.h>


#if (EMU_SYSTEM != NCDZ)

/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

static int rom_fd;


/******************************************************************************
	ROM�t�@�C���ǂݍ���
******************************************************************************/

/*--------------------------------------------------------
	ZIP�t�@�C������t�@�C�����������J��
--------------------------------------------------------*/

int file_open(const char *fname1, const char *fname2, const u32 crc, char *fname)
{
	int found = 0;
	struct zip_find_t file;
	char path[MAX_PATH];

	sprintf(path, "%s/%s.zip", game_dir, fname1);

	if (zip_open(path) != -1)
	{
		if (zip_findfirst(&file))
		{
			if (file.crc32 == crc)
			{
				found = 1;
			}
			else
			{
				while (zip_findnext(&file))
				{
					if (file.crc32 == crc)
					{
						found = 1;
						break;
					}
				}
			}
		}
		if (!found) zip_close();
	}

	if (!found && fname2 != NULL)
	{
		sprintf(path, "%s/%s.zip", game_dir, fname2);

		if (zip_open(path) != -1)
		{
			if (zip_findfirst(&file))
			{
				if (file.crc32 == crc)
				{
					found = 1;
				}
				else
				{
					while (zip_findnext(&file))
					{
						if (file.crc32 == crc)
						{
							found = 1;
							break;
						}
					}
				}
			}

			if (!found) zip_close();
		}

		if (!found)
		{
			sprintf(path, "%sroms/%s.zip", launchDir, fname2);

			if (zip_open(path) != -1)
			{
				if (zip_findfirst(&file))
				{
					if (file.crc32 == crc)
					{
						found = 1;
					}
					else
					{
						while (zip_findnext(&file))
						{
							if (file.crc32 == crc)
							{
								found = 1;
								break;
							}
						}
					}
				}

				if (!found) zip_close();
			}
		}
	}

	if (found)
	{
		if (fname) strcpy(fname, file.name);
		rom_fd = zopen(file.name);
		return rom_fd;
	}

	return -1;
}


/*--------------------------------------------------------
	�t�@�C�������
--------------------------------------------------------*/

void file_close(void)
{
	if (rom_fd != -1)
	{
		zclose(rom_fd);
		zip_close();
		rom_fd = -1;
	}
}


/*--------------------------------------------------------
	�t�@�C������w��o�C�g�ǂݍ���
--------------------------------------------------------*/

int file_read(void *buf, size_t length)
{
	if (rom_fd != -1)
		return zread(rom_fd, buf, length);
	return -1;
}


/*--------------------------------------------------------
	�t�@�C������1�����ǂݍ���
--------------------------------------------------------*/

int file_getc(void)
{
	if (rom_fd != -1)
		return zgetc(rom_fd);
	return -1;
}


/*--------------------------------------------------------
	�L���b�V���t�@�C�����J��
--------------------------------------------------------*/

#if USE_CACHE && (EMU_SYSTEM == MVS)
int cachefile_open(const char *fname)
{
	char path[MAX_PATH];

	sprintf(path, "%s/%s_cache", cache_dir, game_name);
	zip_open(path);
	if ((rom_fd = zopen(fname)) != -1)
		return rom_fd;

	sprintf(path, "%s/%s_cache", cache_dir, cache_parent_name);
	zip_open(path);
	if ((rom_fd = zopen(fname)) != -1)
		return rom_fd;

	return -1;
}
#endif


/*--------------------------------------------------------
	ROM�����[�h����
--------------------------------------------------------*/

int rom_load(struct rom_t *rom, u8 *mem, int idx, int max)
{
	u32 offset, length;

_continue:
	offset = rom[idx].offset;

	if (rom[idx].skip == 0)
	{
		file_read(&mem[offset], rom[idx].length);

		if (rom[idx].type == ROM_WORDSWAP)
			swab(&mem[offset], &mem[offset], rom[idx].length);
	}
	else
	{
		int c;
		int skip = rom[idx].skip + rom[idx].group;

		length = 0;

		if (rom[idx].group == 1)
		{
			if (rom[idx].type == ROM_WORDSWAP)
				offset ^= 1;

			while (length < rom[idx].length)
			{
				if ((c = file_getc()) == EOF) break;
				mem[offset] = c;
				offset += skip;
				length++;
			}
		}
		else
		{
			while (length < rom[idx].length)
			{
				if ((c = file_getc()) == EOF) break;
				mem[offset + 0] = c;
				if ((c = file_getc()) == EOF) break;
				mem[offset + 1] = c;
				offset += skip;
				length += 2;
			}
		}
	}

	if (++idx != max)
	{
		if (rom[idx].type == ROM_CONTINUE)
		{
			goto _continue;
		}
	}

	return idx;
}

#endif /* EMU_SYSTEM */


/******************************************************************************
	�G���[���b�Z�[�W�\��
******************************************************************************/

/*------------------------------------------------------
	�������m�ۃG���[���b�Z�[�W�\��
------------------------------------------------------*/

void error_memory(const char *mem_name)
{
	zip_close();
	msg_printf(TEXT(COULD_NOT_ALLOCATE_x_MEMORY), mem_name);
	msg_printf(TEXT(PRESS_ANY_BUTTON2));
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	CRC�G���[���b�Z�[�W�\��
------------------------------------------------------*/

void error_crc(const char *rom_name)
{
	zip_close();
	msg_printf(TEXT(CRC32_NOT_CORRECT_x), rom_name);
	msg_printf(TEXT(PRESS_ANY_BUTTON2));
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}


/*------------------------------------------------------
	ROM�t�@�C���G���[���b�Z�[�W�\��
------------------------------------------------------*/

void error_file(const char *rom_name)
{
	zip_close();
	msg_printf(TEXT(FILE_NOT_FOUND_x), rom_name);
	msg_printf(TEXT(PRESS_ANY_BUTTON2));
	pad_wait_press(PAD_WAIT_INFINITY);
	Loop = LOOP_BROWSER;
}
