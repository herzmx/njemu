/******************************************************************************

	menu.c

	PSP メニュー

******************************************************************************/

#include "psp.h"
#include "emumain.h"


#define MENU_BLANK	{ "\n", }
#define MENU_RETURN	{ "Return to main menu", }
#define MENU_END	{ "\0", }


/*------------------------------------------------------
	エミュレーションリセット
------------------------------------------------------*/

int menu_reset(void)
{
	if (messagebox(MB_RESETEMULATION))
	{
		video_clear_screen();
		video_flip_screen(1);
		Loop = LOOP_RESET;
		Sleep = 0;
		return 1;
	}
	return 0;
}


/*------------------------------------------------------
	エミュレーション再起動
------------------------------------------------------*/

int menu_restart(void)
{
	if (messagebox(MB_RESTARTEMULATION))
	{
		video_clear_screen();
		video_flip_screen(1);
		Loop = LOOP_RESTART;
		Sleep = 0;
	}
	return 1;
}


/*------------------------------------------------------
	ファイルブラウザに戻る
------------------------------------------------------*/

int menu_browser(void)
{
	Loop = LOOP_BROWSER;
	Sleep = 0;
	return 1;
}


/*------------------------------------------------------
	エミュレータ終了
------------------------------------------------------*/

int menu_exit(void)
{
	if (messagebox(MB_EXITEMULATOR))
	{
		Loop = LOOP_EXIT;
		Sleep = 0;
		return 1;
	}
	return 0;
}


/*------------------------------------------------------
	ゲーム設定メニュー
------------------------------------------------------*/

typedef struct {
	const char *label;
	int *value;
	int flag;
	int old_value;
	int value_max;
	const char *values_label[12];
} gamecfg_t;


#define CFG_CONTINUE	0
#define CFG_RESTART		1

#define INCLUDE_GAMECFG_STRUCT

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "menu/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "menu/mvs.c"
#endif

#undef INCLUDE_GAMECFG_STRUCT

static int menu_gamecfg(void)
{
	int sel = 0, rows = 13, top = 0;
	int i, arrowl, arrowr, prev_sel, update = 1;
	gamecfg_t *gamecfg;
	int gamecfg_num = 0;

#define INCLUDE_GAMECFG_MENU

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "menu/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "menu/mvs.c"
#endif

#undef INCLUDE_GAMECFG_MENU

	while (gamecfg[gamecfg_num].label[0])
		gamecfg_num++;

	for (i = 0; i < gamecfg_num; i++)
	{
		if (gamecfg[i].value)
		{
			if (*gamecfg[i].value < 0)
				*gamecfg[i].value = 0;
			if (*gamecfg[i].value > gamecfg[i].value_max)
				*gamecfg[i].value = gamecfg[i].value_max;

			gamecfg[i].old_value = *gamecfg[i].value;
		}
	}

	pad_wait_clear();
	load_background();
	ui_popup_reset(POPUP_MENU);

	do
	{
		if (update)
		{
			show_background();

			arrowl = 0;
			arrowr = 0;
			if (gamecfg[sel].value)
			{
				int cur = *gamecfg[sel].value;

				if (cur > 0) arrowl = 1;
				if (cur < gamecfg[sel].value_max) arrowr = 1;
			}

			small_icon(8, 3, UI_COLOR(UI_PAL_TITLE), ICON_CONFIG);
			uifont_print(36, 5, UI_COLOR(UI_PAL_TITLE), "Game configuration menu");
			draw_scrollbar(470, 26, 479, 270, rows, gamecfg_num, sel);

			for (i = 0; i < rows; i++)
			{
				if (top + i >= gamecfg_num) break;

				if (gamecfg[top + i].label[0] == '\n')
					continue;

				if ((top + i) == sel)
				{
					uifont_print(16, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), gamecfg[top + i].label);
					if (arrowl)
					{
						uifont_print(180, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), FONT_LEFTTRIANGLE);
					}
				}
				else
					uifont_print(16, 40 + i * 17, UI_COLOR(UI_PAL_NORMAL), gamecfg[top + i].label);

				if (gamecfg[top + i].value)
				{
					int val = *gamecfg[top + i].value;

					if ((top + i) == sel)
					{
						uifont_print(200, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), gamecfg[top + i].values_label[val]);
						if (arrowr)
						{
							int width = uifont_get_string_width(gamecfg[top + i].values_label[val]);
							uifont_print(204 + width, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), FONT_RIGHTTRIANGLE);
						}
					}
					else
						uifont_print(200, 40 + i * 17, UI_COLOR(UI_PAL_NORMAL), gamecfg[top + i].values_label[val]);
				}
			}

			update  = draw_battery_status(1);
			update |= ui_show_popup(1);
			video_flip_screen(1);
		}
		else
		{
			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_wait_vsync();
		}

		prev_sel = sel;

		if (pad_pressed(PSP_CTRL_UP))
		{
			sel--;
			if (sel < 0) sel = gamecfg_num - 1;
			if (gamecfg[sel].label[0] == '\n') sel--;
		}
		else if (pad_pressed(PSP_CTRL_DOWN))
		{
			sel++;
			if (sel > gamecfg_num - 1) sel = 0;
			if (gamecfg[sel].label[0] == '\n') sel++;
		}
		else if (pad_pressed(PSP_CTRL_LEFT))
		{
			if (gamecfg[sel].value)
			{
				int cur = *gamecfg[sel].value;

				if (cur > 0)
				{
					*gamecfg[sel].value = cur - 1;
					update = 1;
				}
			}
		}
		else if (pad_pressed(PSP_CTRL_RIGHT))
		{
			if (gamecfg[sel].value)
			{
				int cur = *gamecfg[sel].value;

				if (cur < gamecfg[sel].value_max)
				{
					*gamecfg[sel].value = cur + 1;
					update = 1;
				}
			}
		}
		else if (pad_pressed(PSP_CTRL_CIRCLE))
		{
			if (sel == gamecfg_num - 1)
				break;
		}
		else if (pad_pressed(PSP_CTRL_RTRIGGER))
		{
			help(HELP_GAMECONFIG);
			update = 1;
		}

		if (top > gamecfg_num - rows) top = gamecfg_num - rows;
		if (top < 0) top = 0;
		if (sel >= gamecfg_num) sel = 0;
		if (sel < 0) sel = gamecfg_num - 1;
		if (sel >= top + rows) top = sel - rows + 1;
		if (sel < top) top = sel;

		if (prev_sel != sel) update = 1;

		pad_update();

		if (Loop == LOOP_EXIT) break;

	} while (!pad_pressed(PSP_CTRL_CROSS));

	for (i = 0; i < gamecfg_num; i++)
	{
		if (gamecfg[i].value && gamecfg[i].flag == CFG_RESTART)
		{
			if (gamecfg[i].old_value != *gamecfg[i].value)
			{
				menu_restart();
				return 1;
			}
		}
	}

	return 0;
}


