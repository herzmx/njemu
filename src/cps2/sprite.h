/******************************************************************************

	sprite.c

	CPS2 スプライトマネージャ

******************************************************************************/

#ifndef CPS2_SPRITE_H
#define CPS2_SPRITE_H

void blit_clear_all_sprite(void);
void blit_palette_mark_dirty(int palno);

void blit_reset(void);
void blit_partial_start(int start, int end);
void blit_finish(void);

void blit_update_object(int x, int y, u32 code, u32 attr);
void blit_draw_object(int x, int y, int z, u32 code, u32 attr);
void blit_finish_object(int start_pri, int end_pri);

void blit_update_scroll1(int x, int y, u32 code, u32 attr);
void blit_draw_scroll1(int x, int y, u32 code, u32 attr);
void blit_finish_scroll1(void);

void blit_update_scroll2(int x, int y, u32 code, u32 attr);
void blit_draw_scroll2(int x, int y, u32 code, u32 attr);
void blit_finish_scroll2(int min_y, int max_y);

void blit_update_scroll3(int x, int y, u32 code, u32 attr);
void blit_draw_scroll3(int x, int y, u32 code, u32 attr);
void blit_finish_scroll3(void);

#endif /* CPS2_SPRITE_H */
