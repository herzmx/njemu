/******************************************************************************

	mvs.c

	MVS�G�~�����[�V�����R�A

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	�O���[�o���ϐ�
******************************************************************************/

int neogeo_bios;
int neogeo_region;
int neogeo_save_sound_flag;


/******************************************************************************
	���[�J���ϐ�
******************************************************************************/

#ifdef ADHOC
static const char *bios[] =
{
	"EURO2", "EURO1",
	"USA2", "USA1",
	"ASIA3",
	"JPN3", "JPN2", "JPN1",
	"AES"
};
#endif


/******************************************************************************
	���[�J���֐�
******************************************************************************/

/*--------------------------------------------------------
	MVS�G�~�����[�V����������
--------------------------------------------------------*/

static int neogeo_init(void)
{
	SceUID fd;
	char path[MAX_PATH];

#ifdef ADHOC
	if (!adhoc_enable)
#endif
	{
		sprintf(path, "%smemcard/%s.bin", launchDir, game_name);
		if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0)
		{
			sceIoRead(fd, neogeo_memcard, 0x800);
			sceIoClose(fd);
		}

		sprintf(path, "%snvram/%s.nv", launchDir, game_name);
		if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0)
		{
			sceIoRead(fd, neogeo_sram16, 0x2000);
			sceIoClose(fd);
			swab(neogeo_sram16, neogeo_sram16, 0x2000);
		}
	}

	neogeo_driver_init();
	neogeo_video_init();

	msg_printf(TEXT(DONE2));
	msg_screen_clear();

	video_clear_screen();

#ifdef ADHOC
	if (adhoc_enable)
	{
		sprintf(adhoc_matching, "%s_%s_%s", PBPNAME_STR, game_name, bios[neogeo_bios]);

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
#endif

	return 1;
}


/*--------------------------------------------------------
	MVS�G�~�����[�V�������Z�b�g
--------------------------------------------------------*/

static void neogeo_reset(void)
{
	video_set_mode(16);

	video_clear_screen();

	timer_reset();
	input_reset();

	neogeo_driver_reset();
	neogeo_video_reset();

	sound_reset();
	blit_clear_all_sprite();
	autoframeskip_reset();

	Loop = LOOP_EXEC;
}


/*--------------------------------------------------------
	MVS�G�~�����[�V�����I��
--------------------------------------------------------*/

static void neogeo_exit(void)
{
	SceUID fd;
	char path[MAX_PATH];

	video_set_mode(32);
	video_clear_screen();

	ui_popup_reset();

	video_clear_screen();
	msg_screen_init(WP_LOGO, ICON_SYSTEM, TEXT(EXIT_EMULATION2));

	msg_printf(TEXT(PLEASE_WAIT2));

#ifdef ADHOC
	if (!adhoc_enable)
#endif
	{
		sprintf(path, "%smemcard/%s.bin", launchDir, game_name);
		if ((fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777)) >= 0)
		{
			sceIoWrite(fd, neogeo_memcard, 0x800);
			sceIoClose(fd);
		}

		sprintf(path, "%snvram/%s.nv", launchDir, game_name);
		if ((fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777)) >= 0)
		{
			swab(neogeo_sram16, neogeo_sram16, 0x2000);
			sceIoWrite(fd, neogeo_sram16, 0x2000);
			sceIoClose(fd);
		}


#ifdef COMMAND_LIST
		free_commandlist();
#endif

		if (neogeo_save_sound_flag) option_sound_enable = 1;
		save_gamecfg(game_name);
	}

	msg_printf(TEXT(DONE2));

#ifdef ADHOC
	if (adhoc_enable) adhocTerm();
#endif

	show_exit_screen();
}


/*--------------------------------------------------------
	MVS�G�~�����[�V�������s
--------------------------------------------------------*/

static void neogeo_run(void)
{
	while (Loop >= LOOP_RESET)
	{
		neogeo_reset();

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
	MVS�G�~�����[�V�������C��
--------------------------------------------------------*/

void neogeo_main(void)
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
					if (neogeo_init())
					{
						neogeo_run();
					}
					neogeo_exit();
				}
				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}
