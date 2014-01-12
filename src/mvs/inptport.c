/******************************************************************************

	inptport.c

	MVS 入力ポートエミュレーション

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

int input_map[MAX_INPUTS];
int af_interval = 1;
int option_controller;
int neogeo_dipswitch;
int neogeo_input_mode;
int analog_sensitivity;

u8 ALIGN_DATA neogeo_port_value[MVS_PORT_MAX];
int input_analog_value[2];


/******************************************************************************
	ローカル変数
******************************************************************************/

static const u8 hotkey_mask[11] =
{
//	0xef,	// A
//	0xdf,	// B
//	0xbf,	// C
//	0x7f,	// D
	0xcf,	// A+B
	0xaf,	// A+C
	0x6f,	// A+D
	0x9f,	// B+C
	0x5f,	// B+D
	0x3f,	// C+D
	0x8f,	// A+B+C
	0x4f,	// A+B+D
	0x2f,	// A+C+D
	0x1f,	// B+C+D
	0x0f	// A+B+C+D
};

static const u8 autofire_mask[4] =
{
	0x10,	// A
	0x20,	// B
	0x40,	// C
	0x80,	// D
};

static u8 ALIGN_DATA input_flag[MAX_INPUTS];
static int ALIGN_DATA af_flag[NEOGEO_BUTTON_MAX];
static int ALIGN_DATA af_counter[NEOGEO_BUTTON_MAX];
static int input_ui_wait;
static int service_switch;


/*------------------------------------------------------
	入力ポートタイプのチェック
 -----------------------------------------------------*/

void check_input_mode(void)
{
	if (machine_init_type == INIT_ms5pcb
	||	machine_init_type == INIT_svcpcb
	||	machine_init_type == INIT_kf2k3pcb)
	{
		neogeo_input_mode = INPUT_MVS;
		return;
	}
	else if (!neogeo_machine_mode)
	{
		if (memory_region_user1[0x00400 >> 1] & 0x8000)
			neogeo_input_mode = INPUT_AES;
		else
			neogeo_input_mode = INPUT_MVS;
	}
	else
	{
		neogeo_input_mode = neogeo_machine_mode - 1;
	}
}


/*------------------------------------------------------
	入力ポートの初期化
 -----------------------------------------------------*/

void input_init(void)
{
	input_ui_wait = 0;
	service_switch = 0;

	memset(af_counter, 0, sizeof(af_counter));
	memset(af_flag, 0, sizeof(af_flag));
	memset(input_flag, 0, sizeof(input_flag));

	neogeo_dipswitch = 0xff;

	input_analog_value[0] = 0x7f;
	input_analog_value[1] = 0x7f;
}


/*------------------------------------------------------
	入力ポートの終了
 -----------------------------------------------------*/

void input_shutdown(void)
{
}


/*------------------------------------------------------
	入力ポートをリセット
 -----------------------------------------------------*/

void input_reset(void)
{
	service_switch = 0;

	memset(neogeo_port_value, 0, sizeof(neogeo_port_value));

	check_input_mode();

	if (neogeo_input_mode)
		neogeo_port_value[3] = neogeo_dipswitch & 0xff;

	input_analog_value[0] = 0x7f;
	input_analog_value[1] = 0x7f;
}


/*------------------------------------------------------
	連射フラグを更新
 -----------------------------------------------------*/

void update_autofire(void)
{
	int i, button;

	button = P1_AF_A;

	for (i = 0; i < 4; i++)
	{
		if (input_flag[button])
		{
			af_counter[i]++;

			if (af_counter[i] >= af_interval)
			{
				af_counter[i] = 0;
				af_flag[i] ^= 1;
			}
		}
		else
		{
			af_counter[i] = 0;
			af_flag[i] = 1;
		}

		button++;
	}
}


/*------------------------------------------------------
	ホットキーフラグを反映
 -----------------------------------------------------*/

static u8 apply_hotkey(u8 value)
{
	int i, button;

	// autofire
	button = P1_AF_A;
	for (i= 0; i < 4; i++)
	{
		if (input_flag[button])
		{
			if (af_flag[i])
				value &= ~autofire_mask[i];
			else
				value |= autofire_mask[i];
		}
		button++;
	}

	// hotkey
	button = P1_AB;
	for (i= 0; i < 11; i++)
	{
		if (input_flag[button]) value &= hotkey_mask[i];
		button++;
	}

	return value;
}


/*------------------------------------------------------
	NEOGEO コントローラ1
 -----------------------------------------------------*/

static void update_inputport0(void)
{
	u8 value = 0xff;

	switch (neogeo_ngh)
	{
	case NGH_popbounc:
		if (!option_controller)
		{
			int button;
			u8 mask = 0x01;

			for (button = P1_UP; button <= P1_BUTTONC; button++, mask <<= 1)
				if (input_flag[button]) value &= ~mask;
			if (input_flag[P1_BUTTONA]) value &= ~0x80;
		}
		break;

	default:
		if (!option_controller)
		{
			int button;
			u8 mask = 0x01;

			for (button = P1_UP; button <= P1_BUTTOND; button++, mask <<= 1)
				if (input_flag[button]) value &= ~mask;

			value = apply_hotkey(value);
		}
		break;
	}

	neogeo_port_value[0] = value;
}


