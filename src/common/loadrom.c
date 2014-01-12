/******************************************************************************

	loadrom.c

	ROMイメージファイルロード関数

******************************************************************************/

#include "emumain.h"


/******************************************************************************
	ローカル関数
******************************************************************************/

static int rom_fd;


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	ZIPファイルからファイルを検索し開く
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
	ファイルを閉じる
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
	ファイルから指定バイト読み込む
--------------------------------------------------------*/

int file_read(void *buf, size_t length)
{
	if (rom_fd != -1)
		return zread(rom_fd, buf, length);
	return -1;
}


/*--------------------------------------------------------
	ファイルから1文字読み込む
--------------------------------------------------------*/

int file_getc(void)
{
	if (rom_fd != -1)
		return zgetc(rom_fd);
	return -1;
}


/*--------------------------------------------------------
	キャッシュファイルを開く
--------------------------------------------------------*/

#if USE_CACHE
int cachefile_open(const char *fname)
{
	char path[MAX_PATH];

	sprintf(path, "%s/%s_cache.zip", cache_dir, game_name);
	if (zip_open(path) != -1)
	{
		if ((rom_fd = zopen(fname)) != -1)
			return rom_fd;
		zip_close();
	}

	if (strlen(cache_parent_name))
	{
		sprintf(path, "%s/%s_cache.zip", cache_dir, cache_parent_name);
		if (zip_open(path) != -1)
		{
			if ((rom_fd = zopen(fname)) != -1)
				return rom_fd;
			zip_close();
		}
	}
	else
	{
		sprintf(path, "%s/%s_cache.zip", cache_dir, parent_name);
		if (zip_open(path) != -1)
		{
			if ((rom_fd = zopen(fname)) != -1)
				return rom_fd;
			zip_close();
		}
	}

	return -1;
}
#endif


/*--------------------------------------------------------
	ROMをロードする
--------------------------------------------------------*/

int rom_load(struct rom_t *rom, u8 *mem, int idx, int max)
{
	int offset, length;

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
