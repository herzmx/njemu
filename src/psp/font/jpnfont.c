/******************************************************************************

	jpnfont.c

	日本語フォント関数 (外部データ使用)

******************************************************************************/

#if !JAPANESE_UI

#include "psp/psp.h"


/******************************************************************************
	マクロ
******************************************************************************/

#define set_font(name, type)				\
{											\
	fread(&size, 1, 4, fp);					\
	fread(&jpnfont[offset], 1, size, fp);	\
	name = (type *)&jpnfont[offset];		\
	offset += size;							\
	offset = (offset + 1) & ~1;				\
}


/******************************************************************************
	グローバル変数
******************************************************************************/

u8 *jpnfont = NULL;
u16 *sjis_table;


/******************************************************************************
	ローカル変数
******************************************************************************/

static int gryph;

static u8 *jpn_h14;
static int *jpn_h14_pos;
static s8 *jpn_h14_width;
static s8 *jpn_h14_height;
static s8 *jpn_h14_skipx;
static s8 *jpn_h14_skipy;
static s8 *jpn_h14_pitch;

static u8 *jpn_z14;
static int *jpn_z14_pos;
static s8 *jpn_z14_width;
static s8 *jpn_z14_height;
static s8 *jpn_z14_skipx;
static s8 *jpn_z14_skipy;
static s8 *jpn_z14_pitch;


/******************************************************************************
	日本語フォント管理用関数
******************************************************************************/

/*------------------------------------------------------
	日本語フォントを読み込む
------------------------------------------------------*/

int load_jpnfont(int gryph_type)
{
	FILE *fp;
	char path[MAX_PATH];
	const char *font_name[2] = { "jpn_p.fnt", "jpn_f.fnt" };
	int fontsize, size;

	if (jpnfont) free_jpnfont();

	gryph = gryph_type;

	sprintf(path, "%sfont/%s", launchDir, font_name[gryph]);

	if ((fp = fopen(path, "rb")) == NULL)
	{
		ui_popup("Could not open \"%s\".", font_name[gryph]);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	fontsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	fontsize += 16;
	if ((jpnfont = memalign(MEM_ALIGN, fontsize)) != NULL)
	{
		u32 offset = 0;

		memset(jpnfont, 0, fontsize);

		set_font(sjis_table,     u16)

		set_font(jpn_h14,        u8)
		set_font(jpn_h14_pos,    int)
		set_font(jpn_h14_width,  s8)
		set_font(jpn_h14_height, s8)
		set_font(jpn_h14_skipx,  s8)
		set_font(jpn_h14_skipy,  s8)
		if (!gryph)
		{
			set_font(jpn_h14_pitch,  s8)
		}

		set_font(jpn_z14,        u8)
		set_font(jpn_z14_pos,    int)
		set_font(jpn_z14_width,  s8)
		set_font(jpn_z14_height, s8)
		set_font(jpn_z14_skipx,  s8)
		set_font(jpn_z14_skipy,  s8)
		if (!gryph)
		{
			set_font(jpn_z14_pitch,  s8)
		}

		fclose(fp);

		return 1;
	}

	fclose(fp);
	free_jpnfont();
	ui_popup("Could not load \"%s\".", font_name[gryph_type]);
	return 0;
}


/*------------------------------------------------------
	日本語フォントを開放する
------------------------------------------------------*/

void free_jpnfont(void)
{
	if (jpnfont)
	{
		free(jpnfont);
		jpnfont = NULL;
	}
}


/*------------------------------------------------------
	半角フォントデータ取得
------------------------------------------------------*/

#define NUM_FONTS	0x3f

#if (EMU_SYSTEM != NCDZ)
int jpn_h14p_get_gryph(struct font_t *font, u16 code)
{
	if (jpnfont && !gryph)
	{
		if (code < NUM_FONTS)
		{
			font->data   = &jpn_h14[jpn_h14_pos[code]];
			font->width  = jpn_h14_width[code];
			font->height = jpn_h14_height[code];
			font->pitch  = jpn_h14_pitch[code];
			font->skipx  = jpn_h14_skipx[code];
			font->skipy  = jpn_h14_skipy[code];
			return 1;
		}
	}
	return 0;
}
#endif

#ifdef COMMAND_LIST
int jpn_h14_get_gryph(struct font_t *font, u16 code)
{
	if (jpnfont && gryph)
	{
		if (code < NUM_FONTS)
		{
			font->data   = &jpn_h14[jpn_h14_pos[code]];
			font->width  = jpn_h14_width[code];
			font->height = jpn_h14_height[code];
			font->pitch  = 7;
			font->skipx  = jpn_h14_skipx[code];
			font->skipy  = jpn_h14_skipy[code];
			return 1;
		}
	}
	return 0;
}
#endif

#if (EMU_SYSTEM != NCDZ)
int jpn_h14p_get_pitch(u16 code)
{
	if (jpnfont && !gryph)
	{
		if (code < NUM_FONTS)
			return jpn_h14_pitch[code];
	}
	return 7;
}
#endif

#undef NUM_FONTS


/*------------------------------------------------------
	全角フォントデータ取得
------------------------------------------------------*/

#define NUM_FONTS	0x1c9b

#if (EMU_SYSTEM != NCDZ)
int jpn_z14p_get_gryph(struct font_t *font, u16 code)
{
	if (jpnfont && !gryph)
	{
		if (code < NUM_FONTS)
		{
			font->data   = &jpn_z14[jpn_z14_pos[code]];
			font->width  = jpn_z14_width[code];
			font->height = jpn_z14_height[code];
			font->pitch  = jpn_z14_pitch[code];
			font->skipx  = jpn_z14_skipx[code];
			font->skipy  = jpn_z14_skipy[code];
			return 1;
		}
	}
	return 0;
}
#endif

#ifdef COMMAND_LIST
int jpn_z14_get_gryph(struct font_t *font, u16 code)
{
	if (jpnfont && gryph)
	{
		if (code < NUM_FONTS)
		{
			font->data   = &jpn_z14[jpn_z14_pos[code]];
			font->width  = jpn_z14_width[code];
			font->height = jpn_z14_height[code];
			font->pitch  = 14;
			font->skipx  = jpn_z14_skipx[code];
			font->skipy  = jpn_z14_skipy[code];
			return 1;
		}
	}
	return 0;
}
#endif

#if (EMU_SYSTEM != NCDZ)
int jpn_z14p_get_pitch(u16 code)
{
	if (jpnfont && !gryph)
	{
		if (code < NUM_FONTS)
			return jpn_z14_pitch[code];
	}
	return 14;
}
#endif

#endif
