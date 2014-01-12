/******************************************************************************

	inptport.c

	CPS1 入力ポートエミュレーション

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

int option_controller;
int cps1_dipswitch[3];
u16 ALIGN_DATA cps1_port_value[CPS1_PORT_MAX];

int ALIGN_DATA input_map[MAX_INPUTS];
int input_max_players;
int input_max_buttons;
int analog_sensitivity;
int af_interval = 1;


/******************************************************************************
	ローカル変数
******************************************************************************/

static u8 ALIGN_DATA input_flag[MAX_INPUTS];
static int ALIGN_DATA af_map1[CPS1_BUTTON_MAX];
static int ALIGN_DATA af_map2[CPS1_BUTTON_MAX];
static int ALIGN_DATA af_flag[CPS1_BUTTON_MAX];
static int ALIGN_DATA af_counter[CPS1_BUTTON_MAX];
static int input_analog_value[2];
static int input_ui_wait;
static int service_switch;


/******************************************************************************
	ローカル関数
******************************************************************************/

/*------------------------------------------------------
	連射フラグを更新
------------------------------------------------------*/

static u32 update_autofire(u32 buttons)
{
	int i;

	for (i = 0; i < input_max_buttons; i++)
	{
		if (af_map1[i])
		{
			if (buttons & af_map1[i])
			{
				buttons &= ~af_map1[i];

				af_counter[i]++;

				if (af_counter[i] >= af_interval)
				{
					af_counter[i] = 0;
					af_flag[i] ^= 1;
				}

				if (af_flag[i])
					buttons &= ~af_map2[i];
				else
					buttons |= af_map2[i];
			}
			else
			{
				af_counter[i] = 0;
				af_flag[i] = 0;
			}
		}
	}

	return buttons;
}


/*------------------------------------------------------
	CPS1 ポート0 (START / COIN)
------------------------------------------------------*/

static void update_inputport0(void)
{
	u16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_sfzch:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON5]) value &= ~0x0101;
			if (input_flag[P1_COIN])    value &= ~0x0404;
			if (input_flag[P1_START])   value &= ~0x1010;
			if (input_flag[P1_BUTTON6]) value &= ~0x4040;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON5]) value &= ~0x0202;
			if (input_flag[P1_COIN])    value &= ~0x0808;
			if (input_flag[P1_START])   value &= ~0x2020;
			if (input_flag[P1_BUTTON6]) value &= ~0x8080;
		}
		break;

	default:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_COIN])  value &= ~0x0101;
			if (input_flag[P1_START]) value &= ~0x1010;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_COIN])  value &= ~0x0202;
			if (input_flag[P1_START]) value &= ~0x2020;
		}
		if (input_flag[SERV_COIN])
		{
			value &= ~0x0400;
		}
		break;
	}

	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
	case INPTYPE_mercs:
	case INPTYPE_sfzch:
		break;

	case INPTYPE_ghouls:
	case INPTYPE_ghoulsu:
	case INPTYPE_daimakai:
		if (input_flag[SERV_SWITCH])
		{
			if (!input_ui_wait)
			{
				service_switch ^= 0x4040;
				if (service_switch == 0)
					Loop = LOOP_RESTART;
				else
					Loop = LOOP_RESET;
				input_ui_wait = 30;
			}
		}
		value &= ~service_switch;
		break;

	case INPTYPE_strider:
	case INPTYPE_stridrua:
	case INPTYPE_dynwar:
	case INPTYPE_ffight:
	case INPTYPE_1941:
		if (input_flag[SERV_SWITCH])
		{
			if (!input_ui_wait)
			{
				service_switch ^= 0x4040;
				Loop = LOOP_RESET;
				input_ui_wait = 30;
			}
		}
		value &= ~service_switch;
		break;

	default:
		if (input_flag[SERV_SWITCH])
		{
			value &= ~0x4040;
		}
		break;
	}

	cps1_port_value[0] = value;
}


/*------------------------------------------------------
	CPS1 ポート1 (コントローラ1/2)
------------------------------------------------------*/

