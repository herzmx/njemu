/******************************************************************************

	help.c

	PSP ヘルプ表示関数

******************************************************************************/

#include "psp.h"


#define HELP_MES_MAX	13

#define HELP_COLOR_ENABLE	&ui_palette[UI_PAL_SELECT].r, &ui_palette[UI_PAL_SELECT].g, &ui_palette[UI_PAL_SELECT].b
#define HELP_COLOR_DISABLE	&ui_palette[UI_PAL_NORMAL].r, &ui_palette[UI_PAL_NORMAL].g, &ui_palette[UI_PAL_NORMAL].b
#define HELP_COLOR_NORMAL	&ui_palette[UI_PAL_NORMAL].r, &ui_palette[UI_PAL_NORMAL].g, &ui_palette[UI_PAL_NORMAL].b

#define HELP_ENABLE(s1, s2)	{ HELP_COLOR_ENABLE, s1, s2 }
#define HELP_DISABLE(s1)	{ HELP_COLOR_DISABLE, s1, "Not use" }
#define HELP_SKIP			{ HELP_COLOR_NORMAL, "\n", "\0" }
#define HELP_EOM			{ HELP_COLOR_NORMAL, "\0", "\0" }


/******************************************************************************
	ローカル構造体
******************************************************************************/

typedef struct message_t
{
	int *r, *g, *b;
	const char *text1;
	const char *text2;
} UI_MESSAGE;

typedef struct help_t
{
	const char *menu_name;
	const UI_MESSAGE mes[HELP_MES_MAX];
} UI_HELP;


