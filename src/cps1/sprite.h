/******************************************************************************

	sprite.c

	CPS1 スプライトマネージャ

******************************************************************************/

#ifndef CPS1_SPRITE_H
#define CPS1_SPRITE_H

void blit_clear_all_sprite(void);
void blit_palette_mark_dirty(int palno);
void blit_scrollh_clear_sprite(u16 tpens);

void blit_reset(int bank_scroll1, int bank_scroll2, int bank_scroll3, u8 *pen_usage16);
void blit_start(int high_layer);
void blit_finish(void);

void blit_update_object(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_object(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_object(void);

void blit_update_scroll1(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_scroll1(s16 x, s16 y, u32 code, u16 attr, u16 gfxset);
void blit_finish_scroll1(void);

void blit_draw_scroll1h(s16 x, s16 y, u32 code, u16 attr, u16 tpens, u16 gfxset);

void blit_set_clip_scroll2(s16 min_y, s16 max_y);
int blit_check_clip_scroll2(s16 sy);
void blit_update_scroll2(s16 x, s16 y, u32 code, u16 attr);
extern void (*blit_draw_scroll2)(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_scroll2(void);

void blit_update_scroll2h(s16 x, s16 y, u32 code, u16 attr);
extern void (*blit_draw_scroll2h)(s16 x, s16 y, u32 code, u16 attr, u16 tpens);
void blit_finish_scroll2h(void);

void blit_update_scroll3(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_scroll3(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_scroll3(void);

void blit_update_scroll3h(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_scroll3h(s16 x, s16 y, u32 code, u16 attr, u16 tpens);

void blit_update_scrollh(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_scrollh(void);

#endif /* CPS1_SPRITE_H */