static void update_inputport1(void)
{
	u16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_dynwar:
	case INPTYPE_ffight:	// button 3 (cheat)
	case INPTYPE_mtwins:
	case INPTYPE_3wonders:
	case INPTYPE_pnickj:
	case INPTYPE_pang3:
	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
	case INPTYPE_sf2:
	case INPTYPE_sf2j:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
		}
		break;

	case INPTYPE_cworld2j:
	case INPTYPE_qad:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
			if (input_flag[P1_BUTTON4]) value &= ~0x0080;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
			if (input_flag[P1_BUTTON4]) value &= ~0x8000;
		}
		break;

	case INPTYPE_forgottn:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
		}
		else if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
		}
		break;

	case INPTYPE_slammast:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
		}
		else if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_BUTTON3]) value &= ~0x0080;
		}
		else if (option_controller == INPUT_PLAYER4)
		{
			if (input_flag[P1_BUTTON3]) value &= ~0x8000;
		}
		break;

	case INPTYPE_sfzch:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
			if (input_flag[P1_BUTTON3]) value &= ~0x0040;
			if (input_flag[P1_BUTTON4]) value &= ~0x0080;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
			if (input_flag[P1_BUTTON3]) value &= ~0x4000;
			if (input_flag[P1_BUTTON4]) value &= ~0x8000;
		}
		break;

	default:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0001;
			if (input_flag[P1_LEFT])    value &= ~0x0002;
			if (input_flag[P1_DOWN])    value &= ~0x0004;
			if (input_flag[P1_UP])      value &= ~0x0008;
			if (input_flag[P1_BUTTON1]) value &= ~0x0010;
			if (input_flag[P1_BUTTON2]) value &= ~0x0020;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0100;
			if (input_flag[P1_LEFT])    value &= ~0x0200;
			if (input_flag[P1_DOWN])    value &= ~0x0400;
			if (input_flag[P1_UP])      value &= ~0x0800;
			if (input_flag[P1_BUTTON1]) value &= ~0x1000;
			if (input_flag[P1_BUTTON2]) value &= ~0x2000;
		}
		break;
	}

	cps1_port_value[1] = value;
}


/*------------------------------------------------------
	CPS1 ポート2 (コントローラ3 / 追加ボタン)
------------------------------------------------------*/

static void update_inputport2(void)
{
	u16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_mercs:
	case INPTYPE_kod:
	case INPTYPE_kodj:
	case INPTYPE_knights:
	case INPTYPE_wof:
	case INPTYPE_dino:
	case INPTYPE_captcomm:
	case INPTYPE_slammast:
		if (option_controller == INPUT_PLAYER3)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0101;
			if (input_flag[P1_LEFT])    value &= ~0x0202;
			if (input_flag[P1_DOWN])    value &= ~0x0404;
			if (input_flag[P1_UP])      value &= ~0x0808;
			if (input_flag[P1_BUTTON1]) value &= ~0x1010;
			if (input_flag[P1_BUTTON2]) value &= ~0x2020;
			if (input_flag[P1_COIN])    value &= ~0x4040;
			if (input_flag[P1_START])   value &= ~0x8080;
		}
		break;

	case INPTYPE_sf2:
	case INPTYPE_sf2j:
		if (option_controller == INPUT_PLAYER1)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x0101;
			if (input_flag[P1_BUTTON5]) value &= ~0x0202;
			if (input_flag[P1_BUTTON6]) value &= ~0x0404;
		}
		else if (option_controller == INPUT_PLAYER2)
		{
			if (input_flag[P1_BUTTON4]) value &= ~0x1010;
			if (input_flag[P1_BUTTON5]) value &= ~0x2020;
			if (input_flag[P1_BUTTON6]) value &= ~0x4040;
		}
		break;
	}

	cps1_port_value[2] = value;
}


/*------------------------------------------------------
	CPS1 ポート3 (コントローラ4)
------------------------------------------------------*/

static void update_inputport3(void)
{
	u16 value = 0xffff;

	switch (machine_input_type)
	{
	case INPTYPE_captcomm:
	case INPTYPE_slammast:
		if (option_controller == INPUT_PLAYER4)
		{
			if (input_flag[P1_RIGHT])   value &= ~0x0101;
			if (input_flag[P1_LEFT])    value &= ~0x0202;
			if (input_flag[P1_DOWN])    value &= ~0x0404;
			if (input_flag[P1_UP])      value &= ~0x0808;
			if (input_flag[P1_BUTTON1]) value &= ~0x1010;
			if (input_flag[P1_BUTTON2]) value &= ~0x2020;
			if (input_flag[P1_COIN])    value &= ~0x4040;
			if (input_flag[P1_START])   value &= ~0x8080;
		}
		break;
	}

	cps1_port_value[3] = value;
}


/*------------------------------------------------------
	forgottn アナログ入力ポート
------------------------------------------------------*/

static void forgottn_update_dial(void)
{
	int delta = 0;

	if (input_flag[P1_DIAL_L])
	{
		switch (analog_sensitivity)
		{
		case 0: delta -= 10; break;
		case 1: delta -= 20; break;
		case 2: delta -= 30; break;
		}
	}
	if (input_flag[P1_DIAL_R])
	{
		switch (analog_sensitivity)
		{
		case 0: delta += 10; break;
		case 1: delta += 20; break;
		case 2: delta += 30; break;
		}
	}
	input_analog_value[option_controller] = (input_analog_value[option_controller] + delta) & 0xfff;
}


