/******************************************************************************

	memintrf.c

	Memory interace

******************************************************************************/

#ifndef NCDZ_MEMINTRF_H
#define NCDZ_MEMINTRF_H

extern u8 *memory_region_cpu1;
extern u8 *memory_region_cpu2;
extern u8 *memory_region_gfx1;
extern u8 *memory_region_gfx2;
extern u8 *memory_region_sound1;
extern u8 *memory_region_user1;

extern u32 memory_length_cpu1;
extern u32 memory_length_cpu2;
extern u32 memory_length_gfx1;
extern u32 memory_length_gfx2;
extern u32 memory_length_sound1;
extern u32 memory_length_user1;

extern u8 neogeo_memcard[0x2000];

int memory_init(void);
void memory_shutdown(void);

u8  m68000_read_memory_8(u32 address);
u16 m68000_read_memory_16(u32 address);
u32 m68000_read_memory_32(u32 address);
void m68000_write_memory_8(u32 address, u8 value);
void m68000_write_memory_16(u32 address, u16 value);
void m68000_write_memory_32(u32 address, u32 value);

u8 z80_read_memory_8(u32 address);
void z80_write_memory_8(u32 address, u8 data);

STATE_SAVE( memory );
STATE_LOAD( memory );

#endif /* NCDZ_MEMINTRF_H */
