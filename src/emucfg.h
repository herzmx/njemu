#ifndef EMUCONFIG_H
#define EMUCONFIG_H

#ifndef EMUSYS_VALUE
#define EMUSYS_VALUE
#define CPS1	0
#define CPS2	1
#define MVS		3
#endif

#define ALIGN_DATA		__attribute__((aligned(64)))

#if defined(BUILD_CPS1PSP)

#define APPNAME_STR		"CAPCOM CPS1 Emulator for PSP"
#define machine_main	cps1_main

#define EMU_SYSTEM		CPS1
#define FPS				60.0
#define USE_CACHE		0
#define EEPROM_SIZE		128
#define GULIST_SIZE		48*1024		// 48KB

#elif defined(BUILD_CPS2PSP)

#define APPNAME_STR		"CAPCOM CPS2 Emulator for PSP"
#define machine_main	cps2_main

#define EMU_SYSTEM		CPS2
#define FPS				59.633333
#define USE_CACHE		1
#define EEPROM_SIZE		128
#define GULIST_SIZE		48*1024		// 48KB

#elif defined(BUILD_MVSPSP)

#define APPNAME_STR		"NEOGEO Emulator for PSP"
#define machine_main	neogeo_main

#define EMU_SYSTEM		MVS
//#define FPS			59.1856
#define FPS				60
#define USE_CACHE		1
#define GULIST_SIZE		96*1024		// 96KB

#endif

#endif /* EMUCONFIG_H */
