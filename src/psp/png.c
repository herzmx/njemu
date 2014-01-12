/***************************************************************************

	png.c

    PSP PNG format image I/O functions. (based on M.A.M.E. PNG functions)

***************************************************************************/

#include "psp.h"
#include <math.h>
#include "zlib/zlib.h"

#define PNG_Signature       "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
#define MNG_Signature       "\x8A\x4D\x4E\x47\x0D\x0A\x1A\x0A"

#define PNG_CN_IHDR 0x49484452L     /* Chunk names */
#define PNG_CN_PLTE 0x504C5445L
#define PNG_CN_IDAT 0x49444154L
#define PNG_CN_IEND 0x49454E44L
#define PNG_CN_gAMA 0x67414D41L
#define PNG_CN_sBIT 0x73424954L
#define PNG_CN_cHRM 0x6348524DL
#define PNG_CN_tRNS 0x74524E53L
#define PNG_CN_bKGD 0x624B4744L
#define PNG_CN_hIST 0x68495354L
#define PNG_CN_tEXt 0x74455874L
#define PNG_CN_zTXt 0x7A545874L
#define PNG_CN_pHYs 0x70485973L
#define PNG_CN_oFFs 0x6F464673L
#define PNG_CN_tIME 0x74494D45L
#define PNG_CN_sCAL 0x7343414CL

#define PNG_PF_None     0   /* Prediction filters */
#define PNG_PF_Sub      1
#define PNG_PF_Up       2
#define PNG_PF_Average  3
#define PNG_PF_Paeth    4

#define MNG_CN_MHDR 0x4D484452L     /* MNG Chunk names */
#define MNG_CN_MEND 0x4D454E44L
#define MNG_CN_TERM 0x5445524DL
#define MNG_CN_BACK 0x4241434BL


/* PNG support */
struct png_info {
	u32 width, height;
	u32 xres, yres;
	struct rectangle screen;
	double xscale, yscale;
	double source_gamma;
	u32 chromaticities[8];
	u32 resolution_unit, offset_unit, scale_unit;
	u8 bit_depth;
	u32 significant_bits[4];
	u32 background_color[4];
	u8 color_type;
	u8 compression_method;
	u8 filter_method;
	u8 interlace_method;
	u32 num_palette;
	u8 *palette;
	u32 num_trans;
	u8 *trans;
	u8 *image;

	/* The rest is private and should not be used
	 * by the public functions
	 */
	u8 bpp;
	u32 rowbytes;
	u8 *zimage;
	u32 zlength;
	u8 *fimage;
};

/********************************************************************************

  Helper functions

********************************************************************************/

static void errormsg(int no)
{
	switch (no)
	{
	case 0:
		ui_popup("Error: Could not allocate memory for PNG.");
		break;

	case 1:
		ui_popup("Error: Could not enecode PNG image.");
		break;
	}
}


/********************************************************************************

  PNG write functions (16bit color only)

********************************************************************************/

struct png_text
{
	char *data;
	int length;
	struct png_text *next;
};

static struct png_text *png_text_list = 0;

static void convert_to_network_order(u32 i, u8 *v)
{
	v[0] = (i >> 24) & 0xff;
	v[1] = (i >> 16) & 0xff;
	v[2] = (i >>  8) & 0xff;
	v[3] = (i >>  0) & 0xff;
}

static int png_add_text(const char *keyword, const char *text)
{
	struct png_text *pt;

	pt = malloc(sizeof(struct png_text));
	if (pt == 0)
		return 0;

	pt->length = strlen(keyword) + strlen(text) + 1;
	pt->data = malloc(pt->length + 1);
	if (pt->data == 0)
		return 0;

	strcpy(pt->data, keyword);
	strcpy(pt->data + strlen(keyword) + 1, text);
	pt->next = png_text_list;
	png_text_list = pt;

	return 1;
}

static int write_chunk(FILE *fp, u32 chunk_type, u8 *chunk_data, u32 chunk_length)
{
	u32 crc;
	u8 v[4];
	int written;

	/* write length */
	convert_to_network_order(chunk_length, v);
	written = fwrite(v, 1, 4, fp);

	/* write type */
	convert_to_network_order(chunk_type, v);
	written += fwrite(v, 1, 4, fp);

	/* calculate crc */
	crc = crc32(0, v, 4);
	if (chunk_length > 0)
	{
		/* write data */
		written += fwrite(chunk_data, 1, chunk_length, fp);
		crc = crc32(crc, chunk_data, chunk_length);
	}
	convert_to_network_order(crc, v);

	/* write crc */
	written += fwrite(v, 1, 4, fp);

	if (written != 3*4+chunk_length)
	{
		errormsg(1);
		return 0;
	}
	return 1;
}

static int png_write_sig(FILE *fp)
{
	/* PNG Signature */
	if (fwrite(PNG_Signature, 1, 8, fp) != 8)
	{
		errormsg(1);
		return 0;
	}
	return 1;
}

