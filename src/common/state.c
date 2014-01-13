/******************************************************************************

	state.c

	�X�e�[�g�Z�[�u/���[�h

******************************************************************************/

#ifdef SAVE_STATE

#include "emumain.h"
#include <time.h>


/******************************************************************************
	�O���[�o���ϐ�
******************************************************************************/

char date_str[16];
char time_str[16];
char stver_str[16];
int  state_version;
int  current_state_version;
#if (EMU_SYSTEM == MVS)
int  state_reload_bios;
#endif


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

#if (EMU_SYSTEM == CPS1)
static const char *current_version_str = "CPS1SV04";
#elif (EMU_SYSTEM == CPS2)
static const char *current_version_str = "CPS2SV07";
#elif (EMU_SYSTEM == MVS)
static const char *current_version_str = "MVSSV003";
#endif


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*------------------------------------------------------
	�T���l�C�������[�N�̈悩��t�@�C���ɕۑ�
------------------------------------------------------*/

static void save_thumbnail(FILE *fp)
{
	int x, y, w, h;
	u16 *src = video_frame_addr(tex_frame, 152, 0);

	if (machine_screen_type)
	{
		w = 112;
		h = 152;
	}
	else
	{
		w = 152;
		h = 112;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			fwrite(&src[x], 1, 2, fp);
		}
		src += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	�T���l�C�����t�@�C�����烏�[�N�̈�ɓǂݍ���
------------------------------------------------------*/

static void load_thumbnail(FILE *fp)
{
	int x, y, w, h;
	u16 *dst = video_frame_addr(tex_frame, 0, 0);

	if (machine_screen_type)
	{
		w = 112;
		h = 152;
	}
	else
	{
		w = 152;
		h = 112;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			fread(&dst[x], 1, 2, fp);
		}
		dst += BUF_WIDTH;
	}
}


/*------------------------------------------------------
	���[�N�̈�̃T���l�C�����N���A
------------------------------------------------------*/

static void clear_thumbnail(void)
{
	int x, y, w, h;
	u16 *dst = video_frame_addr(tex_frame, 0, 0);

	if (machine_screen_type)
	{
		w = 112;
		h = 152;
	}
	else
	{
		w = 152;
		h = 112;
	}

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			dst[x] = 0;
		}
		dst += BUF_WIDTH;
	}
}


/******************************************************************************
	�X�e�[�g�Z�[�u/���[�h�֐�
******************************************************************************/

/*------------------------------------------------------
	�X�e�[�g�Z�[�u
------------------------------------------------------*/

int state_save(int slot)
{
	FILE *fp = NULL;
	char path[MAX_PATH];

	sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, slot);
	remove(path);

	if ((fp = fopen(path, "wb")) != NULL)
	{
		pspTime nowtime;

		sceRtcGetCurrentClockLocalTime(&nowtime);

		// �w�b�_ (0x20 bytes)
		state_draw_progress(2);
		state_save_byte(current_version_str, 8);
		state_save_byte(&nowtime, 16);

		// �T���l�C�� (152 * 112 * 2 bytes)
		state_draw_progress(3);
		save_thumbnail(fp);

		// �X�e�[�g�f�[�^
		state_draw_progress(4);
		state_save_memory(fp);
		state_save_m68000(fp);
		state_save_z80(fp);
		state_save_input(fp);
		state_save_timer(fp);
		state_save_driver(fp);
		state_save_video(fp);
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
		state_save_coin(fp);
#endif
#if (EMU_SYSTEM == CPS1)
		switch (machine_driver_type)
		{
		case MACHINE_qsound:
			state_save_qsound(fp);
			state_save_eeprom(fp);
			break;

		case MACHINE_pang3:
			state_save_eeprom(fp);

		default:
			state_save_ym2151(fp);
			break;
		}
#elif (EMU_SYSTEM == CPS2)
		state_save_qsound(fp);
		state_save_eeprom(fp);
#elif (EMU_SYSTEM == MVS)
		state_save_ym2610(fp);
		state_save_pd4990a(fp);
#endif
		fclose(fp);

		state_draw_progress(5);
		return 1;
	}

	ui_popup("Error: Could not open file \"%s.sv%d\"", game_name, slot);

	return 0;
}


