/******************************************************************************
 *
 * CZ80 (Z80 CPU emulator) version 0.9
 * Compiled with Dev-C++
 * Copyright 2004-2005 Stéphane Dallongeville
 *
 * (Modified by NJ)
 *
 *****************************************************************************/

#ifndef CZ80_H
#define CZ80_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************/
/* Compiler dependant defines */
/******************************/

#ifndef u8
#define u8	unsigned char
#endif

#ifndef s8
#define s8	char
#endif

#ifndef u16
#define u16	unsigned short
#endif

#ifndef s16
#define s16	short
#endif

#ifndef u32
#define u32	unsigned int
#endif

#ifndef s32
#define s32	int
#endif

/*************************************/
/* Z80 core Structures & definitions */
/*************************************/

#define CZ80_FETCH_BITS			4   // [4-12]   default = 8

#define CZ80_FETCH_SFT			(16 - CZ80_FETCH_BITS)
#define CZ80_FETCH_BANK			(1 << CZ80_FETCH_BITS)

#define CZ80_LITTLE_ENDIAN		1
#define CZ80_USE_JUMPTABLE		1
#define CZ80_BIG_FLAGS_ARRAY	1
#ifdef BUILD_CPS1PSP
#define CZ80_ENCRYPTED_ROM		1
#else
#define CZ80_ENCRYPTED_ROM		0
#endif
#define CZ80_EMULATE_R_EXACTLY	0

#define zR8(A)		(*CPU->pzR8[A])
#define zR16(A)		(CPU->pzR16[A]->W)

#define pzAF		&(CPU->AF)
#define zAF			CPU->AF.W
#define zlAF		CPU->AF.B.L
#define zhAF		CPU->AF.B.H
#define zA			zhAF
#define zF			zlAF

#define pzBC		&(CPU->BC)
#define zBC			CPU->BC.W
#define zlBC		CPU->BC.B.L
#define zhBC		CPU->BC.B.H
#define zB			zhBC
#define zC			zlBC

#define pzDE		&(CPU->DE)
#define zDE			CPU->DE.W
#define zlDE		CPU->DE.B.L
#define zhDE		CPU->DE.B.H
#define zD			zhDE
#define zE			zlDE

#define pzHL		&(CPU->HL)
#define zHL			CPU->HL.W
#define zlHL		CPU->HL.B.L
#define zhHL		CPU->HL.B.H
#define zH			zhHL
#define zL			zlHL

#define zAF2		CPU->AF2.W
#define zlAF2		CPU->AF2.B.L
#define zhAF2		CPU->AF2.B.H
#define zA2			zhAF2
#define zF2			zlAF2

#define zBC2		CPU->BC2.W
#define zDE2		CPU->DE2.W
#define zHL2		CPU->HL2.W

#define pzIX		&(CPU->IX)
#define zIX			CPU->IX.W
#define zlIX		CPU->IX.B.L
#define zhIX		CPU->IX.B.H

#define pzIY		&(CPU->IY)
#define zIY			CPU->IY.W
#define zlIY		CPU->IY.B.L
#define zhIY		CPU->IY.B.H

#define pzSP		&(CPU->SP)
#define zSP			CPU->SP.W
#define zlSP		CPU->SP.B.L
#define zhSP		CPU->SP.B.H

#define zRealPC		(PC - CPU->BasePC)
#define zPC			PC

#define zI			CPU->I
#define zIM			CPU->IM

#define zwR			CPU->R.W
#define zR1			CPU->R.B.L
#define zR2			CPU->R.B.H
#define zR			zR1

#define zIFF		CPU->IFF.W
#define zIFF1		CPU->IFF.B.L
#define zIFF2		CPU->IFF.B.H

#define CZ80_SF_SFT	 7
#define CZ80_ZF_SFT	 6
#define CZ80_YF_SFT	 5
#define CZ80_HF_SFT	 4
#define CZ80_XF_SFT	 3
#define CZ80_PF_SFT	 2
#define CZ80_VF_SFT	 2
#define CZ80_NF_SFT	 1
#define CZ80_CF_SFT	 0

#define CZ80_SF		(1 << CZ80_SF_SFT)
#define CZ80_ZF		(1 << CZ80_ZF_SFT)
#define CZ80_YF		(1 << CZ80_YF_SFT)
#define CZ80_HF		(1 << CZ80_HF_SFT)
#define CZ80_XF		(1 << CZ80_XF_SFT)
#define CZ80_PF		(1 << CZ80_PF_SFT)
#define CZ80_VF		(1 << CZ80_VF_SFT)
#define CZ80_NF		(1 << CZ80_NF_SFT)
#define CZ80_CF		(1 << CZ80_CF_SFT)

