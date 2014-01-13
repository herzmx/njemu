/******************************************************************************

	sound.c

	PSP �T�E���h�X���b�h

******************************************************************************/

#include "psp.h"
#include "emumain.h"


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

static volatile int sound_active;
static int sound_handle;
static SceUID sound_thread;
static int sound_volume;
static int sound_enable;
static s16 sound_buffer[2][SOUND_BUFFER_SIZE];

static struct sound_t sound_info;


/******************************************************************************
	�O���[�o���ϐ�
******************************************************************************/

struct sound_t *sound = &sound_info;


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*--------------------------------------------------------
	�T�E���h�X�V�X���b�h
--------------------------------------------------------*/

static int sound_update_thread(SceSize args, void *argp)
{
	int flip = 0;

	video_wait_vsync();

	while (sound_active)
	{
		if (Sleep)
		{
			do
			{
				sceKernelDelayThread(5000000);
			} while (Sleep);
		}

		if (sound_enable)
		{
			(*sound->callback)((int)sound_buffer[flip]);
			sceAudioOutputPannedBlocking(sound_handle, sound_volume, sound_volume, (char *)sound_buffer[flip]);
		}
		else
		{
			memset(sound_buffer[flip], 0, SOUND_BUFFER_SIZE);
			sceAudioOutputPannedBlocking(sound_handle, 0, 0, (char *)sound_buffer[flip]);
		}
		flip ^= 1;
	}
	return 0;
}


/******************************************************************************
	�O���[�o���֐�
******************************************************************************/

/*--------------------------------------------------------
	�T�E���h������
--------------------------------------------------------*/

void sound_thread_init(void)
{
	sound_active = 0;
	sound_thread = -1;
	sound_handle = -1;
	sound_volume = 0;
	sound_enable = 0;
}


/*--------------------------------------------------------
	�T�E���h�I��
--------------------------------------------------------*/

void sound_thread_exit(void)
{
	sound_thread_stop();
}


/*--------------------------------------------------------
	�T�E���h�L��/�����؂�ւ�
--------------------------------------------------------*/

void sound_thread_enable(int enable)
{
	if (sound_active)
	{
		sound_enable = enable;

		if (sound_enable)
			sound_thread_set_volume();
		else
			sound_volume = 0;
	}
}


/*--------------------------------------------------------
	�T�E���h�̃{�����[���ݒ�
--------------------------------------------------------*/

void sound_thread_set_volume(void)
{
	if (sound_active)
		sound_volume = PSP_AUDIO_VOLUME_MAX * (option_sound_volume * 10) / 100;
}


/*--------------------------------------------------------
	�T�E���h�X���b�h�J�n
--------------------------------------------------------*/

int sound_thread_start(void)
{
	int priority = option_vsync ? 0x12 : 0x08;

	sound_active = 0;
	sound_thread = -1;
	sound_handle = -1;
	sound_enable = 0;

	memset((void *)sound_buffer[0], 0, SOUND_BUFFER_SIZE);
	memset((void *)sound_buffer[1], 0, SOUND_BUFFER_SIZE);

	if (sound->stereo)
		sound_handle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, SOUND_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
	else
		sound_handle = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, SOUND_SAMPLES, PSP_AUDIO_FORMAT_MONO);
	if (sound_handle < 0)
	{
		fatalerror("Could not reserve audio channel for sound.");
		return 0;
	}

	sound_thread = sceKernelCreateThread("Sound thread", sound_update_thread, priority, sound->stack, 0, NULL);
	if (sound_thread < 0)
	{
		fatalerror("Could not start sound thread.");
		sceAudioChRelease(sound_handle);
		sound_handle = -1;
		return 0;
	}

	sound_active = 1;
	sceKernelStartThread(sound_thread, 0, 0);

	sound_thread_set_volume();

	return 1;
}


/*--------------------------------------------------------
	�T�E���h�X���b�h��~
--------------------------------------------------------*/

void sound_thread_stop(void)
{
	if (sound_thread >= 0)
	{
		sound_volume = 0;
		sound_enable = 0;

		sound_active = 0;
		sceKernelWaitThreadEnd(sound_thread, NULL);

		sceKernelDeleteThread(sound_thread);
		sound_thread = -1;

		sceAudioChRelease(sound_handle);
		sound_handle = -1;
	}
}


/*--------------------------------------------------------
	�T�E���h�X���b�h�v���C�I���e�B�ύX
--------------------------------------------------------*/

void sound_thread_set_priority(int priority)
{
	sceKernelChangeThreadPriority(sound_thread, priority);
}
