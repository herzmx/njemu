/******************************************************************************

	vidhrdw.c

	NEOGEO CDZ ビデオエミュレーション

******************************************************************************/

#ifndef VIDHRDW_H
#define VIDHRDW_H

extern UINT16 neogeo_videoram[0x20000 / 2];
extern UINT16 videoram_read_buffer;
extern UINT16 videoram_offset;
extern UINT16 videoram_modulo;

extern UINT16 palettes[2][0x2000 / 2];
extern UINT32 palette_bank;

extern UINT16 *video_palette;
extern UINT16 video_palettebank[2][0x2000 / 2];
extern UINT16 video_clut16[0x8000];

extern UINT8  video_fix_usage[0x20000 / 32];
extern UINT8  video_spr_usage[0x400000 / 128];

extern int video_enable;
extern int spr_disable;
extern int fix_disable;

void neogeo_video_init(void);
void neogeo_video_exit(void);
void neogeo_video_reset(void);

void neogeo_partial_screenrefresh(int current_line);
void neogeo_screenrefresh(void);

int neogeo_loading_screenrefresh(int flag, int draw);

#ifdef SAVE_STATE
STATE_SAVE( video );
STATE_LOAD( video );
#endif

#endif /* VIDHRDW_H */
