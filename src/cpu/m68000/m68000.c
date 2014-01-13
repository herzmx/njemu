/******************************************************************************

	m68000.c

	M68000 CPU�C���^�t�F�[�X�֐�

******************************************************************************/

#include "emumain.h"


/******************************************************************************
	CPS2�Í���ROM�p
******************************************************************************/

#if (EMU_SYSTEM == CPS2)
static u32 m68k_encrypt_start;
static u32 m68k_encrypt_end;
static u8  *m68k_decrypted_rom;

/*--------------------------------------------------------
	�Í���ROM�͈͐ݒ�
--------------------------------------------------------*/

void m68000_set_encrypted_range(u32 start, u32 end, void *decrypted_rom)
{
	m68k_encrypt_start = start;
	m68k_encrypt_end   = end;
	m68k_decrypted_rom = (u8 *)decrypted_rom;
}


/*--------------------------------------------------------
	PC�ˑ����������[�h (byte)
--------------------------------------------------------*/

static u8 m68000_read_pcrelative_8(u32 offset)
{
	if (offset >= m68k_encrypt_start && offset <= m68k_encrypt_end)
		return m68k_decrypted_rom[offset ^ 1];
	else
		return m68000_read_memory_8(offset);
}


/*--------------------------------------------------------
	PC�ˑ����������[�h (word)
--------------------------------------------------------*/

static u16 m68000_read_pcrelative_16(u32 offset)
{
	if (offset >= m68k_encrypt_start && offset <= m68k_encrypt_end)
		return *(u16 *)&m68k_decrypted_rom[offset];
	else
		return m68000_read_memory_16(offset);
}
#endif


/******************************************************************************
	M68000�C���^�t�F�[�X�֐�
******************************************************************************/

/*--------------------------------------------------------
	CPU������
--------------------------------------------------------*/

void m68000_init(void)
{
	C68k_Init(&C68K);
#if (EMU_SYSTEM == CPS1)
	C68k_Set_Fetch(&C68K, 0x000000, 0x1fffff, (u32)memory_region_cpu1);
	C68k_Set_Fetch(&C68K, 0x900000, 0x92ffff, (u32)cps1_gfxram);
	C68k_Set_Fetch(&C68K, 0xff0000, 0xffffff, (u32)cps1_ram);
	C68k_Set_ReadB(&C68K, m68000_read_memory_8);
	C68k_Set_ReadW(&C68K, m68000_read_memory_16);
	C68k_Set_WriteB(&C68K, m68000_write_memory_8);
	C68k_Set_WriteW(&C68K, m68000_write_memory_16);
#elif (EMU_SYSTEM == CPS2)
	C68k_Set_Fetch(&C68K, 0x000000, 0x3fffff, (u32)memory_region_user1);
	C68k_Set_Fetch(&C68K, 0x660000, 0x663fff, (u32)cps2_ram);
	C68k_Set_Fetch(&C68K, 0x900000, 0x92ffff, (u32)cps1_gfxram);
	C68k_Set_Fetch(&C68K, 0xff0000, 0xffffff, (u32)cps1_ram);
	C68k_Set_ReadB(&C68K, m68000_read_memory_8);
	C68k_Set_ReadW(&C68K, m68000_read_memory_16);
	C68k_Set_WriteB(&C68K, m68000_write_memory_8);
	C68k_Set_WriteW(&C68K, m68000_write_memory_16);
	C68k_Set_ReadB_PC_Relative(&C68K, m68000_read_pcrelative_8);
	C68k_Set_ReadW_PC_Relative(&C68K, m68000_read_pcrelative_16);
#elif (EMU_SYSTEM == MVS)
	C68k_Set_Fetch(&C68K, 0x000000, 0x0fffff, (u32)memory_region_cpu1);
	C68k_Set_Fetch(&C68K, 0x100000, 0x10ffff, (u32)neogeo_ram);
	if (memory_length_cpu1 > 0x100000)
		C68k_Set_Fetch(&C68K, 0x200000, 0x2fffff, (u32)&memory_region_cpu1[0x100000]);
	else
		C68k_Set_Fetch(&C68K, 0x200000, 0x2fffff, (u32)memory_region_cpu1);
	C68k_Set_Fetch(&C68K, 0xc00000, 0xc00000 + (memory_length_user1 - 1), (u32)memory_region_user1);
	C68k_Set_ReadB(&C68K, m68000_read_memory_8);
	C68k_Set_ReadW(&C68K, m68000_read_memory_16);
	C68k_Set_WriteB(&C68K, m68000_write_memory_8);
	C68k_Set_WriteW(&C68K, m68000_write_memory_16);
#endif
}