/*------------------------------------------------------
	ボタン設定メニュー
------------------------------------------------------*/

static const int button_value[13] =
{
	0,
	PSP_CTRL_UP,
	PSP_CTRL_DOWN,
	PSP_CTRL_LEFT,
	PSP_CTRL_RIGHT,
	PSP_CTRL_CIRCLE,
	PSP_CTRL_CROSS,
	PSP_CTRL_SQUARE,
	PSP_CTRL_TRIANGLE,
	PSP_CTRL_LTRIGGER,
	PSP_CTRL_RTRIGGER,
	PSP_CTRL_START,
	PSP_CTRL_SELECT
};

static const char *button_name[13] =
{
	"Not use",
	FONT_UPARROW,
	FONT_DOWNARROW,
	FONT_LEFTARROW,
	FONT_RIGHTARROW,
	FONT_CIRCLE,
	FONT_CROSS,
	FONT_SQUARE,
	FONT_TRIANGLE,
	"L TRIGGER",
	"R TRIGGER",
	"START",
	"SELECT"
};

typedef struct {
	const char *label;
	int type;
	int value;
	int code;
} keycfg_menu_t;

#define KEYCFG_COMMENT 0
#define KEYCFG_BUTTON  1
#define KEYCFG_NUMBER  2
#define KEYCFG_ANALOG  3

#define INCLUDE_KEYCFG_STRUCT

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "menu/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "menu/mvs.c"
#endif

#undef INCLUDE_KEYCFG_STRUCT