static int png_write_datastream(FILE *fp, struct png_info *p)
{
	u8 ihdr[13];
	struct png_text *pt;

	/* IHDR */
	convert_to_network_order(p->width, ihdr);
	convert_to_network_order(p->height, ihdr + 4);
	*(ihdr +  8) = p->bit_depth;
	*(ihdr +  9) = p->color_type;
	*(ihdr + 10) = p->compression_method;
	*(ihdr + 11) = p->filter_method;
	*(ihdr + 12) = p->interlace_method;

	if (write_chunk(fp, PNG_CN_IHDR, ihdr, 13) == 0)
		return 0;

	/* PLTE */
	if (p->num_palette > 0)
		if (write_chunk(fp, PNG_CN_PLTE, p->palette, p->num_palette*3) == 0)
			return 0;

	/* IDAT */
	if (write_chunk(fp, PNG_CN_IDAT, p->zimage, p->zlength) == 0)
		return 0;

	/* tEXt */
	while (png_text_list)
	{
		pt = png_text_list;
		if (write_chunk(fp, PNG_CN_tEXt, (u8 *)pt->data, pt->length) == 0)
			return 0;
		free(pt->data);

		png_text_list = pt->next;
		free(pt);
	}

	/* IEND */
	if (write_chunk(fp, PNG_CN_IEND, NULL, 0) == 0)
		return 0;

	return 1;
}

static int png_filter(struct png_info *p)
{
	int i;
	u8 *src, *dst;

	if ((p->fimage = (u8 *)malloc(p->height * (p->rowbytes + 1))) == NULL)
	{
		errormsg(0);
		return 0;
	}

	dst = p->fimage;
	src = p->image;

	for (i = 0; i < p->height; i++)
	{
		*dst++ = 0; /* No filter */
		memcpy(dst, src, p->rowbytes);
		src += p->rowbytes;
		dst += p->rowbytes;
	}
	return 1;
}

static int png_deflate_image(struct png_info *p)
{
	unsigned long zbuff_size;

	zbuff_size = (p->height * (p->rowbytes + 1)) * 1.1 + 12;

	if ((p->zimage = (u8 *)malloc(zbuff_size)) == NULL)
	{
		errormsg(0);
		return 0;
	}

	if (compress(p->zimage, &zbuff_size, p->fimage, p->height * (p->rowbytes + 1)) != Z_OK)
	{
		errormsg(1);
		return 0;
	}
	p->zlength = zbuff_size;

	return 1;
}

#if 0
static int png_pack_buffer(struct png_info *p)
{
	u8 *outp, *inp;
	int i,j,k;

	outp = inp = p->image;

	if (p->bit_depth < 8)
	{
		for (i = 0; i < p->height; i++)
		{
			for (j=0; j<p->width/(8/p->bit_depth); j++)
			{
				for (k=8/p->bit_depth-1; k>=0; k--)
					*outp |= *inp++ << k * p->bit_depth;
				outp++;
				*outp = 0;
			}
			if (p->width % (8/p->bit_depth))
			{
				for (k=p->width%(8/p->bit_depth)-1; k>=0; k--)
					*outp |= *inp++ << k * p->bit_depth;
				outp++;
				*outp = 0;
			}
		}
	}
	return 1;
}
#endif

static int png_create_datastream(FILE *fp)
{
	int i, j;
	u8 *ip;
	struct png_info p;
	u16 *vptr, *src;

	memset(&p, 0, sizeof (struct png_info));
	p.xscale = p.yscale = p.source_gamma = 0.0;
	p.palette = p.trans = p.image = p.zimage = p.fimage = NULL;
	p.width = SCR_WIDTH;
	p.height = SCR_HEIGHT;
	p.color_type = 2;
	p.rowbytes = p.width * 3;
	p.bit_depth = 8;

	if ((p.image = (u8 *)malloc(p.height * p.rowbytes))==NULL)
	{
		errormsg(0);
		return 0;
	}

	ip = p.image;

	vptr = video_frame_addr(show_frame, 0, 0);

	for (i = 0; i < p.height; i++)
	{
		src = &vptr[i * BUF_WIDTH];

		for (j = 0; j < p.width; j++)
		{
			u16 color = src[j];
			*ip++ = (u8)GETR15(color);
			*ip++ = (u8)GETG15(color);
			*ip++ = (u8)GETB15(color);
		}
	}

	if (png_filter(&p) == 0)
		return 0;

	if (png_deflate_image(&p) == 0)
		return 0;

	if (png_write_datastream(fp, &p) == 0)
		return 0;

	if (p.palette) free(p.palette);
	if (p.image) free(p.image);
	if (p.zimage) free(p.zimage);
	if (p.fimage) free(p.fimage);
	return 1;
}


/********************************************************************************

  Interface for PSP

********************************************************************************/

/*--------------------------------------------------------
	Save PNG
 -------------------------------------------------------*/

int save_png(const char *path)
{
	FILE *fp;
	int res;

	if ((fp = fopen(path, "wb")) == NULL)
		return 0;

	if ((res = png_add_text("Software", APPNAME_STR " " VERSION_STR)))
	{
		if ((res = png_add_text("System", "PSP")))
		{
			if ((res = png_write_sig(fp)))
			{
				res = png_create_datastream(fp);
			}
		}
	}

	fclose(fp);

	return res;
}
