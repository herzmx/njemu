/******************************************************************************

	dipsw.c

	CPS1 DIP�X�C�b�`�ݒ�

******************************************************************************/

#include "cps1.h"

#define MENU_BLANK					{ "\n", 0, 0x00, 0, 0, { NULL } }
#define MENU_RETURN					{ "Return to main menu", 1, 0x00, 0, 0, { NULL } }
#define MENU_END					{ "\0", 0, 0x00, 0, 0, { NULL } }

#define ACTIVE_HIGH					0
#define ACTIVE_LOW					1

#define load_free_play				(dip++)->value = dip_load_bit(DIP_C, 2, 1);
#define load_freeze					(dip++)->value = dip_load_bit(DIP_C, 3, 1);
#define load_flip_screen			(dip++)->value = dip_load_bit(DIP_C, 4, 1);
#define load_demo_sounds			(dip++)->value = dip_load_bit(DIP_C, 5, 1);
#define load_allow_continue			(dip++)->value = dip_load_bit(DIP_C, 6, 1);
#define load_game_mode				(dip++)->value = dip_load_bit(DIP_C, 7, 1);

#define save_free_play				dip_save_bit(DIP_C, (dip++)->value, 2, 1);
#define save_freeze					dip_save_bit(DIP_C, (dip++)->value, 3, 1);
#define save_flip_screen			dip_save_bit(DIP_C, (dip++)->value, 4, 1);
#define save_demo_sounds			dip_save_bit(DIP_C, (dip++)->value, 5, 1);
#define save_allow_continue			dip_save_bit(DIP_C, (dip++)->value, 6, 1);
#define save_game_mode				dip_save_bit(DIP_C, (dip++)->value, 7, 1);

#define load_demo_sounds2			(dip++)->value = dip_load_bit(DIP_C, 5, 0);
#define load_allow_continue2		(dip++)->value = dip_load_bit(DIP_C, 6, 0);

#define save_demo_sounds2			dip_save_bit(DIP_C, (dip++)->value, 5, 0);
#define save_allow_continue2		dip_save_bit(DIP_C, (dip++)->value, 6, 0);


/*--------------------------------------
  ���� (bit)
--------------------------------------*/

#define dip_on_off		{ "Off","On"  }
#define dip_yes_no		{ "No","Yes" }
#define dip_game_mode	{ "Game","Test" }

static int dip_load_bit(int sw, int shift, int invert)
{
	int value = 0;

	if (invert)
		value = (cps1_dipswitch[sw] & (1 << shift)) ? 0 : 1;
	else
		value = (cps1_dipswitch[sw] & (1 << shift)) ? 1 : 0;

	return value;
}

static void dip_save_bit(int sw, int value, int shift, int invert)
{
	if (invert) value ^= 1;
	cps1_dipswitch[sw] &= ~(1 << shift);
	cps1_dipswitch[sw] |= value << shift;
}


/*--------------------------------------
  ���� (�R�C�� type1)
--------------------------------------*/

#define dip_coin1			\
{							\
	"4 Coins/1 Credit",		\
	"3 Coins/1 Credit",		\
	"2 Coins/1 Credit",		\
	"1 Coins/1 Credit",		\
	"1 Coins/2 Credits",	\
	"1 Coins/3 Credits",	\
	"1 Coins/4 Credits",	\
	"1 Coins/6 Credits"		\
}

static int dip_load_coin1a(void)
{
	switch (cps1_dipswitch[DIP_A] & 0x07)
	{
	case 0x00: return 0;
	case 0x01: return 1;
	case 0x02: return 2;
	case 0x03: return 7;
	case 0x04: return 6;
	case 0x05: return 5;
	case 0x06: return 4;
	case 0x07: return 3;
	}
	return 0;
}

static void dip_save_coin1a(int value)
{
	cps1_dipswitch[DIP_A] &= ~0x07;

	switch (value)
	{
	case 0: cps1_dipswitch[DIP_A] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_A] |= 0x01; break;
	case 2: cps1_dipswitch[DIP_A] |= 0x02; break;
	case 3: cps1_dipswitch[DIP_A] |= 0x07; break;
	case 4: cps1_dipswitch[DIP_A] |= 0x06; break;
	case 5: cps1_dipswitch[DIP_A] |= 0x05; break;
	case 6: cps1_dipswitch[DIP_A] |= 0x04; break;
	case 7: cps1_dipswitch[DIP_A] |= 0x03; break;
	}
}

static int dip_load_coin1b(void)
{
	switch (cps1_dipswitch[DIP_A] & 0x38)
	{
	case 0x00: return 0;
	case 0x08: return 1;
	case 0x10: return 2;
	case 0x18: return 7;
	case 0x20: return 6;
	case 0x28: return 5;
	case 0x30: return 4;
	case 0x38: return 3;
	}
	return 0;
}

static void dip_save_coin1b(int value)
{
	cps1_dipswitch[DIP_A] &= ~0x38;

	switch (value)
	{
	case 0: cps1_dipswitch[DIP_A] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_A] |= 0x08; break;
	case 2: cps1_dipswitch[DIP_A] |= 0x10; break;
	case 3: cps1_dipswitch[DIP_A] |= 0x38; break;
	case 4: cps1_dipswitch[DIP_A] |= 0x30; break;
	case 5: cps1_dipswitch[DIP_A] |= 0x28; break;
	case 6: cps1_dipswitch[DIP_A] |= 0x20; break;
	case 7: cps1_dipswitch[DIP_A] |= 0x18; break;
	}
}

/*--------------------------------------
  ���� (�R�C�� type2)
--------------------------------------*/

#define dip_coin2							\
{											\
	"4 Coins/1 Credit",						\
	"3 Coins/1 Credit",						\
	"2 Coins/1 Credit",						\
	"2 Coins/1 Credit (1 to continue)",		\
	"1 Coins/1 Credit",						\
	"1 Coins/2 Credits",					\
	"1 Coins/3 Credits",					\
	"1 Coins/4 Credits"						\
}

static int dip_load_coin2a(void)
{
	switch (cps1_dipswitch[DIP_A] & 0x07)
	{
	case 0x00: return 3;
	case 0x01: return 0;
	case 0x02: return 1;
	case 0x03: return 2;
	case 0x04: return 7;
	case 0x05: return 6;
	case 0x06: return 5;
	case 0x07: return 4;
	}
	return 0;
}

static void dip_save_coin2a(int value)
{
	cps1_dipswitch[DIP_A] &= ~0x07;

	switch (value)
	{
	case 0: cps1_dipswitch[DIP_A] |= 0x01; break;
	case 1: cps1_dipswitch[DIP_A] |= 0x02; break;
	case 2: cps1_dipswitch[DIP_A] |= 0x03; break;
	case 3: cps1_dipswitch[DIP_A] |= 0x00; break;
	case 4: cps1_dipswitch[DIP_A] |= 0x07; break;
	case 5: cps1_dipswitch[DIP_A] |= 0x06; break;
	case 6: cps1_dipswitch[DIP_A] |= 0x05; break;
	case 7: cps1_dipswitch[DIP_A] |= 0x04; break;
	}
}

static int dip_load_coin2b(void)
{
	switch (cps1_dipswitch[DIP_A] & 0x38)
	{
	case 0x00: return 3;
	case 0x08: return 0;
	case 0x10: return 1;
	case 0x18: return 2;
	case 0x20: return 7;
	case 0x28: return 6;
	case 0x30: return 5;
	case 0x38: return 4;
	}
	return 0;
}