static int menu_keycfg(void)
{
	int sel = 0, prev_sel, rows = 13, top = 0;
	int i, j, arrowl, arrowr, update = 1;
	keycfg_menu_t *keycfg;
	int keycfg_num = 0;

#define INCLUDE_KEYCFG_MENU

#if (EMU_SYSTEM == CPS1 || EMU_SYSTEM == CPS2)
#include "menu/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "menu/mvs.c"
#endif

#undef INCLUDE_KEYCFG_MENU

	while (keycfg[keycfg_num].label[0])
		keycfg_num++;

	for (i = 0; i < keycfg_num; i++)
	{
		keycfg[i].value = 0;

		if (keycfg[i].type == KEYCFG_BUTTON)
		{
			int code = input_map[keycfg[i].code];

			if (code)
			{
				for (j = 0; j < 13; j++)
				{
					if (code == button_value[j])
					{
						keycfg[i].value = j;
						break;
					}
				}
			}
		}
		else if (keycfg[i].type == KEYCFG_NUMBER)
		{
			keycfg[i].value = af_interval;
		}
		else if (keycfg[i].type == KEYCFG_ANALOG)
		{
			keycfg[i].value = analog_sensitivity;
		}
	}

	pad_wait_clear();
	load_background();
	ui_popup_reset(POPUP_MENU);

	do
	{
		if (update)
		{
			show_background();

			small_icon(8, 3, UI_COLOR(UI_PAL_TITLE), ICON_KEYCONFIG);
			uifont_print(36, 5, UI_COLOR(UI_PAL_TITLE), "Key configuration menu");
			draw_scrollbar(470, 26, 479, 270, rows, keycfg_num, sel);

			arrowl = 0;
			arrowr = 0;
			if (keycfg[sel].type)
			{
				if (keycfg[sel].value > 0) arrowl = 1;
				if (keycfg[sel].type == KEYCFG_BUTTON && keycfg[sel].value < 12) arrowr = 1;
				if (keycfg[sel].type == KEYCFG_NUMBER && keycfg[sel].value < 10) arrowr = 1;
				if (keycfg[sel].type == KEYCFG_ANALOG && keycfg[sel].value < 2) arrowr = 1;
			}

			for (i = 0; i < rows; i++)
			{
				int value = keycfg[top + i].value;
				char *name, temp[16];
				const char *label = keycfg[top + i].label;
				const char *sensitivity[3] = { "Low", "Normal", "High" };
#if (EMU_SYSTEM == CPS2)
				const char *progear_p2[2] = { "P1 Start", "P2 Start" };
#endif

				if ((top + i) >= keycfg_num) break;

				if (label[0] == '\n')
					continue;

				switch (keycfg[top + i].type)
				{
				case KEYCFG_BUTTON:
#if (EMU_SYSTEM == CPS2)
					if (!strcmp(driver->name, "progear"))
						if (top + i == 10) label = progear_p2[option_controller ^ 1];
#endif
					name = (char *)button_name[value];
					break;

				case KEYCFG_NUMBER:
					if (value == 0)
						sprintf(temp, "%d frame", value + 1);
					else
						sprintf(temp, "%d frames", value + 1);
					name = temp;
					break;

				case KEYCFG_ANALOG:
					name = (char *)sensitivity[value];
					break;

				default:
					temp[0] = '\0';
					name = temp;
					break;
				}

				if ((top + i) == sel)
				{
					uifont_print(16, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), label);
					uifont_print(190, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), name);

					if (arrowl)
					{
						uifont_print(170, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), FONT_LEFTTRIANGLE);
					}
					if (arrowr)
					{
						int width = uifont_get_string_width(name);
						uifont_print(194 + width, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), FONT_RIGHTTRIANGLE);
					}
				}
				else
				{
					uifont_print(16, 40 + i * 17, UI_COLOR(UI_PAL_NORMAL), label);
					uifont_print(190, 40 + i * 17, UI_COLOR(UI_PAL_NORMAL), name);
				}
			}

			update  = draw_battery_status(1);
			update |= ui_show_popup(1);
			video_flip_screen(1);
		}
		else
		{
			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_wait_vsync();
		}

		prev_sel = sel;

		if (pad_pressed(PSP_CTRL_UP))
		{
			sel--;
			if (sel < 0) sel = keycfg_num - 1;
			if (keycfg[sel].label[0] == '\n') sel--;
		}
		else if (pad_pressed(PSP_CTRL_DOWN))
		{
			sel++;
			if (sel > keycfg_num - 1) sel = 0;
			if (keycfg[sel].label[0] == '\n') sel++;
		}
		else if (pad_pressed(PSP_CTRL_LEFT))
		{
			if (keycfg[sel].type)
			{
				if (keycfg[sel].value > 0)
				{
					keycfg[sel].value--;
					update = 1;
				}
			}
		}
		else if (pad_pressed(PSP_CTRL_RIGHT))
		{
			if (keycfg[sel].type == KEYCFG_BUTTON)
			{
				if (keycfg[sel].value < 12)
				{
					keycfg[sel].value++;
					update = 1;
				}
			}
			else if (keycfg[sel].type == KEYCFG_NUMBER)
			{
				if (keycfg[sel].value < 10)
				{
					keycfg[sel].value++;
					update = 1;
				}
			}
			else if (keycfg[sel].type == KEYCFG_ANALOG)
			{
				if (keycfg[sel].value < 2)
				{
					keycfg[sel].value++;
					update = 1;
				}
			}
		}
		else if (pad_pressed(PSP_CTRL_CIRCLE))
		{
			if (sel == keycfg_num - 1)
				break;
		}
		else if (pad_pressed(PSP_CTRL_RTRIGGER))
		{
			help(HELP_KEYCONFIG);
			update = 1;
		}

		if (top > keycfg_num - rows) top = keycfg_num - rows;
		if (top < 0) top = 0;
		if (sel >= keycfg_num) sel = 0;
		if (sel < 0) sel = keycfg_num - 1;
		if (sel >= top + rows) top = sel - rows + 1;
		if (sel < top) top = sel;

		if (prev_sel != sel) update = 1;

		pad_update();

		if (Loop == LOOP_EXIT) break;

	} while (!pad_pressed(PSP_CTRL_CROSS));

	for (i = 0; i < keycfg_num; i++)
	{
		if (keycfg[i].type == KEYCFG_BUTTON)
		{
			input_map[keycfg[i].code] = button_value[keycfg[i].value];
		}
		else if (keycfg[i].type == KEYCFG_NUMBER)
		{
			af_interval = keycfg[i].value;
		}
		else if (keycfg[i].type == KEYCFG_ANALOG)
		{
			analog_sensitivity = keycfg[i].value;
		}
	}

	return 0;
}


/*============================================================================
	ディップスイッチ設定メニュー
============================================================================*/

