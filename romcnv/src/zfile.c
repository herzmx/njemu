#include "romcnv.h"
#include "unzip.h"

static unzFile unzfile = NULL;

static char basedir[MAX_PATH];
static char *basedirend;
static char zip_cache[4096];
static int  zip_cached_len;
static int  zip_filepos;


int zip_open(const char *path)
{
	if (unzfile != NULL) zip_close();

	if ((unzfile = unzOpen(path)) != NULL)
		return (int)unzfile;

	strcpy(basedir, path);

	basedirend = basedir + strlen(basedir);
	*basedirend++ = '/';

	return -1;
}


void zip_close(void)
{
	if (unzfile)
	{
		unzClose(unzfile);
		unzfile = NULL;
	}
}


int zip_findfirst(struct zip_find_t *file)
{
	if (unzfile)
	{
		if (unzGoToFirstFile(unzfile) == UNZ_OK)
		{
			unz_file_info info;

			unzGetCurrentFileInfo(unzfile, &info, file->name, MAX_PATH, NULL, 0, NULL, 0);
			file->length = info.uncompressed_size;
			file->crc32 = info.crc;
			return 1;
		}
	}
	return 0;
}


int zip_findnext(struct zip_find_t *file)
{
	if (unzfile)
	{
		if (unzGoToNextFile(unzfile) == UNZ_OK)
		{
			unz_file_info info;

			unzGetCurrentFileInfo(unzfile, &info, file->name, MAX_PATH, NULL, 0, NULL, 0);
			file->length = info.uncompressed_size;
			file->crc32 = info.crc;
			return 1;
		}
	}
	return 0;
}


int zopen(const char *filename)
{
	zip_cached_len = 0;

	if (unzfile == NULL)
	{
		FILE *fp;

		strcpy(basedirend, filename);
		return (fp = fopen(basedir, "rb")) ? (int)fp : -1;
	}

	if (unzLocateFile(unzfile, filename, 0) == UNZ_OK)
		if (unzOpenCurrentFile(unzfile) == UNZ_OK)
			return (int)unzfile;

	return -1;
}


int zclose(int fp)
{
	zip_cached_len = 0;

	if (unzfile == NULL)
	{
		if (fp != -1) fclose((FILE *)fp);
		return 0;
	}
	return unzCloseCurrentFile(unzfile);
}


int zread(int fd, void *buf, unsigned size)
{
	if (unzfile == NULL)
		return fread(buf, 1, size, (FILE *)fd);

	return unzReadCurrentFile(unzfile, buf, size);
}


int zgetc(int fd)
{
	if (unzfile == NULL)
		return fgetc((FILE *)fd);

	if (zip_cached_len == 0)
	{
		zip_cached_len = unzReadCurrentFile(unzfile, zip_cache, 4096);
		if (zip_cached_len == 0) return EOF;
		zip_filepos = 0;
	}
	zip_cached_len--;
	return zip_cache[zip_filepos++] & 0xff;
}


int zsize(int fd)
{
	unz_file_info info;

	if (unzfile == NULL)
	{
		FILE *fp = (FILE *)fd;
		int len, pos = ftell(fp);

		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, pos, SEEK_SET);

		return len;
	}

	unzGetCurrentFileInfo(unzfile, &info, NULL, 0, NULL, 0, NULL, 0);

	return info.uncompressed_size;
}


int zcrc(int fd)
{
	unz_file_info info;

	if (unzfile == NULL)
		return 0;

	unzGetCurrentFileInfo(unzfile, &info, NULL, 0, NULL, 0, NULL, 0);

	return info.crc;
}
