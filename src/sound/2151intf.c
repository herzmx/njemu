/***************************************************************************

  2151intf.c

  Support interface YM2151(OPM) + OKIM6295(ADPCM)

***************************************************************************/

#include "emumain.h"


/*------------------------------------------------------
	Start YM2151 + OKIM6295 emulation
 -----------------------------------------------------*/

void YM2151_sh_start(int type)
{
	int samplerate = PSP_SAMPLERATE;

	samplerate >>= (2 - option_samplerate);

	YM2151Init(3579545, samplerate, cps1_sound_interrupt);

	if (type == SOUND_YM2151_TYPE2)
		OKIM6295Init(6061, samplerate);
	else
		OKIM6295Init(7576, samplerate);
}


/*------------------------------------------------------
	Stop YM2151 emulation
 -----------------------------------------------------*/

void YM2151_sh_stop(void)
{
}


/*------------------------------------------------------
	Reset YM2151 emulation
 -----------------------------------------------------*/

void YM2151_sh_reset(void)
{
	YM2151Reset();
	OKIM6295Reset();
}


static int lastreg;

READ8_HANDLER( YM2151_status_port_r )
{
	return YM2151ReadStatus();
}

WRITE8_HANDLER( YM2151_register_port_w )
{
	lastreg = data;
}

WRITE8_HANDLER( YM2151_data_port_w )
{
	YM2151WriteReg(lastreg, data);
}