static int menu_dipswitch(void)
{
#if (EMU_SYSTEM == CPS2)

	ui_popup("This game has no DIP switches.");

#else
	int sel = 0, rows = 13, top = 0, sx = 240;
	int i, arrowl, arrowr, prev_sel, update = 1;
	dipswitch_t *dipswitch;
	int dipswitch_num = 0;

#define INCLUDE_LOAD_DIPSWITCH

#if (EMU_SYSTEM == CPS1)
#include "menu/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "menu/mvs.c"
#endif

#undef INCLUDE_LOAD_DIPSWITCH

	while (dipswitch[dipswitch_num].label[0])
		dipswitch_num++;

	pad_wait_clear();
	load_background();
	ui_popup_reset(POPUP_MENU);

	do
	{
		if (update)
		{
			show_background();

			arrowl = 0;
			arrowr = 0;
			if (dipswitch[sel].mask)
			{
				int cur = dipswitch[sel].value;

				if (cur > 0) arrowl = 1;
				if (cur < dipswitch[sel].value_max) arrowr = 1;
			}

			small_icon(8, 3, UI_COLOR(UI_PAL_TITLE), ICON_DIPSWITCH);
			uifont_print(36, 5, UI_COLOR(UI_PAL_TITLE), "DIP switch settings menu");
			draw_scrollbar(470, 26, 479, 270, rows, dipswitch_num, sel);

			for (i = 0; i < rows; i++)
			{
				if ((top + i) >= dipswitch_num) break;

				if (dipswitch[top + i].label[0] == '\n')
					continue;

				if (top + i == sel)
				{
					if (!dipswitch[top + i].enable)
					{
						uifont_print(16, 40 + i * 17, COLOR_GRAY, dipswitch[top + i].label);
					}
					else
					{
						uifont_print(16, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), dipswitch[top + i].label);
						if (arrowl)
							uifont_print(sx - 20, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), FONT_LEFTTRIANGLE);
					}
				}
				else
				{
					if (!dipswitch[top + i].enable)
						uifont_print(16, 40 + i * 17, COLOR_DARKGRAY, dipswitch[top + i].label);
					else
						uifont_print(16, 40 + i * 17, UI_COLOR(UI_PAL_NORMAL), dipswitch[top + i].label);
				}

				if (dipswitch[top + i].mask)
				{
					int val = dipswitch[top + i].value;

					if (top + i == sel)
					{
						if (!dipswitch[top + i].enable)
						{
							uifont_print(sx, 40 + i * 17, COLOR_GRAY, dipswitch[top + i].values_label[val]);
						}
						else
						{
							uifont_print(sx, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), dipswitch[top + i].values_label[val]);
							if (arrowr)
							{
								int width = uifont_get_string_width(dipswitch[top + i].values_label[val]);
								uifont_print(sx + 4 + width, 40 + i * 17, UI_COLOR(UI_PAL_SELECT), FONT_RIGHTTRIANGLE);
							}
						}
					}
					else
					{
						if (!dipswitch[top + i].enable)
							uifont_print(sx, 40 + i * 17, COLOR_DARKGRAY, dipswitch[top + i].values_label[val]);
						else
							uifont_print(sx, 40 + i * 17, UI_COLOR(UI_PAL_NORMAL), dipswitch[top + i].values_label[val]);
					}
				}
			}

			update  = draw_battery_status(1);
			update |= ui_show_popup(1);
			video_flip_screen(1);
		}
		else
		{
			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_wait_vsync();
		}

		prev_sel = sel;

		if (pad_pressed(PSP_CTRL_UP))
		{
			sel--;
			if (sel < 0) sel = dipswitch_num - 1;
			if (dipswitch[sel].label[0] == '\n') sel--;
		}
		else if (pad_pressed(PSP_CTRL_DOWN))
		{
			sel++;
			if (sel > dipswitch_num - 1) sel = 0;
			if (dipswitch[sel].label[0] == '\n') sel++;
		}
		else if (pad_pressed(PSP_CTRL_LEFT))
		{
			if (dipswitch[sel].mask)
			{
				int cur = dipswitch[sel].value;

				if (cur > 0)
				{
					dipswitch[sel].value = cur - 1;
					update = 1;
				}
			}
		}
		else if (pad_pressed(PSP_CTRL_RIGHT))
		{
			if (dipswitch[sel].mask)
			{
				int cur = dipswitch[sel].value;

				if (cur < dipswitch[sel].value_max)
				{
					dipswitch[sel].value = cur + 1;
					update = 1;
				}
			}
		}
		else if (pad_pressed(PSP_CTRL_CIRCLE))
		{
			if (sel == dipswitch_num - 1)
				break;
		}
		else if (pad_pressed(PSP_CTRL_RTRIGGER))
		{
			help(HELP_DIPSWITCH);
			update = 1;
		}

		if (top > dipswitch_num - rows) top = dipswitch_num - rows;
		if (top < 0) top = 0;
		if (sel >= dipswitch_num) sel = 0;
		if (sel < 0) sel = dipswitch_num - 1;
		if (sel >= top + rows) top = sel - rows + 1;
		if (sel < top) top = sel;

		if (prev_sel != sel) update = 1;

		pad_update();

		if (Loop == LOOP_EXIT) break;

	} while (!pad_pressed(PSP_CTRL_CROSS));

#define INCLUDE_SAVE_DIPSWITCH

#if (EMU_SYSTEM == CPS1)
#include "menu/cps.c"
#elif (EMU_SYSTEM == MVS)
#include "menu/mvs.c"
#endif

