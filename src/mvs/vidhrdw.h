/***************************************************************************

	vidhrdw.c

	NEOGEO / NEOGEO CDZ video emulation

***************************************************************************/

#ifndef MVS_VIDEO_HARDWARE_H
#define MVS_VIDEO_HARDWARE_H

#define FIRST_VISIBLE_LINE		16
#define LAST_VISIBLE_LINE		239

extern u16 neogeo_vidram16[0x20000 / 2];
extern u16 neogeo_vidram16_offset;
extern u16 neogeo_vidram16_modulo;

extern u16 *neogeo_paletteram16;
extern u16 neogeo_palettebank16[2][0x2000 / 2];
extern u32 neogeo_palette_index;

extern u16 *video_palette16;
extern u16 video_palettebank16[2][0x2000 / 2];
extern u16 video_clut16[0x8000];

extern u8  *video_fix_usage[2];
extern u8  *video_spr_usage;

extern int fix_bank;
extern u8  *fix_usage;
extern u8  *fix_memory;
extern int neogeo_fix_bank_type;

void neogeo_video_init(void);
void neogeo_video_exit(void);
void neogeo_video_reset(void);

void neogeo_screenrefresh(void);
void neogeo_partial_screenrefresh(int current_line);
void neogeo_raster_screenrefresh(void);

void video_check_initialize(void);

#ifdef SAVE_STATE
STATE_SAVE( video );
STATE_LOAD( video );
#endif

#endif /* MVS_VIDEO_HARDWARE_H */
