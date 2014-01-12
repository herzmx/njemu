/***************************************************************************

    profiler.c

    Functions to manage profiling of MAME execution.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#if !RELEASE

#include "emumain.h"

static int use_profiler;


#define MEMORY 6

struct _profile_data
{
	u64 count[MEMORY][PROFILER_TOTAL];
	u32 cpu_context_switches[MEMORY];
};
typedef struct _profile_data profile_data;

static profile_data profile;
static int memory;


static int FILO_type[10];
static TICKER FILO_start[10];
static int FILO_length;

void profiler_start(void)
{
	use_profiler = 1;
	FILO_length = 0;
}

void profiler_stop(void)
{
	use_profiler = 0;
}

void profiler_mark(int type)
{
	TICKER curr_cycles;


	if (!use_profiler)
	{
		FILO_length = 0;
		return;
	}

	if (type == PROFILER_CPU1 || type == PROFILER_CPU2)
		profile.cpu_context_switches[memory]++;

	curr_cycles = ticker();

	if (type != PROFILER_END)
	{
		if (FILO_length > 0)
		{
			if (FILO_length >= 10)
			{
				return;
			}

			/* handle nested calls */
			profile.count[memory][FILO_type[FILO_length - 1]] += curr_cycles - FILO_start[FILO_length - 1];
		}
		FILO_type[FILO_length] = type;
		FILO_start[FILO_length] = curr_cycles;
		FILO_length++;
	}
	else
	{
		if (FILO_length <= 0)
		{
			return;
		}

		FILO_length--;
		profile.count[memory][FILO_type[FILO_length]] += curr_cycles - FILO_start[FILO_length];
		if (FILO_length > 0)
		{
			/* handle nested calls */
			FILO_start[FILO_length - 1] = curr_cycles;
		}
	}
}

void profiler_show_text(void)
{
	int i, j;
	u64 total, normalize;
	u64 computed;
	static const char *names[PROFILER_TOTAL] =
	{
		"CPU 1    ",
		"Mem1 rd8 ",
		"Mem1 rd16",
		"Mem1 wr8 ",
		"Mem1 wr16",
		"CPU 2    ",
		"Mem2 rd  ",
		"Mem2 wr  ",
		"Video    ",
		"drawgfx  ",
		"GU Blit  ",
		"Sound    ",
		"Timer    ",
		"Input    ",
		"Cache    ",
		"Draw UI  ",
		"Profilr  ",
		"Idle     ",
	};
	static int showdelay[PROFILER_TOTAL];
	int line = 0;


	if (!use_profiler) return;

	profiler_mark(PROFILER_PROFILER);

	computed = 0;
	i = 0;
	while (i < PROFILER_PROFILER)
	{
		for (j = 0;j < MEMORY;j++)
			computed += profile.count[j][i];
		i++;
	}
	normalize = computed;
	while (i < PROFILER_TOTAL)
	{
		for (j = 0;j < MEMORY;j++)
			computed += profile.count[j][i];
		i++;
	}
	total = computed;

	if (total == 0 || normalize == 0) return;	/* we have been just reset */

	for (i = 0; i < PROFILER_TOTAL; i++)
	{
		computed = 0;

		for (j = 0; j < MEMORY; j++)
			computed += profile.count[j][i];

		if (computed || showdelay[i])
		{
			if (computed) showdelay[i] = 1;
			showdelay[i]--;

			if (i < PROFILER_PROFILER)
				small_font_printf(0, line++, "%s%3d%%%3d%%",names[i], (int)((computed * 100 + total/2) / total), (int)((computed * 100 + normalize/2) / normalize));
			else
				small_font_printf(0, line++, "%s%3d%%",names[i], (int)((computed * 100 + total/2) / total));
		}
	}

	i = 0;
	for (j = 0; j < MEMORY; j++)
		i += profile.cpu_context_switches[j];
	small_font_printf(0, line++, "CPU switches%4d", i / MEMORY);

	/* reset the counters */
	memory = (memory + 1) % MEMORY;
	profile.cpu_context_switches[memory] = 0;
	for (i = 0; i < PROFILER_TOTAL; i++)
		profile.count[memory][i] = 0;

	profiler_mark(PROFILER_END);
}

#endif /* RELEASE */
