// SPDX-License-Identifier: MIT
/*
must fix:
	chk
*/
/*
 * A portable Motorola M680x0 processor emulation engine.
 *
 * Copyright 1998-2001 Karl Stenerud.  All rights reserved.
 */

/* Special thanks to Bart Trzynadlowski for his insight into the
 * undocumented features of this chip:
 *
 * http://dynarec.com/~bart/files/68knotes.txt
 */


/* Input file for m68kmake
 * -----------------------
 *
 * All sections begin with 80 X's in a row followed by an end-of-line
 * sequence.
 * After this, m68kmake will expect to find one of the following section
 * identifiers:
 *    M68KMAKE_PROTOTYPE_HEADER      - header for opcode handler prototypes
 *    M68KMAKE_PROTOTYPE_FOOTER      - footer for opcode handler prototypes
 *    M68KMAKE_TABLE_HEADER          - header for opcode handler jumptable
 *    M68KMAKE_TABLE_FOOTER          - footer for opcode handler jumptable
 *    M68KMAKE_TABLE_BODY            - the table itself
 *    M68KMAKE_OPCODE_HANDLER_HEADER - header for opcode handler implementation
 *    M68KMAKE_OPCODE_HANDLER_FOOTER - footer for opcode handler implementation
 *    M68KMAKE_OPCODE_HANDLER_BODY   - body section for opcode handler implementation
 *
 * NOTE: M68KMAKE_OPCODE_HANDLER_BODY must be last in the file and
 *       M68KMAKE_TABLE_BODY must be second last in the file.
 *
 * The M68KMAKE_OPHANDLER_BODY section contains the opcode handler
 * primitives themselves.  Each opcode handler begins with:
 *    M68KMAKE_OP(A, B, C, D)
 *
 * where A is the opcode handler name, B is the size of the operation,
 * C denotes any special processing mode, and D denotes a specific
 * addressing mode.
 * For C and D where nothing is specified, use "."
 *
 * Example:
 *     M68KMAKE_OP(abcd, 8, rr, .)   abcd, size 8, register to register, default EA
 *     M68KMAKE_OP(abcd, 8, mm, ax7) abcd, size 8, memory to memory, register X is A7
 *     M68KMAKE_OP(tst, 16, ., pcix) tst, size 16, PCIX addressing
 *
 * All opcode handler primitives end with a closing curly brace "}" at column 1
 *
 * NOTE: Do not place a M68KMAKE_OP() directive inside the opcode handler,
 *       and do not put a closing curly brace at column 1 unless it is
 *       marking the end of the handler!
 *
 * Inside the handler, m68kmake will recognize M68KMAKE_GET_OPER_xx_xx,
 * M68KMAKE_GET_EA_xx_xx, and M68KMAKE_CC directives, and create multiple
 * opcode handlers to handle variations in the opcode handler.
 * Note: M68KMAKE_CC will only be interpreted in condition code opcodes.
 * As well, M68KMAKE_GET_EA_xx_xx and M68KMAKE_GET_OPER_xx_xx will only
 * be interpreted on instructions where the corresponding table entry
 * specifies multiple effective addressing modes.
 * Example:
 * clr       32  .     .     0100001010......  A+-DXWL...  U U U   12   6   4
 *
 * This table entry says that the clr.l opcde has 7 variations (A+-DXWL).
 * It is run in user or supervisor mode for all CPUs, and uses 12 cycles for
 * 68000, 6 cycles for 68010, and 4 cycles for 68020.
 */

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_PROTOTYPE_HEADER

#ifndef M68KOPS__HEADER
#define M68KOPS__HEADER

/* ======================================================================== */
/* ============================ OPCODE HANDLERS =========================== */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_PROTOTYPE_FOOTER


/* Build the opcode handler table */
void m68ki_build_opcode_table(void);

extern void (*m68ki_instruction_jump_table[0x10000])(void); /* opcode handler jump table */
extern unsigned char m68ki_cycles[][0x10000];


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* M68KOPS__HEADER */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_TABLE_HEADER

/* ======================================================================== */
/* ========================= OPCODE TABLE BUILDER ========================= */
/* ======================================================================== */

#include <stdio.h>
#include "m68k/m68kops.h"

#define NUM_CPU_TYPES 5

void  (*m68ki_instruction_jump_table[0x10000])(void); /* opcode handler jump table */
unsigned char m68ki_cycles[NUM_CPU_TYPES][0x10000]; /* Cycles used by CPU type */

/* This is used to generate the opcode handler jump table */
typedef struct
{
	void (*opcode_handler)(void);        /* handler function */
	unsigned int  mask;                  /* mask on opcode */
	unsigned int  match;                 /* what to match after masking */
	unsigned char cycles[NUM_CPU_TYPES]; /* cycles each cpu type takes */
} opcode_handler_struct;


/* Opcode handler table */
static const opcode_handler_struct m68k_opcode_handler_table[] =
{
/*   function                      mask    match    000  010  020  040 */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_TABLE_FOOTER

	{0, 0, 0, {0, 0, 0, 0, 0}}
};


/* Build the opcode handler jump table */
void m68ki_build_opcode_table(void)
{
	const opcode_handler_struct *ostruct;
	int cycle_cost;
	int instr;
	int i;
	int j;
	int k;

	for(i = 0; i < 0x10000; i++)
	{
		/* default to illegal */
		m68ki_instruction_jump_table[i] = m68k_op_illegal;
		for(k=0;k<NUM_CPU_TYPES;k++)
			m68ki_cycles[k][i] = 0;
	}

	ostruct = m68k_opcode_handler_table;
	while(ostruct->mask != 0xff00)
	{
		for(i = 0;i < 0x10000;i++)
		{
			if((i & ostruct->mask) == ostruct->match)
			{
				m68ki_instruction_jump_table[i] = ostruct->opcode_handler;
				for(k=0;k<NUM_CPU_TYPES;k++)
					m68ki_cycles[k][i] = ostruct->cycles[k];
			}
		}
		ostruct++;
	}
	while(ostruct->mask == 0xff00)
	{
		for(i = 0;i <= 0xff;i++)
		{
			m68ki_instruction_jump_table[ostruct->match | i] = ostruct->opcode_handler;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | i] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xf1f8)
	{
		for(i = 0;i < 8;i++)
		{
			for(j = 0;j < 8;j++)
			{
				instr = ostruct->match | (i << 9) | j;
				m68ki_instruction_jump_table[instr] = ostruct->opcode_handler;
				for(k=0;k<NUM_CPU_TYPES;k++)
					m68ki_cycles[k][instr] = ostruct->cycles[k];
				// For all shift operations with known shift distance (encoded in instruction word)
				if((instr & 0xf000) == 0xe000 && (!(instr & 0x20)))
				{
					// On the 68000 and 68010 shift distance affect execution time.
					// Add the cycle cost of shifting; 2 times the shift distance
					cycle_cost = ((((i-1)&7)+1)<<1);
					m68ki_cycles[0][instr] += cycle_cost;
					m68ki_cycles[1][instr] += cycle_cost;
					// On the 68020 shift distance does not affect execution time
					m68ki_cycles[2][instr] += 0;
				}
			}
		}
		ostruct++;
	}
	while(ostruct->mask == 0xfff0)
	{
		for(i = 0;i <= 0x0f;i++)
		{
			m68ki_instruction_jump_table[ostruct->match | i] = ostruct->opcode_handler;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | i] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xf1ff)
	{
		for(i = 0;i <= 0x07;i++)
		{
			m68ki_instruction_jump_table[ostruct->match | (i << 9)] = ostruct->opcode_handler;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | (i << 9)] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xfff8)
	{
		for(i = 0;i <= 0x07;i++)
		{
			m68ki_instruction_jump_table[ostruct->match | i] = ostruct->opcode_handler;
			for(k=0;k<NUM_CPU_TYPES;k++)
				m68ki_cycles[k][ostruct->match | i] = ostruct->cycles[k];
		}
		ostruct++;
	}
	while(ostruct->mask == 0xffff)
	{
		m68ki_instruction_jump_table[ostruct->match] = ostruct->opcode_handler;
		for(k=0;k<NUM_CPU_TYPES;k++)
			m68ki_cycles[k][ostruct->match] = ostruct->cycles[k];
		ostruct++;
	}
}


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_OPCODE_HANDLER_HEADER

#include <stdio.h>
#include "m68k/m68kcpu.h"
extern void m68040_fpu_op0(void);
extern void m68040_fpu_op1(void);
extern void m68881_mmu_ops();

/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_OPCODE_HANDLER_FOOTER

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_TABLE_BODY

The following table is arranged as follows:

name:        Opcode mnemonic

size:        Operation size

spec proc:   Special processing mode:
                 .:    normal
                 s:    static operand
                 r:    register operand
                 rr:   register to register
                 mm:   memory to memory
                 er:   effective address to register
                 re:   register to effective address
                 dd:   data register to data register
                 da:   data register to address register
                 aa:   address register to address register
                 cr:   control register to register
                 rc:   register to control register
                 toc:  to condition code register
                 tos:  to status register
                 tou:  to user stack pointer
                 frc:  from condition code register
                 frs:  from status register
                 fru:  from user stack pointer
                 * for move.x, the special processing mode is a specific
                   destination effective addressing mode.

spec ea:     Specific effective addressing mode:
                 .:    normal
                 i:    immediate
                 d:    data register
                 a:    address register
                 ai:   address register indirect
                 pi:   address register indirect with postincrement
                 pd:   address register indirect with predecrement
                 di:   address register indirect with displacement
                 ix:   address register indirect with index
                 aw:   absolute word address
                 al:   absolute long address
                 pcdi: program counter relative with displacement
                 pcix: program counter relative with index
                 a7:   register specified in instruction is A7
                 ax7:  register field X of instruction is A7
                 ay7:  register field Y of instruction is A7
                 axy7: register fields X and Y of instruction are A7

bit pattern: Pattern to recognize this opcode.  "." means don't care.

allowed ea:  List of allowed addressing modes:
                 .: not present
                 A: address register indirect
                 +: ARI (address register indirect) with postincrement
                 -: ARI with predecrement
                 D: ARI with displacement
                 X: ARI with index
                 W: absolute word address
                 L: absolute long address
                 d: program counter indirect with displacement
                 x: program counter indirect with index
                 I: immediate
mode:        CPU operating mode for each cpu type.  U = user or supervisor,
             S = supervisor only, "." = opcode not present.

cpu cycles:  Base number of cycles required to execute this opcode on the
             specified CPU type.
             Use "." if CPU does not have this opcode.



              spec  spec                    allowed ea  mode       cpu cycles