u16 forgottn_read_dial0(void)
{
	return input_analog_value[0];
}

u16 forgottn_read_dial1(void)
{
	return input_analog_value[1];
}


/*------------------------------------------------------
	入力ボタンを画面方向に合わせて調整
------------------------------------------------------*/

static u32 adjust_input(u32 buttons)
{
	u32 buttons2;

	if (!cps_flip_screen && machine_screen_type != SCREEN_VERTICAL)
		return buttons;

	if (!machine_screen_type)
	{
		if (cps_flip_screen)
		{
			buttons2 = buttons & (PSP_CTRL_START | PSP_CTRL_SELECT);

			if (buttons & PSP_CTRL_UP)       buttons2 |= PSP_CTRL_DOWN;
			if (buttons & PSP_CTRL_DOWN)     buttons2 |= PSP_CTRL_UP;
			if (buttons & PSP_CTRL_RIGHT)    buttons2 |= PSP_CTRL_LEFT;
			if (buttons & PSP_CTRL_LEFT)     buttons2 |= PSP_CTRL_RIGHT;
			if (buttons & PSP_CTRL_SQUARE)   buttons2 |= PSP_CTRL_CIRCLE;
			if (buttons & PSP_CTRL_CIRCLE)   buttons2 |= PSP_CTRL_SQUARE;
			if (buttons & PSP_CTRL_TRIANGLE) buttons2 |= PSP_CTRL_CROSS;
			if (buttons & PSP_CTRL_CROSS)    buttons2 |= PSP_CTRL_TRIANGLE;
			if (buttons & PSP_CTRL_RTRIGGER) buttons2 |= PSP_CTRL_LTRIGGER;
			if (buttons & PSP_CTRL_LTRIGGER) buttons2 |= PSP_CTRL_RTRIGGER;

			buttons = buttons2;
		}
	}
	else
	{
		if (!cps_rotate_screen)
		{
			buttons2 = buttons & (PSP_CTRL_START | PSP_CTRL_SELECT | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER);

			if (buttons & PSP_CTRL_UP)       buttons2 |= PSP_CTRL_LEFT;
			if (buttons & PSP_CTRL_DOWN)     buttons2 |= PSP_CTRL_RIGHT;
			if (buttons & PSP_CTRL_RIGHT)    buttons2 |= PSP_CTRL_UP;
			if (buttons & PSP_CTRL_LEFT)     buttons2 |= PSP_CTRL_DOWN;
			if (buttons & PSP_CTRL_TRIANGLE) buttons2 |= PSP_CTRL_SQUARE;
			if (buttons & PSP_CTRL_CIRCLE)   buttons2 |= PSP_CTRL_TRIANGLE;
			if (buttons & PSP_CTRL_SQUARE)   buttons2 |= PSP_CTRL_CROSS;
			if (buttons & PSP_CTRL_CROSS)    buttons2 |= PSP_CTRL_CIRCLE;

			buttons = buttons2;
		}

		if (cps_flip_screen)
		{
			buttons2 = buttons & (PSP_CTRL_START | PSP_CTRL_SELECT);

			if (buttons & PSP_CTRL_UP)       buttons2 |= PSP_CTRL_DOWN;
			if (buttons & PSP_CTRL_DOWN)     buttons2 |= PSP_CTRL_UP;
			if (buttons & PSP_CTRL_RIGHT)    buttons2 |= PSP_CTRL_LEFT;
			if (buttons & PSP_CTRL_LEFT)     buttons2 |= PSP_CTRL_RIGHT;
			if (buttons & PSP_CTRL_SQUARE)   buttons2 |= PSP_CTRL_CIRCLE;
			if (buttons & PSP_CTRL_CIRCLE)   buttons2 |= PSP_CTRL_SQUARE;
			if (buttons & PSP_CTRL_TRIANGLE) buttons2 |= PSP_CTRL_CROSS;
			if (buttons & PSP_CTRL_CROSS)    buttons2 |= PSP_CTRL_TRIANGLE;
			if (buttons & PSP_CTRL_RTRIGGER) buttons2 |= PSP_CTRL_LTRIGGER;
			if (buttons & PSP_CTRL_LTRIGGER) buttons2 |= PSP_CTRL_RTRIGGER;

			buttons = buttons2;
		}
	}

	return buttons;
}


/******************************************************************************
	入力ポートインタフェース関数
******************************************************************************/

/*------------------------------------------------------
	入力ポートの初期化
------------------------------------------------------*/

