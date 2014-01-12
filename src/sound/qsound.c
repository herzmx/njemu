/*****************************************************************************

	qsound.c

	CAPCOM QSound エミュレータ (CPS1/CPS2)

******************************************************************************/

#include "emumain.h"

#define QSOUND_CLOCK    4000000		/* default 4MHz clock */
#define QSOUND_CLOCKDIV 166			/* Clock divider */
#define QSOUND_CHANNELS 16

typedef s8  QSOUND_SRC_SAMPLE;
typedef s16 QSOUND_SAMPLE;
typedef s32 QSOUND_SAMPLE_MIX;


/******************************************************************************
	ローカル変数/構造体
******************************************************************************/

typedef struct
{
	int bank;			/* bank (x16)	*/
	int address;		/* start address */
	int pitch;			/* pitch */
	int loop;			/* loop address */
	int end;			/* end address */
	int vol;			/* master volume */
	int pan;			/* Pan value */

	/* Work variables */
	int key;			/* Key on / key off */

	int lvol;			/* left volume */
	int rvol;			/* right volume */
	int lastdt;			/* last sample value */
	int offset;			/* current offset counter */
} QSOUND_CHANNEL;

static QSOUND_CHANNEL ALIGN_DATA qsound_channel[QSOUND_CHANNELS];

static QSOUND_SRC_SAMPLE *qsound_sample_rom;

static float qsound_frq_ratio;
static int qsound_data;
static int qsound_samplerate;
static int qsound_sample_length;
static int qsound_resample_count;
static int qsound_stream_type;
static int qsound_volume_shift;

static const int ALIGN_DATA qsound_pan_table[33] =
{
	  0, 45, 64, 78, 90,101,110,119,
	128,135,143,150,156,163,169,175,
	181,186,191,197,202,207,212,217,
	221,226,230,235,239,243,247,251,
	256
};

static QSOUND_SAMPLE_MIX ALIGN_DATA qsound_buffer[2][SOUND_SAMPLES];
static void  (*qsound_update_stream)(void);


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	サウンドストリーム生成 (通常)
--------------------------------------------------------*/

static void qsound_update_stream_normal(void)
{
	int ch;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		QSOUND_CHANNEL *pC = &qsound_channel[ch];

		if (pC->key)
		{
			int i;
			QSOUND_SRC_SAMPLE *pST  = qsound_sample_rom + pC->bank;
			QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
			QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX rvol  = (pC->rvol * pC->vol) >> qsound_volume_shift;
			QSOUND_SAMPLE_MIX lvol  = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < qsound_sample_length; i++)
			{
				int count = (pC->offset) >> 16;

				pC->offset &= 0xffff;

				if (count)
				{
					pC->address += count;

					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							pC->key = 0;
							break;
						}
						pC->address = (pC->end - pC->loop) & 0xffff;
					}

					pC->lastdt = pST[pC->address];
				}

				*bufL++ += (pC->lastdt * lvol) >> 6;
				*bufR++ += (pC->lastdt * rvol) >> 6;
				pC->offset += pC->pitch;
			}
		}
	}
}


/*--------------------------------------------------------
	サウンドストリーム生成 (リサンプリング処理)
--------------------------------------------------------*/

static void qsound_update_stream_resample(void)
{
	int ch;

	for (ch = 0; ch < QSOUND_CHANNELS; ch++)
	{
		QSOUND_CHANNEL *pC = &qsound_channel[ch];

		if (pC->key)
		{
			int i, j;
			QSOUND_SRC_SAMPLE *pST  = qsound_sample_rom + pC->bank;
			QSOUND_SAMPLE_MIX *bufL = qsound_buffer[0];
			QSOUND_SAMPLE_MIX *bufR = qsound_buffer[1];
			QSOUND_SAMPLE_MIX rvol  = (pC->rvol * pC->vol) >> qsound_volume_shift;
			QSOUND_SAMPLE_MIX lvol  = (pC->lvol * pC->vol) >> qsound_volume_shift;

			for (i = 0; i < qsound_sample_length; i++)
			{
				for (j = 0; j < qsound_resample_count; j++)
				{
					int count = (pC->offset) >> 16;

					pC->offset &= 0xffff;

					if (count)
					{
						pC->address += count;

						if (pC->address >= pC->end)
						{
							if (!pC->loop)
							{
								pC->key = 0;
								break;
							}
							pC->address = (pC->end - pC->loop) & 0xffff;
						}

						if (pC->lastdt)
							pC->lastdt = (pC->lastdt + pST[pC->address]) >> 1;
						else
							pC->lastdt = pST[pC->address];
					}

					pC->offset += pC->pitch;
				}

				*bufL++ += (pC->lastdt * lvol) >> 6;
				*bufR++ += (pC->lastdt * rvol) >> 6;

				if (!pC->key) break;
			}
		}
	}
}


