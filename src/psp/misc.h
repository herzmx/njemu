/******************************************************************************

	misc.c

    PSP miscellaneous functions.

******************************************************************************/

#ifndef PSP_MISC_H
#define PSP_MISC_H

enum
{
	PSPCLOCK_222 = 0,
	PSPCLOCK_266,
	PSPCLOCK_333,
	PSPCLOCK_MAX
};

extern int psp_cpuclock;

void set_cpu_clock(int value);
void load_background(void);
void show_background(void);
int draw_battery_status(int draw);

#endif /* PSP_MISC_H */
