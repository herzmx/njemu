/******************************************************************************

	cache.c

	メモリキャッシュインタフェース関数

******************************************************************************/

#ifndef MEMORY_CACHE_H
#define MEMORY_CACHE_H

#if USE_CACHE

#if (EMU_SYSTEM == CPS2)
#define GFX_MEMORY			memory_region_gfx1
#define GFX_SIZE			memory_length_gfx1
#define CHECK_FNAME			"block_empty"
#define MAX_CACHE_BLOCKS	0x200
#elif (EMU_SYSTEM == MVS)
#define GFX_MEMORY			memory_region_gfx3
#define GFX_SIZE			memory_length_gfx3
#define PCM_CACHE_SIZE		0x10000
#define PCM_CACHE_MASK		0xffff
#define PCM_CACHE_SHIFT		16
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
#if (EMU_SYSTEM == MVS)
extern int pcm_cache_enable;
#else
extern u8 *block_empty;
extern u32 block_offset[MAX_CACHE_BLOCKS];
#endif

void cache_init(void);
int cache_start(void);
void cache_shutdown(void);
void cache_sleep(int flag);

#if (EMU_SYSTEM == MVS)
u8 pcm_cache_read(int ch, u32 offset);
u8 *pcm_get_cache(int ch);
#endif

#ifdef STATE_SAVE
u8 *cache_alloc_state_buffer(u32 size);
void cache_free_state_buffer(u32 size);
#endif

#endif /* USE_CACHE */

#endif /* MEMORY_CACHE_H */
