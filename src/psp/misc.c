/******************************************************************************

	misc.c

    PSP miscellaneous functions.

******************************************************************************/

#include "psp.h"


/******************************************************************************
	Global variables
******************************************************************************/

int psp_cpuclock;

/*------------------------------------------------------
	Set PSP CPU clock
------------------------------------------------------*/

void set_cpu_clock(int value)
{
	switch (value)
	{
	case PSPCLOCK_266: scePowerSetClockFrequency(266, 266, 133); break;
	case PSPCLOCK_333: scePowerSetClockFrequency(333, 333, 166); break;
	default: scePowerSetClockFrequency(222, 222, 111); break;
	}
}


/*------------------------------------------------------
	Load background image
------------------------------------------------------*/

void load_background(void)
{
	ui_fill_frame(draw_frame, UI_PAL_BG2);

	draw_bar_shadow();

	boxfill_alpha(0, 0, 479, 23, UI_COLOR(UI_PAL_BG1), 10);
	hline_alpha(0, 479, 23, UI_COLOR(UI_PAL_FRAME), 12);
	hline_alpha(0, 479, 24, UI_COLOR(UI_PAL_FRAME), 10);

	video_copy_rect(draw_frame, work_frame, &full_rect, &full_rect);
}


/*------------------------------------------------------
	Show background image
------------------------------------------------------*/

void show_background(void)
{
	video_copy_rect(work_frame, draw_frame, &full_rect, &full_rect);
}


/*------------------------------------------------------
	Draw battery status
------------------------------------------------------*/

int draw_battery_status(int draw)
{
	static u32 counter = 0;
	static int prev_bat = 0, prev_charging = 0;
	int width, icon, update = 0;
	int bat = scePowerGetBatteryLifePercent();
	int charging = scePowerIsBatteryCharging();
	char bat_left[4];

	counter++;

	if (draw)
	{
		if (bat > 66)		icon = ICON_BATTERY1;
		else if (bat > 33)	icon = ICON_BATTERY2;
		else if (bat >= 10) icon = ICON_BATTERY3;
		else				icon = ICON_BATTERY4;

		if ((charging && (counter % 60 < 40)) || !charging)
			small_icon(407, 3, UI_COLOR(UI_PAL_TITLE), icon);

		uifont_print(432, 5, UI_COLOR(UI_PAL_TITLE), "[");
		uifont_print(462, 5, UI_COLOR(UI_PAL_TITLE), "%]");

		if (bat >= 0 && bat <= 100)
			sprintf(bat_left, "%3d", bat);
		else
			strcpy(bat_left, "---");

		width = uifont_get_string_width(bat_left);
		uifont_print(462 - width, 5, UI_COLOR(UI_PAL_TITLE), bat_left);

		if (!charging && (bat < 10) && (counter % 120 < 40))
		{
			int sx, sy, ex, ey;
			char message[128];

			sprintf(message, "Warning: Battery is low (%d%%). Please charge battery!", bat);
			width = uifont_get_string_width(message);

			sx = (SCR_WIDTH - width) >> 1;
			sy = (SCR_HEIGHT - FONTSIZE) >> 1;
			ex = sx + width;
			ey = sy + FONTSIZE;

			draw_dialog(sx - FONTSIZE/2, sy - FONTSIZE/2, ex + FONTSIZE/2, ey + FONTSIZE/2);

			uifont_print_center(sy, UI_COLOR(UI_PAL_WARNING), message);
		}
	}

	update |= (charging != prev_charging);
	update |= (bat != prev_bat);

	if (charging)
		update |= ((counter % 60 == 39) || (counter % 60 == 59));
	else
		update |= ((bat < 10) && ((counter % 120 == 39) || (counter % 120 == 119)));

	prev_bat = bat;
	prev_charging = charging;

	return update;
}
