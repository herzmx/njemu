/******************************************************************************

	mvs.c

	MVSエミュレーションコア

******************************************************************************/

#include "mvs.h"


/******************************************************************************
	グローバル変数
******************************************************************************/

int neogeo_bios;
int neogeo_region;


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	MVSエミュレーション初期化
--------------------------------------------------------*/

static int neogeo_init(void)
{
	FILE *fp;
	char path[MAX_PATH];

	// memory card
	sprintf(path, "%smemcard/%s.bin", launchDir, game_name);
	if ((fp = fopen(path, "rb")) != NULL)
	{
		fread(neogeo_memcard, 1, 0x800, fp);
		fclose(fp);
	}

	// sram
	sprintf(path, "%snvram/%s.nv", launchDir, game_name);
	if ((fp = fopen(path, "rb")) != NULL)
	{
		fread(neogeo_sram16, 1, 0x2000, fp);
		fclose(fp);
		swab(neogeo_sram16, neogeo_sram16, 0x2000);
	}

	neogeo_driver_init();
	neogeo_video_init();

	return 1;
}


/*--------------------------------------------------------
	MVSエミュレーションリセット
--------------------------------------------------------*/

static void neogeo_reset(void)
{
	autoframeskip_reset();

	timer_reset();
	input_reset();
	sound_reset();

	neogeo_driver_reset();
	neogeo_video_reset();

	Loop = LOOP_EXEC;
}


/*--------------------------------------------------------
	MVSエミュレーション終了
--------------------------------------------------------*/

static void neogeo_exit(void)
{
	FILE *fp;
	char path[MAX_PATH];

	ui_popup_reset(POPUP_MENU);

	video_clear_screen();
	msg_screen_init("Exit emulation");

	msg_printf("Please wait.\n");

	sprintf(path, "%smemcard/%s.bin", launchDir, game_name);
	if ((fp = fopen(path, "wb")) != NULL)
	{
		fwrite(neogeo_memcard, 1, 0x800, fp);
		fclose(fp);
	}

	sprintf(path, "%snvram/%s.nv", launchDir, game_name);
	if ((fp = fopen(path, "wb")) != NULL)
	{
		swab(neogeo_sram16, neogeo_sram16, 0x2000);
		fwrite(neogeo_sram16, 1, 0x2000, fp);
		fclose(fp);
	}

	save_gamecfg(game_name);

	msg_printf("Done.\n");

	show_exit_screen();
}


/*--------------------------------------------------------
	MVSエミュレーション実行
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

			if (neogeo_driver_type == NORMAL)
				timer_update_cpu();
			else
				timer_update_cpu_raster();

			if (!skip_this_frame())
			{
				if (neogeo_driver_type == NORMAL)
					neogeo_screenrefresh();
				else
					neogeo_raster_screenrefresh();
			}

			update_screen();
			update_inputport();
		}

		video_clear_screen();
		sound_mute(1);
	}
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	MVSエミュレーションメイン
--------------------------------------------------------*/

void neogeo_main(void)
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

				if (neogeo_init())
				{
					neogeo_run();
				}
				neogeo_exit();

				input_shutdown();
			}
			sound_exit();
		}
		memory_shutdown();
		show_fatal_error();
	}
}