void input_init(void)
{
	input_ui_wait = 0;
	service_switch = 0;

	memset(cps1_port_value, 0xff, sizeof(cps1_port_value));

	memset(af_flag, 0, sizeof(af_flag));
	memset(af_counter, 0, sizeof(af_counter));

	memset(input_flag, 0, sizeof(input_flag));

	input_analog_value[0] = 0;
	input_analog_value[1] = 0;

	switch (machine_input_type)
	{
	case INPTYPE_mercs:
	case INPTYPE_kod:
	case INPTYPE_kodj:
	case INPTYPE_knights:
	case INPTYPE_wof:
	case INPTYPE_dino:
		input_max_players = 3;
		break;

	case INPTYPE_captcomm:
	case INPTYPE_slammast:
		input_max_players = 4;
		break;

	default:
		input_max_players = 2;
		break;
	}

	switch (machine_input_type)
	{
	case INPTYPE_forgottn:
		input_max_buttons = 1;
		break;

	case INPTYPE_dynwar:
	case INPTYPE_ffight:	// button 3 (cheat)
	case INPTYPE_mtwins:
	case INPTYPE_3wonders:
	case INPTYPE_pnickj:
	case INPTYPE_pang3:
	case INPTYPE_megaman:
	case INPTYPE_rockmanj:
	case INPTYPE_slammast:
		input_max_buttons = 3;
		break;

	case INPTYPE_cworld2j:
	case INPTYPE_qad:
	case INPTYPE_qadj:
	case INPTYPE_qtono2:
		input_max_buttons = 4;
		break;

	case INPTYPE_sf2:
	case INPTYPE_sf2j:
	case INPTYPE_sfzch:
		input_max_buttons = 6;
		break;

	default:
		input_max_buttons = 2;
		break;
	}
}


/*------------------------------------------------------
	入力ポートの終了
------------------------------------------------------*/

void input_shutdown(void)
{
}


/*------------------------------------------------------
	入力ポートをリセット
------------------------------------------------------*/

void input_reset(void)
{
	memset(cps1_port_value, 0xff, sizeof(cps1_port_value));
	input_analog_value[0] = 0;
	input_analog_value[1] = 0;

	setup_autofire();
}


/*------------------------------------------------------
	連射フラグを設定
------------------------------------------------------*/

void setup_autofire(void)
{
	int i;

	memset(af_map1, 0, sizeof(af_map1));
	memset(af_map2, 0, sizeof(af_map2));

	for (i = 0; i < input_max_buttons; i++)
	{
		af_map1[i] = input_map[P1_AF_1 + i];
		af_map2[i] = input_map[P1_BUTTON1 + i];
	}
}


/*------------------------------------------------------
	入力ポートを更新
------------------------------------------------------*/

void update_inputport(void)
{
	int i;
	u32 buttons;

	buttons = poll_gamepad();

	if ((buttons & PSP_CTRL_START) && (buttons & PSP_CTRL_SELECT))
	{
		buttons &= ~(PSP_CTRL_START | PSP_CTRL_SELECT);

		showmenu();
		setup_autofire();
	}
	buttons = adjust_input(buttons);
	buttons = update_autofire(buttons);

	for (i = 0; i < MAX_INPUTS; i++)
		input_flag[i] = (buttons & input_map[i]) != 0;

	if ((buttons & PSP_CTRL_SELECT) && (buttons & PSP_CTRL_LTRIGGER) && (buttons & PSP_CTRL_RTRIGGER))
	{
		input_flag[SERV_SWITCH] = 1;
	}

	update_inputport0();
	update_inputport1();
	update_inputport2();
	update_inputport3();
	if (machine_input_type == INPTYPE_forgottn) forgottn_update_dial();

	if (input_flag[SNAPSHOT])
	{
		save_snapshot();
	}
	if (input_flag[SWPLAYER])
	{
		if (!input_ui_wait)
		{
			option_controller++;
			if (option_controller == input_max_players)
				option_controller = INPUT_PLAYER1;
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
	state_save_long(&input_analog_value[0], 1);
	state_save_long(&input_analog_value[1], 1);
	state_save_long(&cps1_dipswitch[0], 1);
	state_save_long(&cps1_dipswitch[1], 1);
	state_save_long(&cps1_dipswitch[2], 1);
	state_save_byte(&service_switch, 1);
}

STATE_LOAD( input )
{
	state_load_long(&option_controller, 1);
	state_load_long(&input_analog_value[0], 1);
	state_load_long(&input_analog_value[1], 1);
	state_load_long(&cps1_dipswitch[0], 1);
	state_load_long(&cps1_dipswitch[1], 1);
	state_load_long(&cps1_dipswitch[2], 1);
	state_load_byte(&service_switch, 1);

	setup_autofire();
	input_ui_wait = 0;
}

#endif /* SAVE_STATE */
