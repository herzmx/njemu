/******************************************************************************

	sndintrf.c

	サウンドインタフェース

******************************************************************************/

#ifndef SOUND_INTERFACE_H
#define SOUND_INTERFACE_H

enum
{
	SOUND_YM2151_TYPE1 = 0,
	SOUND_YM2151_TYPE2,
	SOUND_QSOUND,
	SOUND_YM2610
};

int sound_init(void);
void sound_exit(void);
void sound_reset(void);
void sound_mute(int mute);

#endif /* SOUND_INTERFACE_H */