/*--------------------------------------------------------
	CPU���Z�b�g
--------------------------------------------------------*/

void m68000_reset(void)
{
	C68k_Reset(&C68K);
}


/*--------------------------------------------------------
	CPU��~
--------------------------------------------------------*/

void m68000_exit(void)
{
}


/*--------------------------------------------------------
	CPU���s
--------------------------------------------------------*/

int m68000_execute(int cycles)
{
	return C68k_Exec(&C68K, cycles);
}


/*--------------------------------------------------------
	���荞�ݏ���
--------------------------------------------------------*/

void m68000_set_irq_line(int irqline, int state)
{
	if (irqline == IRQ_LINE_NMI)
		irqline = 7;

	C68k_Set_IRQ(&C68K, irqline, state);
}


/*--------------------------------------------------------
	���荞�݃R�[���o�b�N�֐��ݒ�
--------------------------------------------------------*/

void m68000_set_irq_callback(int (*callback)(int line))
{
	C68k_Set_IRQ_Callback(&C68K, callback);
}


/*--------------------------------------------------------
	���W�X�^�擾
--------------------------------------------------------*/

u32 m68000_get_reg(int regnum)
{
	switch (regnum)
	{
	case M68K_PC:  return C68k_Get_Reg(&C68K, C68K_PC);
	case M68K_USP: return C68k_Get_Reg(&C68K, C68K_USP);
	case M68K_MSP: return C68k_Get_Reg(&C68K, C68K_MSP);
	case M68K_SR:  return C68k_Get_Reg(&C68K, C68K_SR);
	case M68K_D0:  return C68k_Get_Reg(&C68K, C68K_D0);
	case M68K_D1:  return C68k_Get_Reg(&C68K, C68K_D1);
	case M68K_D2:  return C68k_Get_Reg(&C68K, C68K_D2);
	case M68K_D3:  return C68k_Get_Reg(&C68K, C68K_D3);
	case M68K_D4:  return C68k_Get_Reg(&C68K, C68K_D4);
	case M68K_D5:  return C68k_Get_Reg(&C68K, C68K_D5);
	case M68K_D6:  return C68k_Get_Reg(&C68K, C68K_D6);
	case M68K_D7:  return C68k_Get_Reg(&C68K, C68K_D7);
	case M68K_A0:  return C68k_Get_Reg(&C68K, C68K_A0);
	case M68K_A1:  return C68k_Get_Reg(&C68K, C68K_A1);
	case M68K_A2:  return C68k_Get_Reg(&C68K, C68K_A2);
	case M68K_A3:  return C68k_Get_Reg(&C68K, C68K_A3);
	case M68K_A4:  return C68k_Get_Reg(&C68K, C68K_A4);
	case M68K_A5:  return C68k_Get_Reg(&C68K, C68K_A5);
	case M68K_A6:  return C68k_Get_Reg(&C68K, C68K_A6);
	case M68K_A7:  return C68k_Get_Reg(&C68K, C68K_A7);
	default: return 0;
	}
}


/*--------------------------------------------------------
	���W�X�^�ݒ�
--------------------------------------------------------*/

