/******************************************************************************

	emucfg.h

	�G�~�����[�V�����ݒ�

******************************************************************************/

#ifndef EMUCONFIG_H
#define EMUCONFIG_H

#ifndef EMUSYS_VALUE
#define EMUSYS_VALUE
#define CPS1	0
#define CPS2	1
#define MVS		2
#define NCDZ	3
#endif

#define ALIGN_PSPDATA			__attribute__((aligned(16)))
#define ALIGN_DATA				__attribute__((aligned(4)))
#define MEM_ALIGN				4

/******************************************************************************
	PSP�̒萔
******************************************************************************/

#define PSP_REFRESH_RATE		(59.940059)		// (9000000Hz * 1) / (525 * 286)
#define PSP_TICKS_PER_FRAME		(16683.333333)


/******************************************************************************
	AdHoc�̐ݒ�
******************************************************************************/

#define ADHOC_UPDATE_EVERY_FRAME	0
#define ADHOC_TIMEOUT				(500*1000)	// 500 ms


/******************************************************************************
	���̑��̐ݒ�
******************************************************************************/

#define QSOUND_STREAM_48KHz		0	// 1�ɂ���Ɖ������オ�邩������Ȃ�


/******************************************************************************
	CPS1PSP�̐ݒ�
******************************************************************************/

#if defined(BUILD_CPS1PSP)

#define APPNAME_STR				"CAPCOM CPS1 Emulator for PSP"
#define machine_main			cps1_main

#define EMU_SYSTEM				CPS1
#define SYSTEM_NAME				"CPS1"
#define FPS						60.0
#define TICKS_PER_FRAME			16666.666666
#define USE_CACHE				0
#define EEPROM_SIZE				128
#define GULIST_SIZE				48*1024		// 48KB
#define ENABLE_RASTER_OPTION	1			// on


/******************************************************************************
	CPS2PSP�̐ݒ�
******************************************************************************/

#elif defined(BUILD_CPS2PSP)

#define APPNAME_STR				"CAPCOM CPS2 Emulator for PSP"
#define machine_main			cps2_main

#define EMU_SYSTEM				CPS2
#define SYSTEM_NAME				"CPS2"
#define FPS						59.633333
#define TICKS_PER_FRAME			16769.144773
#ifdef PSP_SLIM
#define USE_CACHE				0
#else
#define USE_CACHE				1
#define CACHE_VERSION			"V22"
#endif
#define EEPROM_SIZE				128
#define GULIST_SIZE				48*1024		// 48KB
#define ENABLE_RASTER_OPTION	0			// off


/******************************************************************************
	MVSPSP�̐ݒ�
******************************************************************************/

#elif defined(BUILD_MVSPSP)

#define APPNAME_STR				"NEOGEO Emulator for PSP"
#define machine_main			neogeo_main

#define EMU_SYSTEM				MVS
#define SYSTEM_NAME				"NEO�GEO"
#define FPS						59.185606
#define TICKS_PER_FRAME			16896.0
#define USE_CACHE				1
#define CACHE_VERSION			"V23"
#define GULIST_SIZE				300*1024	// 300KB


/******************************************************************************
	NCDZPSP�̐ݒ�
******************************************************************************/

#elif defined(BUILD_NCDZPSP)

#define APPNAME_STR				"NEOGEO CDZ Emulator for PSP"
#define machine_main			neogeo_main

#define EMU_SYSTEM				NCDZ
#define SYSTEM_NAME				"NEO�GEO CDZ"
#define FPS						59.185606
#define TICKS_PER_FRAME			16896.0
#define USE_CACHE				0
#define GULIST_SIZE				300*1024	// 300KB

#endif

#endif /* EMUCONFIG_H */
