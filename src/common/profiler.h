/***************************************************************************

    profiler.h

    Functions to manage profiling of MAME execution.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __PROFILER_H__
#define __PROFILER_H__

/* profiling */
enum
{
	PROFILER_END = -1,
	PROFILER_CPU1 = 0,
	PROFILER_MEMREAD1_8,
	PROFILER_MEMREAD1_16,
	PROFILER_MEMWRITE1_8,
	PROFILER_MEMWRITE1_16,
	PROFILER_CPU2,
	PROFILER_MEMREAD2,
	PROFILER_MEMWRITE2,
	PROFILER_VIDEO,
	PROFILER_DRAWGFX,
	PROFILER_BLIT,
	PROFILER_SOUND,
	PROFILER_TIMER_CALLBACK,
	PROFILER_INPUT,
	PROFILER_CACHE,
	PROFILER_DRAWUI,

	PROFILER_PROFILER,
	PROFILER_IDLE,
	PROFILER_TOTAL
};


/*
To start profiling a certain section, e.g. video:
profiler_mark(PROFILER_VIDEO);

to end profiling the current section:
profiler_mark(PROFILER_END);

the profiler handles a FILO list so calls may be nested.
*/

#if !RELEASE
void profiler_mark(int type);

/* functions called by usrintf.c */
void profiler_start(void);
void profiler_stop(void);
void profiler_show_text(void);
#else
#define profiler_mark(type)

#define profiler_start()
#define profiler_stop()
#define profiler_show_text()
#endif


#endif	/* __PROFILER_H__ */
