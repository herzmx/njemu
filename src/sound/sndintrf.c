/******************************************************************************

	sndintrf.c

	サウンドインタフェース

******************************************************************************/

#include "emumain.h"


/******************************************************************************
	サウンドインタフェース関数
******************************************************************************/

/*------------------------------------------------------
	サウンドエミュレーション初期化
------------------------------------------------------*/
int sound_init(void)
{
#if (EMU_SYSTEM == CPS1)
	if (machine_sound_type == SOUND_QSOUND)
		qsound_sh_start();
	else
		YM2151_sh_start(machine_sound_type);
#elif (EMU_SYSTEM == CPS2)
	qsound_sh_start();
#elif (EMU_SYSTEM == MVS)
	YM2610_sh_start();
#endif

	return sound_thread_start();
}


/*------------------------------------------------------
	サウンドエミュレーション終了
------------------------------------------------------*/

void sound_exit(void)
{
#if (EMU_SYSTEM == CPS1)
	if (machine_sound_type == SOUND_QSOUND)
		qsound_sh_stop();
	else
		YM2151_sh_stop();
#elif (EMU_SYSTEM == CPS2)
	qsound_sh_stop();
#elif (EMU_SYSTEM == MVS)
	YM2610_sh_stop();
#endif

	sound_thread_stop();
}


/*------------------------------------------------------
	サウンドエミュレーションリセット
------------------------------------------------------*/

void sound_reset(void)
{
#if (EMU_SYSTEM == CPS1)
	if (machine_sound_type == SOUND_QSOUND)
		qsound_sh_reset();
	else
		YM2151_sh_reset();
#elif (EMU_SYSTEM == CPS2)
	qsound_sh_reset();
#elif (EMU_SYSTEM == MVS)
	YM2610_sh_reset();
#endif

	sound_mute(0);
}


/*------------------------------------------------------
	サウンドミュート
------------------------------------------------------*/

void sound_mute(int mute)
{
	if (mute)
		sound_thread_enable(0);
	else
		sound_thread_enable(option_sound_enable);
}
