/******************************************************************************

	cache.c

	メモリキャッシュインタフェース関数

******************************************************************************/

#ifndef MEMORY_CACHE_H
#define MEMORY_CACHE_H

#if USE_CACHE

#if (EMU_SYSTEM == CPS2)
#define MAX_CACHE_BLOCKS	0x200
#elif (EMU_SYSTEM == MVS)
#define MAX_CACHE_BLOCKS	0x400
#endif

enum
{
	CACHE_NOTFOUND = 0,
	CACHE_ZIPFILE,
	CACHE_RAWFILE
};

extern u32 (*read_cache)(u32 offset);
extern void (*update_cache)(u32 offset);
extern u8 block_empty[MAX_CACHE_BLOCKS];
extern int cache_type;

void cache_init(void);
int cache_start(void);
void cache_shutdown(void);
void cache_sleep(int flag);

void cahce_set_update_func(int flag);

#endif /* USE_CACHE */

#endif /* MEMORY_CACHE_H */
