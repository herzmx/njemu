/******************************************************************************

	inptport.c

	CPS1 ���̓|�[�g�G�~�����[�V����

******************************************************************************/

#ifndef CPS1_INPUT_PORT_H
#define CPS1_INPUT_PORT_H

#define CPS1_PORT_MAX		4
#define CPS1_BUTTON_MAX		6

enum
{
	P1_RIGHT = 0,	// 0
	P1_LEFT,		// 1
	P1_DOWN,		// 2
	P1_UP,			// 3
	P1_BUTTON1,		// 4
	P1_BUTTON2,		// 5
	P1_BUTTON3,		// 6
	P1_BUTTON4,		// 7
	P1_BUTTON5,		// 8
	P1_BUTTON6,		// 9
	P1_COIN,		// 10
	P1_START,		// 11

	P1_DIAL_L,		// 12
	P1_DIAL_R,		// 13

	SERV_COIN,		// 14
	SERV_SWITCH,	// 15

	P1_AF_1,		// 16
	P1_AF_2,		// 17
	P1_AF_3,		// 18
	P1_AF_4,		// 19
	P1_AF_5,		// 20
	P1_AF_6,		// 21

	SNAPSHOT,		// 22
	SWPLAYER,		// 23

	MAX_INPUTS
};

enum
{
	INPUT_PLAYER1 = 0,
	INPUT_PLAYER2,
	INPUT_PLAYER3,
	INPUT_PLAYER4
};

enum
{
	COIN_NONE = 0,	// 2P 2�V���[�^�[�Œ� (�`�F�b�N�K�v�Ȃ�)
	COIN_2P1C,		// 2P 1�V���[�^�[
	COIN_2P2C,		// 2P 2�V���[�^�[
	COIN_3P1C,		// 3P 1�V���[�^�[
	COIN_3P2C,		// 3P 2�V���[�^�[
	COIN_3P3C,		// 3P 3�V���[�^�[
	COIN_4P1C,		// 4P 1�V���[�^�[
	COIN_4P2C,		// 4P 2�V���[�^�[
	COIN_4P4C,		// 4P 4�V���[�^�[
	COIN_MAX
};

enum
{
	DIP_A = 0,
	DIP_B,
	DIP_C
};


extern int option_controller;
extern UINT16 cps1_port_value[CPS1_PORT_MAX];
extern int cps1_dipswitch[3];

extern int input_map[MAX_INPUTS];
extern int input_max_players;
extern int input_max_buttons;
extern int analog_sensitivity;
extern int af_interval;

#ifdef ADHOC
void adhoc_input_init(void);
#endif

void input_init(void);
void input_shutdown(void);
void input_reset(void);
void update_inputport(void);

void setup_autofire(void);

UINT16 forgottn_read_dial0(void);
UINT16 forgottn_read_dial1(void);

#ifdef SAVE_STATE
STATE_SAVE( input );
STATE_LOAD( input );
#endif

#endif /* CPS1_INPUT_PORT_H */
