/******************************************************************************

	usrintrf.c

	PSP ユーザインタフェース描画関数

******************************************************************************/

#ifndef PSP_USRINTRF_H
#define PSP_USRINTRF_H

struct font_t
{
	const u8 *data;
	int width;
	int height;
	int pitch;
	int skipx;
	int skipy;
};

#define FONTSIZE			14

#define FONT_UPARROW		"\x10"
#define FONT_DOWNARROW		"\x11"
#define FONT_LEFTARROW		"\x12"
#define FONT_RIGHTARROW		"\x13"
#define FONT_CIRCLE			"\x14"
#define FONT_CROSS			"\x15"
#define FONT_SQUARE			"\x16"
#define FONT_TRIANGLE		"\x17"
#define FONT_LTRIGGER		"\x18"
#define FONT_RTRIGGER		"\x19"
#define FONT_UPTRIANGLE		"\x1b"
#define FONT_DOWNTRIANGLE	"\x1c"
#define FONT_LEFTTRIANGLE	"\x1d"
#define FONT_RIGHTTRIANGLE	"\x1e"

enum
{
	ICON_CONFIG = 0,
	ICON_KEYCONFIG,
	ICON_FOLDER,
	ICON_SYSTEM,
	ICON_RETURN,
	ICON_EXIT,
	ICON_DIPSWITCH,
	ICON_UPPERDIR,
	ICON_MEMSTICK,
	ICON_ZIPFILE,
	ICON_BATTERY1,
	ICON_BATTERY2,
	ICON_BATTERY3,
	ICON_BATTERY4,
	MAX_ICONS
};

typedef struct ui_palette_t
{
	int r;
	int g;
	int b;
} UI_PALETTE;

enum
{
	UI_PAL_TITLE = 0,
	UI_PAL_SELECT,
	UI_PAL_NORMAL,
	UI_PAL_INFO,
	UI_PAL_WARNING,
	UI_PAL_BG1,
	UI_PAL_BG2,
	UI_PAL_FRAME,
	UI_PAL_FILESEL,
	UI_PAL_MAX
};

#define UI_COLOR(no)	ui_palette[no].r,ui_palette[no].g,ui_palette[no].b

#define ui_fill_frame(frame, no)		video_fill_frame(frame, MAKECOL32(ui_palette[no].r,ui_palette[no].g,ui_palette[no].b))
#define ui_clear_rect(frame, no, rect)	video_fill_rect(frame, MAKECOL32(ui_palette[no].r,ui_palette[no].g,ui_palette[no].b0), rect)


/*------------------------------------------------------
	テーブル等
------------------------------------------------------*/

// ユーザインタフェースカラーデータ
extern UI_PALETTE ui_palette[UI_PAL_MAX];

// アルファブレンド処理テーブル
extern const u8 alpha_blend[16][32][32];

// ゲーム画面用スモールフォント
extern const u8 font_s[];

// ダイアログ等の影データ
extern const u8 shadow[9][8][4];


/*------------------------------------------------------
	フォント管理
------------------------------------------------------*/

// プロポーショナルフォント
int graphic_font_get_gryph(struct font_t *font, u16 code);
int graphic_font_get_pitch(u16 code);

int ascii_14p_get_gryph(struct font_t *font, u16 code);
int ascii_14p_get_pitch(u16 code);

// アイコン(小)
int icon_s_get_gryph(struct font_t *font, u16 code);

// アイコン(大)
int icon_l_get_gryph(struct font_t *font, u16 code);


/*------------------------------------------------------
	フォント描画幅取得 (ユーザインタフェース用)
------------------------------------------------------*/

int uifont_get_string_width(const char *s);


/*------------------------------------------------------
	アイコン描画 (ユーザインタフェース用)
------------------------------------------------------*/

void small_icon(int sx, int sy, int r, int g, int b, int no);
void large_icon(int sx, int sy, int r, int g, int b, int no);


/*------------------------------------------------------
	フォント描画 (ユーザインタフェース用)
------------------------------------------------------*/

void uifont_print(int sx, int sy, int r, int g, int b, const char *s);
void uifont_print_center(int sy, int r, int g, int b, const char *s);


/*------------------------------------------------------
	図形描画 (ユーザインタフェース用)
------------------------------------------------------*/

void hline(int sx, int ex, int y, int r, int g, int b);
void hline_alpha(int sx, int ex, int y, int r, int g, int b, int alpha);

void vline(int x, int sy, int ey, int r, int g, int b);
void vline_alpha(int x, int sy, int ey, int r, int g, int b, int alpha);

void box(int sx, int sy, int ex, int ey, int r, int g, int b);

void boxfill(int sx, int sy, int ex, int ey, int r, int g, int b);
void boxfill_alpha(int sx, int sy, int ex, int ey, int r, int g, int b, int alpha);

void draw_dialog(int sx, int sy, int ex, int ey);
void draw_scrollbar(int sx, int sy, int ex, int ey, int disp_lines, int total_lines, int current_line);

void draw_bar_shadow(void);
void draw_box_shadow(int sx, int sy, int ex, int ey);


/*------------------------------------------------------
	フォント描画 (ゲーム画面用)
------------------------------------------------------*/

void create_small_font(void);
void small_font_print(int sx, int sy, const char *s, int bg);
void small_font_printf(int x, int y, const char *text, ...);


/*------------------------------------------------------
	ポップアップメッセージ
------------------------------------------------------*/

#define POPUP_MENU	0
#define POPUP_GAME	1

void ui_popup_reset(int mode);
void ui_popup(const char *text, ...);
int ui_show_popup(int draw);


/*------------------------------------------------------
	書式付文字列
------------------------------------------------------*/

void msg_screen_init(const char *title);
void msg_screen_clear(void);
void msg_set_text_color(u32 color);
void msg_printf(const char *text, ...);

#endif /* PSP_USRINTRF_H */
