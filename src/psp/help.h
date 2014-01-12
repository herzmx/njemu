/******************************************************************************

	help.c

	PSP ÉwÉãÉvï\é¶ä÷êî

******************************************************************************/

#ifndef PSP_HELP_H
#define PSP_HELP_H

enum
{
	HELP_FILEBROWSER = 0,
	HELP_GAMECONFIG,
	HELP_KEYCONFIG,
#if (EMU_SYSTEM != CPS2)
	HELP_DIPSWITCH,
#endif
#ifdef SAVE_STATE
	HELP_STATE,
#endif
	HELP_NUM_MAX
};

int help(int no);

#endif /* PSP_HELP */
