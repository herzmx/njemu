/******************************************************************************

	dipsw.c

	MVS DIPスイッチ設定

******************************************************************************/

#include "mvs.h"

#define MENU_BLANK					{ "\n", 0, 0x00, 0, 0, NULL }
#define MENU_RETURN					{ "Return to main menu", 1, 0x00, 0, 0, NULL }
#define MENU_END					{ "\0", 0, 0x00, 0, 0, NULL }


/*--------------------------------------
  標準
--------------------------------------*/

static dipswitch_t dipswitch_default[] =
{
	{ "Test Switch",              1, 0x01, 0, 1, { "Off","On" } },
	{ "Coin Chutes",              1, 0x02, 0, 1, { "2?", "1?" } },
	{ "Autofire (in some games)", 1, 0x04, 0, 1, { "Off","On" } },
	{ "COMM Settings",            1, 0x38, 0, 4, { "Off","1","2","3","4" } },
	{ "Free Play",                1, 0x40, 0, 1, { "Off","On" } },
	{ "Freeze",                   1, 0x80, 0, 1, { "Off","On" } },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

/*--------------------------------------
  麻雀
--------------------------------------*/

static dipswitch_t dipswitch_mjneogeo[] =
{
	{ "Test Switch",              1, 0x01, 0, 1, { "Off","On" } },
	{ "Coin Chutes",              1, 0x02, 0, 1, { "2?", "1?" } },
	{ "Mahjong Control Panel",    0, 0x04, 0, 1, { "Off","On" } },
	{ "COMM Settings",            1, 0x38, 0, 4, { "Off","1","2","3","4" } },
	{ "Free Play",                1, 0x40, 0, 1, { "Off","On" } },
	{ "Freeze",                   1, 0x80, 0, 1, { "Off","On" } },
	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};


dipswitch_t *load_dipswitch(void)
{
	u8 value = ~neogeo_dipswitch;
	dipswitch_t *dipswitch = NULL;

	switch (neogeo_ngh)
	{
	case NGH_mahretsu:
	case NGH_janshin:
	case NGH_minasan:
	case NGH_bakatono:
		dipswitch = dipswitch_mjneogeo;
		break;

	default:
		dipswitch = dipswitch_default;
		break;
	}

	dipswitch[0].value = (value & 0x01) != 0;
	dipswitch[1].value = (value & 0x02) != 0;
	dipswitch[2].value = (value & 0x04) != 0;
	dipswitch[4].value = (value & 0x40) != 0;
	dipswitch[5].value = (value & 0x80) != 0;

	switch (neogeo_dipswitch & 0x38)
	{
	case 0x00: dipswitch[3].value = 4; break;
	case 0x10: dipswitch[3].value = 3; break;
	case 0x20: dipswitch[3].value = 2; break;
	case 0x30: dipswitch[3].value = 1; break;
	case 0x38: dipswitch[3].value = 0; break;
	}

	return dipswitch;
}


void save_dipswitch(void)
{
	u8 value;
	dipswitch_t *dipswitch = NULL;

	switch (neogeo_ngh)
	{
	case NGH_mahretsu:
	case NGH_janshin:
	case NGH_minasan:
	case NGH_bakatono:
		dipswitch = dipswitch_mjneogeo;
		break;

	default:
		dipswitch = dipswitch_default;
		break;
	}

	value = 0;
	value |= (dipswitch[0].value != 0) ? 0x00: 0x01;
	value |= (dipswitch[1].value != 0) ? 0x00: 0x02;
	value |= (dipswitch[2].value != 0) ? 0x00: 0x04;
	value |= (dipswitch[4].value != 0) ? 0x00: 0x40;
	value |= (dipswitch[5].value != 0) ? 0x00: 0x80;
	switch (dipswitch[3].value)
	{
	case 0: value |= 0x38; break;
	case 1: value |= 0x30; break;
	case 2: value |= 0x20; break;
	case 3: value |= 0x10; break;
	case 4: value |= 0x00; break;
	}

	neogeo_dipswitch = value;
}
