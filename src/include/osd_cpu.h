/******************************************************************************

	osd_cpu.h

	Compatible for MAME.

******************************************************************************/

#ifndef OSD_CPU_H
#define OSD_CPU_H

#ifdef PSP

#define UINT8		u8
#define UINT16		u16
#define UINT32		u32
#define UINT64		u64
#define INT8		s8
#define INT16		s16
#define INT32		s32
#define INT64		s64

#define offs_t		u32
#define data8_t		u8
#define data16_t	u16
#define data32_t	u32

#else

typedef unsigned char       UINT8;
typedef unsigned short      UINT16;
typedef unsigned int        UINT32;
typedef unsigned __int64    UINT64;
typedef signed char         INT8;
typedef signed short        INT16;
typedef signed int          INT32;
typedef signed __int64      INT64;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned __int64    u64;
typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed __int64      s64;

#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif /* OSD_CPU_H */
