/******************************************************************************

	timer.c

	タイマー管理

******************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#define YM2610_TIMERA			0
#define YM2610_TIMERB			1
#define SCANLINE_TIMER			2
#define VBLANK_TIMER			3
#define SOUNDLATCH_TIMER		4
#define SOUNDUPDATE_TIMER		5
#define CPUSPIN_TIMER			6
#define WATCHDOG_TIMER			7
#define MAX_TIMER				8

#define TIME_NOW				(0)
#define TIME_NEVER				(0x7fffffff)
#define TIME_IN_HZ(hz)			(1000000.0 / (hz))

#define SEC_TO_USEC(secs)		(int)((float)(secs) * 1000000.0)

#define USECS_PER_SCANLINE		64

#define SUSPEND_REASON_HALT		0x0001
#define SUSPEND_REASON_RESET	0x0002
#define SUSPEND_REASON_SPIN		0x0004
#define SUSPEND_REASON_TRIGGER	0x0008
#define SUSPEND_REASON_DISABLE	0x0010
#define SUSPEND_ANY_REASON		((UINT32)-1)

#define TIMER_CALLBACK(name)	void name(int param)

void timer_reset(void);
void timer_set_update_handler(void);
void timer_suspend_cpu(int cpunum, int state, int reason);
int timer_enable(int which, int enable);
void timer_adjust(int which, int duration, int param, void (*callback)(int raram));
void timer_set(int which, int duration, int param, void (*callback)(int param));
float timer_get_time(void);
int timer_getscanline(void);
void timer_update_cpu(void);

#ifdef SAVE_STATE
STATE_SAVE( timer );
STATE_LOAD( timer );
#endif

#endif /* TIMER_H */
