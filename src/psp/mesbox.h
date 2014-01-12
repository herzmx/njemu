/******************************************************************************

	mesbox.c

	Message Box

******************************************************************************/

#ifndef PSP_MESSAGEBOX_H
#define PSP_MESSAGEBOX_H

enum
{
	MB_LAUNCHZIPFILE = 0,
	MB_EXITEMULATOR,
	MB_SETSTARTUPDIR,
	MB_RESETEMULATION,
	MB_RESTARTEMULATION,
#ifdef SAVE_STATE
	MB_DELETESTATE,
#endif
	MB_GAMENOTWORK,
	MB_NUM_MAX
};

int messagebox(int no);

#endif /* PSP_MESSAGEBOX_H */
