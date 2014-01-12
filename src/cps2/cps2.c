/******************************************************************************

	cps2.c

	CPS2エミュレーションコア

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	CPS2エミュレーション初期化
--------------------------------------------------------*/

static int cps2_init(void)
{
	cps2_driver_init();

	return cps2_video_init();
}


/*--------------------------------------------------------
	CPS2エミュレーションリセット
--------------------------------------------------------*/

static void cps2_reset(void)
{
	autoframeskip_reset();

	cps2_driver_reset();
	cps2_video_reset();

	timer_reset();
	input_reset();
	sound_reset();

	Loop = LOOP_EXEC;
}


/*--------------------------------------------------------
	CPSエミュレーション終了
--------------------------------------------------------*/

static void cps2_exit(void)
{
	ui_popup_reset(POPUP_MENU);

	video_clear_screen();
	msg_screen_init("Exit emulation");

	msg_printf("Please wait.\n");

	cps2_video_exit();
	cps2_driver_exit();
	save_gamecfg(game_name);

	msg_printf("Done.\n");

	show_exit_screen();
}


/*--------------------------------------------------------
	CPSエミュレーション実行
--------------------------------------------------------*/

static void cps2_run(void)
{
	while (Loop >= LOOP_RESET)
	{
		cps2_reset();

		while (Loop == LOOP_EXEC)
		{
			if (Sleep)
			{
				cache_sleep(1);

				do
				{
					sceKernelDelayThread(5000000);
				} while (Sleep);

				cache_sleep(0);
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
	CPS2エミュレーションメイン
--------------------------------------------------------*/

void cps2_main(void)
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

				if (cps2_init())
				{
					cps2_run();
				}
				cps2_exit();

				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}