const static UI_HELP help_mes[HELP_NUM_MAX] =
{
	// HELP_FILEBROWSER
	{
		"File browser",
		{
			HELP_ENABLE(FONT_UPARROW FONT_DOWNARROW, "Scroll"),
			HELP_ENABLE(FONT_LEFTARROW FONT_RIGHTARROW,"Scroll 1 page"),
			HELP_ENABLE(FONT_CIRCLE, "Launch game"),
			HELP_DISABLE(FONT_CROSS),
#ifdef SOUND_TEST
			HELP_ENABLE(FONT_LTRIGGER, "Launch sound test"),
#else
			HELP_DISABLE(FONT_SQUARE),
#endif
			HELP_ENABLE(FONT_TRIANGLE, "Exit emulator"),
#if (EMU_SYSTEM == MVS)
			HELP_ENABLE(FONT_LTRIGGER, "Show BIOS select menu"),
#else
			HELP_DISABLE(FONT_LTRIGGER),
#endif
			HELP_ENABLE(FONT_RTRIGGER, "Show this help"),
			HELP_DISABLE("START"),
			HELP_ENABLE("SELECT", "Set selected directory as start up"),
			HELP_EOM
		}
	},
	// HELP_GAMECONFIG
	{
		"Game configuration menu",
		{
			HELP_ENABLE(FONT_UPARROW FONT_DOWNARROW, "Scroll"),
			HELP_ENABLE(FONT_LEFTARROW FONT_RIGHTARROW,"Change value"),
			HELP_ENABLE(FONT_CIRCLE, "Select"),
			HELP_ENABLE(FONT_CROSS, "Return to main menu"),
			HELP_DISABLE(FONT_SQUARE),
			HELP_DISABLE(FONT_TRIANGLE),
			HELP_DISABLE(FONT_LTRIGGER),
			HELP_ENABLE(FONT_RTRIGGER, "Show this help"),
			HELP_DISABLE("START"),
			HELP_DISABLE("SELECT"),
			HELP_EOM
		}
	},
	// HELP_KEYCONFIG
	{
		"Key configuration menu",
		{
			HELP_ENABLE(FONT_UPARROW FONT_DOWNARROW, "Scroll"),
			HELP_ENABLE(FONT_LEFTARROW FONT_RIGHTARROW,"Change value"),
			HELP_ENABLE(FONT_CIRCLE, "Select"),
			HELP_ENABLE(FONT_CROSS, "Return to main menu"),
			HELP_DISABLE(FONT_SQUARE),
			HELP_DISABLE(FONT_TRIANGLE),
			HELP_DISABLE(FONT_LTRIGGER),
			HELP_ENABLE(FONT_RTRIGGER, "Show this help"),
			HELP_DISABLE("START"),
			HELP_DISABLE("SELECT"),
			HELP_EOM
		}
	},
#if (EMU_SYSTEM != CPS2)
	// HELP_DIPSWITCH
	{
		"DIP switch settings menu",
		{
			HELP_ENABLE(FONT_UPARROW FONT_DOWNARROW, "Scroll"),
			HELP_ENABLE(FONT_LEFTARROW FONT_RIGHTARROW,"Change value"),
			HELP_ENABLE(FONT_CIRCLE, "Select"),
			HELP_ENABLE(FONT_CROSS, "Return to main menu"),
			HELP_DISABLE(FONT_SQUARE),
			HELP_DISABLE(FONT_TRIANGLE),
			HELP_DISABLE(FONT_LTRIGGER),
			HELP_ENABLE(FONT_RTRIGGER, "Show this help"),
			HELP_DISABLE("START"),
			HELP_DISABLE("SELECT"),
			HELP_EOM
		}
	},
#endif
#ifdef SAVE_STATE
	// HELP_STATE
	{
		"Save/Load State",
		{
			HELP_ENABLE(FONT_UPARROW FONT_DOWNARROW, "Scroll"),
			HELP_ENABLE(FONT_LEFTARROW FONT_RIGHTARROW,"Change function"),
			HELP_ENABLE(FONT_CIRCLE, "Select"),
			HELP_DISABLE(FONT_CROSS),
			HELP_DISABLE(FONT_SQUARE),
			HELP_DISABLE(FONT_TRIANGLE),
			HELP_ENABLE(FONT_LTRIGGER, "Return to main menu"),
			HELP_ENABLE(FONT_RTRIGGER, "Show this help"),
			HELP_DISABLE("START"),
			HELP_DISABLE("SELECT"),
			HELP_EOM
		}
	},
#endif
#ifdef SOUND_TEST
	// HELP_SOUNDTEST
	{
		"Sound test",
		{
			HELP_ENABLE(FONT_UPARROW FONT_DOWNARROW, "Scroll"),
			HELP_ENABLE(FONT_LEFTARROW FONT_RIGHTARROW,"Scroll 1 page"),
			HELP_ENABLE(FONT_CIRCLE, "Play music"),
			HELP_ENABLE(FONT_CROSS, "Stop music"),
			HELP_DISABLE(FONT_SQUARE),
			HELP_ENABLE(FONT_TRIANGLE, "Exit sound test"),
			HELP_DISABLE(FONT_LTRIGGER),
			HELP_DISABLE(FONT_RTRIGGER),
			HELP_DISABLE("START"),
			HELP_DISABLE("SELECT"),
			HELP_EOM
		}
	}
#endif
};


/*--------------------------------------------------------
	ヘルプ表示
--------------------------------------------------------*/

int help(int no)
{
	int i;
	char title[256];

	if (no > HELP_NUM_MAX) return 0;

	video_copy_rect(show_frame, draw_frame, &full_rect, &full_rect);

	boxfill_alpha(0, 0, SCR_WIDTH - 1, SCR_HEIGHT - 1, COLOR_BLACK, 8);
	draw_dialog(59, 34, 419, 264);

	sprintf(title, "Help - %s", help_mes[no].menu_name);
	uifont_print_center(43, UI_COLOR(UI_PAL_INFO), title);

	for (i = 0; help_mes[no].mes[i].text1[0]; i++)
	{
		int r = *help_mes[no].mes[i].r;
		int g = *help_mes[no].mes[i].g;
		int b = *help_mes[no].mes[i].b;

		uifont_print(73, 70 + (i << 4), r, g, b, help_mes[no].mes[i].text1);
		uifont_print(143, 70 + (i << 4), r, g, b, help_mes[no].mes[i].text2);
	}

	uifont_print_center(240, UI_COLOR(UI_PAL_SELECT), "Press any button to return to menu.");

	video_flip_screen(1);
	pad_wait_press(PAD_WAIT_INFINITY);
	video_flip_screen(1);

	return 0;
}