#undef INCLUDE_SAVE_DIPSWITCH

#endif

	return 0;
}


/*------------------------------------------------------
	セーブ/ロードステート
------------------------------------------------------*/

#ifdef SAVE_STATE

#define STATE_FUNC_SAVE		0
#define STATE_FUNC_LOAD		1
#define STATE_FUNC_DEL		2


static u8 slot[10];
static int state_func;
static int state_sel;


static void state_delete_slot(void)
{
	if (messagebox(MB_DELETESTATE))
	{
		char path[MAX_PATH];

		sprintf(path, "%sstate/%s.sv%d", launchDir, game_name, state_sel);
		if (remove(path) != 0)
		{
			ui_popup("Error: faild to delete file \"%s\".", strrchr(path, '/') + 1);
		}
		else
		{
			slot[state_sel] = 0;
			state_func = STATE_FUNC_SAVE;
			state_clear_thumbnail();
		}
	}
}


void state_draw_thumbnail(void)
{
	if (machine_screen_type)
	{
		RECT clip1 = { 0, 0, 112, 152 };
		RECT clip2 = { 317, 34, 317+112, 34+152 };
		video_copy_rect(tex_frame, draw_frame, &clip1, &clip2);
	}
	else
	{
		RECT clip1 = { 0, 0, 152, 112 };
		RECT clip2 = { 298, 52, 298+152, 52+112 };
		video_copy_rect(tex_frame, draw_frame, &clip1, &clip2);
	}
}


int state_draw_progress(int progress)
{
	if (state_func >= STATE_FUNC_DEL) return 0;

	if (progress == 1)
		video_copy_rect(show_frame, work_frame, &full_rect, &full_rect);

	show_background();

	if (state_func == STATE_FUNC_SAVE && progress == 6)
	{
		state_draw_thumbnail();
		if (machine_screen_type)
		{
			uifont_print(300, 198, UI_COLOR(UI_PAL_NORMAL), "Play Date");
			uifont_print(378, 198, UI_COLOR(UI_PAL_NORMAL), date_str);
			uifont_print(300, 218, UI_COLOR(UI_PAL_NORMAL), "Save Time");
			uifont_print(378, 218, UI_COLOR(UI_PAL_NORMAL), time_str);
			uifont_print(300, 238, UI_COLOR(UI_PAL_NORMAL), "Version");
			uifont_print(378, 238, UI_COLOR(UI_PAL_NORMAL), stver_str);
		}
		else
		{
			uifont_print(290, 190, UI_COLOR(UI_PAL_NORMAL), "Play Date");
			uifont_print(368, 190, UI_COLOR(UI_PAL_NORMAL), date_str);
			uifont_print(290, 210, UI_COLOR(UI_PAL_NORMAL), "Save Time");
			uifont_print(368, 210, UI_COLOR(UI_PAL_NORMAL), time_str);
			uifont_print(290, 230, UI_COLOR(UI_PAL_NORMAL), "Version");
			uifont_print(368, 230, UI_COLOR(UI_PAL_NORMAL), stver_str);
		}
	}

	draw_dialog(80, 34, 399, 261);
	uifont_print_center(42, UI_COLOR(UI_PAL_INFO), state_func ? "Load state" : "Save state");

	if (progress == 1)
	{
		if (state_func)
			uifont_print(90, 62, UI_COLOR(UI_PAL_SELECT), "Press " FONT_CIRCLE " to start load state. (" FONT_CROSS " to cancel)");
		else
			uifont_print(90, 62, UI_COLOR(UI_PAL_SELECT), "Press " FONT_CIRCLE " to start save state. (" FONT_CROSS " to cancel)");

		video_flip_screen(1);
		pad_wait_clear();

		do
		{
			if (pad_pressed(PSP_CTRL_CIRCLE))
			{
				pad_wait_clear();
				return 1;
			}

			if (Loop == LOOP_EXIT) break;

			pad_update();

		} while (!pad_pressed(PSP_CTRL_CROSS));

		pad_wait_clear();
	}
	else
	{
		if (state_func == STATE_FUNC_LOAD)
		{
			if (progress > 1) uifont_print(90, 62 + 18*0, UI_COLOR(UI_PAL_SELECT), "Skip header...");
			if (progress > 2) uifont_print(90, 62 + 18*1, UI_COLOR(UI_PAL_SELECT), "Skip thumbnail...");
			if (progress > 3) uifont_print(90, 62 + 18*2, UI_COLOR(UI_PAL_SELECT), "Read state data...");
			if (progress > 4)
			{
				uifont_print(90, 62 + 18*4, UI_COLOR(UI_PAL_SELECT), "Complete.");
				uifont_print_center(239, UI_COLOR(UI_PAL_SELECT), "Press any button to return to game.");
			}
			video_flip_screen(1);
			if (progress == 6) pad_wait_press(PAD_WAIT_INFINITY);
		}
		else
		{
			if (progress > 1) uifont_print(90, 62 + 18*0, UI_COLOR(UI_PAL_SELECT), "Write header...");
			if (progress > 2) uifont_print(90, 62 + 18*1, UI_COLOR(UI_PAL_SELECT), "Write thumbnail...");
			if (progress > 3) uifont_print(90, 62 + 18*2, UI_COLOR(UI_PAL_SELECT), "Write state data...");
			if (progress > 4)
			{
				uifont_print(90, 62 + 18*4, UI_COLOR(UI_PAL_SELECT), "Complete.");
				uifont_print_center(221, UI_COLOR(UI_PAL_SELECT), "Press " FONT_CROSS " to return to game.");
				uifont_print_center(239, UI_COLOR(UI_PAL_SELECT), "Press other button to return to menu.");
			}
			else
			{
				uifont_print_center(221, UI_COLOR(UI_PAL_WARNING), "Save state is very slow.");
				uifont_print_center(239, UI_COLOR(UI_PAL_WARNING), "Please wait for a while.");
			}
			video_flip_screen(1);

			if (progress == 6)
			{
				pad_wait_clear();

				do
				{
					if (pad_pressed(PSP_CTRL_CROSS))
					{
						pad_wait_clear();
						return 1;
					}

					if (Loop == LOOP_EXIT) break;

					pad_update();

				} while (!pad_pressed_any(PSP_CTRL_CROSS));

				pad_wait_clear();
			}
		}
	}
	return 0;
}


