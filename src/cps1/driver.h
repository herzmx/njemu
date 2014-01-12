/******************************************************************************

	driver.c

	CPS1 ƒhƒ‰ƒCƒo

******************************************************************************/

#ifndef CPS1_DRIVER_H
#define CPS1_DRIVER_H

#define RASTER_LINES	262

#define CPS1_KLUDGE_FORGOTTN	1
#define CPS1_KLUDGE_GHOULS		2
#define CPS1_KLUDGE_MERCS		3
#define CPS1_KLUDGE_3WONDERS	4
#define CPS1_KLUDGE_PANG3		5
#define CPS1_KLUDGE_SF2CEB		6

enum
{
	MACHINE_cps1 = 0,
	MACHINE_forgottn,
	MACHINE_sf2,
	MACHINE_qsound,
	MACHINE_pang3,
	MACHINE_MAX
};

enum
{
	INPTYPE_cps1 = 0,
	INPTYPE_forgottn,
	INPTYPE_ghouls,
	INPTYPE_ghoulsu,
	INPTYPE_daimakai,
	INPTYPE_strider,
	INPTYPE_stridrua,
	INPTYPE_dynwar,
	INPTYPE_willow,
	INPTYPE_unsquad,
	INPTYPE_ffight,
	INPTYPE_1941,
	INPTYPE_mercs,
	INPTYPE_mtwins,
	INPTYPE_msword,
	INPTYPE_cawing,
	INPTYPE_nemo,
	INPTYPE_sf2,
	INPTYPE_sf2j,
	INPTYPE_3wonders,
	INPTYPE_kod,
	INPTYPE_kodj,
	INPTYPE_captcomm,
	INPTYPE_knights,
	INPTYPE_varth,
	INPTYPE_cworld2j,
	INPTYPE_qad,
	INPTYPE_qadj,
	INPTYPE_qtono2,
	INPTYPE_megaman,
	INPTYPE_rockmanj,
	INPTYPE_wof,
	INPTYPE_dino,
	INPTYPE_punisher,
	INPTYPE_slammast,
	INPTYPE_pnickj,
	INPTYPE_pang3,
	INPTYPE_sfzch,
	INPTYPE_MAX
};

enum
{
	INIT_cps1 = 0,
	INIT_wof,
	INIT_dino,
	INIT_punisher,
	INIT_slammast,
	INIT_pang3,
	INIT_MAX
};

enum
{
	SCREEN_NORMAL = 0,
	SCREEN_VERTICAL,
	SCREENTYPE_MAX
};

struct tile_t
{
	u16 start;
	u16 end;
};

struct driver_t
{
	const char *name;
	const u16 cpsb_addr;
	const u16 cpsb_value;
	const u8  mult_factor1;
	const u8  mult_factor2;
	const u8  mult_result_lo;
	const u8  mult_result_hi;
	const u8  priority[4];
	const u8  layer_enable_mask[5];
	const u8  layer_control;
	const u8  control_reg;
	const u8  kludge;
	const u8  has_stars;
	const u8  bank_scroll1;
	const u8  bank_scroll2;
	const u8  bank_scroll3;
	const u32 gfx_limit;
	const struct tile_t scroll1;
	const struct tile_t scroll2;
	const struct tile_t scroll3;
};

extern struct driver_t CPS1_driver[];
extern struct driver_t *driver;

WRITE16_HANDLER( cps1_coinctrl_w );
WRITE16_HANDLER( cps1_coinctrl2_w );

READ16_HANDLER( cps1_dsw_a_r );
READ16_HANDLER( cps1_dsw_b_r );
READ16_HANDLER( cps1_dsw_c_r );

READ16_HANDLER( cps1_inputport0_r );
READ16_HANDLER( cps1_inputport1_r );
READ16_HANDLER( cps1_inputport2_r );
READ16_HANDLER( cps1_inputport3_r );

READ16_HANDLER( forgottn_dial_0_r );
READ16_HANDLER( forgottn_dial_1_r );
WRITE16_HANDLER( forgottn_dial_0_reset_w );
WRITE16_HANDLER( forgottn_dial_1_reset_w );

WRITE8_HANDLER( cps1_snd_bankswitch_w );

WRITE8_HANDLER( cps1_oki_pin7_w );

READ8_HANDLER( cps1_sound_fade_timer_r );
WRITE16_HANDLER( cps1_sound_fade_timer_w );

READ8_HANDLER( cps1_sound_command_r );
WRITE16_HANDLER( cps1_sound_command_w );

READ16_HANDLER( cps1_eeprom_port_r );
WRITE16_HANDLER( cps1_eeprom_port_w );

READ16_HANDLER( qsound_rom_r );
READ16_HANDLER( qsound_sharedram1_r );
WRITE16_HANDLER( qsound_sharedram1_w );
READ16_HANDLER( qsound_sharedram2_r );
WRITE16_HANDLER( qsound_sharedram2_w );
WRITE8_HANDLER( qsound_banksw_w );

TIMER_CALLBACK( cps1_vblank_interrupt );
TIMER_CALLBACK( cps1_qsound_interrupt );

void cps1_sound_interrupt(int state);

void pang3_decode(void);

int cps1_driver_init(void);
void cps1_driver_reset(void);
void cps1_driver_exit(void);

#ifdef SAVE_STATE
STATE_SAVE( driver );
STATE_LOAD( driver );
#endif

/* kabuki.c */
void wof_decode(void);
void dino_decode(void);
void punisher_decode(void);
void slammast_decode(void);

#endif /* CPS1_DRIVER_H */