name    size  proc   ea   bit pattern       A+-DXWLdxI  0 1 2 3 4  000 010 020 030 040  comments
======  ====  ====  ====  ================  ==========  = = = = =  === === === === === =============
M68KMAKE_TABLE_START
1010       0  .     .     1010............  ..........  U U U U U   4   4   4   4   4
1111       0  .     .     1111............  ..........  U U U U U   4   4   4   4   4
abcd       8  rr    .     1100...100000...  ..........  U U U U U   6   6   4   4   4
abcd       8  mm    ax7   1100111100001...  ..........  U U U U U  18  18  16  16  16
abcd       8  mm    ay7   1100...100001111  ..........  U U U U U  18  18  16  16  16
abcd       8  mm    axy7  1100111100001111  ..........  U U U U U  18  18  16  16  16
abcd       8  mm    .     1100...100001...  ..........  U U U U U  18  18  16  16  16
add        8  er    d     1101...000000...  ..........  U U U U U   4   4   2   2   2
add        8  er    .     1101...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
add       16  er    d     1101...001000...  ..........  U U U U U   4   4   2   2   2
add       16  er    a     1101...001001...  ..........  U U U U U   4   4   2   2   2
add       16  er    .     1101...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
add       32  er    d     1101...010000...  ..........  U U U U U   6   6   2   2   2
add       32  er    a     1101...010001...  ..........  U U U U U   6   6   2   2   2
add       32  er    .     1101...010......  A+-DXWLdxI  U U U U U   6   6   2   2   2
add        8  re    .     1101...100......  A+-DXWL...  U U U U U   8   8   4   4   4
add       16  re    .     1101...101......  A+-DXWL...  U U U U U   8   8   4   4   4
add       32  re    .     1101...110......  A+-DXWL...  U U U U U  12  12   4   4   4
adda      16  .     d     1101...011000...  ..........  U U U U U   8   8   2   2   2
adda      16  .     a     1101...011001...  ..........  U U U U U   8   8   2   2   2
adda      16  .     .     1101...011......  A+-DXWLdxI  U U U U U   8   8   2   2   2
adda      32  .     d     1101...111000...  ..........  U U U U U   6   6   2   2   2
adda      32  .     a     1101...111001...  ..........  U U U U U   6   6   2   2   2
adda      32  .     .     1101...111......  A+-DXWLdxI  U U U U U   6   6   2   2   2
addi       8  .     d     0000011000000...  ..........  U U U U U   8   8   2   2   2
addi       8  .     .     0000011000......  A+-DXWL...  U U U U U  12  12   4   4   4
addi      16  .     d     0000011001000...  ..........  U U U U U   8   8   2   2   2
addi      16  .     .     0000011001......  A+-DXWL...  U U U U U  12  12   4   4   4
addi      32  .     d     0000011010000...  ..........  U U U U U  16  14   2   2   2
addi      32  .     .     0000011010......  A+-DXWL...  U U U U U  20  20   4   4   4
addq       8  .     d     0101...000000...  ..........  U U U U U   4   4   2   2   2
addq       8  .     .     0101...000......  A+-DXWL...  U U U U U   8   8   4   4   4
addq      16  .     d     0101...001000...  ..........  U U U U U   4   4   2   2   2
addq      16  .     a     0101...001001...  ..........  U U U U U   4   4   2   2   2
addq      16  .     .     0101...001......  A+-DXWL...  U U U U U   8   8   4   4   4
addq      32  .     d     0101...010000...  ..........  U U U U U   8   8   2   2   2
addq      32  .     a     0101...010001...  ..........  U U U U U   8   8   2   2   2
addq      32  .     .     0101...010......  A+-DXWL...  U U U U U  12  12   4   4   4
addx       8  rr    .     1101...100000...  ..........  U U U U U   4   4   2   2   2
addx      16  rr    .     1101...101000...  ..........  U U U U U   4   4   2   2   2
addx      32  rr    .     1101...110000...  ..........  U U U U U   8   6   2   2   2
addx       8  mm    ax7   1101111100001...  ..........  U U U U U  18  18  12  12  12
addx       8  mm    ay7   1101...100001111  ..........  U U U U U  18  18  12  12  12
addx       8  mm    axy7  1101111100001111  ..........  U U U U U  18  18  12  12  12
addx       8  mm    .     1101...100001...  ..........  U U U U U  18  18  12  12  12
addx      16  mm    .     1101...101001...  ..........  U U U U U  18  18  12  12  12
addx      32  mm    .     1101...110001...  ..........  U U U U U  30  30  12  12  12
and        8  er    d     1100...000000...  ..........  U U U U U   4   4   2   2   2
and        8  er    .     1100...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
and       16  er    d     1100...001000...  ..........  U U U U U   4   4   2   2   2
and       16  er    .     1100...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
and       32  er    d     1100...010000...  ..........  U U U U U   6   6   2   2   2
and       32  er    .     1100...010......  A+-DXWLdxI  U U U U U   6   6   2   2   2
and        8  re    .     1100...100......  A+-DXWL...  U U U U U   8   8   4   4   4
and       16  re    .     1100...101......  A+-DXWL...  U U U U U   8   8   4   4   4
and       32  re    .     1100...110......  A+-DXWL...  U U U U U  12  12   4   4   4
andi      16  toc   .     0000001000111100  ..........  U U U U U  20  16  12  12  12
andi      16  tos   .     0000001001111100  ..........  S S S S S  20  16  12  12  12
andi       8  .     d     0000001000000...  ..........  U U U U U   8   8   2   2   2
andi       8  .     .     0000001000......  A+-DXWL...  U U U U U  12  12   4   4   4
andi      16  .     d     0000001001000...  ..........  U U U U U   8   8   2   2   2
andi      16  .     .     0000001001......  A+-DXWL...  U U U U U  12  12   4   4   4
andi      32  .     d     0000001010000...  ..........  U U U U U  14  14   2   2   2
andi      32  .     .     0000001010......  A+-DXWL...  U U U U U  20  20   4   4   4
asr        8  s     .     1110...000000...  ..........  U U U U U   6   6   6   6   6
asr       16  s     .     1110...001000...  ..........  U U U U U   6   6   6   6   6
asr       32  s     .     1110...010000...  ..........  U U U U U   8   8   6   6   6
asr        8  r     .     1110...000100...  ..........  U U U U U   6   6   6   6   6
asr       16  r     .     1110...001100...  ..........  U U U U U   6   6   6   6   6
asr       32  r     .     1110...010100...  ..........  U U U U U   8   8   6   6   6
asr       16  .     .     1110000011......  A+-DXWL...  U U U U U   8   8   5   5   5
asl        8  s     .     1110...100000...  ..........  U U U U U   6   6   8   8   8
asl       16  s     .     1110...101000...  ..........  U U U U U   6   6   8   8   8
asl       32  s     .     1110...110000...  ..........  U U U U U   8   8   8   8   8
asl        8  r     .     1110...100100...  ..........  U U U U U   6   6   8   8   8
asl       16  r     .     1110...101100...  ..........  U U U U U   6   6   8   8   8
asl       32  r     .     1110...110100...  ..........  U U U U U   8   8   8   8   8
asl       16  .     .     1110000111......  A+-DXWL...  U U U U U   8   8   6   6   6
bcc        8  .     .     0110............  ..........  U U U U U  10  10   6   6   6
bcc       16  .     .     0110....00000000  ..........  U U U U U  10  10   6   6   6
bcc       32  .     .     0110....11111111  ..........  U U U U U  10  10   6   6   6
bchg       8  r     .     0000...101......  A+-DXWL...  U U U U U   8   8   4   4   4
bchg      32  r     d     0000...101000...  ..........  U U U U U   8   8   4   4   4
bchg       8  s     .     0000100001......  A+-DXWL...  U U U U U  12  12   4   4   4
bchg      32  s     d     0000100001000...  ..........  U U U U U  12  12   4   4   4
bclr       8  r     .     0000...110......  A+-DXWL...  U U U U U   8  10   4   4   4
bclr      32  r     d     0000...110000...  ..........  U U U U U  10  10   4   4   4
bclr       8  s     .     0000100010......  A+-DXWL...  U U U U U  12  12   4   4   4
bclr      32  s     d     0000100010000...  ..........  U U U U U  14  14   4   4   4
bra        8  .     .     01100000........  ..........  U U U U U  10  10  10  10  10
bra       16  .     .     0110000000000000  ..........  U U U U U  10  10  10  10  10
bra       32  .     .     0110000011111111  ..........  U U U U U  10  10  10  10  10
bset      32  r     d     0000...111000...  ..........  U U U U U   8   8   4   4   4
bset       8  r     .     0000...111......  A+-DXWL...  U U U U U   8   8   4   4   4
bset       8  s     .     0000100011......  A+-DXWL...  U U U U U  12  12   4   4   4
bset      32  s     d     0000100011000...  ..........  U U U U U  12  12   4   4   4
bsr        8  .     .     01100001........  ..........  U U U U U  18  18   7   7   7
bsr       16  .     .     0110000100000000  ..........  U U U U U  18  18   7   7   7
bsr       32  .     .     0110000111111111  ..........  U U U U U  18  18   7   7   7
btst       8  r     .     0000...100......  A+-DXWLdxI  U U U U U   4   4   4   4   4
btst      32  r     d     0000...100000...  ..........  U U U U U   6   6   4   4   4
btst       8  s     .     0000100000......  A+-DXWLdx.  U U U U U   8   8   4   4   4
btst      32  s     d     0000100000000...  ..........  U U U U U  10  10   4   4   4
chk       16  .     d     0100...110000...  ..........  U U U U U  10   8   8   8   8
chk       16  .     .     0100...110......  A+-DXWLdxI  U U U U U  10   8   8   8   8
clr        8  .     d     0100001000000...  ..........  U U U U U   4   4   2   2   2
clr        8  .     .     0100001000......  A+-DXWL...  U U U U U   8   4   4   4   4
clr       16  .     d     0100001001000...  ..........  U U U U U   4   4   2   2   2
clr       16  .     .     0100001001......  A+-DXWL...  U U U U U   8   4   4   4   4
clr       32  .     d     0100001010000...  ..........  U U U U U   6   6   2   2   2
clr       32  .     .     0100001010......  A+-DXWL...  U U U U U  12   6   4   4   4
cmp        8  .     d     1011...000000...  ..........  U U U U U   4   4   2   2   2
cmp        8  .     .     1011...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
cmp       16  .     d     1011...001000...  ..........  U U U U U   4   4   2   2   2
cmp       16  .     a     1011...001001...  ..........  U U U U U   4   4   2   2   2
cmp       16  .     .     1011...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
cmp       32  .     d     1011...010000...  ..........  U U U U U   6   6   2   2   2
cmp       32  .     a     1011...010001...  ..........  U U U U U   6   6   2   2   2
cmp       32  .     .     1011...010......  A+-DXWLdxI  U U U U U   6   6   2   2   2
cmpa      16  .     d     1011...011000...  ..........  U U U U U   6   6   4   4   4
cmpa      16  .     a     1011...011001...  ..........  U U U U U   6   6   4   4   4
cmpa      16  .     .     1011...011......  A+-DXWLdxI  U U U U U   6   6   4   4   4
cmpa      32  .     d     1011...111000...  ..........  U U U U U   6   6   4   4   4
cmpa      32  .     a     1011...111001...  ..........  U U U U U   6   6   4   4   4
cmpa      32  .     .     1011...111......  A+-DXWLdxI  U U U U U   6   6   4   4   4
cmpi       8  .     d     0000110000000...  ..........  U U U U U   8   8   2   2   2
cmpi       8  .     .     0000110000......  A+-DXWL...  U U U U U   8   8   2   2   2
cmpi      16  .     d     0000110001000...  ..........  U U U U U   8   8   2   2   2
cmpi      16  .     .     0000110001......  A+-DXWL...  U U U U U   8   8   2   2   2
cmpi      32  .     d     0000110010000...  ..........  U U U U U  14  12   2   2   2
cmpi      32  .     .     0000110010......  A+-DXWL...  U U U U U  12  12   2   2   2
cmpm       8  .     ax7   1011111100001...  ..........  U U U U U  12  12   9   9   9
cmpm       8  .     ay7   1011...100001111  ..........  U U U U U  12  12   9   9   9
cmpm       8  .     axy7  1011111100001111  ..........  U U U U U  12  12   9   9   9
cmpm       8  .     .     1011...100001...  ..........  U U U U U  12  12   9   9   9
cmpm      16  .     .     1011...101001...  ..........  U U U U U  12  12   9   9   9
cmpm      32  .     .     1011...110001...  ..........  U U U U U  20  20   9   9   9
dbt       16  .     .     0101000011001...  ..........  U U U U U  12  12   6   6   6
dbf       16  .     .     0101000111001...  ..........  U U U U U  12  12   6   6   6
dbcc      16  .     .     0101....11001...  ..........  U U U U U  12  12   6   6   6
divs      16  .     d     1000...111000...  ..........  U U U U U 158 122  56  56  56
divs      16  .     .     1000...111......  A+-DXWLdxI  U U U U U 158 122  56  56  56
divu      16  .     d     1000...011000...  ..........  U U U U U 140 108  44  44  44
divu      16  .     .     1000...011......  A+-DXWLdxI  U U U U U 140 108  44  44  44
eor        8  .     d     1011...100000...  ..........  U U U U U   4   4   2   2   2
eor        8  .     .     1011...100......  A+-DXWL...  U U U U U   8   8   4   4   4
eor       16  .     d     1011...101000...  ..........  U U U U U   4   4   2   2   2
eor       16  .     .     1011...101......  A+-DXWL...  U U U U U   8   8   4   4   4
eor       32  .     d     1011...110000...  ..........  U U U U U   8   6   2   2   2
eor       32  .     .     1011...110......  A+-DXWL...  U U U U U  12  12   4   4   4
eori      16  toc   .     0000101000111100  ..........  U U U U U  20  16  12  12  12
eori      16  tos   .     0000101001111100  ..........  S S S S S  20  16  12  12  12
eori       8  .     d     0000101000000...  ..........  U U U U U   8   8   2   2   2
eori       8  .     .     0000101000......  A+-DXWL...  U U U U U  12  12   4   4   4
eori      16  .     d     0000101001000...  ..........  U U U U U   8   8   2   2   2
eori      16  .     .     0000101001......  A+-DXWL...  U U U U U  12  12   4   4   4
eori      32  .     d     0000101010000...  ..........  U U U U U  16  14   2   2   2
eori      32  .     .     0000101010......  A+-DXWL...  U U U U U  20  20   4   4   4
exg       32  dd    .     1100...101000...  ..........  U U U U U   6   6   2   2   2
exg       32  aa    .     1100...101001...  ..........  U U U U U   6   6   2   2   2
exg       32  da    .     1100...110001...  ..........  U U U U U   6   6   2   2   2
ext       16  .     .     0100100010000...  ..........  U U U U U   4   4   4   4   4
ext       32  .     .     0100100011000...  ..........  U U U U U   4   4   4   4   4
illegal    0  .     .     0100101011111100  ..........  U U U U U   4   4   4   4   4
jmp       32  .     .     0100111011......  A..DXWLdx.  U U U U U   4   4   0   0   0
jsr       32  .     .     0100111010......  A..DXWLdx.  U U U U U  12  12   0   0   0
lea       32  .     .     0100...111......  A..DXWLdx.  U U U U U   0   0   2   2   2
link      16  .     a7    0100111001010111  ..........  U U U U U  16  16   5   5   5
link      16  .     .     0100111001010...  ..........  U U U U U  16  16   5   5   5
lsr        8  s     .     1110...000001...  ..........  U U U U U   6   6   4   4   4
lsr       16  s     .     1110...001001...  ..........  U U U U U   6   6   4   4   4
lsr       32  s     .     1110...010001...  ..........  U U U U U   8   8   4   4   4
lsr        8  r     .     1110...000101...  ..........  U U U U U   6   6   6   6   6
lsr       16  r     .     1110...001101...  ..........  U U U U U   6   6   6   6   6
lsr       32  r     .     1110...010101...  ..........  U U U U U   8   8   6   6   6
lsr       16  .     .     1110001011......  A+-DXWL...  U U U U U   8   8   5   5   5
lsl        8  s     .     1110...100001...  ..........  U U U U U   6   6   4   4   4
lsl       16  s     .     1110...101001...  ..........  U U U U U   6   6   4   4   4
lsl       32  s     .     1110...110001...  ..........  U U U U U   8   8   4   4   4
lsl        8  r     .     1110...100101...  ..........  U U U U U   6   6   6   6   6
lsl       16  r     .     1110...101101...  ..........  U U U U U   6   6   6   6   6
lsl       32  r     .     1110...110101...  ..........  U U U U U   8   8   6   6   6
lsl       16  .     .     1110001111......  A+-DXWL...  U U U U U   8   8   5   5   5
move       8  d     d     0001...000000...  ..........  U U U U U   4   4   2   2   2
move       8  d     .     0001...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
move       8  ai    d     0001...010000...  ..........  U U U U U   8   8   4   4   4
move       8  ai    .     0001...010......  A+-DXWLdxI  U U U U U   8   8   4   4   4
move       8  pi    d     0001...011000...  ..........  U U U U U   8   8   4   4   4
move       8  pi    .     0001...011......  A+-DXWLdxI  U U U U U   8   8   4   4   4
move       8  pi7   d     0001111011000...  ..........  U U U U U   8   8   4   4   4
move       8  pi7   .     0001111011......  A+-DXWLdxI  U U U U U   8   8   4   4   4
move       8  pd    d     0001...100000...  ..........  U U U U U   8   8   5   5   5
move       8  pd    .     0001...100......  A+-DXWLdxI  U U U U U   8   8   5   5   5
move       8  pd7   d     0001111100000...  ..........  U U U U U   8   8   5   5   5
move       8  pd7   .     0001111100......  A+-DXWLdxI  U U U U U   8   8   5   5   5
move       8  di    d     0001...101000...  ..........  U U U U U  12  12   5   5   5
move       8  di    .     0001...101......  A+-DXWLdxI  U U U U U  12  12   5   5   5
move       8  ix    d     0001...110000...  ..........  U U U U U  14  14   7   7   7
move       8  ix    .     0001...110......  A+-DXWLdxI  U U U U U  14  14   7   7   7
move       8  aw    d     0001000111000...  ..........  U U U U U  12  12   4   4   4
move       8  aw    .     0001000111......  A+-DXWLdxI  U U U U U  12  12   4   4   4
move       8  al    d     0001001111000...  ..........  U U U U U  16  16   6   6   6
move       8  al    .     0001001111......  A+-DXWLdxI  U U U U U  16  16   6   6   6
move      16  d     d     0011...000000...  ..........  U U U U U   4   4   2   2   2
move      16  d     a     0011...000001...  ..........  U U U U U   4   4   2   2   2
move      16  d     .     0011...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
move      16  ai    d     0011...010000...  ..........  U U U U U   8   8   4   4   4
move      16  ai    a     0011...010001...  ..........  U U U U U   8   8   4   4   4
move      16  ai    .     0011...010......  A+-DXWLdxI  U U U U U   8   8   4   4   4
move      16  pi    d     0011...011000...  ..........  U U U U U   8   8   4   4   4
move      16  pi    a     0011...011001...  ..........  U U U U U   8   8   4   4   4
move      16  pi    .     0011...011......  A+-DXWLdxI  U U U U U   8   8   4   4   4
move      16  pd    d     0011...100000...  ..........  U U U U U   8   8   5   5   5
move      16  pd    a     0011...100001...  ..........  U U U U U   8   8   5   5   5
move      16  pd    .     0011...100......  A+-DXWLdxI  U U U U U   8   8   5   5   5
move      16  di    d     0011...101000...  ..........  U U U U U  12  12   5   5   5
move      16  di    a     0011...101001...  ..........  U U U U U  12  12   5   5   5
move      16  di    .     0011...101......  A+-DXWLdxI  U U U U U  12  12   5   5   5
move      16  ix    d     0011...110000...  ..........  U U U U U  14  14   7   7   7
move      16  ix    a     0011...110001...  ..........  U U U U U  14  14   7   7   7
move      16  ix    .     0011...110......  A+-DXWLdxI  U U U U U  14  14   7   7   7
move      16  aw    d     0011000111000...  ..........  U U U U U  12  12   4   4   4
move      16  aw    a     0011000111001...  ..........  U U U U U  12  12   4   4   4
move      16  aw    .     0011000111......  A+-DXWLdxI  U U U U U  12  12   4   4   4
move      16  al    d     0011001111000...  ..........  U U U U U  16  16   6   6   6
move      16  al    a     0011001111001...  ..........  U U U U U  16  16   6   6   6
move      16  al    .     0011001111......  A+-DXWLdxI  U U U U U  16  16   6   6   6
move      32  d     d     0010...000000...  ..........  U U U U U   4   4   2   2   2
move      32  d     a     0010...000001...  ..........  U U U U U   4   4   2   2   2
move      32  d     .     0010...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
move      32  ai    d     0010...010000...  ..........  U U U U U  12  12   4   4   4
move      32  ai    a     0010...010001...  ..........  U U U U U  12  12   4   4   4
move      32  ai    .     0010...010......  A+-DXWLdxI  U U U U U  12  12   4   4   4
move      32  pi    d     0010...011000...  ..........  U U U U U  12  12   4   4   4
move      32  pi    a     0010...011001...  ..........  U U U U U  12  12   4   4   4
move      32  pi    .     0010...011......  A+-DXWLdxI  U U U U U  12  12   4   4   4
move      32  pd    d     0010...100000...  ..........  U U U U U  12  14   5   5   5
move      32  pd    a     0010...100001...  ..........  U U U U U  12  14   5   5   5
move      32  pd    .     0010...100......  A+-DXWLdxI  U U U U U  12  14   5   5   5
move      32  di    d     0010...101000...  ..........  U U U U U  16  16   5   5   5
move      32  di    a     0010...101001...  ..........  U U U U U  16  16   5   5   5
move      32  di    .     0010...101......  A+-DXWLdxI  U U U U U  16  16   5   5   5
move      32  ix    d     0010...110000...  ..........  U U U U U  18  18   7   7   7
move      32  ix    a     0010...110001...  ..........  U U U U U  18  18   7   7   7
move      32  ix    .     0010...110......  A+-DXWLdxI  U U U U U  18  18   7   7   7
move      32  aw    d     0010000111000...  ..........  U U U U U  16  16   4   4   4
move      32  aw    a     0010000111001...  ..........  U U U U U  16  16   4   4   4
move      32  aw    .     0010000111......  A+-DXWLdxI  U U U U U  16  16   4   4   4
move      32  al    d     0010001111000...  ..........  U U U U U  20  20   6   6   6
move      32  al    a     0010001111001...  ..........  U U U U U  20  20   6   6   6
move      32  al    .     0010001111......  A+-DXWLdxI  U U U U U  20  20   6   6   6
movea     16  .     d     0011...001000...  ..........  U U U U U   4   4   2   2   2
movea     16  .     a     0011...001001...  ..........  U U U U U   4   4   2   2   2
movea     16  .     .     0011...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
movea     32  .     d     0010...001000...  ..........  U U U U U   4   4   2   2   2
movea     32  .     a     0010...001001...  ..........  U U U U U   4   4   2   2   2
movea     32  .     .     0010...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
move      16  toc   d     0100010011000...  ..........  U U U U U  12  12   4   4   4
move      16  toc   .     0100010011......  A+-DXWLdxI  U U U U U  12  12   4   4   4
move      16  frs   d     0100000011000...  ..........  U S S S S   6   4   8   8   8  U only for 000
move      16  frs   .     0100000011......  A+-DXWL...  U S S S S   8   8   8   8   8  U only for 000
move      16  tos   d     0100011011000...  ..........  S S S S S  12  12   8   8   8
move      16  tos   .     0100011011......  A+-DXWLdxI  S S S S S  12  12   8   8   8
move      32  fru   .     0100111001101...  ..........  S S S S S   4   6   2   2   2
move      32  tou   .     0100111001100...  ..........  S S S S S   4   6   2   2   2
movem     16  re    pd    0100100010100...  ..........  U U U U U   8   8   4   4   4
movem     16  re    .     0100100010......  A..DXWL...  U U U U U   8   8   4   4   4
movem     32  re    pd    0100100011100...  ..........  U U U U U   8   8   4   4   4
movem     32  re    .     0100100011......  A..DXWL...  U U U U U   8   8   4   4   4
movem     16  er    pi    0100110010011...  ..........  U U U U U  12  12   8   8   8
movem     16  er    pcdi  0100110010111010  ..........  U U U U U  16  16   9   9   9
movem     16  er    pcix  0100110010111011  ..........  U U U U U  18  18  11  11  11
movem     16  er    .     0100110010......  A..DXWL...  U U U U U  12  12   8   8   8
movem     32  er    pi    0100110011011...  ..........  U U U U U  12  12   8   8   8
movem     32  er    pcdi  0100110011111010  ..........  U U U U U  16  16   9   9   9
movem     32  er    pcix  0100110011111011  ..........  U U U U U  18  18  11  11  11
movem     32  er    .     0100110011......  A..DXWL...  U U U U U  12  12   8   8   8
movep     16  er    .     0000...100001...  ..........  U U U U U  16  16  12  12  12
movep     32  er    .     0000...101001...  ..........  U U U U U  24  24  18  18  18
movep     16  re    .     0000...110001...  ..........  U U U U U  16  16  11  11  11
movep     32  re    .     0000...111001...  ..........  U U U U U  24  24  17  17  17
moveq     32  .     .     0111...0........  ..........  U U U U U   4   4   2   2   2
muls      16  .     d     1100...111000...  ..........  U U U U U  54  32  27  27  27
muls      16  .     .     1100...111......  A+-DXWLdxI  U U U U U  54  32  27  27  27
mulu      16  .     d     1100...011000...  ..........  U U U U U  54  30  27  27  27
mulu      16  .     .     1100...011......  A+-DXWLdxI  U U U U U  54  30  27  27  27
nbcd       8  .     d     0100100000000...  ..........  U U U U U   6   6   6   6   6
nbcd       8  .     .     0100100000......  A+-DXWL...  U U U U U   8   8   6   6   6
neg        8  .     d     0100010000000...  ..........  U U U U U   4   4   2   2   2
neg        8  .     .     0100010000......  A+-DXWL...  U U U U U   8   8   4   4   4
neg       16  .     d     0100010001000...  ..........  U U U U U   4   4   2   2   2
neg       16  .     .     0100010001......  A+-DXWL...  U U U U U   8   8   4   4   4
neg       32  .     d     0100010010000...  ..........  U U U U U   6   6   2   2   2
neg       32  .     .     0100010010......  A+-DXWL...  U U U U U  12  12   4   4   4
negx       8  .     d     0100000000000...  ..........  U U U U U   4   4   2   2   2
negx       8  .     .     0100000000......  A+-DXWL...  U U U U U   8   8   4   4   4
negx      16  .     d     0100000001000...  ..........  U U U U U   4   4   2   2   2
negx      16  .     .     0100000001......  A+-DXWL...  U U U U U   8   8   4   4   4
negx      32  .     d     0100000010000...  ..........  U U U U U   6   6   2   2   2
negx      32  .     .     0100000010......  A+-DXWL...  U U U U U  12  12   4   4   4
nop        0  .     .     0100111001110001  ..........  U U U U U   4   4   2   2   2
not        8  .     d     0100011000000...  ..........  U U U U U   4   4   2   2   2
not        8  .     .     0100011000......  A+-DXWL...  U U U U U   8   8   4   4   4
not       16  .     d     0100011001000...  ..........  U U U U U   4   4   2   2   2
not       16  .     .     0100011001......  A+-DXWL...  U U U U U   8   8   4   4   4
not       32  .     d     0100011010000...  ..........  U U U U U   6   6   2   2   2
not       32  .     .     0100011010......  A+-DXWL...  U U U U U  12  12   4   4   4
or         8  er    d     1000...000000...  ..........  U U U U U   4   4   2   2   2
or         8  er    .     1000...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
or        16  er    d     1000...001000...  ..........  U U U U U   4   4   2   2   2
or        16  er    .     1000...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
or        32  er    d     1000...010000...  ..........  U U U U U   6   6   2   2   2
or        32  er    .     1000...010......  A+-DXWLdxI  U U U U U   6   6   2   2   2
or         8  re    .     1000...100......  A+-DXWL...  U U U U U   8   8   4   4   4
or        16  re    .     1000...101......  A+-DXWL...  U U U U U   8   8   4   4   4
or        32  re    .     1000...110......  A+-DXWL...  U U U U U  12  12   4   4   4
ori       16  toc   .     0000000000111100  ..........  U U U U U  20  16  12  12  12
ori       16  tos   .     0000000001111100  ..........  S S S S S  20  16  12  12  12
ori        8  .     d     0000000000000...  ..........  U U U U U   8   8   2   2   2
ori        8  .     .     0000000000......  A+-DXWL...  U U U U U  12  12   4   4   4
ori       16  .     d     0000000001000...  ..........  U U U U U   8   8   2   2   2
ori       16  .     .     0000000001......  A+-DXWL...  U U U U U  12  12   4   4   4
ori       32  .     d     0000000010000...  ..........  U U U U U  16  14   2   2   2
ori       32  .     .     0000000010......  A+-DXWL...  U U U U U  20  20   4   4   4
pea       32  .     .     0100100001......  A..DXWLdx.  U U U U U   6   6   5   5   5
reset      0  .     .     0100111001110000  ..........  S S S S S   0   0   0   0   0
ror        8  s     .     1110...000011...  ..........  U U U U U   6   6   8   8   8
ror       16  s     .     1110...001011...  ..........  U U U U U   6   6   8   8   8
ror       32  s     .     1110...010011...  ..........  U U U U U   8   8   8   8   8
ror        8  r     .     1110...000111...  ..........  U U U U U   6   6   8   8   8
ror       16  r     .     1110...001111...  ..........  U U U U U   6   6   8   8   8
ror       32  r     .     1110...010111...  ..........  U U U U U   8   8   8   8   8
ror       16  .     .     1110011011......  A+-DXWL...  U U U U U   8   8   7   7   7
rol        8  s     .     1110...100011...  ..........  U U U U U   6   6   8   8   8
rol       16  s     .     1110...101011...  ..........  U U U U U   6   6   8   8   8
rol       32  s     .     1110...110011...  ..........  U U U U U   8   8   8   8   8
rol        8  r     .     1110...100111...  ..........  U U U U U   6   6   8   8   8
rol       16  r     .     1110...101111...  ..........  U U U U U   6   6   8   8   8
rol       32  r     .     1110...110111...  ..........  U U U U U   8   8   8   8   8
rol       16  .     .     1110011111......  A+-DXWL...  U U U U U   8   8   7   7   7
roxr       8  s     .     1110...000010...  ..........  U U U U U   6   6  12  12  12
roxr      16  s     .     1110...001010...  ..........  U U U U U   6   6  12  12  12
roxr      32  s     .     1110...010010...  ..........  U U U U U   8   8  12  12  12
roxr       8  r     .     1110...000110...  ..........  U U U U U   6   6  12  12  12
roxr      16  r     .     1110...001110...  ..........  U U U U U   6   6  12  12  12
roxr      32  r     .     1110...010110...  ..........  U U U U U   8   8  12  12  12
roxr      16  .     .     1110010011......  A+-DXWL...  U U U U U   8   8   5   5   5
roxl       8  s     .     1110...100010...  ..........  U U U U U   6   6  12  12  12
roxl      16  s     .     1110...101010...  ..........  U U U U U   6   6  12  12  12
roxl      32  s     .     1110...110010...  ..........  U U U U U   8   8  12  12  12
roxl       8  r     .     1110...100110...  ..........  U U U U U   6   6  12  12  12
roxl      16  r     .     1110...101110...  ..........  U U U U U   6   6  12  12  12
roxl      32  r     .     1110...110110...  ..........  U U U U U   8   8  12  12  12
roxl      16  .     .     1110010111......  A+-DXWL...  U U U U U   8   8   5   5   5
rte       32  .     .     0100111001110011  ..........  S S S S S  20  24  20  20  20  bus fault not emulated
rtr       32  .     .     0100111001110111  ..........  U U U U U  20  20  14  14  14
rts       32  .     .     0100111001110101  ..........  U U U U U  16  16  10  10  10
sbcd       8  rr    .     1000...100000...  ..........  U U U U U   6   6   4   4   4
sbcd       8  mm    ax7   1000111100001...  ..........  U U U U U  18  18  16  16  16
sbcd       8  mm    ay7   1000...100001111  ..........  U U U U U  18  18  16  16  16
sbcd       8  mm    axy7  1000111100001111  ..........  U U U U U  18  18  16  16  16
sbcd       8  mm    .     1000...100001...  ..........  U U U U U  18  18  16  16  16
st         8  .     d     0101000011000...  ..........  U U U U U   6   4   4   4   4
st         8  .     .     0101000011......  A+-DXWL...  U U U U U   8   8   6   6   6
sf         8  .     d     0101000111000...  ..........  U U U U U   4   4   4   4   4
sf         8  .     .     0101000111......  A+-DXWL...  U U U U U   8   8   6   6   6
scc        8  .     d     0101....11000...  ..........  U U U U U   4   4   4   4   4
scc        8  .     .     0101....11......  A+-DXWL...  U U U U U   8   8   6   6   6
stop       0  .     .     0100111001110010  ..........  S S S S S   4   4   8   8   8
sub        8  er    d     1001...000000...  ..........  U U U U U   4   4   2   2   2
sub        8  er    .     1001...000......  A+-DXWLdxI  U U U U U   4   4   2   2   2
sub       16  er    d     1001...001000...  ..........  U U U U U   4   4   2   2   2
sub       16  er    a     1001...001001...  ..........  U U U U U   4   4   2   2   2
sub       16  er    .     1001...001......  A+-DXWLdxI  U U U U U   4   4   2   2   2
sub       32  er    d     1001...010000...  ..........  U U U U U   6   6   2   2   2
sub       32  er    a     1001...010001...  ..........  U U U U U   6   6   2   2   2
sub       32  er    .     1001...010......  A+-DXWLdxI  U U U U U   6   6   2   2   2
sub        8  re    .     1001...100......  A+-DXWL...  U U U U U   8   8   4   4   4
sub       16  re    .     1001...101......  A+-DXWL...  U U U U U   8   8   4   4   4
sub       32  re    .     1001...110......  A+-DXWL...  U U U U U  12  12   4   4   4
suba      16  .     d     1001...011000...  ..........  U U U U U   8   8   2   2   2
suba      16  .     a     1001...011001...  ..........  U U U U U   8   8   2   2   2
suba      16  .     .     1001...011......  A+-DXWLdxI  U U U U U   8   8   2   2   2
suba      32  .     d     1001...111000...  ..........  U U U U U   6   6   2   2   2
suba      32  .     a     1001...111001...  ..........  U U U U U   6   6   2   2   2
suba      32  .     .     1001...111......  A+-DXWLdxI  U U U U U   6   6   2   2   2
subi       8  .     d     0000010000000...  ..........  U U U U U   8   8   2   2   2
subi       8  .     .     0000010000......  A+-DXWL...  U U U U U  12  12   4   4   4
subi      16  .     d     0000010001000...  ..........  U U U U U   8   8   2   2   2
subi      16  .     .     0000010001......  A+-DXWL...  U U U U U  12  12   4   4   4
subi      32  .     d     0000010010000...  ..........  U U U U U  16  14   2   2   2
subi      32  .     .     0000010010......  A+-DXWL...  U U U U U  20  20   4   4   4
subq       8  .     d     0101...100000...  ..........  U U U U U   4   4   2   2   2
subq       8  .     .     0101...100......  A+-DXWL...  U U U U U   8   8   4   4   4
subq      16  .     d     0101...101000...  ..........  U U U U U   4   4   2   2   2
subq      16  .     a     0101...101001...  ..........  U U U U U   8   4   2   2   2
subq      16  .     .     0101...101......  A+-DXWL...  U U U U U   8   8   4   4   4
subq      32  .     d     0101...110000...  ..........  U U U U U   8   8   2   2   2
subq      32  .     a     0101...110001...  ..........  U U U U U   8   8   2   2   2
subq      32  .     .     0101...110......  A+-DXWL...  U U U U U  12  12   4   4   4
subx       8  rr    .     1001...100000...  ..........  U U U U U   4   4   2   2   2
subx      16  rr    .     1001...101000...  ..........  U U U U U   4   4   2   2   2
subx      32  rr    .     1001...110000...  ..........  U U U U U   8   6   2   2   2
subx       8  mm    ax7   1001111100001...  ..........  U U U U U  18  18  12  12  12
subx       8  mm    ay7   1001...100001111  ..........  U U U U U  18  18  12  12  12
subx       8  mm    axy7  1001111100001111  ..........  U U U U U  18  18  12  12  12
subx       8  mm    .     1001...100001...  ..........  U U U U U  18  18  12  12  12
subx      16  mm    .     1001...101001...  ..........  U U U U U  18  18  12  12  12
subx      32  mm    .     1001...110001...  ..........  U U U U U  30  30  12  12  12
swap      32  .     .     0100100001000...  ..........  U U U U U   4   4   4   4   4
tas        8  .     d     0100101011000...  ..........  U U U U U   4   4   4   4   4
tas        8  .     .     0100101011......  A+-DXWL...  U U U U U  14  14  12  12  12
trap       0  .     .     010011100100....  ..........  U U U U U   4   4   4   4   4
trapv      0  .     .     0100111001110110  ..........  U U U U U   4   4   4   4   4
tst        8  .     d     0100101000000...  ..........  U U U U U   4   4   2   2   2
tst        8  .     .     0100101000......  A+-DXWL...  U U U U U   4   4   2   2   2
tst       16  .     d     0100101001000...  ..........  U U U U U   4   4   2   2   2
tst       16  .     .     0100101001......  A+-DXWL...  U U U U U   4   4   2   2   2
tst       32  .     d     0100101010000...  ..........  U U U U U   4   4   2   2   2
tst       32  .     .     0100101010......  A+-DXWL...  U U U U U   4   4   2   2   2
unlk      32  .     a7    0100111001011111  ..........  U U U U U  12  12   6   6   6
unlk      32  .     .     0100111001011...  ..........  U U U U U  12  12   6   6   6



XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_OPCODE_HANDLER_BODY

M68KMAKE_OP(1010, 0, ., .)
{
	m68ki_exception_1010();
}


M68KMAKE_OP(1111, 0, ., .)
{
	m68ki_exception_1111();
}


M68KMAKE_OP(abcd, 8, rr, .)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


M68KMAKE_OP(abcd, 8, mm, ax7)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(abcd, 8, mm, ay7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(abcd, 8, mm, axy7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(abcd, 8, mm, .)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(add, 8, er, d)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(add, 8, er, .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_8;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(add, 16, er, d)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(add, 16, er, a)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(add, 16, er, .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_16;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(add, 32, er, d)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(add, 32, er, a)
{
	uint* r_dst = &DX;
	uint src = AY;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(add, 32, er, .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_32;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(add, 8, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(add, 16, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(add, 32, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(adda, 16, ., d)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY));
}


M68KMAKE_OP(adda, 16, ., a)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY));
}


M68KMAKE_OP(adda, 16, ., .)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(M68KMAKE_GET_OPER_AY_16);

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


M68KMAKE_OP(adda, 32, ., d)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + DY);
}


M68KMAKE_OP(adda, 32, ., a)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + AY);
}


M68KMAKE_OP(adda, 32, ., .)
{
	uint src = M68KMAKE_GET_OPER_AY_32;
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


M68KMAKE_OP(addi, 8, ., d)
{
	uint* r_dst = &DY;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(addi, 8, ., .)
{
	uint src = OPER_I_8();
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(addi, 16, ., d)
{
	uint* r_dst = &DY;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(addi, 16, ., .)
{
	uint src = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(addi, 32, ., d)
{
	uint* r_dst = &DY;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(addi, 32, ., .)
{
	uint src = OPER_I_32();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(addq, 8, ., d)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(addq, 8, ., .)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(addq, 16, ., d)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(addq, 16, ., a)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


M68KMAKE_OP(addq, 16, ., .)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(addq, 32, ., d)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(addq, 32, ., a)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


M68KMAKE_OP(addq, 32, ., .)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(addx, 8, rr, .)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


M68KMAKE_OP(addx, 16, rr, .)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


M68KMAKE_OP(addx, 32, rr, .)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


M68KMAKE_OP(addx, 8, mm, ax7)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(addx, 8, mm, ay7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(addx, 8, mm, axy7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(addx, 8, mm, .)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(addx, 16, mm, .)
{
	uint src = OPER_AY_PD_16();
	uint ea  = EA_AX_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


M68KMAKE_OP(addx, 32, mm, .)
{
	uint src = OPER_AY_PD_32();
	uint ea  = EA_AX_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


M68KMAKE_OP(and, 8, er, d)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (DY | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(and, 8, er, .)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (M68KMAKE_GET_OPER_AY_8 | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(and, 16, er, d)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (DY | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(and, 16, er, .)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (M68KMAKE_GET_OPER_AY_16 | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(and, 32, er, d)
{
	FLAG_Z = DX &= DY;

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(and, 32, er, .)
{
	FLAG_Z = DX &= M68KMAKE_GET_OPER_AY_32;

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(and, 8, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(and, 16, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(and, 32, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


M68KMAKE_OP(andi, 8, ., d)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DY &= (OPER_I_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(andi, 8, ., .)
{
	uint src = OPER_I_8();
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(andi, 16, ., d)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DY &= (OPER_I_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(andi, 16, ., .)
{
	uint src = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


M68KMAKE_OP(andi, 32, ., d)
{
	FLAG_Z = DY &= (OPER_I_32());

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(andi, 32, ., .)
{
	uint src = OPER_I_32();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


M68KMAKE_OP(andi, 16, toc, .)
{
	m68ki_set_ccr(m68ki_get_ccr() & OPER_I_8());
}


M68KMAKE_OP(andi, 16, tos, .)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() & src);
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(asr, 8, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_8(src))
		res |= m68ki_shift_8_table[shift];

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


M68KMAKE_OP(asr, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_16(src))
		res |= m68ki_shift_16_table[shift];

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


M68KMAKE_OP(asr, 32, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_32(src))
		res |= m68ki_shift_32_table[shift];

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


M68KMAKE_OP(asr, 8, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 8)
		{
			if(GET_MSB_8(src))
				res |= m68ki_shift_8_table[shift];

			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

			FLAG_X = FLAG_C = src << (9-shift);
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_8(src))
		{
			*r_dst |= 0xff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(asr, 16, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 16)
		{
			if(GET_MSB_16(src))
				res |= m68ki_shift_16_table[shift];

			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_16(src))
		{
			*r_dst |= 0xffff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(asr, 32, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			if(GET_MSB_32(src))
				res |= m68ki_shift_32_table[shift];

			*r_dst = res;

			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_32(src))
		{
			*r_dst = 0xffffffff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(asr, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


M68KMAKE_OP(asl, 8, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_X = FLAG_C = src << shift;
	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	src &= m68ki_shift_8_table[shift + 1];
	FLAG_V = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;
}


M68KMAKE_OP(asl, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (8-shift);
	src &= m68ki_shift_16_table[shift + 1];
	FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
}


M68KMAKE_OP(asl, 32, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (24-shift);
	src &= m68ki_shift_32_table[shift + 1];
	FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
}


M68KMAKE_OP(asl, 8, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			src &= m68ki_shift_8_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_8_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = FLAG_C = ((shift == 8 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(asl, 16, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_X = FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			src &= m68ki_shift_16_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = FLAG_C = ((shift == 16 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(asl, 32, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_X = FLAG_C = (src >> (32 - shift)) << 8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			src &= m68ki_shift_32_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = ((shift == 32 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(asl, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


M68KMAKE_OP(bcc, 8, ., .)
{
	if(M68KMAKE_CC)
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


M68KMAKE_OP(bcc, 16, ., .)
{
	if(M68KMAKE_CC)
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


M68KMAKE_OP(bcc, 32, ., .)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(M68KMAKE_CC)
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	else
	{
		if(M68KMAKE_CC)
		{
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
			return;
		}
		USE_CYCLES(CYC_BCC_NOTAKE_B);
	}
}


M68KMAKE_OP(bchg, 32, r, d)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst ^= mask;
}


M68KMAKE_OP(bchg, 8, r, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


M68KMAKE_OP(bchg, 32, s, d)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst ^= mask;
}


M68KMAKE_OP(bchg, 8, s, .)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


M68KMAKE_OP(bclr, 32, r, d)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst &= ~mask;
}


M68KMAKE_OP(bclr, 8, r, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


M68KMAKE_OP(bclr, 32, s, d)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst &= ~mask;
}


M68KMAKE_OP(bclr, 8, s, .)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


M68KMAKE_OP(bra, 8, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


M68KMAKE_OP(bra, 16, ., .)
{
	uint offset = OPER_I_16();
	REG_PC -= 2;
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68ki_branch_16(offset);
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


M68KMAKE_OP(bra, 32, ., .)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint offset = OPER_I_32();
		REG_PC -= 4;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset);
		if(REG_PC == REG_PPC)
			USE_ALL_CYCLES();
		return;
	}
	else
	{
		m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		if(REG_PC == REG_PPC)
			USE_ALL_CYCLES();
	}
}


M68KMAKE_OP(bset, 32, r, d)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst |= mask;
}


M68KMAKE_OP(bset, 8, r, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


M68KMAKE_OP(bset, 32, s, d)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst |= mask;
}


M68KMAKE_OP(bset, 8, s, .)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


M68KMAKE_OP(bsr, 8, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


M68KMAKE_OP(bsr, 16, ., .)
{
	uint offset = OPER_I_16();
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	REG_PC -= 2;
	m68ki_branch_16(offset);
}


M68KMAKE_OP(bsr, 32, ., .)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint offset = OPER_I_32();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_push_32(REG_PC);
		REG_PC -= 4;
		m68ki_branch_32(offset);
		return;
	}
	else
	{
		m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
		m68ki_push_32(REG_PC);
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
	}
}


M68KMAKE_OP(btst, 32, r, d)
{
	FLAG_Z = DY & (1 << (DX & 0x1f));
}


M68KMAKE_OP(btst, 8, r, .)
{
	FLAG_Z = M68KMAKE_GET_OPER_AY_8 & (1 << (DX & 7));
}


M68KMAKE_OP(btst, 32, s, d)
{
	FLAG_Z = DY & (1 << (OPER_I_8() & 0x1f));
}


M68KMAKE_OP(btst, 8, s, .)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = M68KMAKE_GET_OPER_AY_8 & (1 << bit);
}


M68KMAKE_OP(chk, 16, ., d)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(DY);

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


M68KMAKE_OP(chk, 16, ., .)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(M68KMAKE_GET_OPER_AY_16);

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


M68KMAKE_OP(clr, 8, ., d)
{
	DY &= 0xffffff00;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


M68KMAKE_OP(clr, 8, ., .)
{
	m68ki_write_8(M68KMAKE_GET_EA_AY_8, 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


M68KMAKE_OP(clr, 16, ., d)
{
	DY &= 0xffff0000;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


M68KMAKE_OP(clr, 16, ., .)
{
	m68ki_write_16(M68KMAKE_GET_EA_AY_16, 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


M68KMAKE_OP(clr, 32, ., d)
{
	DY = 0;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


M68KMAKE_OP(clr, 32, ., .)
{
	m68ki_write_32(M68KMAKE_GET_EA_AY_32, 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


M68KMAKE_OP(cmp, 8, ., d)
{
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmp, 8, ., .)
{
	uint src = M68KMAKE_GET_OPER_AY_8;
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmp, 16, ., d)
{
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


M68KMAKE_OP(cmp, 16, ., a)
{
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


M68KMAKE_OP(cmp, 16, ., .)
{
	uint src = M68KMAKE_GET_OPER_AY_16;
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


M68KMAKE_OP(cmp, 32, ., d)
{
	uint src = DY;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmp, 32, ., a)
{
	uint src = AY;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmp, 32, ., .)
{
	uint src = M68KMAKE_GET_OPER_AY_32;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpa, 16, ., d)
{
	uint src = MAKE_INT_16(DY);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpa, 16, ., a)
{
	uint src = MAKE_INT_16(AY);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpa, 16, ., .)
{
	uint src = MAKE_INT_16(M68KMAKE_GET_OPER_AY_16);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpa, 32, ., d)
{
	uint src = DY;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpa, 32, ., a)
{
	uint src = AY;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpa, 32, ., .)
{
	uint src = M68KMAKE_GET_OPER_AY_32;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpi, 8, ., d)
{
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(DY);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmpi, 8, ., .)
{
	uint src = OPER_I_8();
	uint dst = M68KMAKE_GET_OPER_AY_8;
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmpi, 16, ., d)
{
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(DY);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


M68KMAKE_OP(cmpi, 16, ., .)
{
	uint src = OPER_I_16();
	uint dst = M68KMAKE_GET_OPER_AY_16;
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


M68KMAKE_OP(cmpi, 32, ., d)
{
	uint src = OPER_I_32();
	uint dst = DY;
	uint res = dst - src;

	m68ki_cmpild_callback(src, REG_IR & 7);		   /* auto-disable (see m68kcpu.h) */
	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpi, 32, ., .)
{
	uint src = OPER_I_32();
	uint dst = M68KMAKE_GET_OPER_AY_32;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(cmpm, 8, ., ax7)
{
	uint src = OPER_AY_PI_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmpm, 8, ., ay7)
{
	uint src = OPER_A7_PI_8();
	uint dst = OPER_AX_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmpm, 8, ., axy7)
{
	uint src = OPER_A7_PI_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmpm, 8, ., .)
{
	uint src = OPER_AY_PI_8();
	uint dst = OPER_AX_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


M68KMAKE_OP(cmpm, 16, ., .)
{
	uint src = OPER_AY_PI_16();
	uint dst = OPER_AX_PI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


M68KMAKE_OP(cmpm, 32, ., .)
{
	uint src = OPER_AY_PI_32();
	uint dst = OPER_AX_PI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


M68KMAKE_OP(dbt, 16, ., .)
{
	REG_PC += 2;
}


M68KMAKE_OP(dbf, 16, ., .)
{
	uint* r_dst = &DY;
	uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
	if(res != 0xffff)
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		USE_CYCLES(CYC_DBCC_F_NOEXP);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_DBCC_F_EXP);
}


M68KMAKE_OP(dbcc, 16, ., .)
{
	if(M68KMAKE_NOT_CC)
	{
		uint* r_dst = &DY;
		uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint offset = OPER_I_16();
			REG_PC -= 2;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			USE_CYCLES(CYC_DBCC_F_NOEXP);
			return;
		}
		REG_PC += 2;
		USE_CYCLES(CYC_DBCC_F_EXP);
		return;
	}
	REG_PC += 2;
}


M68KMAKE_OP(divs, 16, ., d)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(DY);
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


M68KMAKE_OP(divs, 16, ., .)
{
	uint* r_dst = &DX;
	sint src = MAKE_INT_16(M68KMAKE_GET_OPER_AY_16);
	sint quotient;
	sint remainder;

	if(src != 0)
	{
		if((uint32)*r_dst == 0x80000000 && src == -1)
		{
			FLAG_Z = 0;
			FLAG_N = NFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


M68KMAKE_OP(divu, 16, ., d)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


M68KMAKE_OP(divu, 16, ., .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_16;

	if(src != 0)
	{
		uint quotient = *r_dst / src;
		uint remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			FLAG_Z = quotient;
			FLAG_N = NFLAG_16(quotient);
			FLAG_V = VFLAG_CLEAR;
			FLAG_C = CFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		FLAG_V = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


M68KMAKE_OP(eor, 8, ., d)
{
	uint res = MASK_OUT_ABOVE_8(DY ^= MASK_OUT_ABOVE_8(DX));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eor, 8, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eor, 16, ., d)
{
	uint res = MASK_OUT_ABOVE_16(DY ^= MASK_OUT_ABOVE_16(DX));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eor, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eor, 32, ., d)
{
	uint res = DY ^= DX;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eor, 32, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = DX ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 8, ., d)
{
	uint res = MASK_OUT_ABOVE_8(DY ^= OPER_I_8());

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 8, ., .)
{
	uint src = OPER_I_8();
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 16, ., d)
{
	uint res = MASK_OUT_ABOVE_16(DY ^= OPER_I_16());

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 16, ., .)
{
	uint src = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 32, ., d)
{
	uint res = DY ^= OPER_I_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 32, ., .)
{
	uint src = OPER_I_32();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(eori, 16, toc, .)
{
	m68ki_set_ccr(m68ki_get_ccr() ^ OPER_I_8());
}


M68KMAKE_OP(eori, 16, tos, .)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() ^ src);
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(exg, 32, dd, .)
{
	uint* reg_a = &DX;
	uint* reg_b = &DY;
	uint tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


M68KMAKE_OP(exg, 32, aa, .)
{
	uint* reg_a = &AX;
	uint* reg_b = &AY;
	uint tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


M68KMAKE_OP(exg, 32, da, .)
{
	uint* reg_a = &DX;
	uint* reg_b = &AY;
	uint tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


M68KMAKE_OP(ext, 16, ., .)
{
	uint* r_dst = &DY;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xff00 : 0);

	FLAG_N = NFLAG_16(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(ext, 32, ., .)
{
	uint* r_dst = &DY;

	*r_dst = MASK_OUT_ABOVE_16(*r_dst) | (GET_MSB_16(*r_dst) ? 0xffff0000 : 0);

	FLAG_N = NFLAG_32(*r_dst);
	FLAG_Z = *r_dst;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(illegal, 0, ., .)
{
	m68ki_exception_illegal();
}

M68KMAKE_OP(jmp, 32, ., .)
{
	m68ki_jump(M68KMAKE_GET_EA_AY_32);
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


M68KMAKE_OP(jsr, 32, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_jump(ea);
}


M68KMAKE_OP(lea, 32, ., .)
{
	AX = M68KMAKE_GET_EA_AY_32;
}


M68KMAKE_OP(link, 16, ., a7)
{
	REG_A[7] -= 4;
	m68ki_write_32(REG_A[7], REG_A[7]);
	REG_A[7] = MASK_OUT_ABOVE_32(REG_A[7] + MAKE_INT_16(OPER_I_16()));
}


M68KMAKE_OP(link, 16, ., .)
{
	uint* r_dst = &AY;

	m68ki_push_32(*r_dst);
	*r_dst = REG_A[7];
	REG_A[7] = MASK_OUT_ABOVE_32(REG_A[7] + MAKE_INT_16(OPER_I_16()));
}


M68KMAKE_OP(lsr, 8, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsr, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsr, 32, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsr, 8, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << (9-shift);
			FLAG_N = NFLAG_CLEAR;
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsr, 16, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_CLEAR;
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsr, 32, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_CLEAR;
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = (shift == 32 ? GET_MSB_32(src)>>23 : 0);
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsr, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_CLEAR;
	FLAG_Z = res;
	FLAG_C = FLAG_X = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 8, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src << shift;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (8-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 32, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (24-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 8, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 16, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift <= 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_X = FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = XFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 32, r, .)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_X = FLAG_C = (src >> (32 - shift)) << 8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = ((shift == 32 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(lsl, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, d, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, d, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, ai, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, ai, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pi7, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pi, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pi7, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pi, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pd7, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pd, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pd7, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, pd, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, di, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, di, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, ix, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, ix, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, aw, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, aw, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AW_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, al, d)
{
	uint res = MASK_OUT_ABOVE_8(DY);
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 8, al, .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;
	uint ea = EA_AL_8();

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, d, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, d, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, d, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, ai, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, ai, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, ai, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, pi, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, pi, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, pi, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, pd, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, pd, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, pd, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, di, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, di, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, di, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, ix, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, ix, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, ix, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, aw, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, aw, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, aw, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AW_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, al, d)
{
	uint res = MASK_OUT_ABOVE_16(DY);
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, al, a)
{
	uint res = MASK_OUT_ABOVE_16(AY);
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 16, al, .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;
	uint ea = EA_AL_16();

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, d, d)
{
	uint res = DY;
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, d, a)
{
	uint res = AY;
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, d, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint* r_dst = &DX;

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, ai, d)
{
	uint res = DY;
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, ai, a)
{
	uint res = AY;
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, ai, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, pi, d)
{
	uint res = DY;
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, pi, a)
{
	uint res = AY;
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, pi, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, pd, d)
{
	uint res = DY;
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, pd, a)
{
	uint res = AY;
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, pd, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, di, d)
{
	uint res = DY;
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, di, a)
{
	uint res = AY;
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, di, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, ix, d)
{
	uint res = DY;
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, ix, a)
{
	uint res = AY;
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, ix, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, aw, d)
{
	uint res = DY;
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, aw, a)
{
	uint res = AY;
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, aw, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AW_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, al, d)
{
	uint res = DY;
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, al, a)
{
	uint res = AY;
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(move, 32, al, .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;
	uint ea = EA_AL_32();

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(movea, 16, ., d)
{
	AX = MAKE_INT_16(DY);
}


M68KMAKE_OP(movea, 16, ., a)
{
	AX = MAKE_INT_16(AY);
}


M68KMAKE_OP(movea, 16, ., .)
{
	AX = MAKE_INT_16(M68KMAKE_GET_OPER_AY_16);
}


M68KMAKE_OP(movea, 32, ., d)
{
	AX = DY;
}


M68KMAKE_OP(movea, 32, ., a)
{
	AX = AY;
}


M68KMAKE_OP(movea, 32, ., .)
{
	AX = M68KMAKE_GET_OPER_AY_32;
}


M68KMAKE_OP(move, 16, toc, d)
{
	m68ki_set_ccr(DY);
}


M68KMAKE_OP(move, 16, toc, .)
{
	m68ki_set_ccr(M68KMAKE_GET_OPER_AY_16);
}


M68KMAKE_OP(move, 16, frs, d)
{
	if(CPU_TYPE_IS_000(CPU_TYPE) || FLAG_S)	/* NS990408 */
	{
		DY = MASK_OUT_BELOW_16(DY) | m68ki_get_sr();
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(move, 16, frs, .)
{
	if(CPU_TYPE_IS_000(CPU_TYPE) || FLAG_S)	/* NS990408 */
	{
		uint ea = M68KMAKE_GET_EA_AY_16;
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(move, 16, tos, d)
{
	if(FLAG_S)
	{
		m68ki_set_sr(DY);
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(move, 16, tos, .)
{
	if(FLAG_S)
	{
		uint new_sr = M68KMAKE_GET_OPER_AY_16;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(move, 32, fru, .)
{
	if(FLAG_S)
	{
		AY = REG_USP;
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(move, 32, tou, .)
{
	if(FLAG_S)
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_USP = AY;
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(movem, 16, re, pd)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			ea -= 2;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[15-i]));
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, re, .)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 32, re, pd)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			ea -= 4;
			m68ki_write_16(ea+2, REG_DA[15-i] & 0xFFFF );
			m68ki_write_16(ea, (REG_DA[15-i] >> 16) & 0xFFFF );
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, re, .)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA[i]);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 16, er, pi)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, er, pcdi)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCDI_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, er, pcix)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCIX_16();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 16, er, .)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_W);
}


M68KMAKE_OP(movem, 32, er, pi)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = AY;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}
	AY = ea;

	USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, er, pcdi)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCDI_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_pcrel_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, er, pcix)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = EA_PCIX_32();
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_pcrel_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movem, 32, er, .)
{
	uint i = 0;
	uint register_list = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	USE_CYCLES(count<<CYC_MOVEM_L);
}


M68KMAKE_OP(movep, 16, re, .)
{
	uint ea = EA_AY_DI_16();
	uint src = DX;

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


M68KMAKE_OP(movep, 32, re, .)
{
	uint ea = EA_AY_DI_32();
	uint src = DX;

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 24));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 16));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


M68KMAKE_OP(movep, 16, er, .)
{
	uint ea = EA_AY_DI_16();
	uint* r_dst = &DX;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | ((m68ki_read_8(ea) << 8) + m68ki_read_8(ea + 2));
}


M68KMAKE_OP(movep, 32, er, .)
{
	uint ea = EA_AY_DI_32();

	DX = (m68ki_read_8(ea) << 24) + (m68ki_read_8(ea + 2) << 16)
		+ (m68ki_read_8(ea + 4) << 8) + m68ki_read_8(ea + 6);
}


M68KMAKE_OP(moveq, 32, ., .)
{
	uint res = DX = MAKE_INT_8(MASK_OUT_ABOVE_8(REG_IR));

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(muls, 16, ., d)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(DY) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(muls, 16, ., .)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_32(MAKE_INT_16(M68KMAKE_GET_OPER_AY_16) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(mulu, 16, ., d)
{
	uint* r_dst = &DX;
	uint res = MASK_OUT_ABOVE_16(DY) * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(mulu, 16, ., .)
{
	uint* r_dst = &DX;
	uint res = M68KMAKE_GET_OPER_AY_16 * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	FLAG_Z = res;
	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(nbcd, 8, ., d)
{
	uint* r_dst = &DY;
	uint dst = *r_dst;
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


M68KMAKE_OP(nbcd, 8, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint dst = m68ki_read_8(ea);
	uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

	if(res != 0x9a)
	{
		FLAG_V = ~res; /* Undefined V behavior */

		if((res & 0x0f) == 0xa)
			res = (res & 0xf0) + 0x10;

		res = MASK_OUT_ABOVE_8(res);

		FLAG_V &= res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		FLAG_Z |= res;
		FLAG_C = CFLAG_SET;
		FLAG_X = XFLAG_SET;
	}
	else
	{
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
	}
	FLAG_N = NFLAG_8(res);	/* Undefined N behavior */
}


M68KMAKE_OP(neg, 8, ., d)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_8(*r_dst);

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = *r_dst & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(neg, 8, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_8(res);
	FLAG_C = FLAG_X = CFLAG_8(res);
	FLAG_V = src & res;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(neg, 16, ., d)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_16(*r_dst);

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (*r_dst & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(neg, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_16(res);
	FLAG_C = FLAG_X = CFLAG_16(res);
	FLAG_V = (src & res)>>8;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(neg, 32, ., d)
{
	uint* r_dst = &DY;
	uint res = 0 - *r_dst;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(*r_dst, 0, res);
	FLAG_V = (*r_dst & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(neg, 32, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint src = m68ki_read_32(ea);
	uint res = 0 - src;

	FLAG_N = NFLAG_32(res);
	FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(negx, 8, ., d)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_8(*r_dst) - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = *r_dst & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


M68KMAKE_OP(negx, 8, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = m68ki_read_8(ea);
	uint res = 0 - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = src & res;

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(negx, 16, ., d)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_16(*r_dst) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (*r_dst & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


M68KMAKE_OP(negx, 16, ., .)
{
	uint ea  = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


M68KMAKE_OP(negx, 32, ., d)
{
	uint* r_dst = &DY;
	uint res = 0 - MASK_OUT_ABOVE_32(*r_dst) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(*r_dst, 0, res);
	FLAG_V = (*r_dst & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


M68KMAKE_OP(negx, 32, ., .)
{
	uint ea  = M68KMAKE_GET_EA_AY_32;
	uint src = m68ki_read_32(ea);
	uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
	FLAG_V = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


M68KMAKE_OP(nop, 0, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
}


M68KMAKE_OP(not, 8, ., d)
{
	uint* r_dst = &DY;
	uint res = MASK_OUT_ABOVE_8(~*r_dst);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(not, 8, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(not, 16, ., d)
{
	uint* r_dst = &DY;
	uint res = MASK_OUT_ABOVE_16(~*r_dst);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(not, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(not, 32, ., d)
{
	uint* r_dst = &DY;
	uint res = *r_dst = MASK_OUT_ABOVE_32(~*r_dst);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(not, 32, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 8, er, d)
{
	uint res = MASK_OUT_ABOVE_8((DX |= MASK_OUT_ABOVE_8(DY)));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 8, er, .)
{
	uint res = MASK_OUT_ABOVE_8((DX |= M68KMAKE_GET_OPER_AY_8));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 16, er, d)
{
	uint res = MASK_OUT_ABOVE_16((DX |= MASK_OUT_ABOVE_16(DY)));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 16, er, .)
{
	uint res = MASK_OUT_ABOVE_16((DX |= M68KMAKE_GET_OPER_AY_16));

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 32, er, d)
{
	uint res = DX |= DY;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 32, er, .)
{
	uint res = DX |= M68KMAKE_GET_OPER_AY_32;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 8, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 16, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(or, 32, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = DX | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 8, ., d)
{
	uint res = MASK_OUT_ABOVE_8((DY |= OPER_I_8()));

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 8, ., .)
{
	uint src = OPER_I_8();
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 16, ., d)
{
	uint res = MASK_OUT_ABOVE_16(DY |= OPER_I_16());

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 16, ., .)
{
	uint src = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 32, ., d)
{
	uint res = DY |= OPER_I_32();

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 32, ., .)
{
	uint src = OPER_I_32();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ori, 16, toc, .)
{
	m68ki_set_ccr(m68ki_get_ccr() | OPER_I_8());
}


M68KMAKE_OP(ori, 16, tos, .)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() | src);
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(pea, 32, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;

	m68ki_push_32(ea);
}

M68KMAKE_OP(reset, 0, ., .)
{
	if(FLAG_S)
	{
		m68ki_output_reset();		   /* auto-disable (see m68kcpu.h) */
		USE_CYCLES(CYC_RESET);
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(ror, 8, s, .)
{
	uint* r_dst = &DY;
	uint orig_shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROR_8(src, shift);

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = src << (9-orig_shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ror, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROR_16(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ror, 32, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src = *r_dst;
	uint res = ROR_32(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = src << (9-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ror, 8, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROR_8(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		FLAG_C = src << (8-((shift-1)&7));
		FLAG_N = NFLAG_8(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ror, 16, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 15;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROR_16(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		FLAG_C = (src >> ((shift - 1) & 15)) << 8;
		FLAG_N = NFLAG_16(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ror, 32, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 31;
	uint64 src = *r_dst;
	uint res = ROR_32(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = res;
		FLAG_C = (src >> ((shift - 1) & 31)) << 8;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(ror, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src << 8;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 8, s, .)
{
	uint* r_dst = &DY;
	uint orig_shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROL_8(src, shift);

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = src << orig_shift;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROL_16(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> (8-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 32, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src = *r_dst;
	uint res = ROL_32(src, shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = src >> (24-shift);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 8, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 7;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROL_8(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		if(shift != 0)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}
		FLAG_C = (src & 1)<<8;
		FLAG_N = NFLAG_8(src);
		FLAG_Z = src;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 16, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 15;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, shift));

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		if(shift != 0)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}
		FLAG_C = (src & 1)<<8;
		FLAG_N = NFLAG_16(src);
		FLAG_Z = src;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 32, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift & 31;
	uint64 src = *r_dst;
	uint res = ROL_32(src, shift);

	if(orig_shift != 0)
	{
		USE_CYCLES(orig_shift<<CYC_SHIFT);

		*r_dst = res;

		FLAG_C = (src >> ((32 - shift) & 0x1f)) << 8;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rol, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = src >> 7;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxr, 8, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROR_9(src | (XFLAG_AS_1() << 8), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxr, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxr, 32, s, .)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src   = *r_dst;
	uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	res = ROR_33_64(res, shift);

	FLAG_C = FLAG_X = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst =  res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROR_33(src, shift) & ~(1 << (32 - shift))) | (XFLAG_AS_1() << (32 - shift)));
	uint new_x_flag = src & (1 << (shift - 1));

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_C = FLAG_X = (new_x_flag != 0)<<8;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


M68KMAKE_OP(roxr, 8, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint shift = orig_shift % 9;
		uint src   = MASK_OUT_ABOVE_8(*r_dst);
		uint res   = ROR_9(src | (XFLAG_AS_1() << 8), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		FLAG_N = NFLAG_8(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_8(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxr, 16, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint shift = orig_shift % 17;
		uint src   = MASK_OUT_ABOVE_16(*r_dst);
		uint res   = ROR_17(src | (XFLAG_AS_1() << 16), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		FLAG_N = NFLAG_16(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_16(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxr, 32, r, .)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint   shift = orig_shift % 33;
		uint64 src   = *r_dst;
		uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

		res = ROR_33_64(res, shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 24;
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = res;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(*r_dst);
	FLAG_Z = *r_dst;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift % 33;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROR_33(src, shift) & ~(1 << (32 - shift))) | (XFLAG_AS_1() << (32 - shift)));
	uint new_x_flag = src & (1 << (shift - 1));

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	if(shift != 0)
	{
		*r_dst = res;
		FLAG_X = (new_x_flag != 0)<<8;
	}
	else
		res = src;
	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


M68KMAKE_OP(roxr, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxl, 8, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = ROL_9(src | (XFLAG_AS_1() << 8), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxl, 16, s, .)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxl, 32, s, .)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint64 src   = *r_dst;
	uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	res = ROL_33_64(res, shift);

	FLAG_C = FLAG_X = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROL_33(src, shift) & ~(1 << (shift - 1))) | (XFLAG_AS_1() << (shift - 1)));
	uint new_x_flag = src & (1 << (32 - shift));

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_C = FLAG_X = (new_x_flag != 0)<<8;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


M68KMAKE_OP(roxl, 8, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;


	if(orig_shift != 0)
	{
		uint shift = orig_shift % 9;
		uint src   = MASK_OUT_ABOVE_8(*r_dst);
		uint res   = ROL_9(src | (XFLAG_AS_1() << 8), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		FLAG_N = NFLAG_8(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_8(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxl, 16, r, .)
{
	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint shift = orig_shift % 17;
		uint src   = MASK_OUT_ABOVE_16(*r_dst);
		uint res   = ROL_17(src | (XFLAG_AS_1() << 16), shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		FLAG_N = NFLAG_16(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_16(*r_dst);
	FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(roxl, 32, r, .)
{
#if M68K_USE_64_BIT

	uint*  r_dst = &DY;
	uint   orig_shift = DX & 0x3f;

	if(orig_shift != 0)
	{
		uint   shift = orig_shift % 33;
		uint64 src   = *r_dst;
		uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

		res = ROL_33_64(res, shift);

		USE_CYCLES(orig_shift<<CYC_SHIFT);

		FLAG_C = FLAG_X = res >> 24;
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = res;
		FLAG_N = NFLAG_32(res);
		FLAG_Z = res;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(*r_dst);
	FLAG_Z = *r_dst;
	FLAG_V = VFLAG_CLEAR;

#else

	uint* r_dst = &DY;
	uint orig_shift = DX & 0x3f;
	uint shift = orig_shift % 33;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32((ROL_33(src, shift) & ~(1 << (shift - 1))) | (XFLAG_AS_1() << (shift - 1)));
	uint new_x_flag = src & (1 << (32 - shift));

	if(orig_shift != 0)
		USE_CYCLES(orig_shift<<CYC_SHIFT);

	if(shift != 0)
	{
		*r_dst = res;
		FLAG_X = (new_x_flag != 0)<<8;
	}
	else
		res = src;
	FLAG_C = FLAG_X;
	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;

#endif
}


M68KMAKE_OP(roxl, 16, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = m68ki_read_16(ea);
	uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

	FLAG_C = FLAG_X = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(rte, 32, ., .)
{
	if(FLAG_S)
	{
		uint new_sr;
		uint new_pc;
		uint format_word;

		m68ki_rte_callback();		   /* auto-disable (see m68kcpu.h) */
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */

		if(CPU_TYPE_IS_000(CPU_TYPE))
		{
			new_sr = m68ki_pull_16();
			new_pc = m68ki_pull_32();
			m68ki_jump(new_pc);
			m68ki_set_sr(new_sr);

			CPU_INSTR_MODE = INSTRUCTION_YES;
			CPU_RUN_MODE = RUN_MODE_NORMAL;

			return;
		}

		if(CPU_TYPE_IS_010(CPU_TYPE))
		{
			format_word = m68ki_read_16(REG_A[7]+6) >> 12;
			if(format_word == 0)
			{
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
			} else if (format_word == 8) {
				/* Format 8 stack frame -- 68010 only. 29 word bus/address error */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_fake_pull_16();	/* special status word */
				m68ki_fake_pull_32();	/* fault address */
				m68ki_fake_pull_16();	/* unused/reserved */
				m68ki_fake_pull_16();	/* data output buffer */
				m68ki_fake_pull_16();	/* unused/reserved */
				m68ki_fake_pull_16();	/* data input buffer */
				m68ki_fake_pull_16();	/* unused/reserved */
				m68ki_fake_pull_16();	/* instruction input buffer */
				m68ki_fake_pull_32();	/* internal information, 16 words */
				m68ki_fake_pull_32();	/* (actually, we use 8 DWORDs) */
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_fake_pull_32();
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
			}
			CPU_INSTR_MODE = INSTRUCTION_YES;
			CPU_RUN_MODE = RUN_MODE_NORMAL;
			/* Not handling other exception types (9) */
			m68ki_exception_format_error();
			return;
		}

		/* Otherwise it's 020 */
rte_loop:
		format_word = m68ki_read_16(REG_A[7]+6) >> 12;
		switch(format_word)
		{
			case 0: /* Normal */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
			case 1: /* Throwaway */
				new_sr = m68ki_pull_16();
				m68ki_fake_pull_32();	/* program counter */
				m68ki_fake_pull_16();	/* format word */
				m68ki_set_sr_noint(new_sr);
				goto rte_loop;
			case 2: /* Trap */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();	/* format word */
				m68ki_fake_pull_32();	/* address */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				CPU_INSTR_MODE = INSTRUCTION_YES;
				CPU_RUN_MODE = RUN_MODE_NORMAL;
				return;
		}
		/* Not handling long or short bus fault */
		CPU_INSTR_MODE = INSTRUCTION_YES;
		CPU_RUN_MODE = RUN_MODE_NORMAL;
		m68ki_exception_format_error();
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(rtr, 32, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_set_ccr(m68ki_pull_16());
	m68ki_jump(m68ki_pull_32());
}


M68KMAKE_OP(rts, 32, ., .)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_jump(m68ki_pull_32());
}


M68KMAKE_OP(sbcd, 8, rr, .)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


M68KMAKE_OP(sbcd, 8, mm, ax7)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(sbcd, 8, mm, ay7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(sbcd, 8, mm, axy7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(sbcd, 8, mm, .)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res -= 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res += 0xa0;

	res = MASK_OUT_ABOVE_8(res);

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(st, 8, ., d)
{
	DY |= 0xff;
}


M68KMAKE_OP(st, 8, ., .)
{
	m68ki_write_8(M68KMAKE_GET_EA_AY_8, 0xff);
}


M68KMAKE_OP(sf, 8, ., d)
{
	DY &= 0xffffff00;
}


M68KMAKE_OP(sf, 8, ., .)
{
	m68ki_write_8(M68KMAKE_GET_EA_AY_8, 0);
}


M68KMAKE_OP(scc, 8, ., d)
{
	if(M68KMAKE_CC)
	{
		DY |= 0xff;
		USE_CYCLES(CYC_SCC_R_TRUE);
		return;
	}
	DY &= 0xffffff00;
}


M68KMAKE_OP(scc, 8, ., .)
{
	m68ki_write_8(M68KMAKE_GET_EA_AY_8, M68KMAKE_CC ? 0xff : 0);
}


M68KMAKE_OP(stop, 0, ., .)
{
	if(FLAG_S)
	{
		uint new_sr = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		CPU_STOPPED |= STOP_LEVEL_STOP;
		m68ki_set_sr(new_sr);
		if(m68ki_remaining_cycles >= CYC_INSTRUCTION[REG_IR])
			m68ki_remaining_cycles = CYC_INSTRUCTION[REG_IR];
		else
			USE_ALL_CYCLES();
		return;
	}
	m68ki_exception_privilege_violation();
}


M68KMAKE_OP(sub, 8, er, d)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(sub, 8, er, .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_8;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(sub, 16, er, d)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(sub, 16, er, a)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(sub, 16, er, .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_16;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(sub, 32, er, d)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(sub, 32, er, a)
{
	uint* r_dst = &DX;
	uint src = AY;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(sub, 32, er, .)
{
	uint* r_dst = &DX;
	uint src = M68KMAKE_GET_OPER_AY_32;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(sub, 8, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(sub, 16, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(sub, 32, re, .)
{
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(suba, 16, ., d)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(DY));
}


M68KMAKE_OP(suba, 16, ., a)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(AY));
}


M68KMAKE_OP(suba, 16, ., .)
{
	uint* r_dst = &AX;
	uint src = MAKE_INT_16(M68KMAKE_GET_OPER_AY_16);

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


M68KMAKE_OP(suba, 32, ., d)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - DY);
}


M68KMAKE_OP(suba, 32, ., a)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - AY);
}


M68KMAKE_OP(suba, 32, ., .)
{
	uint src = M68KMAKE_GET_OPER_AY_32;
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


M68KMAKE_OP(subi, 8, ., d)
{
	uint* r_dst = &DY;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(subi, 8, ., .)
{
	uint src = OPER_I_8();
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(subi, 16, ., d)
{
	uint* r_dst = &DY;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(subi, 16, ., .)
{
	uint src = OPER_I_16();
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(subi, 32, ., d)
{
	uint* r_dst = &DY;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(subi, 32, ., .)
{
	uint src = OPER_I_32();
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(subq, 8, ., d)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(subq, 8, ., .)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint dst = m68ki_read_8(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, FLAG_Z);
}


M68KMAKE_OP(subq, 16, ., d)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


M68KMAKE_OP(subq, 16, ., a)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((REG_IR >> 9) - 1) & 7) + 1));
}


M68KMAKE_OP(subq, 16, ., .)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = M68KMAKE_GET_EA_AY_16;
	uint dst = m68ki_read_16(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, FLAG_Z);
}


M68KMAKE_OP(subq, 32, ., d)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = *r_dst;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	*r_dst = FLAG_Z;
}


M68KMAKE_OP(subq, 32, ., a)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((REG_IR >> 9) - 1) & 7) + 1));
}


M68KMAKE_OP(subq, 32, ., .)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = M68KMAKE_GET_EA_AY_32;
	uint dst = m68ki_read_32(ea);
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, FLAG_Z);
}


M68KMAKE_OP(subx, 8, rr, .)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


M68KMAKE_OP(subx, 16, rr, .)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


M68KMAKE_OP(subx, 32, rr, .)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


M68KMAKE_OP(subx, 8, mm, ax7)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(subx, 8, mm, ay7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(subx, 8, mm, axy7)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(subx, 8, mm, .)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


M68KMAKE_OP(subx, 16, mm, .)
{
	uint src = OPER_AY_PD_16();
	uint ea  = EA_AX_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


M68KMAKE_OP(subx, 32, mm, .)
{
	uint src = OPER_AY_PD_32();
	uint ea  = EA_AX_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = dst - src - XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


M68KMAKE_OP(swap, 32, ., .)
{
	uint* r_dst = &DY;

	FLAG_Z = MASK_OUT_ABOVE_32(*r_dst<<16);
	*r_dst = (*r_dst>>16) | FLAG_Z;

	FLAG_Z = *r_dst;
	FLAG_N = NFLAG_32(*r_dst);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


M68KMAKE_OP(tas, 8, ., d)
{
	uint* r_dst = &DY;

	FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
	FLAG_N = NFLAG_8(*r_dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	*r_dst |= 0x80;
}


M68KMAKE_OP(tas, 8, ., .)
{
	uint ea = M68KMAKE_GET_EA_AY_8;
	uint dst = m68ki_read_8(ea);
	uint allow_writeback;

	FLAG_Z = dst;
	FLAG_N = NFLAG_8(dst);
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;

	/* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
	allow_writeback = m68ki_tas_callback();

	if (allow_writeback==1) m68ki_write_8(ea, dst | 0x80);
}


M68KMAKE_OP(trap, 0, ., .)
{
	/* Trap#n stacks exception frame type 0 */
	m68ki_exception_trapN(EXCEPTION_TRAP_BASE + (REG_IR & 0xf));	/* HJB 990403 */
}


M68KMAKE_OP(trapv, 0, ., .)
{
	if(COND_VC())
	{
		return;
	}
	m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
}


M68KMAKE_OP(tst, 8, ., d)
{
	uint res = MASK_OUT_ABOVE_8(DY);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(tst, 8, ., .)
{
	uint res = M68KMAKE_GET_OPER_AY_8;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(tst, 16, ., d)
{
	uint res = MASK_OUT_ABOVE_16(DY);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(tst, 16, ., .)
{
	uint res = M68KMAKE_GET_OPER_AY_16;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(tst, 32, ., d)
{
	uint res = DY;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(tst, 32, ., .)
{
	uint res = M68KMAKE_GET_OPER_AY_32;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
}


M68KMAKE_OP(unlk, 32, ., a7)
{
	REG_A[7] = m68ki_read_32(REG_A[7]);
}


M68KMAKE_OP(unlk, 32, ., .)
{
	uint* r_dst = &AY;

	REG_A[7] = *r_dst;
	*r_dst = m68ki_pull_32();
}


XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
M68KMAKE_END