int menu_state(void)
{
	int i, prev_sel = -1, prev_func = -1, update = 1;
	int thumbnail_loaded = 0;
	char name[16], state[32];

	state_func = STATE_FUNC_SAVE;
	state_sel = 0;
	thumbnail_loaded = 0;

	pad_wait_clear();
	load_background();
	ui_popup_reset(POPUP_MENU);

	memset(slot, 0, sizeof(slot));
	state_clear_thumbnail();

	find_state_file(slot);

	do
	{
		if (prev_sel != state_sel)
		{
			if (!slot[state_sel])
			{
				state_clear_thumbnail();
				state_func = STATE_FUNC_SAVE;
			}
			else
			{
				state_load_thumbnail(state_sel);
				if (state_version == current_state_version)
					state_func = STATE_FUNC_LOAD;
				else
					state_func = STATE_FUNC_SAVE;
			}
			update = 1;
		}

		if (update & 1)
		{
			show_background();
			if (machine_screen_type)
				draw_box_shadow(318, 33, 429, 184);
			else
				draw_box_shadow(298, 52, 449, 163);
			state_draw_thumbnail();

			uifont_print(8, 5, UI_COLOR(UI_PAL_NORMAL), FONT_LTRIGGER " Main menu");
			uifont_print(98, 5, UI_COLOR(UI_PAL_NORMAL), "|");
			uifont_print(106, 5, UI_COLOR(UI_PAL_TITLE), "Save/Load state " FONT_RTRIGGER);

			for (i = 0; i < 10; i++)
			{
				sprintf(name, "Slot %d:", i);
				if (slot[i])
					sprintf(state, "%s.sv%d", game_name, i);
				else
					strcpy(state, "Empty");

				if (i == state_sel)
				{
					small_icon(12, 38 + i * 22, UI_COLOR(UI_PAL_SELECT), ICON_MEMSTICK);
					uifont_print(40, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), name);
					uifont_print(90, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), state);

					if (slot[state_sel])
					{
						if (state_func == STATE_FUNC_LOAD)
						{
							uifont_print(192, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), FONT_LEFTTRIANGLE);
							uifont_print(212, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), "Load");
							uifont_print(244, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), FONT_RIGHTTRIANGLE);
						}
						else if (state_func == STATE_FUNC_DEL)
						{
							uifont_print(192, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), FONT_LEFTTRIANGLE);
							uifont_print(212, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), "Delete");
						}
						else
						{
							uifont_print(212, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), "Save");
							uifont_print(244, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), FONT_RIGHTTRIANGLE);
						}
					}
					else
					{
						uifont_print(212, 40 + i * 22, UI_COLOR(UI_PAL_SELECT), "Save");
					}

					hline_alpha(40, 266, 56 + i * 22, UI_COLOR(UI_PAL_SELECT), 12);
					hline_alpha(41, 267, 57 + i * 22, COLOR_BLACK, 8);
				}
				else
				{
					small_icon(12, 38 + i * 22, UI_COLOR(UI_PAL_NORMAL), ICON_MEMSTICK);
					uifont_print(40, 40 + i * 22, UI_COLOR(UI_PAL_NORMAL), name);
					uifont_print(90, 40 + i * 22, UI_COLOR(UI_PAL_NORMAL), state);
				}
			}

			if (state_version == current_state_version || !slot[state_sel])
			{
				if (machine_screen_type)
				{
					uifont_print(300, 198, UI_COLOR(UI_PAL_NORMAL), "Play Date");
					uifont_print(378, 198, UI_COLOR(UI_PAL_NORMAL), date_str);
					uifont_print(300, 218, UI_COLOR(UI_PAL_NORMAL), "Save Time");
					uifont_print(378, 218, UI_COLOR(UI_PAL_NORMAL), time_str);
					uifont_print(300, 238, UI_COLOR(UI_PAL_NORMAL), "Version");
					uifont_print(378, 238, UI_COLOR(UI_PAL_NORMAL), stver_str);
				}
				else
				{
					uifont_print(290, 190, UI_COLOR(UI_PAL_NORMAL), "Play Date");
					uifont_print(368, 190, UI_COLOR(UI_PAL_NORMAL), date_str);
					uifont_print(290, 210, UI_COLOR(UI_PAL_NORMAL), "Save Time");
					uifont_print(368, 210, UI_COLOR(UI_PAL_NORMAL), time_str);
					uifont_print(290, 230, UI_COLOR(UI_PAL_NORMAL), "Version");
					uifont_print(368, 230, UI_COLOR(UI_PAL_NORMAL), stver_str);
				}
			}
			else
			{
				if (machine_screen_type)
				{
					uifont_print(300, 198, COLOR_GRAY, "Play Date");
					uifont_print(378, 198, COLOR_GRAY, date_str);
					uifont_print(300, 218, COLOR_GRAY, "Save Time");
					uifont_print(378, 218, COLOR_GRAY, time_str);
					uifont_print(300, 238, COLOR_GRAY, "Version");
					uifont_print(378, 238, COLOR_GRAY, stver_str);
				}
				else
				{
					uifont_print(290, 190, COLOR_GRAY, "Play Date");
					uifont_print(368, 190, COLOR_GRAY, date_str);
					uifont_print(290, 210, COLOR_GRAY, "Save Time");
					uifont_print(368, 210, COLOR_GRAY, time_str);
					uifont_print(290, 230, COLOR_GRAY, "Version");
					uifont_print(368, 230, COLOR_GRAY, stver_str);
				}
			}

			update  = draw_battery_status(1);
			update |= ui_show_popup(1);
			video_flip_screen(1);
		}
		else if (update & 2)
		{
			int x, y, w, h;
			RECT clip1, clip2;

			show_background();

			small_icon(12, 38 + state_sel * 22, UI_COLOR(UI_PAL_SELECT), ICON_MEMSTICK);

			x = 12;
			y = 38 + state_sel * 22;
			w = 24 + 8;
			h = 18 + 8;

			clip1.left   = x - 4;
			clip1.top    = y - 4;
			clip1.right  = clip1.left + w;
			clip1.bottom = clip1.top  + h;

			clip2.left   = 0;
			clip2.top    = 112;
			clip2.right  = clip2.left + w;
			clip2.bottom = clip2.top  + h;

			video_copy_rect(draw_frame, tex_frame, &clip1, &clip2);
			video_copy_rect(show_frame, draw_frame, &full_rect, &full_rect);
			video_copy_rect(tex_frame, draw_frame, &clip2, &clip1);

			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_flip_screen(1);
		}
		else
		{
			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_wait_vsync();
		}

		prev_sel = state_sel;
		prev_func = state_func;

		if (pad_pressed(PSP_CTRL_UP))
		{
			state_sel--;
			if (state_sel < 0) state_sel = 10 - 1;
		}
		else if (pad_pressed(PSP_CTRL_DOWN))
		{
			state_sel++;
			if (state_sel >= 10) state_sel = 0;
		}
		else if (pad_pressed(PSP_CTRL_LEFT))
		{
			if (state_func > STATE_FUNC_SAVE) state_func--;
			if (state_version != current_state_version && state_func == STATE_FUNC_LOAD)
				state_func--;
		}
		else if (pad_pressed(PSP_CTRL_RIGHT))
		{
			if (slot[state_sel])
			{
				if (state_func < STATE_FUNC_DEL) state_func++;
				if (state_version != current_state_version && state_func == STATE_FUNC_LOAD)
					state_func++;
			}
		}
		else if (pad_pressed(PSP_CTRL_CIRCLE))
		{
			if (state_func == STATE_FUNC_LOAD)
			{
				if (slot[state_sel])
				{
					if (state_draw_progress(1))
					{
						int res;

						set_cpu_clock(psp_cpuclock);
						res = state_load(state_sel);
						set_cpu_clock(PSPCLOCK_222);

						if (res)
						{
							state_draw_progress(6);
							return 1;
						}
					}
					load_background();
					update = 1;
				}
			}
			else if (state_func == STATE_FUNC_DEL)
			{
				if (slot[state_sel])
				{
					state_delete_slot();
					update = 1;
				}
			}
			else
			{
				int res = 0;

				if (state_draw_progress(1))
				{
					set_cpu_clock(psp_cpuclock);

					if (state_save(state_sel))
					{
						find_state_file(slot);
						state_load_thumbnail(state_sel);
						res = state_draw_progress(6);
					}

					set_cpu_clock(PSPCLOCK_222);
				}
				load_background();
				update = 1;

				if (res) return 1;
			}
		}
		else if (pad_pressed(PSP_CTRL_RTRIGGER))
		{
			help(HELP_STATE);
			update = 1;
		}

		if (prev_func != state_func) update = 1;

		pad_update();

		if (Loop == LOOP_EXIT) break;

	} while (!pad_pressed(PSP_CTRL_LTRIGGER) && !pad_pressed(PSP_CTRL_CROSS));

	return 0;
}