static void dip_save_coin2b(int value)
{
	cps1_dipswitch[DIP_A] &= ~0x38;

	switch (value)
	{
	case 0: cps1_dipswitch[DIP_A] |= 0x08; break;
	case 1: cps1_dipswitch[DIP_A] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_A] |= 0x18; break;
	case 3: cps1_dipswitch[DIP_A] |= 0x00; break;
	case 4: cps1_dipswitch[DIP_A] |= 0x38; break;
	case 5: cps1_dipswitch[DIP_A] |= 0x30; break;
	case 6: cps1_dipswitch[DIP_A] |= 0x28; break;
	case 7: cps1_dipswitch[DIP_A] |= 0x20; break;
	}
}

/*--------------------------------------
  ���� (➑�)
--------------------------------------*/

#define dip_cabinet				\
{								\
	"Upright 1 Player",			\
	"Upright 2 Players",		\
	"Cocktail"					\
}

static int dip_load_cabinet(void)
{
	switch (cps1_dipswitch[DIP_A] & 0xc0)
	{
	case 0x00: return 2;
	case 0x80: return 1;
	case 0xc0: return 0;
	}
	return 0;
}

static void dip_save_cabinet(int value)
{
	cps1_dipswitch[DIP_A] &= ~0xc0;

	switch (value)
	{
	case 0: cps1_dipswitch[DIP_A] |= 0xc0; break;
	case 1: cps1_dipswitch[DIP_A] |= 0x80; break;
	case 2: cps1_dipswitch[DIP_A] |= 0x00; break;
	}
}

/*--------------------------------------
  ���� (��Փx)
--------------------------------------*/

#define dip_difficulty		\
{							\
	"1 (Easiest)",			\
	"2",					\
	"3",					\
	"4 (Normal)",			\
	"5",					\
	"6",					\
	"7",					\
	"8 (Hardest)"			\
}

static int dip_load_difficulty1(void)
{
	switch (cps1_dipswitch[DIP_B] & 0x07)
	{
	case 0x00: return 7;
	case 0x01: return 6;
	case 0x02: return 5;
	case 0x03: return 4;
	case 0x04: return 3;
	case 0x05: return 2;
	case 0x06: return 1;
	case 0x07: return 0;
	}
	return 0;
}

static void dip_save_difficulty1(int value)
{
	switch (value)
	{
	case 0: cps1_dipswitch[DIP_B] |= 0x07; break;
	case 1: cps1_dipswitch[DIP_B] |= 0x06; break;
	case 2: cps1_dipswitch[DIP_B] |= 0x05; break;
	case 3: cps1_dipswitch[DIP_B] |= 0x04; break;
	case 4: cps1_dipswitch[DIP_B] |= 0x03; break;
	case 5: cps1_dipswitch[DIP_B] |= 0x02; break;
	case 6: cps1_dipswitch[DIP_B] |= 0x01; break;
	case 7: cps1_dipswitch[DIP_B] |= 0x00; break;
	}
}

static int dip_load_difficulty2(void)
{
	switch (cps1_dipswitch[DIP_B] & 0x07)
	{
	case 0x00: return 7;
	case 0x01: return 6;
	case 0x02: return 5;
	case 0x03: return 4;
	case 0x04: return 0;
	case 0x05: return 1;
	case 0x06: return 2;
	case 0x07: return 3;
	}
	return 0;
}

static void dip_save_difficulty2(int value)
{
	cps1_dipswitch[DIP_B] &= ~0x07;

	switch (value)
	{
	case 0: cps1_dipswitch[DIP_B] |= 0x04; break;
	case 1: cps1_dipswitch[DIP_B] |= 0x05; break;
	case 2: cps1_dipswitch[DIP_B] |= 0x06; break;
	case 3: cps1_dipswitch[DIP_B] |= 0x07; break;
	case 4: cps1_dipswitch[DIP_B] |= 0x03; break;
	case 5: cps1_dipswitch[DIP_B] |= 0x02; break;
	case 6: cps1_dipswitch[DIP_B] |= 0x01; break;
	case 7: cps1_dipswitch[DIP_B] |= 0x00; break;
	}
}

/*--------------------------------------
  forgottn
--------------------------------------*/

static dipswitch_t dipswitch_forgottn[] =
{
	// DIP A
	{ "Coin A",      1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",      1, 0x38, 0, 7, dip_coin1      },
	{ "Demo Sounds", 1, 0x40, 0, 1, dip_on_off     },
	{ "Flip Screen", 1, 0x80, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",  1, 0x07, 0, 7, dip_difficulty },
	{ "Service Mode",1, 0x40, 0, 1, dip_on_off     },
	{ "Freeze",      1, 0x80, 0, 1, dip_on_off     },

	// DIP C

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_forgottn(void)
{
	dipswitch_t *dip = dipswitch_forgottn;

	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 0);
	(dip++)->value = dip_load_bit(DIP_A, 7, 1);

	(dip++)->value = dip_load_difficulty1();
	(dip++)->value = dip_load_bit(DIP_B, 6, 1);
	(dip++)->value = dip_load_bit(DIP_B, 7, 0);
}

static void dip_save_forgottn(void)
{
	dipswitch_t *dip = dipswitch_forgottn;

	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 0);
	dip_save_bit(DIP_A, (dip++)->value, 7, 1);

	dip_save_difficulty1((dip++)->value);
	dip_save_bit(DIP_B, (dip++)->value, 6, 1);
	dip_save_bit(DIP_B, (dip++)->value, 7, 0);
}

/*--------------------------------------
  ghouls / ghoulsu / daimakai
--------------------------------------*/

static dipswitch_t dipswitch_ghouls[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin1      },
	{ "Cabinet",        1, 0xc0, 0, 2, dip_cabinet    },

	// DIP B
	{ "Difficulty",     1, 0x07, 0, 7, dip_difficulty },
	{ "Bonus Life",     1, 0x30, 0, 3, {"10K, 30K and every 30K","20K, 50K and every 70K","30K, 60K and every 70K","40K, 70K and every 80K"} },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"3","4","5","6"} },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_ghoulsu[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin1      },
	{ "Cabinet",        1, 0xc0, 0, 2, dip_cabinet    },

	// DIP B
	{ "Difficulty",     1, 0x07, 0, 7, dip_difficulty },
	{ "Bonus Life",     1, 0x30, 0, 3, {"10K, 30K and every 30K","20K, 50K and every 70K","30K, 60K and every 70K","40K, 70K and every 80K"} },
	{ "Freeze",         1, 0x80, 0, 1, dip_on_off     },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"2","3","4","5"} },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_daimakai[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin1      },
	{ "Cabinet",        1, 0xc0, 0, 2, dip_cabinet    },

	// DIP B
	{ "Difficulty",     1, 0x07, 0, 7, dip_difficulty },
	{ "Bonus Life",     1, 0x30, 0, 3, {"10K, 30K and every 30K","20K, 50K and every 70K","30K, 60K and every 70K","40K, 70K and every 80K"} },
	{ "Freeze",         1, 0x80, 0, 1, dip_on_off     },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"3","4","5","6"} },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_ghouls(int type)
{
	dipswitch_t *dip;

	switch (type)
	{
	case 1:  dip = dipswitch_ghoulsu; break;
	case 2:  dip = dipswitch_daimakai; break;
	default: dip = dipswitch_ghouls; break;
	}

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_cabinet();

	// DIP B
	(dip++)->value = dip_load_difficulty2();
	switch (cps1_dipswitch[DIP_B] & 0x30)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x20: (dip++)->value = 0; break;
	case 0x30: (dip++)->value = 2; break;
	}
	if (type != 0) (dip++)->value = dip_load_bit(DIP_B, 7, 1);

	// DIP C
	if (type == 1)
	{
		switch (cps1_dipswitch[DIP_C] & 0x03)
		{
		case 0x00: (dip++)->value = 0; break;
		case 0x01: (dip++)->value = 3; break;
		case 0x02: (dip++)->value = 2; break;
		case 0x03: (dip++)->value = 1; break;
		}
	}
	else
	{
		switch (cps1_dipswitch[DIP_C] & 0x03)
		{
		case 0x00: (dip++)->value = 3; break;
		case 0x01: (dip++)->value = 2; break;
		case 0x02: (dip++)->value = 1; break;
		case 0x03: (dip++)->value = 0; break;
		}
	}
	load_flip_screen;
	load_demo_sounds2;
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_ghouls(int type)
{
	dipswitch_t *dip;

	switch (type)
	{
	case 1:  dip = dipswitch_ghoulsu; break;
	case 2:  dip = dipswitch_daimakai; break;
	default: dip = dipswitch_ghouls; break;
	}

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_cabinet((dip++)->value);

	// DIP B
	dip_save_difficulty2((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x30; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	if (type != 0) dip_save_bit(DIP_B, (dip++)->value, 7, 1);

	// DIP C
	if (type == 1)
	{
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
		case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
		case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
		case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
		}
	}
	else
	{
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
		case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
		case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
		case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
		}
	}
	save_flip_screen;
	save_demo_sounds2;
	save_allow_continue2;
	save_game_mode;
}

