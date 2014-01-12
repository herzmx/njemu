/******************************************************************************

	memintrf.c

	MVSメモリインタフェース関数

******************************************************************************/

#ifndef MVS_MEMORY_INTERFACE_H
#define MVS_MEMORY_INTERFACE_H

extern u8 *memory_region_cpu1;
extern u8 *memory_region_cpu2;
extern u8 *memory_region_gfx1;
extern u8 *memory_region_gfx2;
extern u8 *memory_region_gfx3;
extern u8 *memory_region_sound1;
extern u8 *memory_region_sound2;
extern u8 *memory_region_user1;

extern u32 memory_length_cpu1;
extern u32 memory_length_cpu2;
extern u32 memory_length_gfx1;
extern u32 memory_length_gfx2;
extern u32 memory_length_gfx3;
extern u32 memory_length_sound1;
extern u32 memory_length_sound2;
extern u32 memory_length_user1;

extern u8  neogeo_memcard[0x800];
extern u8  neogeo_ram[0x10000];
extern u16 neogeo_sram16[0x8000];

extern int neogeo_machine_mode;

int memory_init(void);
void memory_shutdown(void);

u8 m68000_read_memory_8(u32 offset);
u16 m68000_read_memory_16(u32 offset);
void m68000_write_memory_8(u32 offset, u8 data);
void m68000_write_memory_16(u32 offset, u16 data);

u8 z80_read_memory_8(u32 address);
void z80_write_memory_8(u32 address, u8 data);

#ifdef SAVE_STATE
STATE_SAVE( memory );
STATE_LOAD( memory );
int reload_bios(void);
#endif

#endif /* MVS_MEMORY_INTERFACE_H */
