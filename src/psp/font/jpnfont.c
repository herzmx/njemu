/******************************************************************************

	jpnfont.c

	Japanese font functions.

******************************************************************************/

#include "psp/psp.h"
#include "psp/font/jpnfont.h"


/*------------------------------------------------------
	日本語フォント用関数
------------------------------------------------------*/

u8 *jpnfont = NULL;
u16 *sjis_table;

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

#define set_font(name, type)				\
{											\
	fread(&size, 1, 4, fp);					\
	fread(&jpnfont[offset], 1, size, fp);	\
	name = (type *)&jpnfont[offset];		\
	offset += size;							\
	offset = (offset + 1) & ~1;				\
}

int load_jpnfont(void)
{
	FILE *fp;
	char path[MAX_PATH];
	int fontsize, size;

	if (jpnfont) return 1;

	sprintf(path, "%sfont/jp/jpnfont.dat", launchDir);

	if ((fp = fopen(path, "rb")) == NULL)
	{
		ui_popup("Could not open \"jpnfont.dat\".");
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
		set_font(jpn_h14_pitch,  s8)

		set_font(jpn_z14,        u8)
		set_font(jpn_z14_pos,    int)
		set_font(jpn_z14_width,  s8)
		set_font(jpn_z14_height, s8)
		set_font(jpn_z14_skipx,  s8)
		set_font(jpn_z14_skipy,  s8)
		set_font(jpn_z14_pitch,  s8)

		fclose(fp);

		return 1;
	}

	fclose(fp);
	free_jpnfont();
	ui_popup("Could not load \"jpnfont.dat\".");
	return 0;
}


void free_jpnfont(void)
{
	if (jpnfont)
	{
		free(jpnfont);
		jpnfont = NULL;
	}
}


#define NUM_FONTS	0x3f

int jpn_h14p_get_gryph(struct font_t *font, u16 code)
{
	if (jpnfont)
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

int jpn_h14p_get_pitch(u16 code)
{
	if (jpnfont)
	{
		if (code < NUM_FONTS)
			return jpn_h14_pitch[code];
	}
	return 7;
}


#undef NUM_FONTS
#define NUM_FONTS	0x1c9b

int jpn_z14p_get_gryph(struct font_t *font, u16 code)
{
	if (jpnfont)
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

int jpn_z14p_get_pitch(u16 code)
{
	if (jpnfont)
	{
		if (code < NUM_FONTS)
			return jpn_z14_pitch[code];
	}
	return 14;
}
