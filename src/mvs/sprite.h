/******************************************************************************

	sprite.c

	MVS スプライトマネージャ

******************************************************************************/

#ifndef MVS_SPRITE_H
#define MVS_SPRITE_H

void blit_palette_mark_dirty(int palno);
void blit_clear_all_sprite(void);
void blit_clear_fix_sprite(void);
void blit_clear_spr_sprite(void);

void blit_reset(void);
void blit_partial_start(int start, int end);
void blit_finish(void);

void blit_draw_fix(int x, int y, u32 code, u32 attr);
void blit_finish_fix(void);

void blit_draw_spr(int x, int y, int w, int h, u32 code, u32 attr);
void blit_finish_spr(void);

void blit_reset_callback(void);

#endif /* MVS_SPRITE_H */
