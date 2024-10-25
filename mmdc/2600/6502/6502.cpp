// 6502 instruction set:
// aaabbbcc
// aaa and cc determine instruction
// bbb determines address mode
// aaabbbcc	instruction		bbb	addressing mode
//
// 000xxx01	ORA				000	(zero page,X)
// 001xxx01	AND				001 zero page
// 010xxx01	EOR				010	#immediate
// 011xxx01	ADC				011	absolute
// 100xxx01	STA				100	(zero page),Y
// 101xxx01	LDA				101	zero page,X
// 110xxx01	CMP				110	absolute,Y
// 111xxx01	SBC				111	absolute,X
//
// 000xxx10	ASL				000	#immediate
// 001xxx10	ROL				001 zero page
// 010xxx10	LSR				010	accumulator
// 011xxx10	ROR				011	absolute
// 100xxx10	STX				
// 101xxx10	LDX				101	zero page,X
// 110xxx10	DEC				
// 111xxx10	INC				111	absolute,X
//
// 000xxx00					000	#immediate
// 001xxx00	BIT				001 zero page
// 010xxx00	JMP
// 011xxx00	JMP (abs)		011	absolute
// 100xxx00	STY
// 101xxx00	LDY				101	zero page,X
// 110xxx00	CPY
// 111xxx00	CPX				111	absolute,X

// aaabbb11 - NO INSTRUCTIONS
//
// conditional branch instructions:
// xxy10000
// 00010000	BPL
// 00110000	BMI
// 01010000 BVC
// 01110000 BVS
// 10010000 BCC
// 10110000 BCS
// 11010000 BNE
// 11010000 BEQ

// 00010000 BRK
// 00100000 JSR abs
// 01000000 RTI
// 01100000 RTS

//		x0	x1	x2	x3	x4	x5	x6	x7	x8	x9	xA	xB	xC	xD	xE	xF
//0x	BRK	ORA				ORA	ASL		PHP	ORA	ASL			ORA	ASL
//1x	BPL	ORA				ORA	ASL		CLC	ORA				ORA	ASL
//2x	JSR	AND			BIT	AND	ROL		PLP	AND	ROL		BIT	AND	ROL
//3x	BMI	AND				AND	ROL		SEC	AND				AND	ROL
//4x	RTI	EOR				EOR	LSR		PHA	EOR	LSR		JMP	EOR	LSR
//5x	BVC	EOR				EOR	LSR		CLI	EOR				EOR	LSR
//6x	RTS	ADC				ADC	ROR		PLA	ADC	ROR		JMP	ADC	ROR
//7x	BVS	ADC				ADC	ROR		SEI	ADC				ADC	ROR
//8x		STA			STY	STA	STX		DEY		TXA		STY	STA	STX
//9x	BCC	STA			STY	STA	STX		TYA	STA	TXS			STA
//Ax	LDY	LDA	LDX		LDY	LDA	LDX		TAY	LDA	TAX		LDY	LDA	LDX
//Bx	BCS	LDA			LDY	LDA	LDX		CLV	LDA	TSX		LDY	LDA	LDX
//Cx	CPY	CMP			CPY	CMP	DEC		INY	CMP	DEX		CPY	CMP	DEC
//Dx	BNE	CMP				CMP	DEC		CLD	CMP				CMP	DEC
//Ex	CPX	SBC			CPX	SBC	INC		INX	SBC	NOP		CPX	SBC	INC
//Fx	BEQ	SBC				SBC	INC		SED	SBC				SBC	INC
//
//Processor Status
//NV-BDIZC
//N - Negative
//V - Overflow
//B - Break
//D - Decimal
//I - Interrupt
//Z - Zero
//C - Carry


#include <memory.h>
#include <stdio.h>
#include <io.h>
#include "6502.h"

typedef unsigned char byte;
typedef unsigned short ushort;

static char* acc = "A";
static char* imm = "#$%02X";
static char* zp = "$%02X";
static char* zpx = "$%02X,X";
static char* zpy = "$%02X,Y";
static char* izx = "($%02X,X)";
static char* izy = "($%02X),Y";
static char* abs = "$%04X";
static char* abx = "$%04X,X";
static char* aby = "$%04X,Y";
static char* ind = "($%04X)";
static char* rel = "$%02X";

