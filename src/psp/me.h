/******************************************************************************

	me.c

	PSP MediaEngineêßå‰

******************************************************************************/

#ifndef PSP_ME_H
#define PSP_ME_H

#define PSP_UNCACHE_PTR(p) (((int)(p)) | 0x40000000)

void me_init(void);
void me_exit(void);
void me_start(void (*func)(int p), int param);
void me_stop(void);

#endif	/* PSP_ME_H */
