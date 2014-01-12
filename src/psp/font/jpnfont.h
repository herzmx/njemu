/******************************************************************************

	jpnfont.c

    Japanese font functions.

******************************************************************************/

#ifndef PSP_JPNFONT_H
#define PSP_JPNFONT_H

extern u32 jpnfont_offset;

extern u8 *jpnfont;
extern u16 *sjis_table;

int load_jpnfont(void);
void free_jpnfont(void);

int jpn_h14p_get_gryph(struct font_t *font, u16 code);
int jpn_h14p_get_pitch(u16 code);

int jpn_z14p_get_gryph(struct font_t *font, u16 code);
int jpn_z14p_get_pitch(u16 code);

#endif /* PSP__JPNFONT_H */