void m68000_set_reg(int regnum, u32 val)
{
	switch (regnum)
	{
	case M68K_PC:  C68k_Set_Reg(&C68K, C68K_PC, val); break;
	case M68K_USP: C68k_Set_Reg(&C68K, C68K_USP, val); break;
	case M68K_MSP: C68k_Set_Reg(&C68K, C68K_MSP, val); break;
	case M68K_SR:  C68k_Set_Reg(&C68K, C68K_SR, val); break;
	case M68K_D0:  C68k_Set_Reg(&C68K, C68K_D0, val); break;
	case M68K_D1:  C68k_Set_Reg(&C68K, C68K_D1, val); break;
	case M68K_D2:  C68k_Set_Reg(&C68K, C68K_D2, val); break;
	case M68K_D3:  C68k_Set_Reg(&C68K, C68K_D3, val); break;
	case M68K_D4:  C68k_Set_Reg(&C68K, C68K_D4, val); break;
	case M68K_D5:  C68k_Set_Reg(&C68K, C68K_D5, val); break;
	case M68K_D6:  C68k_Set_Reg(&C68K, C68K_D6, val); break;
	case M68K_D7:  C68k_Set_Reg(&C68K, C68K_D7, val); break;
	case M68K_A0:  C68k_Set_Reg(&C68K, C68K_A0, val); break;
	case M68K_A1:  C68k_Set_Reg(&C68K, C68K_A1, val); break;
	case M68K_A2:  C68k_Set_Reg(&C68K, C68K_A2, val); break;
	case M68K_A3:  C68k_Set_Reg(&C68K, C68K_A3, val); break;
	case M68K_A4:  C68k_Set_Reg(&C68K, C68K_A4, val); break;
	case M68K_A5:  C68k_Set_Reg(&C68K, C68K_A5, val); break;
	case M68K_A6:  C68k_Set_Reg(&C68K, C68K_A6, val); break;
	case M68K_A7:  C68k_Set_Reg(&C68K, C68K_A7, val); break;
	default: break;
	}
}


/*------------------------------------------------------
	�Z�[�u/���[�h �X�e�[�g
------------------------------------------------------*/

#ifdef SAVE_STATE

STATE_SAVE( m68000 )
{
	int i;
	u32 pc = C68k_Get_Reg(&C68K, C68K_PC);

	for (i = 0; i < 8; i++)
		state_save_long(&C68K.D[i], 1);
	for (i = 0; i < 8; i++)
		state_save_long(&C68K.A[i], 1);

	state_save_long(&C68K.flag_C, 1);
	state_save_long(&C68K.flag_V, 1);
	state_save_long(&C68K.flag_Z, 1);
	state_save_long(&C68K.flag_N, 1);
	state_save_long(&C68K.flag_X, 1);
	state_save_long(&C68K.flag_I, 1);
	state_save_long(&C68K.flag_S, 1);
	state_save_long(&C68K.USP, 1);
	state_save_long(&pc, 1);
	state_save_long(&C68K.HaltState, 1);
	state_save_long(&C68K.IRQLine, 1);
	state_save_long(&C68K.IRQState, 1);
}

STATE_LOAD( m68000 )
{
	int i;
	u32 pc;

	for (i = 0; i < 8; i++)
		state_load_long(&C68K.D[i], 1);
	for (i = 0; i < 8; i++)
		state_load_long(&C68K.A[i], 1);

	state_load_long(&C68K.flag_C, 1);
	state_load_long(&C68K.flag_V, 1);
	state_load_long(&C68K.flag_Z, 1);
	state_load_long(&C68K.flag_N, 1);
	state_load_long(&C68K.flag_X, 1);
	state_load_long(&C68K.flag_I, 1);
	state_load_long(&C68K.flag_S, 1);
	state_load_long(&C68K.USP, 1);
	state_load_long(&pc, 1);
	state_load_long(&C68K.HaltState, 1);
	state_load_long(&C68K.IRQLine, 1);
	state_load_long(&C68K.IRQState, 1);

	C68k_Set_Reg(&C68K, C68K_PC, pc);
}

#endif /* SAVE_STATE */