#endif


/*------------------------------------------------------
	メインメニュー
------------------------------------------------------*/

typedef struct
{
	const char *label;
	int (*menu_func)(void);
	int icon;
	const char *help;
} menu_t;


menu_t mainmenu[] =
{
	{"Game configuration",     menu_gamecfg,   ICON_CONFIG,    "Change game settings." },
	{"Key configuration",      menu_keycfg,    ICON_KEYCONFIG, "Change buttons and autofire/hotkey settings." },
	{"DIP switch settings",    menu_dipswitch, ICON_DIPSWITCH, "Change hardware DIP switch settings." },
#if (EMU_SYSTEM == CPS1)
	{"Reset emulation",        menu_reset,     ICON_SYSTEM,    "Reset CPS1 emulation."},
#elif (EMU_SYSTEM == CPS2)
	{"Reset emulation",        menu_reset,     ICON_SYSTEM,    "Reset CPS2 emulation."},
#elif (EMU_SYSTEM == MVS)
	{"Reset emulation",        menu_reset,     ICON_SYSTEM,    "Reset NEO･GEO emulation."},
#endif
	{"Return to file browser", menu_browser,   ICON_FOLDER,    "Stop emulation and return to file browser." },
	{"Return to game",         NULL,           ICON_RETURN,    "Return to game emulation." },
	{"Exit emulator",          menu_exit,      ICON_EXIT,      "Exit emulation and return to PSP menu." },
	MENU_END
};


