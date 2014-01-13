/******************************************************************************

	dipsw.c

	MVS DIP�X�C�b�`�ݒ�

******************************************************************************/

#ifndef MVS_DIP_SWITCH_H
#define MVS_DIP_SWITCH_H

#define MAX_DIPSWITCHS		8

typedef struct
{
	const char *label;
	u8 enable;
	u8 mask;
	u8 value;
	u8 value_max;
	const char *values_label[MAX_DIPSWITCHS + 1];
} dipswitch_t;


dipswitch_t *load_dipswitch(void);
void save_dipswitch(void);

#endif /* MVS_DIP_SWITCH_H */
