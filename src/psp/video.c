/******************************************************************************

	video.c

	PSPビデオ制御関数 (16bitカラーのみ)

******************************************************************************/

#include "psp.h"


/******************************************************************************
	グローバル変数/構造体
******************************************************************************/

u8 ALIGN_DATA gulist[GULIST_SIZE];
void *show_frame;
void *draw_frame;
void *work_frame;
void *tex_frame;

RECT full_rect = { 0, 0, SCR_WIDTH, SCR_HEIGHT };


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	VRAMクリア
--------------------------------------------------------*/

static void clear_vram(int flag)
{
	int i;
	u32 *vptr = (u32 *)(u8 *)0x44000000;

	if (flag)
		i = 2048 * 1024 >> 2;	// clear all VRAM
	else
		i = (FRAMESIZE32 * 3) >> 2;	// do not clear texture frame

	while (i--) *vptr++ = 0;
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	VSYNCを待つ
--------------------------------------------------------*/

void video_wait_vsync(void)
{
	sceDisplayWaitVblankStart();
}


/*--------------------------------------------------------
	スクリーンをフリップ
--------------------------------------------------------*/

void video_flip_screen(int vsync)
{
	if (vsync) sceDisplayWaitVblankStart();
	show_frame = draw_frame;
	draw_frame = sceGuSwapBuffers();
}


/*--------------------------------------------------------
	ビデオ処理初期化
--------------------------------------------------------*/

void video_init(void)
{
	draw_frame = (void *)(FRAMESIZE * 0);
	show_frame = (void *)(FRAMESIZE * 1);
	work_frame = (void *)(FRAMESIZE * 2);
	tex_frame  = (void *)(FRAMESIZE * 3);

	sceGuInit();
	sceGuDisplay(GU_FALSE);
	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBuffer(GU_PSM_5551, draw_frame, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, show_frame, BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH / 2), 2048 - (SCR_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);

	sceGuEnable(GU_SCISSOR_TEST);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);

	sceGuDisable(GU_ALPHA_TEST);
	sceGuAlphaFunc(GU_LEQUAL, 0, 1);

	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthRange(65535, 0);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDepthMask(GU_TRUE);

	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
	sceGuTexScale(1.0f / BUF_WIDTH, 1.0f / BUF_WIDTH);
	sceGuTexOffset(0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFlush();

	sceGuClearDepth(0);
	sceGuClearColor(0);
	sceGuClear(GU_COLOR_BUFFER_BIT);

	sceGuFinish();
	sceGuSync(0, 0);

	video_flip_screen(1);

	sceGuDisplay(GU_TRUE);

	clear_vram(0);

	create_small_font();
}


/*--------------------------------------------------------
	ビデオ処理終了
--------------------------------------------------------*/

void video_exit(int flag)
{
	sceGuDisplay(GU_FALSE);
	sceGuTerm();
	clear_vram(flag);
}


/*--------------------------------------------------------
	VRAMのアドレスを取得
--------------------------------------------------------*/

u16 *video_frame_addr(void *frame, int x, int y)
{
	return (u16 *)(((u32)frame | 0x44000000) + ((x + (y << 9)) << 1));
}


/*--------------------------------------------------------
	描画/表示フレームをクリア
--------------------------------------------------------*/

void video_clear_screen(void)
{
	video_clear_frame(show_frame);
	video_clear_frame(draw_frame);
}


/*--------------------------------------------------------
	指定したフレームをクリア
--------------------------------------------------------*/

void video_clear_frame(void *frame)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, frame, BUF_WIDTH);
	sceGuScissor(0, 0, BUF_WIDTH, SCR_HEIGHT);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	指定した矩形範囲をクリア
--------------------------------------------------------*/

void video_clear_rect(void *frame, RECT *rect)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, frame, BUF_WIDTH);
	sceGuScissor(rect->left, rect->top, rect->right, rect->bottom);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	指定したフレームを塗りつぶし
--------------------------------------------------------*/

void video_fill_frame(void *frame, u32 color)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, frame, BUF_WIDTH);
	sceGuScissor(0, 0, BUF_WIDTH, SCR_HEIGHT);
	sceGuClearColor(color);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuClearColor(0);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	指定した矩形範囲を塗りつぶし
--------------------------------------------------------*/