/******************************************************************************
	Qsound インタフェース関数
******************************************************************************/

/*--------------------------------------------------------
	QSound インタフェース初期化
--------------------------------------------------------*/

void qsound_sh_start(void)
{
	sound->stack    = 0x1000;
	sound->stereo   = 1;
	sound->callback = qsound_update;

	qsound_sample_rom   = (QSOUND_SRC_SAMPLE *)memory_region_sound1;
	qsound_stream_type  = 0;
	qsound_volume_shift = 6;

#if (EMU_SYSTEM == CPS1)
	if (!strncmp(driver->name, "wof", 3)
	||	!strncmp(driver->name, "slammas", 7)
	||	!strncmp(driver->name, "mbomb", 5))
	{
		qsound_stream_type = 1;
	}
	if (!strncmp(driver->name, "punish", 6))
	{
		qsound_volume_shift = 4;
	}
#else
	if (!strcmp(driver->name, "dstlk")
	||	!strcmp(driver->name, "nwarr")
	||	!strcmp(driver->name, "sfa2")
	||	!strcmp(driver->name, "vsav")
	||	!strcmp(driver->name, "vhunt2")
	||	!strcmp(driver->name, "vsav2"))
	{
		qsound_stream_type = 1;
	}

	if (!strcmp(driver->name, "csclub"))
	{
		qsound_volume_shift = 4;
	}
	else
	if (!strcmp(driver->name, "ddsom")
	||	!strcmp(driver->name, "vsav")
	||	!strcmp(driver->name, "vsav2"))
	{
		qsound_volume_shift = 5;
	}
	else
	if (!strcmp(driver->name, "batcir")
	||	!strcmp(driver->name, "spf2t")
	||	!strcmp(driver->name, "gigawing")
	||	!strcmp(driver->name, "mpangj")
	||	!strcmp(driver->name, "pzloop2j"))
	{
		qsound_volume_shift = 7;
	}
#endif

	qsound_set_samplerate();
}


/*--------------------------------------------------------
	QSound インタフェース終了
--------------------------------------------------------*/

void qsound_sh_stop(void)
{
}


/*--------------------------------------------------------
	QSound インタフェースリセット
--------------------------------------------------------*/

void qsound_sh_reset(void)
{
	memset(&qsound_channel, 0, sizeof(qsound_channel));
	memset(qsound_buffer, 0, sizeof(qsound_buffer));
	qsound_data = 0;
}


/*--------------------------------------------------------
	QSound サンプルレート設定
--------------------------------------------------------*/

void qsound_set_samplerate(void)
{
	int samplerate = 44100;

	qsound_samplerate = option_samplerate;

	if (!qsound_stream_type || qsound_samplerate == 2)
	{
		samplerate >>= (2 - option_samplerate);
		qsound_update_stream = qsound_update_stream_normal;
	}
	else
	{
		qsound_resample_count = 1 << (2 - qsound_samplerate);
		qsound_update_stream = qsound_update_stream_resample;
	}

	qsound_sample_length = SOUND_SAMPLES >> (2 - qsound_samplerate);

	qsound_frq_ratio = ((float)QSOUND_CLOCK / (float)QSOUND_CLOCKDIV) / (float)samplerate;
	qsound_frq_ratio *= 16.0;
}


/******************************************************************************
	メモリハンドラ
******************************************************************************/

/*--------------------------------------------------------
	サウンドステータスリード
--------------------------------------------------------*/

READ8_HANDLER( qsound_status_r )
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}


/*--------------------------------------------------------
	データライト (high)
--------------------------------------------------------*/

WRITE8_HANDLER( qsound_data_h_w )
{
	qsound_data = (qsound_data & 0xff) | (data << 8);
}


/*--------------------------------------------------------
	データライト (low)
--------------------------------------------------------*/

WRITE8_HANDLER( qsound_data_l_w )
{
	qsound_data = (qsound_data & 0xff00) | data;
}


/*--------------------------------------------------------
	サウンドコマンドライト
--------------------------------------------------------*/