void showmenu(void)
{
	static int sel = 0;
	int i, prev_sel = 0, update = 1;
	int mainmenu_num = 0;

#if USE_CACHE
	cache_sleep(1);
#endif

	while (mainmenu[mainmenu_num].label[0])
		mainmenu_num++;

#ifdef SAVE_STATE
	state_make_thumbnail();
#endif

	sound_thread_enable(0);
	set_cpu_clock(PSPCLOCK_222);
	video_clear_screen();
	ui_popup_reset(POPUP_MENU);
	pad_wait_clear();
	load_background();

	do
	{
		if (update)
		{
			show_background();

#ifdef SAVE_STATE
			uifont_print(8, 5, UI_COLOR(UI_PAL_TITLE), FONT_LTRIGGER " Main menu");
			uifont_print(98, 5, UI_COLOR(UI_PAL_NORMAL), "|");
			uifont_print(106, 5, UI_COLOR(UI_PAL_NORMAL), "Save/Load state " FONT_RTRIGGER);
#else
			small_icon(8, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
			uifont_print(36, 5, UI_COLOR(UI_PAL_TITLE), "Main menu");
#endif

			for (i = 0; i < mainmenu_num; i++)
			{
				if (i == sel)
				{
					large_icon(12, 40 + i * 32, UI_COLOR(UI_PAL_SELECT), mainmenu[i].icon);
					uifont_print(54, 37 + i * 32, UI_COLOR(UI_PAL_SELECT), mainmenu[i].label);
					uifont_print(54, 56 + i * 32, UI_COLOR(UI_PAL_SELECT), mainmenu[i].help);

					hline_alpha(54, 460, 53 + i * 32, UI_COLOR(UI_PAL_SELECT), 12);
					hline_alpha(55, 461, 54 + i * 32, COLOR_BLACK, 8);
				}
				else
				{
					small_icon(16, 42 + i * 32, UI_COLOR(UI_PAL_NORMAL), mainmenu[i].icon);
					uifont_print(56, 44 + i * 32, UI_COLOR(UI_PAL_NORMAL), mainmenu[i].label);
				}
			}

			update  = draw_battery_status(1);
			update |= ui_show_popup(1);
			video_flip_screen(1);
		}
		else
		{
			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_wait_vsync();
		}

		prev_sel = sel;

		if (pad_pressed(PSP_CTRL_UP))
		{
			sel--;
			if (sel < 0) sel = mainmenu_num - 1;
		}
		else if (pad_pressed(PSP_CTRL_DOWN))
		{
			sel++;
			if (sel >= mainmenu_num) sel = 0;
		}
		else if (pad_pressed(PSP_CTRL_CIRCLE))
		{
			if (mainmenu[sel].menu_func)
			{
				int res = mainmenu[sel].menu_func();

				if (res == 0)
				{
					pad_wait_clear();
					load_background();
					ui_popup_reset(POPUP_MENU);
					update = 1;
				}
				else break;
			}
			else break;
		}
#ifdef SAVE_STATE
		else if (pad_pressed(PSP_CTRL_RTRIGGER))
		{
			if (menu_state()) break;
			update = 1;
		}
#endif

		if (prev_sel != sel) update |= 1;

		pad_update();

		if (Loop == LOOP_EXIT) break;

	} while (!pad_pressed(PSP_CTRL_CROSS));

	autoframeskip_reset();
	blit_clear_all_sprite();

	sound_thread_set_priority(option_vsync ? 0x12 : 0x08);
#ifdef KERNEL_MODE
	main_thread_set_priority(option_vsync ? 0x08 : 0x11);
#endif

	pad_wait_clear();
	ui_popup_reset(POPUP_GAME);
	video_clear_screen();
	video_clear_frame(work_frame);
	set_cpu_clock(psp_cpuclock);

#if (EMU_SYSTEM == CPS2)
	qsound_set_samplerate();
#endif
	if (Loop == LOOP_EXEC)
	{
		sound_thread_set_volume();
		sound_thread_enable(option_sound_enable);
	}

#if USE_CACHE
	cache_sleep(0);
#endif
}
