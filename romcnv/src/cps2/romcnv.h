#ifndef ROMCNV_H
#define ROMCNV_H

typedef unsigned char						u8;
typedef signed char 						s8;
typedef unsigned short						u16;
typedef signed short						s16;
typedef unsigned int						u32;
typedef signed int							s32;
#ifdef _MSC_VER
typedef unsigned __int64					u64;
typedef signed __int64						s64;
#else
__extension__ typedef unsigned long long	u64;
__extension__ typedef signed long long		s64;
#endif

#ifdef WIN32
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#endif
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <string.h>
#include <malloc.h>
#include "zlib/zlib.h"
#include "zfile.h"

#ifndef MAX_PATH
#define MAX_PATH	256
#endif

#endif /* ROMCNV_H */
