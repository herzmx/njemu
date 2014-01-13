/******************************************************************************

	sndintrf.c

	�T�E���h�C���^�t�F�[�X

******************************************************************************/

#include "emumain.h"


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

static INT32 ALIGN_DATA mixing_buffer[2 * SOUND_SAMPLES];


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*------------------------------------------------------
	�T�E���h�X�V(�X�e���I)
------------------------------------------------------*/

static void sound_update_stereo(INT16 *buffer)
{
	int length = SOUND_SAMPLES >> (2 - option_samplerate);
	INT32 *src = mixing_buffer;
	INT16 *dst = buffer;
	INT32 lt, rt;

	(*sound->callback)(src, length);

	switch (option_samplerate)
	{
	case 0:
		while (length--)
		{
			lt = *src++;
			rt = *src++;
			Limit(lt, MAXOUT, MINOUT);
			Limit(rt, MAXOUT, MINOUT);
			*dst++ = lt;
			*dst++ = rt;
			*dst++ = lt;
			*dst++ = rt;
			*dst++ = lt;
			*dst++ = rt;
			*dst++ = lt;
			*dst++ = rt;
		}
		break;

	case 1:
		while (length--)
		{
			lt = *src++;
			rt = *src++;
			Limit(lt, MAXOUT, MINOUT);
			Limit(rt, MAXOUT, MINOUT);
			*dst++ = lt;
			*dst++ = rt;
			*dst++ = lt;
			*dst++ = rt;
		}
		break;

	case 2:
		while (length--)
		{
			lt = *src++;
			rt = *src++;
			Limit(lt, MAXOUT, MINOUT);
			Limit(rt, MAXOUT, MINOUT);
			*dst++ = lt;
			*dst++ = rt;
		}
		break;
	}
}


/*------------------------------------------------------
	�T�E���h�X�V(���m����)
------------------------------------------------------*/

#if (EMU_SYSTEM == CPS1)
static void sound_update_mono(INT16 *buffer)
{
	int length = SOUND_SAMPLES >> (2 - option_samplerate);
	INT32 *src = mixing_buffer;
	INT16 *dst = buffer;
	INT32 sample;

	(*sound->callback)(src, length);

	switch (option_samplerate)
	{
	case 0:
		while (length--)
		{
			sample = *src++;
			Limit(sample, MAXOUT, MINOUT);
			*dst++ = sample;
			*dst++ = sample;
			*dst++ = sample;
			*dst++ = sample;
		}
		break;

	case 1:
		while (length--)
		{
			sample = *src++;
			Limit(sample, MAXOUT, MINOUT);
			*dst++ = sample;
			*dst++ = sample;
		}
		break;

	case 2:
		while (length--)
		{
			sample = *src++;
			Limit(sample, MAXOUT, MINOUT);
			*dst++ = sample;
		}
		break;
	}
}
#endif


/******************************************************************************
	�T�E���h�C���^�t�F�[�X�֐�
******************************************************************************/

/*------------------------------------------------------
	�T�E���h�G�~�����[�V����������
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
#elif (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
	YM2610_sh_start();
#endif

#if (EMU_SYSTEM == CPS1)
	if (!sound->stereo)
		sound->update = sound_update_mono;
	else
#endif
		sound->update = sound_update_stereo;

	return sound_thread_start();
}


/*------------------------------------------------------
	�T�E���h�G�~�����[�V�����I��
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
#elif (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
	YM2610_sh_stop();
#endif

	sound_thread_stop();
}


/*------------------------------------------------------
	�T�E���h�G�~�����[�V�������Z�b�g
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
#elif (EMU_SYSTEM == MVS || EMU_SYSTEM == NCDZ)
	YM2610_sh_reset();
#endif

	sound_mute(0);
}


/*------------------------------------------------------
	�T�E���h�~���[�g
------------------------------------------------------*/

void sound_mute(int mute)
{
	if (mute)
		sound_thread_enable(0);
	else
		sound_thread_enable(option_sound_enable);
}