/*------------------------------------------------------
	NEOGEO コントローラ2
 -----------------------------------------------------*/

static void update_inputport1(void)
{
	u8 value = 0xff;

	switch (neogeo_ngh)
	{
	case NGH_irrmaze:
		{
			int button;
			u8 mask = 0x10;

			for (button = P1_BUTTONA; button <= P1_BUTTONB; button++, mask <<= 1)
				if (input_flag[button]) value &= ~mask;
		}
		break;

	case NGH_popbounc:
		if (option_controller)
		{
			int button;
			u8 mask = 0x01;

			for (button = P1_UP; button <= P1_BUTTONC; button++, mask <<= 1)
				if (input_flag[button]) value &= ~mask;
			if (input_flag[P1_BUTTONA]) value &= ~0x80;
		}
		break;

	default:
		if (option_controller)
		{
			int button;
			u8 mask = 0x01;

			for (button = P1_UP; button <= P1_BUTTOND; button++, mask <<= 1)
				if (input_flag[button]) value &= ~mask;

			value = apply_hotkey(value);
		}
		break;
	}

	neogeo_port_value[1] = value;
}


/*------------------------------------------------------
	NEOGEO スタートボタン
 -----------------------------------------------------*/

static void update_inputport2(void)
{
	u8 value = 0xff;

	switch (neogeo_ngh)
	{
	case NGH_vliner:
		if (input_flag[P1_START]) value &= ~0x01;
		break;

	case NGH_jockeygp:
		break;

	default:
		{
			u8 mask = option_controller ? 0x04 : 0x01;
			if (input_flag[P1_START]) value &= ~mask;
			if (!neogeo_input_mode)
			{
				if (input_flag[P1_COIN]) value &= ~(mask << 1);
			}
		}
		break;
	}

	neogeo_port_value[2] = value;
}


/*------------------------------------------------------
	NEOGEO コイン/サービススイッチ
 -----------------------------------------------------*/

static void update_inputport4(void)
{
	u8 value;

	switch (neogeo_ngh)
	{
	case NGH_vliner:
		{
			static int coin_wait = 0;

			value = 0xff;
			if (coin_wait == 0)
			{
				if (input_flag[P1_COIN])
				{
					value &= ~0x01;
					coin_wait = 12;	// コイン投入ウェイト
				}
			}
			else if (coin_wait)
			{
				// コイン投入ウェイト処理
				if (coin_wait > 4) value &= ~0x01;
				coin_wait--;
			}
			if (input_flag[OTHER1]) value &= ~0x10;
			if (input_flag[OTHER2]) value &= ~0x20;
			if (input_flag[OTHER3]) value &= ~0x80;
		}
		break;

	default:
		value = 0x3f;
		if (neogeo_input_mode)
		{
			u8 mask = option_controller ? 0x02 : 0x01;

			if (input_flag[P1_COIN]) value &= ~mask;
			if (input_flag[SERV_COIN]) value &= ~0x04;
		}
		break;
	}

	neogeo_port_value[4] = value;
}


/*------------------------------------------------------
	NEOGEO テストスイッチ
 -----------------------------------------------------*/

static void update_inputport5(void)
{
	u8 value = 0xc0;

	if (neogeo_input_mode)
	{
		if (input_flag[TEST_SWITCH]) value &= ~0x80;
	}

	neogeo_port_value[5] = value;
}


/*------------------------------------------------------
	irrmaze アナログ入力ポート
 -----------------------------------------------------*/

static void irrmaze_update_analog_port(u16 value)
{
	int axis, delta;
	int current, pad_value[2];

	pad_value[0] = value & 0xff;
	pad_value[1] = value >> 8;

	for (axis = 0; axis < 2; axis++)
	{
		current = pad_value[axis];

		delta = 0;
		if (axis)
		{
			if (input_flag[P1_UP]) delta = -1;
			if (input_flag[P1_DOWN]) delta = 1;
		}
		else
		{
			if (input_flag[P1_LEFT]) delta = -1;
			if (input_flag[P1_RIGHT]) delta = 1;
		}
		switch (analog_sensitivity)
		{
		case 0:
			if (current > 0x80)
			{
				if (current >= 0xe0) delta = 2;
				else if (current >= 0xa0) delta = 1;
			}
			else
			{
				if (current <= 0x1f) delta = -3;
				else if (current <= 0x5f) delta = -1;
			}
			break;

		case 1:
			if (current > 0x80)
			{
				if (current >= 0xf0) delta = 3;
				else if (current >= 0xd0) delta = 2;
				else if (current >= 0xa0) delta = 1;
			}
			else
			{
				if (current <= 0x0f) delta = -3;
				else if (current <= 0x2f) delta = -2;
				else if (current <= 0x5f) delta = -1;
			}
			break;

		case 2:
			if (current > 0x80)
			{
				if (current >= 0xf8) delta = 4;
				else if (current >= 0xe8) delta = 3;
				else if (current >= 0xd0) delta = 2;
				else if (current >= 0x98) delta = 1;
			}
			else
			{
				if (current <= 0x07) delta = -4;
				else if (current <= 0x17) delta = -3;
				else if (current <= 0x2f) delta = -2;
				else if (current <= 0x67) delta = -1;
			}
			break;
		}

		// reverse
		delta = -delta;

		input_analog_value[axis] += delta;
		input_analog_value[axis] &= 0xff;
	}
}


