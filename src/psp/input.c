/******************************************************************************

	input.c

    PSP�R���g���[�����͐���

******************************************************************************/

#include "psp.h"


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

static u32 pad;
static u8 pressed_check;
static u8 pressed_count;
static u8 pressed_delay;
static TICKER curr_time;
static TICKER prev_time;


/******************************************************************************
	�O���[�o���֐�
******************************************************************************/

/*--------------------------------------------------------
	�p�b�h��������
--------------------------------------------------------*/

void pad_init(void)
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	pad = 0;
	pressed_check = 0;
	pressed_count = 0;
	pressed_delay = 0;
}


/*--------------------------------------------------------
	�p�b�h�̉�����Ԏ擾
--------------------------------------------------------*/

u32 poll_gamepad(void)
{
	SceCtrlData paddata;

	sceCtrlPeekBufferPositive(&paddata, 1);

#if (EMU_SYSTEM == MVS)
	if (!(paddata.Buttons & PSP_CTRL_UP)    && paddata.Ly >= 0xD0) paddata.Buttons |= PSP_CTRL_DOWN;  // DOWN
	if (!(paddata.Buttons & PSP_CTRL_DOWN)  && paddata.Ly <= 0x30) paddata.Buttons |= PSP_CTRL_UP;    // UP
	if (!(paddata.Buttons & PSP_CTRL_RIGHT) && paddata.Lx <= 0x30) paddata.Buttons |= PSP_CTRL_LEFT;  // LEFT
	if (!(paddata.Buttons & PSP_CTRL_LEFT)  && paddata.Lx >= 0xD0) paddata.Buttons |= PSP_CTRL_RIGHT; // RIGHT
#else
	if (paddata.Ly >= 0xD0) paddata.Buttons |= PSP_CTRL_DOWN;  // DOWN
	if (paddata.Ly <= 0x30) paddata.Buttons |= PSP_CTRL_UP;    // UP
	if (paddata.Lx <= 0x30) paddata.Buttons |= PSP_CTRL_LEFT;  // LEFT
	if (paddata.Lx >= 0xD0) paddata.Buttons |= PSP_CTRL_RIGHT; // RIGHT
#endif
#ifdef KERNEL_MODE
	return paddata.Buttons | home_button;
#else
	return paddata.Buttons;
#endif
}


/*--------------------------------------------------------
	�p�b�h�̉�����Ԏ擾(�A�i���O)
--------------------------------------------------------*/

#if (EMU_SYSTEM == MVS)

u32 poll_gamepad_analog(void)
{
	u32 data;
	SceCtrlData paddata;

	sceCtrlPeekBufferPositive(&paddata, 1);

	if (paddata.Ly >= 0xD0) paddata.Buttons |= PSP_CTRL_DOWN;  // DOWN
	if (paddata.Ly <= 0x30) paddata.Buttons |= PSP_CTRL_UP;    // UP
	if (paddata.Lx <= 0x30) paddata.Buttons |= PSP_CTRL_LEFT;  // LEFT
	if (paddata.Lx >= 0xD0) paddata.Buttons |= PSP_CTRL_RIGHT; // RIGHT

	data  = paddata.Buttons & 0xffff;
	data |= paddata.Lx << 16;
	data |= paddata.Ly << 24;

#ifdef KERNEL_MODE
	return data | home_button;
#else
	return data;
#endif
}

#endif


/*--------------------------------------------------------
	�p�b�h�������X�V
--------------------------------------------------------*/

void pad_update(void)
{
	u32 data;

	data = poll_gamepad() & PSP_CTRL_ANY;

	if (data)
	{
		if (!pressed_check)
		{
			pressed_check = 1;
			pressed_count = 0;
			pressed_delay = 8;
			prev_time = ticker();
		}
		else
		{
			int count;

			curr_time = ticker();
			count = (int)((curr_time - prev_time) / (TICKS_PER_SEC / 60));
			prev_time = curr_time;

			pressed_count += count;

			if (pressed_count > pressed_delay)
			{
				pressed_count = 0;
				if (pressed_delay > 2) pressed_delay -= 2;
			}
			else data = 0;
		}
	}
	else pressed_check = 0;

	pad = data;
}


/*--------------------------------------------------------
	�{�^��������Ԃ̎擾
--------------------------------------------------------*/

int pad_pressed(u32 code)
{
	return (pad & code) != 0;
}


/*--------------------------------------------------------
	�w��R�[�h�ȊO�̑S�{�^���̉�����Ԏ擾
--------------------------------------------------------*/

int pad_pressed_any(u32 disable_code)
{
	return (pad & (PSP_CTRL_ANY ^ disable_code)) != 0;
}


/*--------------------------------------------------------
	�{�^���̉�����Ԃ��N���A�����܂ő҂�
--------------------------------------------------------*/

void pad_wait_clear(void)
{
	while (poll_gamepad())
	{
		video_wait_vsync();
		if (!Loop) break;
	}

	pad = 0;
	pressed_check = 0;
}


/*--------------------------------------------------------
	�����{�^�����������܂ő҂�
--------------------------------------------------------*/

void pad_wait_press(int msec)
{
	pad_wait_clear();

	if (msec == PAD_WAIT_INFINITY)
	{
		while (!poll_gamepad())
		{
			video_wait_vsync();
			if (!Loop) break;
		}
	}
	else
	{
		TICKER curr = ticker();
		TICKER target, prev;

		target = curr + msec * (TICKS_PER_SEC / 1000);

		while (curr < target)
		{
			prev = curr;
			curr = ticker();

			video_wait_vsync();
			if (poll_gamepad()) break;
			if (!Loop) break;
		}
	}

	pad_wait_clear();
}
