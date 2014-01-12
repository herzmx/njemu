/******************************************************************************

	mesbox.c

	Message Box

******************************************************************************/

#include "psp.h"

#define MB_MES_MAX	16

#define MB_COLOR_INFO		&ui_palette[UI_PAL_INFO].r,    &ui_palette[UI_PAL_INFO].g,    &ui_palette[UI_PAL_INFO].b
#define MB_COLOR_WARNING	&ui_palette[UI_PAL_WARNING].r, &ui_palette[UI_PAL_WARNING].g, &ui_palette[UI_PAL_WARNING].b
#define MB_COLOR_NORMAL		&ui_palette[UI_PAL_SELECT].r,  &ui_palette[UI_PAL_SELECT].g, &ui_palette[UI_PAL_SELECT].b

#define MB_INFO(s)			{ MB_COLOR_INFO, s }
#define MB_NORMAL(s)		{ MB_COLOR_NORMAL, s }
#define MB_WARNING(s)		{ MB_COLOR_WARNING, s }
#define MB_SKIP				{ MB_COLOR_NORMAL, "\n" }
#define MB_EOM				{ MB_COLOR_NORMAL, "\0" }

enum
{
	MB_OKONLY = 0,
	MB_YESNO,
	MB_TYPE_MAX
};


/******************************************************************************
	Private structures
******************************************************************************/

typedef struct message_t
{
	int *r, *g, *b;
	const char *text;
} UI_MESSAGE;

typedef struct dialog_t
{
	const int type;
	const UI_MESSAGE mes[MB_MES_MAX];
} UI_MESSAGEBOX;


const static UI_MESSAGEBOX mesbox[MB_NUM_MAX] =
{
	// MB_LAUNCHZIPFILE
	{
		MB_YESNO,
		{
			MB_INFO("Start emulation."),
			MB_SKIP,
			MB_NORMAL(FONT_CIRCLE " to confirm, " FONT_CROSS " to cancel"),
			MB_EOM
		}
	},
	// MB_EXITEMULATOR
	{
		MB_YESNO,
		{
			MB_INFO("Exit emulation."),
			MB_SKIP,
			MB_NORMAL(FONT_CIRCLE " to confirm, " FONT_CROSS " to cancel"),
			MB_EOM
		}
	},
	// MB_SETSTARTUPDIR
	{
		MB_YESNO,
		{
			MB_INFO("Do you make this directory a startup directory?"),
			MB_SKIP,
			MB_NORMAL(FONT_CIRCLE " to confirm, " FONT_CROSS " to cancel"),
			MB_EOM
		}
	},
	// MB_RESETEMULATION
	{
		MB_YESNO,
		{
			MB_INFO("Reset emulation."),
			MB_SKIP,
			MB_NORMAL(FONT_CIRCLE " to confirm, " FONT_CROSS " to cancel"),
			MB_EOM
		}
	},
	// MB_RESTARTEMULATION
	{
		MB_OKONLY,
		{
			MB_INFO("Need to restart emulation."),
			MB_SKIP,
			MB_NORMAL("Press any button."),
			MB_EOM
		}
	},
#ifdef SAVE_STATE
	// MB_DELETESTATE
	{
		MB_YESNO,
		{
			MB_INFO("Delete state file."),
			MB_SKIP,
			MB_NORMAL(FONT_CIRCLE " to confirm, " FONT_CROSS " to cancel"),
			MB_EOM
		}
	},
#endif
	// MB_GAMENOTWORK
	{
		MB_OKONLY,
		{
			MB_INFO("THIS GAME DOESN'T WORK."),
			MB_SKIP,
			MB_NORMAL("You won't be able to make it work correctly."),
			MB_NORMAL("Don't bother."),
			MB_SKIP,
			MB_NORMAL("Press any button."),
			MB_EOM
		}
	}
};


/*--------------------------------------------------------
	メッセージボックス表示
--------------------------------------------------------*/

int messagebox(int no)
{
	int i, w, lines, width, height;
	int sx, sy, ex, ey, ret = 0;
	const UI_MESSAGE *mes;

	if (no > MB_NUM_MAX) return 0;

	video_copy_rect(show_frame, draw_frame, &full_rect, &full_rect);

	boxfill_alpha(0, 0, SCR_WIDTH - 1, SCR_HEIGHT - 1, COLOR_BLACK, 8);

	lines = 0;
	width = 0;

	mes = mesbox[no].mes;

	while (mes[lines].text[0] != '\0' && lines < MB_MES_MAX)
		lines++;

	for (i = 0; i < lines; i++)
	{
		w = uifont_get_string_width(mes[i].text);
		if (width < w) width = w;
	}

	width >>= 1;			// width / 2
	height = lines << 3;	// (line * (FONTSIZE + 2)) / 2

	sx = SCR_WIDTH / 2 - width;;
	ex = SCR_WIDTH / 2 + width;
	sy = SCR_HEIGHT / 2 - height;
	ey = SCR_HEIGHT / 2 + height;

	draw_dialog(sx - 21, sy - 21, ex + 21, ey + 21);

	sy++;

	for (i = 0; i < lines; i++)
	{
		int r = *mesbox[no].mes[i].r;
		int g = *mesbox[no].mes[i].g;
		int b = *mesbox[no].mes[i].b;

		uifont_print_center(sy + (i << 4), r, g, b, mesbox[no].mes[i].text);
	}

	video_flip_screen(1);
	pad_wait_clear();

	if (mesbox[no].type)
	{
		do
		{
			video_wait_vsync();
			pad_update();

			if (pad_pressed(PSP_CTRL_CIRCLE))
			{
				ret = 1;
				pad_wait_clear();
				break;
			}

			if (Loop == LOOP_EXIT) break;

		} while (!pad_pressed(PSP_CTRL_CROSS));
	}
	else
	{
		while (1)
		{
			video_wait_vsync();
			pad_update();

			if (pad_pressed(PSP_CTRL_ANY))
			{
				ret = 1;
				pad_wait_clear();
				break;
			}

 			if (Loop == LOOP_EXIT) break;
		}
	}

	pad_wait_clear();
	video_flip_screen(1);

	return ret;
}
