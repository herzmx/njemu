/******************************************************************************

	cdrom.c

	NEOGEO CDZ CD-ROMエミュレーション

******************************************************************************/

#ifndef NCDZ_CDROM_H
#define NCDZ_CDROM_H

extern int neogeo_loadscreen;
extern int neogeo_cdspeed_limit;
extern int neogeo_loadfinished;

void cdrom_init(void);
void cdrom_shutdown(void);
int cdrom_process_ipl(void);
void cdrom_loadfiles(void);
void cdrom_load_files(void);

void neogeo_decode_spr(u8 *mem, u32 offset, u32 length);
void neogeo_decode_fix(u8 *mem, u32 offset, u32 length);
void neogeo_undecode_fix(u8 *mem, u32 offset, u32 length);
void neogeo_apply_patch(u16 *src, int bank, u32 offset);

#ifdef SAVE_STATE
#define MAX_CDROM_STATE		0x100
STATE_SAVE( cdrom );
STATE_LOAD( cdrom );
#endif

#endif /* NCDZ_CDROM_H */
