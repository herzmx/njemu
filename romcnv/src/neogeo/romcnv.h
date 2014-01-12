#ifndef ROMCNV_H
#define ROMCNV_H

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef char			s8;
typedef short			s16;
typedef int				s32;

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <string.h>
#include <malloc.h>
#include "zlib/zlib.h"
#include "zfile.h"
#include "neogeo.h"


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


#endif /* ROMCNV_H */
