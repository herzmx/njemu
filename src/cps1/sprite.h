/******************************************************************************

	sprite.c

	CPS1 スプライトマネージャ

******************************************************************************/

#ifndef CPS1_SPRITE_H
#define CPS1_SPRITE_H

void blit_clear_all_sprite(void);
void blit_palette_mark_dirty(int palno);
void blit_scrollh_clear_sprite(int tpens);

void blit_reset(int bank_scroll1, int bank_scroll2, int bank_scroll3);
void blit_start(int high_layer);
void blit_finish(void);

void blit_update_object(int x, int y, u32 code, u32 attr);
void blit_draw_object(int x, int y, u32 code, u32 attr);
void blit_finish_object(void);

void blit_update_scroll1(int x, int y, u32 code, u32 attr);
void blit_draw_scroll1(int x, int y, u32 code, u32 attr, int gfxset);
void blit_finish_scroll1(void);

void blit_draw_scroll1h(int x, int y, u32 code, u32 attr, u16 tpens, int gfxset);

void blit_set_clip_scroll2(int min_y, int max_y);
void blit_update_scroll2(int x, int y, u32 code, u32 attr);
void blit_draw_scroll2(int x, int y, u32 code, u32 attr);
void blit_finish_scroll2(void);

void blit_update_scroll2h(int x, int y, u32 code, u32 attr);
void blit_draw_scroll2h(int x, int y, u32 code, u32 attr, u16 tpens);
void blit_finish_scroll2h(void);

void blit_update_scroll3(int x, int y, u32 code, u32 attr);
void blit_draw_scroll3(int x, int y, u32 code, u32 attr);
void blit_finish_scroll3(void);

void blit_update_scroll3h(int x, int y, u32 code, u32 attr);

void blit_update_scrollh(int x, int y, u32 code, u32 attr);
void blit_finish_scrollh(void);

#endif /* CPS1_SPRITE_H */
