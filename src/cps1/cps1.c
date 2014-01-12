/******************************************************************************

	cps1.c

	CPS1エミュレーションコア

******************************************************************************/

#include "cps1.h"


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	CPS1エミュレーション初期化
--------------------------------------------------------*/

static int cps1_init(void)
{
	cps1_driver_init();

	return cps1_video_init();
}


/*--------------------------------------------------------
	CPS1エミュレーションリセット
--------------------------------------------------------*/

static void cps1_reset(void)
{
	autoframeskip_reset();

	cps1_driver_reset();
	cps1_video_reset();

	timer_reset();
	input_reset();
	sound_reset();

	Loop = LOOP_EXEC;
}


/*--------------------------------------------------------
	CPSエミュレーション終了
--------------------------------------------------------*/

static void cps1_exit(void)
{
	ui_popup_reset(POPUP_MENU);

	video_clear_screen();
	msg_screen_init("Exit emulation");

	msg_printf("Please wait.\n");

	cps1_video_exit();
	cps1_driver_exit();
	save_gamecfg(game_name);

	msg_printf("Done.\n");

	show_exit_screen();
}


/*--------------------------------------------------------
	CPS1エミュレーション実行
--------------------------------------------------------*/

static void cps1_run(void)
{
	while (Loop >= LOOP_RESET)
	{
		cps1_reset();

		while (Loop == LOOP_EXEC)
		{
			if (Sleep)
			{
				do
				{
					sceKernelDelayThread(5000000);
				} while (Sleep);

				autoframeskip_reset();
			}

			timer_update_cpu();
			update_inputport();
			update_screen();
		}

		video_clear_screen();
		sound_mute(1);
	}
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	CPS1エミュレーションメイン
--------------------------------------------------------*/

void cps1_main(void)
{
	Loop = LOOP_RESET;

	while (Loop >= LOOP_RESTART)
	{
		Loop = LOOP_EXEC;

		ui_popup_reset(POPUP_GAME);

		fatal_error = 0;

		video_clear_screen();

		if (memory_init())
		{
			if (sound_init())
			{
				input_init();

				if (cps1_init())
				{
					cps1_run();
				}
				cps1_exit();

				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}
