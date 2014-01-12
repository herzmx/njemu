/******************************************************************************

	sprite.c

	MVS スプライトマネージャ

******************************************************************************/

#ifndef MVS_SPRITE_H
#define MVS_SPRITE_H

void blit_clear_all_sprite(void);
void blit_set_sprite_clear_flag(void);

void blit_reset(void);
void blit_start(int start, int end);
void blit_finish(void);

void blit_draw_fix(int x, int y, u32 code, u32 attr);
void blit_finish_fix(void);

void blit_draw_spr(int x, int y, int w, int h, u32 code, u32 attr);
void blit_finish_spr(void);

#endif /* MVS_SPRITE_H */
