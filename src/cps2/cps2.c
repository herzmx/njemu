/******************************************************************************

	cps2.c

	CPS2�G�~�����[�V�����R�A

******************************************************************************/

#include "cps2.h"


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*--------------------------------------------------------
	CPS2�G�~�����[�V����������
--------------------------------------------------------*/

static int cps2_init(void)
{
	if (!cps2_driver_init())
		return 0;

	msg_printf(TEXT(DONE2));
	msg_screen_clear();

	video_clear_screen();

#ifdef ADHOC
	if (!cps2_video_init())
		return 0;

	if (adhoc_enable)
	{
		sprintf(adhoc_matching, "%s_%s", PBPNAME_STR, game_name);

		Loop = LOOP_EXEC;

		if (adhocInit(adhoc_matching) == 0)
		{
			if ((adhoc_server = adhocSelect()) >= 0)
			{
				video_clear_screen();

				if (adhoc_server)
				{
					option_controller = INPUT_PLAYER1;

					return adhoc_send_state(NULL);
				}
				else
				{
					option_controller = INPUT_PLAYER2;

					return adhoc_recv_state(NULL);
				}
			}
		}

		Loop = LOOP_BROWSER;
		return 0;
	}

	return 1;
#else
	return cps2_video_init();
#endif
}


/*--------------------------------------------------------
	CPS2�G�~�����[�V�������Z�b�g
--------------------------------------------------------*/

static void cps2_reset(void)
{
	video_set_mode(16);
	video_clear_screen();

	Loop = LOOP_EXEC;

	autoframeskip_reset();

	cps2_driver_reset();
	cps2_video_reset();

	timer_reset();
	input_reset();
	sound_reset();

	blit_clear_all_sprite();
}


/*--------------------------------------------------------
	CPS�G�~�����[�V�����I��
--------------------------------------------------------*/

static void cps2_exit(void)
{
	video_set_mode(32);
	video_clear_screen();

	ui_popup_reset();

	video_clear_screen();
	msg_screen_init(WP_LOGO, ICON_SYSTEM, TEXT(EXIT_EMULATION2));

	msg_printf(TEXT(PLEASE_WAIT2));

	cps2_video_exit();
	cps2_driver_exit();

#ifdef ADHOC
	if (!adhoc_enable)
#endif
	{
#ifdef COMMAND_LIST
		free_commandlist();
#endif
		save_gamecfg(game_name);
	}

	msg_printf(TEXT(DONE2));

#ifdef ADHOC
	if (adhoc_enable) adhocTerm();
#endif

	show_exit_screen();
}


/*--------------------------------------------------------
	CPS�G�~�����[�V�������s
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
#if USE_CACHE
				cache_sleep(1);
#endif

				do
				{
					sceKernelDelayThread(5000000);
				} while (Sleep);

#if USE_CACHE
				cache_sleep(0);
#endif
				autoframeskip_reset();
			}

			timer_update_cpu();
			update_screen();
			update_inputport();
		}

		video_clear_screen();
		sound_mute(1);
	}
}


/******************************************************************************
	�O���[�o���֐�
******************************************************************************/

/*--------------------------------------------------------
	CPS2�G�~�����[�V�������C��
--------------------------------------------------------*/

void cps2_main(void)
{
	Loop = LOOP_RESET;

	while (Loop >= LOOP_RESTART)
	{
		Loop = LOOP_EXEC;

		ui_popup_reset();

		fatal_error = 0;

		video_clear_screen();

		if (memory_init())
		{
			if (sound_init())
			{
				if (input_init())
				{
					if (cps2_init())
					{
						cps2_run();
					}
					cps2_exit();
				}
				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}
