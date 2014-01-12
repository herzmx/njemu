/******************************************************************************

	menu.c

	PSP ÉÅÉjÉÖÅ[

******************************************************************************/

#ifndef PSP_MENU_H
#define PSP_MENU_H

void showmenu(void);

#ifdef SAVE_STATE
int state_draw_progress(int progress);
#endif

#endif /* PSP_MENU_H */