void video_fill_rect(void *frame, u32 color, RECT *rect)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDrawBufferList(GU_PSM_5551, frame, BUF_WIDTH);
	sceGuScissor(rect->left, rect->top, rect->right, rect->bottom);
	sceGuClearColor(color);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuClearColor(0);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	指定したDepthBufferをクリア
--------------------------------------------------------*/

void video_clear_depth(void *frame)
{
	sceGuStart(GU_DIRECT, gulist);
	sceGuDepthBuffer(frame, BUF_WIDTH);
	sceGuScissor(0, 0, BUF_WIDTH, SCR_HEIGHT);
	sceGuClear(GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	矩形範囲をコピー
--------------------------------------------------------*/

void video_copy_rect(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	int j, sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, dst, BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuDisable(GU_ALPHA_TEST);

	sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, GU_FRAME_ADDR(src));
	if (sw == dw && sh == dh)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	{
    	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->left + j + SLICE_SIZE;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	if (j < sw)
	{
		vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->right;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->right;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	矩形範囲を左右反転してコピー
--------------------------------------------------------*/

void video_copy_rect_flip(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	int j, sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, dst, BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuDisable(GU_ALPHA_TEST);

	sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, GU_FRAME_ADDR(src));
	if (sw == dw && sh == dh)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	{
    	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->right - j * dw / sw;
		vertices[0].y = dst_rect->bottom;

		vertices[1].u = src_rect->left + j + SLICE_SIZE;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->right - (j + SLICE_SIZE) * dw / sw;
		vertices[1].y = dst_rect->top;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	if (j < sw)
	{
		vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->right - j * dw / sw;
		vertices[0].y = dst_rect->bottom;

		vertices[1].u = src_rect->right;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->left;
		vertices[1].y = dst_rect->top;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	矩形範囲を透過処理ありでコピー
--------------------------------------------------------*/

void video_copy_rect_alpha(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	int j, sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, dst, BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuEnable(GU_ALPHA_TEST);

	sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, GU_FRAME_ADDR(src));
	if (sw == dw && sh == dh)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	for (j = 0; (j + SLICE_SIZE) < sw; j = j + SLICE_SIZE)
	{
    	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->left + j + SLICE_SIZE;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->left + (j + SLICE_SIZE) * dw / sw;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	if (j < sw)
	{
		vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

		vertices[0].u = src_rect->left + j;
		vertices[0].v = src_rect->top;
		vertices[0].x = dst_rect->left + j * dw / sw;
		vertices[0].y = dst_rect->top;

		vertices[1].u = src_rect->right;
		vertices[1].v = src_rect->bottom;
		vertices[1].x = dst_rect->right;
		vertices[1].y = dst_rect->bottom;

		sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);
	}

	sceGuFinish();
	sceGuSync(0, 0);
}


/*--------------------------------------------------------
	矩形範囲を270度回転してコピー
--------------------------------------------------------*/

void video_copy_rect_rotate(void *src, void *dst, RECT *src_rect, RECT *dst_rect)
{
	int sw, dw, sh, dh;
	struct Vertex *vertices;

	sw = src_rect->right - src_rect->left;
	dw = dst_rect->right - dst_rect->left;
	sh = src_rect->bottom - src_rect->top;
	dh = dst_rect->bottom - dst_rect->top;

	sceGuStart(GU_DIRECT, gulist);

	sceGuDrawBufferList(GU_PSM_5551, dst, BUF_WIDTH);
	sceGuScissor(dst_rect->left, dst_rect->top, dst_rect->right, dst_rect->bottom);
	sceGuDisable(GU_ALPHA_TEST);

	sceGuTexMode(GU_PSM_5551, 0, 0, GU_FALSE);
	sceGuTexImage(0, 512, 512, BUF_WIDTH, GU_FRAME_ADDR(src));
	if (sw == dh && sh == dw)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);

	vertices = (struct Vertex *)sceGuGetMemory(2 * sizeof(struct Vertex));

	vertices[0].u = src_rect->right;
	vertices[1].v = src_rect->top;
	vertices[0].x = dst_rect->right;
	vertices[0].y = dst_rect->top;

	vertices[1].u = src_rect->left;
	vertices[0].v = src_rect->bottom;
	vertices[1].x = dst_rect->left;
	vertices[1].y = dst_rect->bottom;

	sceGuDrawArray(GU_SPRITES, TEXTURE_FLAGS, 2, 0, vertices);

	sceGuFinish();
	sceGuSync(0, 0);
}