/*------------------------------------------------------
	popbounc アナログ入力ポート
 -----------------------------------------------------*/

static void popbounc_update_analog_port(u16 value)
{
	int delta, current;

	delta = 0;
	current = value & 0xff;

	switch (analog_sensitivity)
	{
	case 0:
		if (current > 0x80)
		{
			if (current >= 0xf0) delta = 3;
			else if (current >= 0xd0) delta = 2;
			else if (current >= 0xa0) delta = 1;
		}
		else
		{
			if (current <= 0x0f) delta = -3;
			else if (current <= 0x2f) delta = -2;
			else if (current <= 0x5f) delta = -1;
		}
		break;

	case 1:
		if (current > 0x80)
		{
			if (current >= 0xf8) delta = 4;
			else if (current >= 0xe8) delta = 3;
			else if (current >= 0xd0) delta = 2;
			else if (current >= 0x98) delta = 1;
		}
		else
		{
			if (current <= 0x07) delta = -4;
			else if (current <= 0x17) delta = -3;
			else if (current <= 0x2f) delta = -2;
			else if (current <= 0x67) delta = -1;
		}
		break;

	case 2:
		if (current > 0x80)
		{
			if (current >= 0xf8) delta = 5;
			else if (current >= 0xe8) delta = 4;
			else if (current >= 0xd8) delta = 3;
			else if (current >= 0xc0) delta = 2;
			else if (current >= 0x98) delta = 1;
		}
		else
		{
			if (current <= 0x07) delta = -5;
			else if (current <= 0x17) delta = -4;
			else if (current <= 0x27) delta = -3;
			else if (current <= 0x3f) delta = -2;
			else if (current <= 0x67) delta = -1;
		}
		break;
	}

	input_analog_value[option_controller] += delta;
	if (input_analog_value[option_controller] < 0)
		input_analog_value[option_controller] = 0;
	if (input_analog_value[option_controller] > 0xff)
		input_analog_value[option_controller] = 0xff;
}


/*------------------------------------------------------
	入力ポートを更新
 -----------------------------------------------------*/

void update_inputport(void)
{
	int i;
	u32 buttons;

	update_autofire();

	if (neogeo_ngh == NGH_irrmaze)
	{
		buttons = poll_gamepad_analog();
		irrmaze_update_analog_port(buttons >> 16);
		buttons &= 0xffff;
	}
	else if (neogeo_ngh == NGH_popbounc)
	{
		buttons = poll_gamepad_analog();
		popbounc_update_analog_port(buttons >> 16);
		buttons &= 0xffff;
	}
	else buttons = poll_gamepad();

	if ((buttons & PSP_CTRL_START) && (buttons & PSP_CTRL_SELECT))
	{
		buttons &= ~(PSP_CTRL_START | PSP_CTRL_SELECT);

		showmenu();

		if (neogeo_input_mode)
			neogeo_port_value[3] = neogeo_dipswitch & 0xff;
		else
			neogeo_port_value[3] = 0xff;
	}

	memset(input_flag, 0, sizeof(input_flag));

	if ((buttons & TEST_SWITCH_FLAGS) == TEST_SWITCH_FLAGS)
	{
		buttons &= ~TEST_SWITCH_FLAGS;
		input_flag[TEST_SWITCH] = 1;
	}

	for (i = 0; i < MAX_INPUTS; i++)
		input_flag[i] |= (buttons & input_map[i]) != 0;

	update_inputport0();
	update_inputport1();
	update_inputport2();
	update_inputport4();
	update_inputport5();

	if (input_flag[SNAPSHOT])
	{
		save_snapshot();
	}
	if (input_flag[SWPLAYER])
	{
		if (!input_ui_wait)
		{
			option_controller ^= 1;
			ui_popup("Controller: Player %d", option_controller + 1);
			input_ui_wait = 30;
		}
	}
	if (input_ui_wait > 0) input_ui_wait--;
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( input )
{
	state_save_long(&option_controller, 1);
	state_save_long(&neogeo_dipswitch, 1);
	state_save_long(&input_analog_value[0], 1);
	state_save_long(&input_analog_value[1], 1);
}

STATE_LOAD( input )
{
	state_load_long(&option_controller, 1);
	state_load_long(&neogeo_dipswitch, 1);
	state_load_long(&input_analog_value[0], 1);
	state_load_long(&input_analog_value[1], 1);

	check_input_mode();
}

#endif /* SAVE_STATE */
