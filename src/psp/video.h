/******************************************************************************

	video.c

	PSPビデオ制御関数 (16bitカラーのみ)

******************************************************************************/

#ifndef PSP_VIDEO_H
#define PSP_VIDEO_H

#define SCR_WIDTH			480
#define SCR_HEIGHT			272
#define BUF_WIDTH			512
#define	FRAMESIZE			(BUF_WIDTH * SCR_HEIGHT * sizeof(u16))
#define	FRAMESIZE32			(BUF_WIDTH * SCR_HEIGHT * sizeof(u32))

#define SLICE_SIZE			64 // change this to experiment with different page-cache sizes
#define TEXTURE_FLAGS		(GU_TEXTURE_16BIT | GU_COLOR_5551 | GU_VERTEX_16BIT | GU_TRANSFORM_2D)

#define MAKECOL15(r, g, b)	(((b & 0xf8) << 7) | ((g & 0xf8) << 2) | ((r & 0xf8) >> 3))
#define GETR15(col)			(((col << 3) & 0xf8) | ((col >>  2) & 0x07))
#define GETG15(col)			(((col >> 2) & 0xf8) | ((col >>  7) & 0x07))
#define GETB15(col)			(((col >> 7) & 0xf8) | ((col >> 12) & 0x07))

#define MAKECOL32(r, g, b)	(0xff000000 | ((b & 0xff) << 16) | ((g & 0xff) << 8) | (r & 0xff))
#define GETR32(col)			((col >>  0) & 0xff)
#define GETG32(col)			((col >>  8) & 0xff)
#define GETB32(col)			((col >> 16) & 0xff)

#define COLOR_BLACK			  0,  0,  0
#define COLOR_RED			255,  0,  0
#define COLOR_GREEN			  0,255,  0
#define COLOR_BLUE			  0,  0,255
#define COLOR_YELLOW		255,255,  0
#define COLOR_PURPLE		255,  0,255
#define COLOR_CYAN			  0,255,255
#define COLOR_WHITE			255,255,255
#define COLOR_GRAY			127,127,127
#define COLOR_DARKRED		127,  0,  0
#define COLOR_DARKGREEN		  0,127,  0
#define COLOR_DARKBLUE		  0,  0,127
#define COLOR_DARKYELLOW	127,127,  0
#define COLOR_DARKPURPLE	127,  0,127
#define COLOR_DARKCYAN		  0,127,127
#define COLOR_DARKGRAY		 63, 63, 63

#define GU_FRAME_ADDR(frame)		(u16 *)((u32)frame | 0x44000000)
#define CNVCOL15TO32(c)				((GETB15(c) << 16) | (GETG15(c) << 8) | GETR15(c))

#define SWIZZLED_UV(tex, u, v)		&tex[((((v) & ~7) << 9) | (((u) & ~7) << 3))]
#define SWIZZLED_8x8(tex, idx)		&tex[(idx) << 6]
#define SWIZZLED_16x16(tex, idx)	&tex[((idx & ~31) << 8) | ((idx & 31) << 7)]
#define SWIZZLED_32x32(tex, idx)	&tex[((idx & ~15) << 10) | ((idx & 15) << 8)]
#define SWIZZLED_GAP				((BUF_WIDTH << 3) - 8*8)


struct Vertex
{
	u16 u, v;
	u16 color;
	s16 x, y, z;
};

struct rectangle
{
	int min_x;
	int max_x;
	int min_y;
	int max_y;
};

typedef struct rect_t
{
	int left;
	int top;
	int right;
	int bottom;
} RECT;


extern u8 gulist[GULIST_SIZE];
extern void *show_frame;
extern void *draw_frame;
extern void *work_frame;
extern void *tex_frame;
extern RECT full_rect;
extern int psp_refreshrate;

void video_wait_vsync(void);
void video_flip_screen(int vsync);

void video_init(void);
void video_exit(int flag);
u16 *video_frame_addr(void *frame, int x, int y);
void video_clear_screen(void);
void video_clear_frame(void *frame);
void video_clear_rect(void *frame, RECT *rect);
void video_fill_frame(void *frame, u32 color);
void video_fill_rect(void *frame, u32 color, RECT *rect);
void video_clear_depth(void *frame);

void video_copy_rect(void *src, void *dst, RECT *src_rect, RECT *dst_rect);
void video_copy_rect_flip(void *src, void *dst, RECT *src_rect, RECT *dst_rect);
void video_copy_rect_alpha(void *src, void *dst, RECT *src_rect, RECT *dst_rect);
void video_copy_rect_rotate(void *src, void *dst, RECT *src_rect, RECT *dst_rect);

void video_calc_refreshrate(void);

#endif /* PSP_VIDE_H */