#define CZ80_IFF_SFT	CZ80_PF_SFT
#define CZ80_IFF		CZ80_PF

#ifndef IRQ_LINE_STATE
#define IRQ_LINE_STATE
#define CLEAR_LINE		0		/* clear (a fired, held or pulsed) line */
#define ASSERT_LINE		1		/* assert an interrupt immediately */
#define HOLD_LINE		2		/* hold interrupt line until acknowledged */
#define PULSE_LINE		3		/* pulse interrupt line for one instruction */
#define IRQ_LINE_NMI	127		/* IRQ line for NMIs */
#endif

enum
{
	CZ80_PC = 1,
	CZ80_SP,
	CZ80_AF,
	CZ80_BC,
	CZ80_DE,
	CZ80_HL,
	CZ80_IX,
	CZ80_IY,
	CZ80_AF2,
	CZ80_BC2,
	CZ80_DE2,
	CZ80_HL2,
	CZ80_R,
	CZ80_I,
	CZ80_IM,
	CZ80_IFF1,
	CZ80_IFF2,
	CZ80_HALT,
	CZ80_IRQ
};

typedef union
{
	struct
	{
#if CZ80_LITTLE_ENDIAN
		u8 L;
		u8 H;
#else
		u8 H;
		u8 L;
#endif
	} B;
	u16 W;
} union16;

typedef struct cz80_t
{
	union
	{
		u8 r8[8];
		union16 r16[4];
		struct
		{
			union16 BC;
			union16 DE;
			union16 HL;
			union16 AF;
		};
	};

	union16 IX;
	union16 IY;
	union16 SP;
	u32 PC;

	union16 BC2;
	union16 DE2;
	union16 HL2;
	union16 AF2;

	union16 R;
	union16 IFF;

	u8 I;
	u8 IM;
	u8 HaltState;
	u8 dummy;

	s32 IRQLine;
	s32 IRQState;
	s32 ICount;
	s32 ExtraCycles;

	u32 BasePC;
	u32 Fetch[CZ80_FETCH_BANK];
#if CZ80_ENCRYPTED_ROM
	s32 OPBase;
	s32 OPFetch[CZ80_FETCH_BANK];
#endif

	u8 *pzR8[8];
	union16 *pzR16[4];

	u8   (*Read_Byte)(u32 address);
	void (*Write_Byte)(u32 address, u8 data);

	u8   (*IN_Port)(u16 port);
	void (*OUT_Port)(u16 port, u8 value);

	s32  (*Interrupt_Callback)(s32 irqline);

} cz80_struc;


/*************************/
/* Publics Z80 variables */
/*************************/

extern cz80_struc CZ80;

/*************************/
/* Publics Z80 functions */
/*************************/

void Cz80_Init(cz80_struc *CPU);

void Cz80_Reset(cz80_struc *CPU);

s32  Cz80_Exec(cz80_struc *CPU, s32 cycles);

void Cz80_Set_IRQ(cz80_struc *CPU, s32 line, s32 state);

u32  Cz80_Get_Reg(cz80_struc *CPU, s32 regnum);
void Cz80_Set_Reg(cz80_struc *CPU, s32 regnum, u32 value);

void Cz80_Set_Fetch(cz80_struc *CPU, u32 low_adr, u32 high_adr, u32 fetch_adr);
#if CZ80_ENCRYPTED_ROM
void Cz80_Set_Encrypt_Range(cz80_struc *CPU, u32 low_adr, u32 high_adr, u32 decrypted_rom);
#endif

void Cz80_Set_ReadB(cz80_struc *CPU, u8 (*Func)(u32 address));
void Cz80_Set_WriteB(cz80_struc *CPU, void (*Func)(u32 address, u8 data));

void Cz80_Set_INPort(cz80_struc *CPU, u8 (*Func)(u16 port));
void Cz80_Set_OUTPort(cz80_struc *CPU, void (*Func)(u16 port, u8 value));

void Cz80_Set_IRQ_Callback(cz80_struc *CPU, s32 (*Func)(s32 irqline));

#ifdef __cplusplus
};
#endif

#endif	/* CZ80_H */
