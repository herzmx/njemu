/******************************************************************************

	sprite.c

	CPS2 スプライトマネージャ

******************************************************************************/

#ifndef CPS2_SPRITE_H
#define CPS2_SPRITE_H

void blit_clear_all_sprite(void);

void blit_reset(void);
void blit_start(int start, int end);
void blit_finish(void);

void blit_update_object(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_object(s16 x, s16 y, u16 z, s16 priority, u32 code, u16 attr);
extern void (*blit_finish_object)(int start_pri, int end_pri);

void blit_update_scroll1(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_scroll1(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_scroll1(void);

void blit_set_clip_scroll2(s16 min_y, s16 max_y);
int blit_check_clip_scroll2(s16 sy);
void blit_update_scroll2(s16 x, s16 y, u32 code, u16 attr);
extern void (*blit_draw_scroll2)(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_scroll2(void);

void blit_update_scroll3(s16 x, s16 y, u32 code, u16 attr);
void blit_draw_scroll3(s16 x, s16 y, u32 code, u16 attr);
void blit_finish_scroll3(void);

#endif /* CPS2_SPRITE_H */