/*------------------------------------------------------
	�X�e�[�g���[�h
------------------------------------------------------*/

int state_load(int slot)
{
	FILE *fp;
	char path[MAX_PATH];

	sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, slot);

#if (EMU_SYSTEM == MVS)
	state_reload_bios = 0;
#endif

	if ((fp = fopen(path, "rb")) != NULL)
	{
		// �w�b�_ (�X�L�b�v)
		state_draw_progress(2);
		state_load_skip((8+16));

		// �T���l�C�� (�X�L�b�v)
		state_draw_progress(3);
		state_load_skip((152*112*2));

		// �X�e�[�g�f�[�^
		state_draw_progress(4);
		state_load_memory(fp);
		state_load_m68000(fp);
		state_load_z80(fp);
		state_load_input(fp);
		state_load_timer(fp);
		state_load_driver(fp);
		state_load_video(fp);
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
		state_load_coin(fp);
#endif
#if (EMU_SYSTEM == CPS1)
		switch (machine_driver_type)
		{
		case MACHINE_qsound:
			state_load_qsound(fp);
			state_load_eeprom(fp);
			break;

		case MACHINE_pang3:
			state_load_ym2151(fp);
			state_load_eeprom(fp);

		default:
			state_load_ym2151(fp);
			break;
		}
#elif (EMU_SYSTEM == CPS2)
		state_load_qsound(fp);
		state_load_eeprom(fp);
#elif (EMU_SYSTEM == MVS)
		state_load_ym2610(fp);
		state_load_pd4990a(fp);
#endif
		fclose(fp);

#if (EMU_SYSTEM == CPS2)
		blit_update_all_cache();
#elif (EMU_SYSTEM == MVS)
//		blit_update_all_cache();

		if (state_reload_bios)
		{
			state_reload_bios = 0;

			if (!reload_bios())
			{
				fatalerror("Error: Could not reload BIOS.");
				Loop = LOOP_BROWSER;
				return 0;
			}
		}
#endif

		state_draw_progress(5);
		return 1;
	}

	ui_popup("Error: Could not open file \"%s.sv%d\"", game_name, slot);

	return 0;
}


/*------------------------------------------------------
	�T���l�C���쐬
------------------------------------------------------*/

void state_make_thumbnail(void)
{
#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
	RECT clip1 = { 64, 16, 64 + 384, 16 + 224 };

	if (machine_screen_type)
	{
		RECT clip2 = { 152, 0, 152 + 112, 152 };
		video_copy_rect_rotate(work_frame, tex_frame, &clip1, &clip2);
	}
	else
	{
		RECT clip2 = { 152, 0, 152 + 152, 112 };
		video_copy_rect(work_frame, tex_frame, &clip1, &clip2);
	}
#elif (EMU_SYSTEM == MVS)
	RECT clip1 = { 8, 16, 8 + 304, 16 + 224 };
	RECT clip2 = { 152, 0, 152 + 152, 112 };

	video_copy_rect(work_frame, tex_frame, &clip1, &clip2);
#endif
}


/*------------------------------------------------------
	�T���l�C���ǂݍ���
------------------------------------------------------*/

int state_load_thumbnail(int slot)
{
	FILE *fp;
	char path[MAX_PATH];

	clear_thumbnail();

	sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, slot);

	if ((fp = fopen(path, "rb")) != NULL)
	{
		pspTime t;

		memset(stver_str, 0, 16);

		fread(stver_str, 1, 8, fp);
		fread(&t, 1, 16, fp);
		load_thumbnail(fp);
		fclose(fp);

		current_state_version = current_version_str[7] - '0';
		state_version = stver_str[7] - '0';

		sprintf(date_str, "%04d/%02d/%02d", t.year, t.month, t.day);
		sprintf(time_str, "%02d:%02d:%02d", t.hour, t.minutes, t.seconds);

		return 1;
	}

	ui_popup("Error: Could not open file \"%s.sv%d\"", game_name, slot);

	return 0;
}


/*------------------------------------------------------
	�T���l�C������
------------------------------------------------------*/

void state_clear_thumbnail(void)
{
	strcpy(date_str, "----/--/--");
	strcpy(time_str, "--:--:--");
	strcpy(stver_str, "--------");

	state_version = 0;

	clear_thumbnail();
}

#endif /* SAVE_STATE */