WRITE8_HANDLER( qsound_cmd_w )
{
	int ch, reg;

	if (data < 0x80)
	{
		ch = data >> 3;
		reg = data & 0x07;
	}
	else if (data < 0x90)
	{
		ch = data - 0x80;
		reg = 8;
	}
	else
	{
		/* Unknown registers */
		return;
	}

	switch (reg)
	{
	case 0: /* Bank */
		ch = (ch + 1) & 0x0f;	/* strange ... */
		qsound_channel[ch].bank = (qsound_data & 0x7f) << 16;
		break;

	case 1: /* start */
		qsound_channel[ch].address = qsound_data;
		break;

	case 2: /* pitch */
		qsound_channel[ch].pitch = (long)((float)qsound_data * qsound_frq_ratio);
		if (!qsound_data)
		{
			/* Key off */
			qsound_channel[ch].key = 0;
		}
		break;

	case 4: /* loop offset */
		qsound_channel[ch].loop = qsound_data;
		break;

	case 5: /* end */
		qsound_channel[ch].end = qsound_data;
		break;

	case 6: /* master volume */
		if (!qsound_data)
		{
			/* Key off */
			qsound_channel[ch].key = 0;
		}
		else if (!qsound_channel[ch].key)
		{
			/* Key on */
			qsound_channel[ch].key = 1;
			qsound_channel[ch].offset = 0;
			qsound_channel[ch].lastdt = 0;
		}
		qsound_channel[ch].vol = qsound_data;
		break;

	case 8: /* pan and L/R volume */
		qsound_channel[ch].pan = qsound_data;
		qsound_data = (qsound_data - 0x10) & 0x3f;
		if (qsound_data > 32) qsound_data = 32;
		qsound_channel[ch].rvol = qsound_pan_table[qsound_data];
		qsound_channel[ch].lvol = qsound_pan_table[32 - qsound_data];
		break;
	}
}


/******************************************************************************
	ストリーム更新コールバック関数
******************************************************************************/

void qsound_update(int p)
{
	int i;
	s16 *buffer = (s16 *)p;
	QSOUND_SAMPLE_MIX lt, rt;

	memset(qsound_buffer, 0, sizeof(qsound_buffer));

	(*qsound_update_stream)();

	switch (qsound_samplerate)
	{
	case 0:
		for (i = 0; i < SOUND_SAMPLES >> 2; i++)
		{
			lt = qsound_buffer[0][i];
			rt = qsound_buffer[1][i];

			Limit(lt, MAXOUT, MINOUT);
			Limit(rt, MAXOUT, MINOUT);

			*buffer++ = lt;
			*buffer++ = rt;
			*buffer++ = lt;
			*buffer++ = rt;
			*buffer++ = lt;
			*buffer++ = rt;
			*buffer++ = lt;
			*buffer++ = rt;
		}
		break;

	case 1:
		for (i = 0; i < SOUND_SAMPLES >> 1; i++)
		{
			lt = qsound_buffer[0][i];
			rt = qsound_buffer[1][i];

			Limit(lt, MAXOUT, MINOUT);
			Limit(rt, MAXOUT, MINOUT);

			*buffer++ = lt;
			*buffer++ = rt;
			*buffer++ = lt;
			*buffer++ = rt;
		}
		break;

	case 2:
		for (i = 0; i < SOUND_SAMPLES; i++)
		{
			lt = qsound_buffer[0][i];
			rt = qsound_buffer[1][i];

			Limit(lt, MAXOUT, MINOUT);
			Limit(rt, MAXOUT, MINOUT);

			*buffer++ = lt;
			*buffer++ = rt;
		}
		break;
	}
}


/******************************************************************************
	セーブ/ロード ステート
******************************************************************************/

#ifdef SAVE_STATE

STATE_SAVE( qsound )
{
	int i;

	for (i = 0; i < QSOUND_CHANNELS; i++)
	{
		state_save_long(&qsound_channel[i].bank, 1);
		state_save_long(&qsound_channel[i].address, 1);
		state_save_long(&qsound_channel[i].pitch, 1);
		state_save_long(&qsound_channel[i].loop, 1);
		state_save_long(&qsound_channel[i].end, 1);
		state_save_long(&qsound_channel[i].vol, 1);
		state_save_long(&qsound_channel[i].pan, 1);
		state_save_long(&qsound_channel[i].key, 1);
		state_save_long(&qsound_channel[i].lvol, 1);
		state_save_long(&qsound_channel[i].rvol, 1);
		state_save_long(&qsound_channel[i].lastdt, 1);
		state_save_long(&qsound_channel[i].offset, 1);
	}
	state_save_long(&qsound_data, 1);
}

STATE_LOAD( qsound )
{
	int i;

	for (i = 0; i < QSOUND_CHANNELS; i++)
	{
		state_load_long(&qsound_channel[i].bank, 1);
		state_load_long(&qsound_channel[i].address, 1);
		state_load_long(&qsound_channel[i].pitch, 1);
		state_load_long(&qsound_channel[i].loop, 1);
		state_load_long(&qsound_channel[i].end, 1);
		state_load_long(&qsound_channel[i].vol, 1);
		state_load_long(&qsound_channel[i].pan, 1);
		state_load_long(&qsound_channel[i].key, 1);
		state_load_long(&qsound_channel[i].lvol, 1);
		state_load_long(&qsound_channel[i].rvol, 1);
		state_load_long(&qsound_channel[i].lastdt, 1);
		state_load_long(&qsound_channel[i].offset, 1);
	}
	state_load_long(&qsound_data, 1);
}

#endif /* SAVE_STATE */