/*--------------------------------------
  srtider / stridrua
--------------------------------------*/

static dipswitch_t dipswitch_strider[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin1      },
	{ "Cabinet",        1, 0xc0, 0, 2, dip_cabinet    },

	// DIP B
	{ "Difficulty",                  1, 0x07, 0, 7, dip_difficulty },
	{ "Bonus Life",                  1, 0x30, 0, 3, {"20K, 40K then every 60K","30K, 50K then every 70K","20K & 60K only","30K & 60K only"} },
	{ "Internal Diff. on Life Loss", 1, 0xc0, 0, 2, {"-3","-1","Default"} },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"2","3","4","5"} },
	{ "Freeze",         1, 0x04, 0, 1, dip_on_off     },
	{ "Free Play",      1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_stridrua[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin1      },
	{ "Cabinet",        1, 0xc0, 0, 2, dip_cabinet    },

	// DIP B
	{ "Difficulty",                      1, 0x07, 0, 7, dip_difficulty },
	{ "2 Coins to Start, 1 to Continue", 1, 0x08, 0, 1, dip_on_off     },
	{ "Bonus Life",                      1, 0x30, 0, 3, {"20K, 40K then every 60K","30K, 50K then every 70K","20K & 60K only","30K & 60K only"} },
	{ "Internal Diff. on Life Loss",     1, 0xc0, 0, 2, {"-3","-1","Default"} },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"2","3","4","5"} },
	{ "Freeze",         1, 0x04, 0, 1, dip_on_off     },
	{ "Free Play",      1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_strider(int type)
{
	dipswitch_t *dip;

	if (type)
		dip = dipswitch_stridrua;
	else
		dip = dipswitch_strider;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_cabinet();

	// DIP B
	(dip++)->value = dip_load_difficulty2();
	if (type == 1) (dip++)->value = dip_load_bit(DIP_B, 3, 1);
	switch (cps1_dipswitch[DIP_B] & 0x30)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x10: (dip++)->value = 2; break;
	case 0x20: (dip++)->value = 1; break;
	case 0x30: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0xc0)
	{
	case 0x00: (dip++)->value = 1; break;
	case 0x80: (dip++)->value = 2; break;
	case 0xc0: (dip++)->value = 0; break;
	}

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x01: (dip++)->value = 3; break;
	case 0x02: (dip++)->value = 2; break;
	case 0x03: (dip++)->value = 1; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds2;
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_strider(int type)
{
	dipswitch_t *dip;

	if (type)
		dip = dipswitch_stridrua;
	else
		dip = dipswitch_strider;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_cabinet((dip++)->value);

	// DIP B
	dip_save_difficulty2((dip++)->value);
	if (type == 1) dip_save_bit(DIP_B, (dip++)->value, 3, 1);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x30; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x00; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x80; break;
	}

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds2;
	save_allow_continue2;
	save_game_mode;
}

/*--------------------------------------
  dynwar
--------------------------------------*/