#define OP(x, len, cycles, fmt) (new CPU6502::instruction(&x, len, cycles, #x, fmt))
CPU6502::instruction* CPU6502::instLookup[] =
{
			//x0			x1				x2				x3	x4				x5				x6				x7	x8				x9				xA				xB	xC				xD				xE				xF
	/*0x*/	OP(BRK,1,7,""),	OP(ORA,2,6,izx),0,				0,	0,				OP(ORA,2,3,zp), OP(ASL,2,5,zp),	0,	OP(PHP,1,3,""),	OP(ORA,2,2,imm),OP(ASL,1,2,acc),0,	0,				OP(ORA,3,4,abs),OP(ASL,3,6,abs),0,
	/*1x*/	OP(BPL,2,2,rel),OP(ORA,2,5,izy),0,				0,	0,				OP(ORA,2,4,zpx),OP(ASL,2,6,zpx),0,	OP(CLC,1,2,""),	OP(ORA,3,4,aby),0,				0,	0,				OP(ORA,3,4,abx),OP(ASL,3,7,abx),0,
	/*2x*/	OP(JSR,3,6,abs),OP(AND,2,6,izx),0,				0,	OP(BIT,2,3,zp), OP(AND,2,3,zp), OP(ROL,2,5,zp),	0,	OP(PLP,1,4,""),	OP(AND,2,2,imm),OP(ROL,1,2,acc),0,	OP(BIT,3,4,abs),OP(AND,3,4,abs),OP(ROL,3,6,abs),0,
	/*3x*/	OP(BMI,2,2,rel),OP(AND,2,5,izy),0,				0,	0,				OP(AND,2,4,zpx),OP(ROL,2,6,zpx),0,	OP(SEC,1,2,""),	OP(AND,3,4,aby),0,				0,	0,				OP(AND,3,4,abx),OP(ROL,3,7,abx),0,
	/*4x*/	OP(RTI,1,6,""),	OP(EOR,2,6,izx),0,				0,	0,				OP(EOR,2,3,zp),	OP(LSR,2,5,zp),	0,	OP(PHA,1,3,""),	OP(EOR,2,2,imm),OP(LSR,1,2,acc),0,	OP(JMP,3,3,abs),OP(EOR,3,4,abs),OP(LSR,3,6,abs),0,
	/*5x*/	OP(BVC,2,2,rel),OP(EOR,2,5,izy),0,				0,	0,				OP(EOR,2,4,zpx),OP(LSR,2,6,zpx),0,	OP(CLI,1,2,""),	OP(EOR,3,4,aby),0,				0,	0,				OP(EOR,3,4,abx),OP(LSR,3,7,abx),0,
	/*6x*/	OP(RTS,1,6,""), OP(ADC,2,6,izx),0,				0,	0,				OP(ADC,2,3,zp), OP(ROR,2,5,zp), 0,	OP(PLA,1,4,""),	OP(ADC,2,2,imm),OP(ROR,1,2,acc),0,	OP(JMP,3,5,ind),OP(ADC,3,4,abs),OP(ROR,3,6,abs),0,
	/*7x*/	OP(BVS,2,2,rel),OP(ADC,2,5,izy),0,				0,	0,				OP(ADC,2,4,zpx),OP(ROR,2,6,zpx),0,	OP(SEI,1,2,""),	OP(ADC,3,4,aby),0,				0,	0,				OP(ADC,3,4,abx),OP(ROR,3,7,abx),0,

	/*8x*/	0,				OP(STA,2,6,izx),0,				0,	OP(STY,2,3,zp), OP(STA,2,3,zp), OP(STX,2,3,zp),	0,	OP(DEY,1,2,""),	0,				OP(TXA,1,2,""),	0,	OP(STY,3,4,abs),OP(STA,3,4,abs),OP(STX,3,4,abs),0,
	/*9x*/	OP(BCC,2,2,rel),OP(STA,2,6,izy),0,				0,	OP(STY,2,4,zpx),OP(STA,2,4,zpx),OP(STX,2,4,zpy),0,	OP(TYA,1,2,""),	OP(STA,3,5,aby),OP(TXS,1,2,""),	0,	0,				OP(STA,3,5,abx),0,				0,
	/*Ax*/	OP(LDY,2,2,imm),OP(LDA,2,6,izx),OP(LDX,2,2,imm),0,	OP(LDY,2,3,zp), OP(LDA,2,3,zp),	OP(LDX,2,3,zp),	0,	OP(TAY,1,2,""),	OP(LDA,2,2,imm),OP(TAX,1,2,""),	0,	OP(LDY,3,4,abs),OP(LDA,3,4,abs),OP(LDX,3,4,abs),0,
	/*Bx*/	OP(BCS,2,2,rel),OP(LDA,2,5,izy),0,				0,	OP(LDY,2,4,zpx),OP(LDA,2,4,zpx),OP(LDX,2,4,zpy),0,	OP(CLV,1,2,""),	OP(LDA,3,4,aby),OP(TSX,1,2,""), 0,	OP(LDY,3,4,abx),OP(LDA,3,4,abx),OP(LDX,3,4,aby),0,
	/*Cx*/	OP(CPY,2,2,imm),OP(CMP,2,6,izx),0,				0,	OP(CPY,2,3,zp),	OP(CMP,2,3,zp),	OP(DEC,2,5,zp),	0,	OP(INY,1,2,""),	OP(CMP,2,2,imm),OP(DEX,1,2,""), 0,	OP(CPY,3,4,abs),OP(CMP,3,4,abs),OP(DEC,3,6,abs),0,
	/*Dx*/	OP(BNE,2,2,rel),OP(CMP,2,6,izy),0,				0,	0,				OP(CMP,2,4,zpx),OP(DEC,2,6,zpx),0,	OP(CLD,1,2,""),	OP(CMP,3,4,aby),0,				0,	0,				OP(CMP,3,4,abx),OP(DEC,3,7,abx),0,
	/*Ex*/	OP(CPX,2,2,imm),OP(SBC,2,6,izx),0,				0,	OP(CPX,2,3,zp),	OP(SBC,2,3,zp),	OP(INC,2,5,zp),	0,	OP(INX,1,2,""),	OP(SBC,2,2,imm),OP(NOP,1,2,""),	0,	OP(CPX,3,4,abs),OP(SBC,3,4,abs),OP(INC,3,6,abs),0,
	/*Fx*/	OP(BEQ,2,2,rel),OP(SBC,2,5,izy),0,				0,	0,				OP(SBC,2,4,zpx),OP(INC,2,6,zpx),0,	OP(SED,1,2,""),	OP(SBC,3,4,aby),0,				0,	0,				OP(SBC,3,4,abx),OP(INC,3,7,abx),0
};

byte systemRam[64 * 1024];

byte* defaultMapper(int address)
{
	return &systemRam[address & 0xffff];
}


//byte program[] = {0x78,0xD8,0xA2,0xFF,0x9A,0xA2,0x5D};
void test6502()
{
	CPU6502 cpu;
	
//	cpu.load(program,0xf000,7);
//	cpu.load("combat.bin",0xf000);
	cpu.load("6502_functional_test.bin",0);
	cpu.reset(0x0400);
	cpu.run();
}

