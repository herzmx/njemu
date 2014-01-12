#ifndef ZFILE_H
#define ZFILE_H

struct zip_find_t
{
	char name[MAX_PATH];
	u32  length;
	u32  crc32;
};

int zip_open(const char *path);
void zip_close(void);

int zip_findfirst(struct zip_find_t *file);
int zip_findnext(struct zip_find_t *file);

int zopen(const char *filename);
int zread(int fd, void *buf, unsigned size);
int zgetc(int fd);
int zclose(int fd);
int zsize(int fd);
int zcrc(int fd);

int zlength(const char *filename);

#endif