static dipswitch_t dipswitch_dynwar[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin2      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin2      },
	{ "Free Play",      1, 0x80, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",     1, 0x07, 0, 7, dip_difficulty },

	// DIP C
	{ "Freeze",         1, 0x01, 0, 1, dip_on_off     },
	{ "Turbo Mode",     1, 0x02, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_dynwar(void)
{
	dipswitch_t *dip = dipswitch_dynwar;

	// DIP A
	(dip++)->value = dip_load_coin2a();
	(dip++)->value = dip_load_coin2b();
	(dip++)->value = dip_load_bit(DIP_A, 7, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty2();

	// DIP C
	(dip++)->value = dip_load_bit(DIP_C, 0, 1);
	(dip++)->value = dip_load_bit(DIP_C, 1, 1);
	load_flip_screen;
	load_demo_sounds2;
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_dynwar(void)
{
	dipswitch_t *dip = dipswitch_dynwar;

	// DIP A
	dip_save_coin2a((dip++)->value);
	dip_save_coin2b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 7, 1);

	// DIP B
	dip_save_difficulty2((dip++)->value);

	// DIP C
	dip_save_bit(DIP_C, (dip++)->value, 0, 1);
	dip_save_bit(DIP_C, (dip++)->value, 1, 1);
	save_flip_screen;
	save_demo_sounds2;
	save_allow_continue2;
	save_game_mode;
}


/*--------------------------------------
  willow
--------------------------------------*/

static dipswitch_t dipswitch_willow[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin2      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin2      },
	{ "Cabinet",        1, 0xc0, 0, 3, dip_cabinet    },

	// DIP B
	{ "Difficulty",           1, 0x07, 0, 7, dip_difficulty },
	{ "Nando Speed",          1, 0x18, 0, 3, {"Slow","Normal","Fast","Very Fast"}},
	{ "Stage Magic Continue", 1, 0x80, 0, 1, dip_on_off      },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"1","2","3","4"} },
	{ "Vitality",       1, 0x0c, 0, 3, {"2","3","4","5"} },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_willow(void)
{
	dipswitch_t *dip = dipswitch_willow;

	// DIP A
	(dip++)->value = dip_load_coin2a();
	(dip++)->value = dip_load_coin2b();
	(dip++)->value = dip_load_cabinet();

	// DIP B
	(dip++)->value = dip_load_difficulty2();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 0; break;
	case 0x18: (dip++)->value = 1; break;
	}
	(dip++)->value = dip_load_bit(DIP_B, 7, 1);

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x01: (dip++)->value = 0; break;
	case 0x02: (dip++)->value = 2; break;
	case 0x03: (dip++)->value = 1; break;
	}
	switch (cps1_dipswitch[DIP_C] & 0x0c)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x04: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x0c: (dip++)->value = 1; break;
	}
	load_flip_screen;
	load_demo_sounds2;
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_willow(void)
{
	dipswitch_t *dip = dipswitch_willow;

	// DIP A
	dip_save_coin2a((dip++)->value);
	dip_save_coin2b((dip++)->value);
	dip_save_cabinet((dip++)->value);

	// DIP B
	dip_save_difficulty2((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	dip_save_bit(DIP_B, (dip++)->value, 7, 1);

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x0c; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x04; break;
	}
	save_flip_screen;
	save_demo_sounds2;
	save_allow_continue2;
	save_game_mode;
}

/*--------------------------------------
  unsquad
--------------------------------------*/

static dipswitch_t dipswitch_unsquad[] =
{
	// DIP A
	{ "Coin A",         1, 0x07, 0, 7, dip_coin2      },
	{ "Coin B",         1, 0x38, 0, 7, dip_coin2      },

	// DIP B
	{ "Difficulty",     1, 0x07, 0, 7, dip_difficulty },
	{ "Damage",         1, 0x18, 0, 3, {"Small","Normal","Big","Biggest"}},

	// DIP C
	{ "Free Play",      1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",         1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_unsquad(void)
{
	dipswitch_t *dip = dipswitch_unsquad;

	// DIP A
	(dip++)->value = dip_load_coin2a();
	(dip++)->value = dip_load_coin2b();

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 0; break;
	case 0x18: (dip++)->value = 1; break;
	}

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_unsquad(void)
{
	dipswitch_t *dip = dipswitch_unsquad;

	// DIP A
	dip_save_coin2a((dip++)->value);
	dip_save_coin2b((dip++)->value);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  ffight
--------------------------------------*/

static dipswitch_t dipswitch_ffight[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty Level 1", 1, 0x07, 0, 7, {"Easiest","Easier","Easy","Normal","Medium","Hard","Harder","Hardest"} },
	{ "Difficulty Level 2", 1, 0x18, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Bonus Life",         1, 0x60, 0, 3, {"100k","200k","100k and every 200k","None"} },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"1","2","3","4"} },
	{ "Free Play",      1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",         1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_ffight(void)
{
	dipswitch_t *dip = dipswitch_ffight;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x18: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0x60)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x20: (dip++)->value = 2; break;
	case 0x40: (dip++)->value = 1; break;
	case 0x60: (dip++)->value = 0; break;
	}

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x01: (dip++)->value = 3; break;
	case 0x02: (dip++)->value = 2; break;
	case 0x03: (dip++)->value = 1; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_ffight(void)
{
	dipswitch_t *dip = dipswitch_ffight;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x60; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x40; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  1941
--------------------------------------*/

static dipswitch_t dipswitch_1941[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Level Up Timer",      1, 0x18, 0, 3, {"More Slowly","Slowly","Quickly","More Quickly"} },
	{ "Bullet's Speed",      1, 0x60, 0, 3, {"Very Slow","Slow","Fast","Very Fast"} },
	{ "Initial Vitality",    1, 0x80, 0, 1, {"3 Bars","4 Bars"} },

	// DIP C
	{ "Throttle Game Speed", 1, 0x01, 0, 1, dip_on_off     },
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_1941(void)
{
	dipswitch_t *dip = dipswitch_1941;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x18: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0x60)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x20: (dip++)->value = 2; break;
	case 0x40: (dip++)->value = 1; break;
	case 0x60: (dip++)->value = 0; break;
	}
	(dip++)->value = dip_load_bit(DIP_B, 7, 1);

	// DIP C
	(dip++)->value = dip_load_bit(DIP_C, 0, 0);
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_1941(void)
{
	dipswitch_t *dip = dipswitch_1941;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x60; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x40; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x60; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	dip_save_bit(DIP_B, (dip++)->value, 7, 1);

	// DIP C
	dip_save_bit(DIP_C, (dip++)->value, 0, 0);
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  mercs
--------------------------------------*/

static dipswitch_t dipswitch_mercs[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Coin Slots",          1, 0x08, 0, 1, {"1","3"} },
	{ "Play Mode",           1, 0x10, 0, 1, {"2 Players","3 Players"} },

	// DIP C
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Service Mode",        1, 0x80, 0, 1, dip_on_off     },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_mercs(void)
{
	dipswitch_t *dip = dipswitch_mercs;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	(dip++)->value = dip_load_bit(DIP_B, 3, 1);
	(dip++)->value = dip_load_bit(DIP_B, 4, 1);

	// DIP C
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_mercs(void)
{
	dipswitch_t *dip = dipswitch_mercs;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	dip_save_bit(DIP_B, (dip++)->value, 3, 1);
	dip_save_bit(DIP_B, (dip++)->value, 4, 1);

	// DIP C
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  mtwins
--------------------------------------*/

static dipswitch_t dipswitch_mtwins[] =
{
	// DIP A
	{ "Coin A",              1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",              1, 0x38, 0, 7, dip_coin1      },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Lives",               1, 0x38, 0, 3, {"1","2","3","4"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_mtwins(void)
{
	dipswitch_t *dip = dipswitch_mtwins;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x38)
	{
	case 0x00: case 0x20: (dip++)->value = 3; break;
	case 0x08: case 0x28: (dip++)->value = 2; break;
	case 0x10: case 0x30: (dip++)->value = 0; break;
	case 0x18: case 0x38: (dip++)->value = 1; break;
	}

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_mtwins(void)
{
	dipswitch_t *dip = dipswitch_mtwins;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  msword
--------------------------------------*/

static dipswitch_t dipswitch_msword[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Player's vitality consumption",        1, 0x07, 0, 7, dip_difficulty },
	{ "Enemy's vitality and attacking power", 1, 0x38, 0, 7, dip_difficulty },
	{ "Stage Select",                         1, 0x40, 0, 1, dip_on_off     },

	// DIP C
	{ "Vitality Packs",      1, 0x03, 0, 1, {"1","2","3 (2 when continue)","4 (3 when continue"} },
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_msword(void)
{
	dipswitch_t *dip = dipswitch_msword;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x38)
	{
	case 0x00: (dip++)->value = 7; break;
	case 0x08: (dip++)->value = 6; break;
	case 0x10: (dip++)->value = 5; break;
	case 0x18: (dip++)->value = 4; break;
	case 0x20: (dip++)->value = 0; break;
	case 0x28: (dip++)->value = 1; break;
	case 0x30: (dip++)->value = 2; break;
	case 0x38: (dip++)->value = 3; break;
	}
	(dip++)->value = dip_load_bit(DIP_B, 6, 1);

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x01: (dip++)->value = 3; break;
	case 0x02: (dip++)->value = 2; break;
	case 0x03: (dip++)->value = 1; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_msword(void)
{
	dipswitch_t *dip = dipswitch_msword;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x28; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x30; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x38; break;
	case 4: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 5: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 6: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 7: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	dip_save_bit(DIP_B, (dip++)->value, 6, 1);

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  cawing
--------------------------------------*/

static dipswitch_t dipswitch_cawing[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty Level (Enemy's Strength)",  1, 0x07, 0, 7, dip_difficulty },
	{ "Difficulty Level (Player's Strength)", 1, 0x18, 0, 3, {"Easy","Normal","Hard","Hardest"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_cawing(void)
{
	dipswitch_t *dip = dipswitch_cawing;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 0; break;
	case 0x18: (dip++)->value = 1; break;
	}

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_cawing(void)
{
	dipswitch_t *dip = dipswitch_cawing;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  nemo
--------------------------------------*/

static dipswitch_t dipswitch_nemo[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Life Bar",            1, 0x18, 0, 2, {"Minimum","Medium","Maximum"} },

	// DIP C
	{ "Lives",               1, 0x03, 0, 3, {"1","2","3","4"} },
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_nemo(void)
{
	dipswitch_t *dip = dipswitch_nemo;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x18: (dip++)->value = 1; break;
	}

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x01: (dip++)->value = 2; break;
	case 0x02: (dip++)->value = 0; break;
	case 0x03: (dip++)->value = 1; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_nemo(void)
{
	dipswitch_t *dip = dipswitch_nemo;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	}

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  sf2 / sf2j
--------------------------------------*/

static dipswitch_t dipswitch_sf2[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_sf2j[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "2 Players Game",      1, 0x08, 0, 1, {"1 Credit/No Continue","2 Credits/Winner Continue"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_sf2(int type)
{
	dipswitch_t *dip;

	dip = (type == 0) ? dipswitch_sf2 : dipswitch_sf2j;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	if (type == 1) (dip++)->value = dip_load_bit(DIP_B, 3, 1);

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_sf2(int type)
{
	dipswitch_t *dip;

	dip = (type == 0) ? dipswitch_sf2 : dipswitch_sf2j;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	if (type == 1) dip_save_bit(DIP_B, (dip++)->value, 3, 1);

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  3wonders
--------------------------------------*/

static dipswitch_t dipswitch_3wonders[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },
	{ "Freeze",                          1, 0x80, 0, 1, dip_on_off     },

	// DIP B
	{ "Lives (Midnight Wanderers)",      1, 0x03, 0, 3, {"1","2","3","5"} },
	{ "Difficulty (Midnight Wanderers)", 1, 0x0c, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Lives (Chariot)",                 1, 0x30, 0, 3, {"1","2","3","5"} },
	{ "Difficulty (Chariot)",            1, 0xc0, 0, 3, {"Easy","Normal","Hard","Hardest"} },

	// DIP C
	{ "Lives (Don't Pull)",      1, 0x03, 0, 3, {"1","2","3","5"} },
	{ "Difficulty (Don't Pull)", 1, 0x08, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Flip Screen",             1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",             1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",          1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",               1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_3wonders(void)
{
	dipswitch_t *dip = dipswitch_3wonders;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);
	(dip++)->value = dip_load_bit(DIP_A, 7, 1);

	// DIP B
	switch (cps1_dipswitch[DIP_B] & 0x03)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x01: (dip++)->value = 2; break;
	case 0x02: (dip++)->value = 1; break;
	case 0x03: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0x0c)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x04: (dip++)->value = 2; break;
	case 0x08: (dip++)->value = 1; break;
	case 0x0c: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0x30)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x10: (dip++)->value = 2; break;
	case 0x20: (dip++)->value = 1; break;
	case 0x30: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0xc0)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x40: (dip++)->value = 2; break;
	case 0x80: (dip++)->value = 1; break;
	case 0xc0: (dip++)->value = 0; break;
	}

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x01: (dip++)->value = 2; break;
	case 0x02: (dip++)->value = 1; break;
	case 0x03: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_C] & 0x0c)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x04: (dip++)->value = 2; break;
	case 0x08: (dip++)->value = 1; break;
	case 0x0c: (dip++)->value = 0; break;
	}
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_3wonders(void)
{
	dipswitch_t *dip = dipswitch_3wonders;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);
	dip_save_bit(DIP_A, (dip++)->value, 7, 1);

	// DIP B
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x03; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x02; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x01; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x0c; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x04; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x30; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x30; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x80; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x40; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x0c; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x08; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x04; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x0c; cps1_dipswitch[DIP_C] |= 0x00; break;
	}
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  kod / kodj
--------------------------------------*/

static dipswitch_t dipswitch_kod[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin Slots",                      1, 0x08, 0, 1, {"1","3"}      },
	{ "Play Mode",                       1, 0x10, 0, 1, {"2Players","3 Players"} },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Lives",               1, 0x38, 0, 7, {"1","2","3","4","5","6","7","8"} },
	{ "Bonus Life",          1, 0xc0, 0, 3, {"80k and every 400k","100k and every 450k","160k and every 450k","None"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_kodj[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "Coin Slots",                      1, 0x08, 0, 1, {"1","3"}      },
	{ "Play Mode",                       1, 0x10, 0, 1, {"2Players","3 Players"} },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Lives",               1, 0x38, 0, 7, {"1","2","3","4","5","6","7","8"} },
	{ "Bonus Life",          1, 0xc0, 0, 3, {"80k and every 400k","200k and every 450k","160k and every 450k","None"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_kod(int type)
{
	dipswitch_t *dip;

	dip = (type == 0) ? dipswitch_kod : dipswitch_kodj;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 3, 1);
	(dip++)->value = dip_load_bit(DIP_A, 4, 1);
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x38)
	{
	case 0x00: (dip++)->value = 7; break;
	case 0x08: (dip++)->value = 6; break;
	case 0x10: (dip++)->value = 5; break;
	case 0x18: (dip++)->value = 4; break;
	case 0x20: (dip++)->value = 3; break;
	case 0x28: (dip++)->value = 2; break;
	case 0x30: (dip++)->value = 0; break;
	case 0x38: (dip++)->value = 1; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0xc0)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x40: (dip++)->value = 2; break;
	case 0x80: (dip++)->value = 0; break;
	case 0xc0: (dip++)->value = 1; break;
	}

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_kod(int type)
{
	dipswitch_t *dip;

	dip = (type == 0) ? dipswitch_kod : dipswitch_kodj;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 3, 1);
	dip_save_bit(DIP_A, (dip++)->value, 4, 1);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x30; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x38; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x28; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 4: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 5: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 6: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 7: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x80; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x40; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  captcomm
--------------------------------------*/

static dipswitch_t dipswitch_captcomm[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty 1",   1, 0x07, 0, 7, dip_difficulty },
	{ "Difficulty 2",   1, 0x18, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Play Mode",      1, 0xc0, 0, 3, {"1 Player","2 Players","3 Players","4 Players"} },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"1","2","3","4"} },
	{ "Free Play",      1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",         1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_captcomm(void)
{
	dipswitch_t *dip = dipswitch_captcomm;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x18: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0xc0)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x40: (dip++)->value = 0; break;
	case 0x80: (dip++)->value = 2; break;
	case 0xc0: (dip++)->value = 1; break;
	}

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x01: (dip++)->value = 3; break;
	case 0x02: (dip++)->value = 2; break;
	case 0x03: (dip++)->value = 1; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_captcomm(void)
{
	dipswitch_t *dip = dipswitch_captcomm;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x40; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x80; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  knights
--------------------------------------*/

static dipswitch_t dipswitch_knights[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Enemy's attack frequency", 1, 0x07, 0, 7, dip_difficulty },
	{ "Enemy's attack power",     1, 0x38, 0, 7, dip_difficulty },
	{ "Coin Slots",               1, 0x40, 0, 1, {"1","3"}      },
	{ "Play Mode",                1, 0x80, 0, 1, {"2 Players","3 Players"} },

	// DIP C
	{ "Lives",          1, 0x03, 0, 3, {"1","2","3","4"} },
	{ "Free Play",      1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",         1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",    1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",    1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue", 1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",      1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_knights(void)
{
	dipswitch_t *dip = dipswitch_knights;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x38)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x08: (dip++)->value = 1; break;
	case 0x10: (dip++)->value = 2; break;
	case 0x18: (dip++)->value = 7; break;
	case 0x20: (dip++)->value = 6; break;
	case 0x28: (dip++)->value = 5; break;
	case 0x30: (dip++)->value = 4; break;
	case 0x38: (dip++)->value = 3; break;
	}
	(dip++)->value = dip_load_bit(DIP_B, 6, 1);
	(dip++)->value = dip_load_bit(DIP_B, 7, 1);

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x01: (dip++)->value = 3; break;
	case 0x02: (dip++)->value = 2; break;
	case 0x03: (dip++)->value = 1; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_knights(void)
{
	dipswitch_t *dip = dipswitch_knights;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x38; break;
	case 4: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x30; break;
	case 5: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x28; break;
	case 6: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x20; break;
	case 7: cps1_dipswitch[DIP_B] &= ~0x38; cps1_dipswitch[DIP_B] |= 0x18; break;
	}
	dip_save_bit(DIP_B, (dip++)->value, 6, 1);
	dip_save_bit(DIP_B, (dip++)->value, 7, 1);

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  varth
--------------------------------------*/

static dipswitch_t dipswitch_varth[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "Coin B",                          1, 0x38, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 7, dip_difficulty },
	{ "Bonus Life",          1, 0x18, 0, 3, {"600k and every 1.400k","600k 2.000k and 4500k","1.200k 3.500k","2000k only"} },

	// DIP C
	{ "Lives",               1, 0x03, 0, 3, {"1","2","3","4"} },
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_varth(void)
{
	dipswitch_t *dip = dipswitch_varth;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_coin1b();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x08: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x18: (dip++)->value = 0; break;
	}

	// DIP C
	switch (cps1_dipswitch[DIP_C] & 0x03)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x01: (dip++)->value = 1; break;
	case 0x02: (dip++)->value = 0; break;
	case 0x03: (dip++)->value = 2; break;
	}
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue;
	load_game_mode;
}

static void dip_save_varth(void)
{
	dipswitch_t *dip = dipswitch_varth;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_coin1b((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x02; break;
	case 1: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x01; break;
	case 2: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x03; break;
	case 3: cps1_dipswitch[DIP_C] &= ~0x03; cps1_dipswitch[DIP_C] |= 0x00; break;
	}
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue;
	save_game_mode;
}

/*--------------------------------------
  cworld2j
--------------------------------------*/

static dipswitch_t dipswitch_cworld2j[] =
{
	// DIP A
	{ "Coin A",                          1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },
	{ "Extended Test Mode",              1, 0x80, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 4, {"Easiest","Easy","Normal","Hard","Hardest"} },
	{ "Extend",              1, 0x18, 0, 2, {"N","E","D"} },
	{ "Lives",               1, 0xe0, 0, 4, {"1","2","3","4","5"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_cworld2j(void)
{
	dipswitch_t *dip = dipswitch_cworld2j;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);
	(dip++)->value = dip_load_bit(DIP_A, 7, 1);

	// DIP B
	switch (cps1_dipswitch[DIP_B] & 0x07)
	{
	case 0x02: (dip++)->value = 4; break;
	case 0x03: (dip++)->value = 3; break;
	case 0x04: (dip++)->value = 2; break;
	case 0x05: (dip++)->value = 1; break;
	case 0x06: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0x18)
	{
	case 0x00: (dip++)->value = 2; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x18: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0xe0)
	{
	case 0x00: (dip++)->value = 0; break;
	case 0x80: (dip++)->value = 1; break;
	case 0xa0: (dip++)->value = 3; break;
	case 0xc0: (dip++)->value = 4; break;
	case 0xe0: (dip++)->value = 2; break;
	}

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_cworld2j(void)
{
	dipswitch_t *dip = dipswitch_cworld2j;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);
	dip_save_bit(DIP_A, (dip++)->value, 7, 1);

	// DIP B
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x06; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x05; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x04; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x03; break;
	case 4: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x02; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0x00; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0x80; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xe0; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xa0; break;
	case 4: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	}

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue2;
	save_game_mode;
}

/*--------------------------------------
  qad / qadj
--------------------------------------*/

static dipswitch_t dipswitch_qad[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },
	{ "Extended Test Mode",              1, 0x80, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 4, {"Easiest","Easy","Normal","Hard","Hardest"} },
	{ "Wisdom",              1, 0x18, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Lives",               1, 0xe0, 0, 4, {"1","2","3","4","5"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_qadj[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },
	{ "Extended Test Mode",              1, 0x80, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",          1, 0x07, 0, 4, {"Easiest","Easy","Normal","Hard","Hardest"} },
	{ "Lives",               1, 0xe0, 0, 2, {"1","2","3"} },

	// DIP C
	{ "Free Play",           1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",              1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",         1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_qad(int type)
{
	dipswitch_t *dip;

	if (type == 0)
		dip = dipswitch_qad;
	else
		dip = dipswitch_qadj;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);
	(dip++)->value = dip_load_bit(DIP_A, 7, 1);

	// DIP B
	if (type == 0)
	{
		switch (cps1_dipswitch[DIP_B] & 0x07)
		{
		case 0x02: (dip++)->value = 4; break;
		case 0x03: (dip++)->value = 3; break;
		case 0x04: (dip++)->value = 2; break;
		case 0x05: (dip++)->value = 1; break;
		case 0x06: (dip++)->value = 0; break;
		}
		switch (cps1_dipswitch[DIP_B] & 0x18)
		{
		case 0x00: (dip++)->value = 3; break;
		case 0x08: (dip++)->value = 2; break;
		case 0x10: (dip++)->value = 1; break;
		case 0x18: (dip++)->value = 0; break;
		}
		switch (cps1_dipswitch[DIP_B] & 0xe0)
		{
		case 0x60: (dip++)->value = 0; break;
		case 0x80: (dip++)->value = 1; break;
		case 0xa0: (dip++)->value = 2; break;
		case 0xc0: (dip++)->value = 3; break;
		case 0xe0: (dip++)->value = 4; break;
		}
	}
	else
	{
		switch (cps1_dipswitch[DIP_B] & 0x07)
		{
		case 0x03: (dip++)->value = 4; break;
		case 0x04: (dip++)->value = 3; break;
		case 0x05: (dip++)->value = 2; break;
		case 0x06: (dip++)->value = 1; break;
		case 0x07: (dip++)->value = 0; break;
		}
		switch (cps1_dipswitch[DIP_B] & 0xe0)
		{
		case 0xa0: (dip++)->value = 0; break;
		case 0xc0: (dip++)->value = 1; break;
		case 0xe0: (dip++)->value = 2; break;
		}
	}

	// DIP C
	load_free_play;
	load_freeze;
	load_flip_screen;
	if (type == 0)
	{
		load_demo_sounds2;
	}
	else
	{
		load_demo_sounds;
	}
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_qad(int type)
{
	dipswitch_t *dip;

	if (type == 0)
		dip = dipswitch_qad;
	else
		dip = dipswitch_qadj;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);
	dip_save_bit(DIP_A, (dip++)->value, 7, 1);

	// DIP B
	if (type == 0)
	{
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x06; break;
		case 1: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x05; break;
		case 2: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x04; break;
		case 3: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x03; break;
		case 4: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x02; break;
		}
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x18; break;
		case 1: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x10; break;
		case 2: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x08; break;
		case 3: cps1_dipswitch[DIP_B] &= ~0x18; cps1_dipswitch[DIP_B] |= 0x00; break;
		}
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0x60; break;
		case 1: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0x80; break;
		case 2: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xa0; break;
		case 3: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xc0; break;
		case 4: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xe0; break;
		}
	}
	else
	{
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x07; break;
		case 1: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x06; break;
		case 2: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x05; break;
		case 3: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x04; break;
		case 4: cps1_dipswitch[DIP_B] &= ~0x07; cps1_dipswitch[DIP_B] |= 0x03; break;
		}
		switch ((dip++)->value)
		{
		case 0: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xa0; break;
		case 1: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xc0; break;
		case 2: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xe0; break;
		}
	}

	// DIP C
	save_free_play;
	save_freeze;
	save_flip_screen;
	if (type == 0)
	{
		save_demo_sounds2;
	}
	else
	{
		save_demo_sounds;
	}
	save_allow_continue2;
	save_game_mode;
}

/*--------------------------------------
  qtono2
--------------------------------------*/

static dipswitch_t dipswitch_qtono2[] =
{
	// DIP A
	{ "Coinage",                         1, 0x07, 0, 7, dip_coin1      },
	{ "2 Coins to Start, 1 to Continue", 1, 0x40, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",             1, 0x07, 0, 7, dip_difficulty },
	{ "Lives",                  1, 0xe0, 0, 4, {"1","2","3","4","5"} },

	// DIP C
	{ "Infinite Lives (Cheat)", 1, 0x02, 0, 1, dip_on_off     },
	{ "Free Play",              1, 0x04, 0, 1, dip_on_off     },
	{ "Freeze",                 1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",            1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",            1, 0x20, 0, 1, dip_on_off     },
	{ "Allow Continue",         1, 0x40, 0, 1, dip_yes_no     },
	{ "Game Mode",              1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_qtono2(void)
{
	dipswitch_t *dip = dipswitch_qtono2;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 6, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0xe0)
	{
	case 0x60: (dip++)->value = 0; break;
	case 0x80: (dip++)->value = 1; break;
	case 0xa0: (dip++)->value = 3; break;
	case 0xc0: (dip++)->value = 4; break;
	case 0xe0: (dip++)->value = 2; break;
	}

	// DIP C
	(dip++)->value = dip_load_bit(DIP_C, 1, 1);
	load_free_play;
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_allow_continue2;
	load_game_mode;
}

static void dip_save_qtono2(void)
{
	dipswitch_t *dip = dipswitch_qtono2;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 6, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0x60; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0x80; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xe0; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xa0; break;
	case 4: cps1_dipswitch[DIP_B] &= ~0xe0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	}

	// DIP C
	dip_save_bit(DIP_C, (dip++)->value, 1, 1);
	save_free_play;
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_allow_continue2;
	save_game_mode;
}

/*--------------------------------------
  megaman / rockmanj
--------------------------------------*/

#define dip_coin_megaman				\
{										\
	"9 Coins/1 Credit",					\
	"8 Coins/1 Credit",					\
	"7 Coins/1 Credit",					\
	"6 Coins/1 Credit",					\
	"5 Coins/1 Credit",					\
	"4 Coins/1 Credit",					\
	"3 Coins/1 Credit",					\
	"2 Coins/1 Credit",					\
	"2 Coins to Start, 1 to Continue",	\
	"1 Coins/1 Credit",					\
	"1 Coins/2 Credits",				\
	"1 Coins/3 Credits",				\
	"1 Coins/4 Credits",				\
	"1 Coins/6 Credits",				\
	"1 Coins/7 Credits",				\
	"1 Coins/8 Credits",				\
	"1 Coins/9 Credits",				\
	"Free Play"							\
}

static dipswitch_t dipswitch_megaman[] =
{
	// DIP A
	{ "Coinage",             1, 0x1f, 0, 18, dip_coin_megaman },
	{ "Coin slots",          1, 0x60, 0, 2, {"1, Individual","1, Common","2, Common"} },

	// DIP B
	{ "Difficulty",          1, 0x03, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Time",                1, 0x0c, 0, 3, {"100","90","70","60"} },
	{ "Voice",               1, 0x40, 0, 1, dip_on_off },

	// DIP C
	{ "Flip Screen",         1, 0x01, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x02, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x04, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static dipswitch_t dipswitch_rockmanj[] =
{
	// DIP A
	{ "Coinage",             1, 0x1f, 0, 18, dip_coin_megaman },
	{ "Coin slots",          1, 0x60, 0, 2, {"1, Individual","1, Common","2, Common"} },

	// DIP B
	{ "Difficulty",          1, 0x03, 0, 3, {"Easy","Normal","Hard","Hardest"} },
	{ "Time",                1, 0x0c, 0, 3, {"100","90","70","60"} },

	// DIP C
	{ "Flip Screen",         1, 0x01, 0, 1, dip_on_off     },
	{ "Demo Sounds",         1, 0x02, 0, 1, dip_on_off     },
	{ "Allow Continue",      1, 0x04, 0, 1, dip_yes_no     },
	{ "Game Mode",           1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_megaman(int type)
{
	dipswitch_t *dip;

	if (type == 0)
		dip = dipswitch_megaman;
	else
		dip = dipswitch_rockmanj;

	// DIP A
	switch (cps1_dipswitch[DIP_A] & 0x1f)
	{
	case 0x0d: (dip++)->value = 18; break;
	case 0x0e: (dip++)->value = 8; break;
	case 0x0f: (dip++)->value = 0; break;
	case 0x10: (dip++)->value = 1; break;
	case 0x11: (dip++)->value = 2; break;
	case 0x12: (dip++)->value = 3; break;
	case 0x13: (dip++)->value = 4; break;
	case 0x14: (dip++)->value = 5; break;
	case 0x15: (dip++)->value = 6; break;
	case 0x16: (dip++)->value = 7; break;
	case 0x17: (dip++)->value = 17; break;
	case 0x18: (dip++)->value = 16; break;
	case 0x19: (dip++)->value = 15; break;
	case 0x1a: (dip++)->value = 14; break;
	case 0x1b: (dip++)->value = 13; break;
	case 0x1c: (dip++)->value = 12; break;
	case 0x1d: (dip++)->value = 11; break;
	case 0x1e: (dip++)->value = 10; break;
	case 0x1f: (dip++)->value = 9; break;
	}
	switch (cps1_dipswitch[DIP_A] & 0x60)
	{
	case 0x20: (dip++)->value = 1; break;
	case 0x40: (dip++)->value = 0; break;
	case 0x60: (dip++)->value = 2; break;
	}

	// DIP B
	switch (cps1_dipswitch[DIP_B] & 0x03)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x01: (dip++)->value = 2; break;
	case 0x02: (dip++)->value = 1; break;
	case 0x03: (dip++)->value = 0; break;
	}
	switch (cps1_dipswitch[DIP_B] & 0x0c)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x04: (dip++)->value = 2; break;
	case 0x08: (dip++)->value = 1; break;
	case 0x0c: (dip++)->value = 0; break;
	}
	if (type == 0) (dip++)->value = dip_load_bit(DIP_B, 6, 1);

	// DIP C
	(dip++)->value = dip_load_bit(DIP_C, 0, 1);
	(dip++)->value = dip_load_bit(DIP_C, 1, 0);
	(dip++)->value = dip_load_bit(DIP_C, 2, type);
	load_game_mode;
}

static void dip_save_megaman(int type)
{
	dipswitch_t *dip;

	if (type == 0)
		dip = dipswitch_megaman;
	else
		dip = dipswitch_rockmanj;

	// DIP A
	switch ((dip++)->value)
	{
	case 0:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x0f; break;
	case 1:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x10; break;
	case 2:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x11; break;
	case 3:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x12; break;
	case 4:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x13; break;
	case 5:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x14; break;
	case 6:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x15; break;
	case 7:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x16; break;
	case 8:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x0e; break;
	case 9:  cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x1f; break;
	case 10: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x1e; break;
	case 11: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x1d; break;
	case 12: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x1c; break;
	case 13: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x1b; break;
	case 14: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x1a; break;
	case 15: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x19; break;
	case 16: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x18; break;
	case 17: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x17; break;
	case 18: cps1_dipswitch[DIP_A] &= ~0x1f; cps1_dipswitch[DIP_A] |= 0x0d; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_A] &= ~0x60; cps1_dipswitch[DIP_A] |= 0x40; break;
	case 1: cps1_dipswitch[DIP_A] &= ~0x60; cps1_dipswitch[DIP_A] |= 0x20; break;
	case 2: cps1_dipswitch[DIP_A] &= ~0x60; cps1_dipswitch[DIP_A] |= 0x60; break;
	}

	// DIP B
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x03; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x02; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x01; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x03; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x0c; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x08; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x04; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0x0c; cps1_dipswitch[DIP_B] |= 0x00; break;
	}
	if (type == 0) dip_save_bit(DIP_B, (dip++)->value, 6, 1);

	// DIP C
	dip_save_bit(DIP_C, (dip++)->value, 0, 1);
	dip_save_bit(DIP_C, (dip++)->value, 1, 0);
	dip_save_bit(DIP_C, (dip++)->value, 2, type);
	save_game_mode;
}

/*--------------------------------------
  pnickj
--------------------------------------*/

static dipswitch_t dipswitch_pnickj[] =
{
	// DIP A
	{ "Coinage",       1, 0x07, 0, 7, dip_coin1      },
	{ "Coin Slots",    1, 0x08, 0, 1, dip_on_off     },

	// DIP B
	{ "Difficulty",    1, 0x07, 0, 7, dip_difficulty },
	{ "Vs Play Mode",  1, 0xc0, 0, 3, {"1 Game Match","3 Games Match","5 Games Match","7 Games Match"} },

	// DIP C
	{ "Freeze",        1, 0x08, 0, 1, dip_on_off     },
	{ "Flip Screen",   1, 0x10, 0, 1, dip_on_off     },
	{ "Demo Sounds",   1, 0x20, 0, 1, dip_on_off     },
	{ "Game Mode",     1, 0x80, 0, 1, dip_game_mode  },

	MENU_BLANK,
	MENU_RETURN,
	MENU_END,
};

static void dip_load_pnickj(void)
{
	dipswitch_t *dip = dipswitch_pnickj;

	// DIP A
	(dip++)->value = dip_load_coin1a();
	(dip++)->value = dip_load_bit(DIP_A, 3, 1);

	// DIP B
	(dip++)->value = dip_load_difficulty1();
	switch (cps1_dipswitch[DIP_B] & 0xc0)
	{
	case 0x00: (dip++)->value = 3; break;
	case 0x40: (dip++)->value = 2; break;
	case 0x80: (dip++)->value = 1; break;
	case 0xc0: (dip++)->value = 0; break;
	}

	// DIP C
	load_freeze;
	load_flip_screen;
	load_demo_sounds;
	load_game_mode;
}

static void dip_save_pnickj(void)
{
	dipswitch_t *dip = dipswitch_pnickj;

	// DIP A
	dip_save_coin1a((dip++)->value);
	dip_save_bit(DIP_A, (dip++)->value, 3, 1);

	// DIP B
	dip_save_difficulty1((dip++)->value);
	switch ((dip++)->value)
	{
	case 0: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0xc0; break;
	case 1: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x80; break;
	case 2: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x40; break;
	case 3: cps1_dipswitch[DIP_B] &= ~0xc0; cps1_dipswitch[DIP_B] |= 0x00; break;
	}

	// DIP C
	save_freeze;
	save_flip_screen;
	save_demo_sounds;
	save_game_mode;
}

dipswitch_t *load_dipswitch(int *sx)
{
	dipswitch_t *dipswitch = NULL;

	switch (machine_input_type)
	{
	case INPTYPE_forgottn: dip_load_forgottn(); dipswitch = dipswitch_forgottn; break;
	case INPTYPE_ghouls:   dip_load_ghouls(0);  dipswitch = dipswitch_ghouls; break;
	case INPTYPE_ghoulsu:  dip_load_ghouls(1);  dipswitch = dipswitch_ghoulsu; break;
	case INPTYPE_daimakai: dip_load_ghouls(2);  dipswitch = dipswitch_daimakai; break;
	case INPTYPE_strider:  dip_load_strider(0); dipswitch = dipswitch_strider; break;
	case INPTYPE_stridrua: dip_load_strider(1); dipswitch = dipswitch_stridrua; break;
	case INPTYPE_dynwar:   dip_load_dynwar();   dipswitch = dipswitch_dynwar; break;
	case INPTYPE_willow:   dip_load_willow();   dipswitch = dipswitch_willow; break;
	case INPTYPE_unsquad:  dip_load_unsquad();  dipswitch = dipswitch_unsquad; break;
	case INPTYPE_ffight:   dip_load_ffight();   dipswitch = dipswitch_ffight; break;
	case INPTYPE_1941:     dip_load_1941();     dipswitch = dipswitch_1941; break;
	case INPTYPE_mercs:    dip_load_mercs();    dipswitch = dipswitch_mercs; break;
	case INPTYPE_mtwins:   dip_load_mtwins();   dipswitch = dipswitch_mtwins; break;
	case INPTYPE_msword:   dip_load_msword();   dipswitch = dipswitch_msword; *sx = 270; break;
	case INPTYPE_cawing:   dip_load_cawing();   dipswitch = dipswitch_cawing; *sx = 270; break;
	case INPTYPE_nemo:     dip_load_nemo();     dipswitch = dipswitch_nemo; break;
	case INPTYPE_sf2:      dip_load_sf2(0);     dipswitch = dipswitch_sf2; break;
	case INPTYPE_sf2j:     dip_load_sf2(1);     dipswitch = dipswitch_sf2j; break;
	case INPTYPE_3wonders: dip_load_3wonders(); dipswitch = dipswitch_3wonders; break;
	case INPTYPE_kod:      dip_load_kod(0);     dipswitch = dipswitch_kod; break;
	case INPTYPE_kodj:     dip_load_kod(1);     dipswitch = dipswitch_kodj; break;
	case INPTYPE_captcomm: dip_load_captcomm(); dipswitch = dipswitch_captcomm; break;
#if !RELEASE
	case INPTYPE_knightsh:
#endif
	case INPTYPE_knights:  dip_load_knights();  dipswitch = dipswitch_knights; break;
	case INPTYPE_varth:    dip_load_varth();    dipswitch = dipswitch_varth; break;
	case INPTYPE_cworld2j: dip_load_cworld2j(); dipswitch = dipswitch_cworld2j; break;
	case INPTYPE_qad:      dip_load_qad(0);     dipswitch = dipswitch_qad; break;
	case INPTYPE_qadj:     dip_load_qad(1);     dipswitch = dipswitch_qadj; break;
	case INPTYPE_qtono2:   dip_load_qtono2();   dipswitch = dipswitch_qtono2; break;
	case INPTYPE_megaman:  dip_load_megaman(0); dipswitch = dipswitch_megaman; break;
	case INPTYPE_rockmanj: dip_load_megaman(1); dipswitch = dipswitch_rockmanj; break;
	case INPTYPE_pnickj:   dip_load_pnickj();   dipswitch = dipswitch_pnickj; break;
	}

	return dipswitch;
}


void save_dipswitch(void)
{
	switch (machine_input_type)
	{
	case INPTYPE_forgottn: dip_save_forgottn(); break;
	case INPTYPE_ghouls:   dip_save_ghouls(0); break;
	case INPTYPE_ghoulsu:  dip_save_ghouls(1); break;
	case INPTYPE_daimakai: dip_save_ghouls(2); break;
	case INPTYPE_strider:  dip_save_strider(0); break;
	case INPTYPE_stridrua: dip_save_strider(1); break;
	case INPTYPE_dynwar:   dip_save_dynwar(); break;
	case INPTYPE_willow:   dip_save_willow(); break;
	case INPTYPE_unsquad:  dip_save_unsquad(); break;
	case INPTYPE_ffight:   dip_save_ffight(); break;
	case INPTYPE_1941:     dip_save_1941(); break;
	case INPTYPE_mercs:    dip_save_mercs(); break;
	case INPTYPE_mtwins:   dip_save_mtwins(); break;
	case INPTYPE_msword:   dip_save_msword(); break;
	case INPTYPE_cawing:   dip_save_cawing(); break;
	case INPTYPE_nemo:     dip_save_nemo(); break;
	case INPTYPE_sf2:      dip_save_sf2(0); break;
	case INPTYPE_sf2j:     dip_save_sf2(1); break;
	case INPTYPE_3wonders: dip_save_3wonders(); break;
	case INPTYPE_kod:      dip_save_kod(0); break;
	case INPTYPE_kodj:     dip_save_kod(1); break;
	case INPTYPE_captcomm: dip_save_captcomm(); break;
#if !RELEASE
	case INPTYPE_knightsh:
#endif
	case INPTYPE_knights:  dip_save_knights(); break;
	case INPTYPE_varth:    dip_save_varth(); break;
	case INPTYPE_cworld2j: dip_save_cworld2j(); break;
	case INPTYPE_qad:      dip_save_qad(0); break;
	case INPTYPE_qadj:     dip_save_qad(1); break;
	case INPTYPE_qtono2:   dip_save_qtono2(); break;
	case INPTYPE_megaman:  dip_save_megaman(0); break;
	case INPTYPE_rockmanj: dip_save_megaman(1); break;
	case INPTYPE_pnickj:   dip_save_pnickj(); break;
	}
}
