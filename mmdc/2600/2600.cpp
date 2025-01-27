#include "2600.h"
#include <stdio.h>
#include <io.h>

// CPU 8K Address Space Mirrors(Step 2000h)
// The 6507 CPU is having only 13 address pins(8KBytes, 0000h - 1FFFh).
// All memory is mirrored in steps of ^2000h(ie.at 2000h - 3FFFh, 4000h - 5FFFh, and so on, up to E000h - FFFFh).
// The lower half of each area contains internal memory(RAM, I / O Ports),
// the upper half contains external memory(cartridge ROM or expansion RAM).

// TIA Mirrors(Step 10h / 40h, 100h)
// The TIA chip is addressed by A12 = 0, A7 = 0.
// Located in memory at 0000h - 007Fh, 0100h - 017Fh, 0200h - 027Fh, etc., 0F00h - 0F7Fh.
// Output ports are mirrored twice in each window(at XX00h and XX40h),
// input port are mirrored eight times in each window(at XX00h, XX10h, XX20h, XX30h, etc.XX70h).

// PIA RAM Mirrors(Step 100h, 400h)
// PIA RAM is selected by A12 = 0, A9 = 0, A7 = 1.
// Located in memory at 0080h - 00FFh, 0180h - 01FFh, 0480h - 04FFh, 0580h - 05FFh, etc.
// The mirror at 0180h is particulary important because it allows the CPU to access RAM for stack operations.

// PIA I / O Mirrors(Step 2h, 8h, 100h, 400h)
// PIA I / O is selected by A12 = 0, A9 = 1, A7 = 1.
// Located in memory at 0280h - 02FFh, 0380h - 03FFh, 0680h - 06FFh, 0780h - 07FFh, etc.
// Each 80h - area contains 16 mirrors of the PIA I / O ports at XX80h, XX88h, XX90h, XX98h, etc.And, each 8h - area contains two copies of INTIM(eg. 0284h, 0286h) and INSTAT(eg. 0285h, 0287h).

// Cartridge Mirrors
// Cartridge memory is selected by A12 = 1.
// Small 2K cartridges are usually mirrored twice into the 4K cartridge area.
// ROM - Bank ports(eg. 1FF8h) aren't necessarily connected to A12, and may overlap the upper PIA I/O mirror (eg. 0FF8h).

// Note(Commonly used Mirrors)
// In some cases, two different ports are located at the same memory address,
// one port being Read - only, and the other being Write - only,
// of course, there is no physical conflict with such 'overlapping' ports.
// However, human users may prefer to use different memory addresses for different ports.
// The following mirrors are used by many games(and in this document) : 
// Timer outputs at 294h - 297h(instead of 284h - 287h),
// TIA inputs at 30h - 3Dh(instead of 00h - 0Dh).

#define A0	(1<<0)
#define A1	(1<<1)
#define A2	(1<<2)
#define A3	(1<<3)
#define A4	(1<<4)
#define A5	(1<<5)
#define A6	(1<<6)
#define A7	(1<<7)
#define A8	(1<<8)
#define A9	(1<<9)
#define A10	(1<<10)
#define A11	(1<<11)
#define A12	(1<<12)

#define TIA_MASK (A12 | A7)
#define TIA_READ_ADDRESS_MASK 0xF
#define TIA_WRITE_ADDRESS_MASK 0x3F
#define TIA_SELECT 0
#define PIA_MASK (A12 | A9 | A7)
#define PIA_RAM_SELECT A7
#define PIA_RAM_MASK (A7-1)
#define PIA_IO_SELECT (A9 | A7)
#define PIA_READ_ADDRESS_MASK 0xF
#define ROM_SELECT A12
#define ROM_ADDRESS_MASK  (A12-1)

#define OFFSET(strct, field) ((int)&((strct*0)->(field)))
#define TIA_ADDRESS(x) int x = OFFSET(TIA_WRITE,x)
/*
TIA_ADDRESS(VSYNC);	//ds 1// $00   0000 00x0   Vertical Sync Set - Clear
TIA_ADDRESS(VBLANK);	//ds 1// $01   xx00 00x0   Vertical Blank Set - Clear
TIA_ADDRESS(WSYNC);	//ds 1// $02---- ----Wait for Horizontal Blank
TIA_ADDRESS(RSYNC);	//ds 1// $03---- ----Reset Horizontal Sync Counter
TIA_ADDRESS(NUSIZ0);	//ds 1// $04   00xx 0xxx   Number - Size player / missle 0
TIA_ADDRESS(NUSIZ1);	//ds 1// $05   00xx 0xxx   Number - Size player / missle 1
TIA_ADDRESS(COLUP0);	//ds 1// $06   xxxx xxx0   Color - Luminance Player 0
TIA_ADDRESS(COLUP1);	//ds 1// $07   xxxx xxx0   Color - Luminance Player 1
TIA_ADDRESS(COLUPF);	//ds 1// $08   xxxx xxx0   Color - Luminance Playfield
TIA_ADDRESS(COLUBK);	//ds 1// $09   xxxx xxx0   Color - Luminance Background
TIA_ADDRESS(CTRLPF);	//ds 1// $0A   00xx 0xxx   Control Playfield, Ball, Collisions
TIA_ADDRESS(REFP0);	//ds 1// $0B   0000 x000   Reflection Player 0
TIA_ADDRESS(REFP1);	//ds 1// $0C   0000 x000   Reflection Player 1
TIA_ADDRESS(PF0);	//ds 1// $0D   xxxx 0000   Playfield Register Byte 0
TIA_ADDRESS(PF1);	//ds 1// $0E   xxxx xxxx   Playfield Register Byte 1
TIA_ADDRESS(PF2);	//ds 1// $0F   xxxx xxxx   Playfield Register Byte 2
TIA_ADDRESS(RESP0);	//ds 1// $10---- ----Reset Player 0
TIA_ADDRESS(RESP1);	//ds 1// $11---- ----Reset Player 1
TIA_ADDRESS(RESM0);	//ds 1// $12---- ----Reset Missle 0
TIA_ADDRESS(RESM1);	//ds 1// $13---- ----Reset Missle 1
TIA_ADDRESS(RESBL);	//ds 1// $14---- ----Reset Ball
TIA_ADDRESS(AUDC0);	//ds 1// $15   0000 xxxx   Audio Control 0
TIA_ADDRESS(AUDC1);	//ds 1// $16   0000 xxxx   Audio Control 1
TIA_ADDRESS(AUDF0);	//ds 1// $17   000x xxxx   Audio Frequency 0
TIA_ADDRESS(AUDF1);	//ds 1// $18   000x xxxx   Audio Frequency 1
TIA_ADDRESS(AUDV0);	//ds 1// $19   0000 xxxx   Audio Volume 0
TIA_ADDRESS(AUDV1);	//ds 1// $1A   0000 xxxx   Audio Volume 1
TIA_ADDRESS(GRP0);	//ds 1// $1B   xxxx xxxx   Graphics Register Player 0
TIA_ADDRESS(GRP1);	//ds 1// $1C   xxxx xxxx   Graphics Register Player 1
TIA_ADDRESS(ENAM0);	//ds 1// $1D   0000 00x0   Graphics Enable Missle 0
TIA_ADDRESS(ENAM1);	//ds 1// $1E   0000 00x0   Graphics Enable Missle 1
TIA_ADDRESS(ENABL);	//ds 1// $1F   0000 00x0   Graphics Enable Ball
TIA_ADDRESS(HMP0);	//ds 1// $20   xxxx 0000   Horizontal Motion Player 0
TIA_ADDRESS(HMP1);	//ds 1// $21   xxxx 0000   Horizontal Motion Player 1
TIA_ADDRESS(HMM0);	//ds 1// $22   xxxx 0000   Horizontal Motion Missle 0
TIA_ADDRESS(HMM1);	//ds 1// $23   xxxx 0000   Horizontal Motion Missle 1
TIA_ADDRESS(HMBL);	//ds 1// $24   xxxx 0000   Horizontal Motion Ball
TIA_ADDRESS(VDELP0); //ds 1// $25   0000 000x   Vertical Delay Player 0
TIA_ADDRESS(VDELP1); //ds 1// $26   0000 000x   Vertical Delay Player 1
TIA_ADDRESS(VDELBL); //ds 1// $27   0000 000x   Vertical Delay Ball
TIA_ADDRESS(RESMP0); //ds 1// $28   0000 00x0   Reset Missle 0 to Player 0
TIA_ADDRESS(RESMP1); //ds 1// $29   0000 00x0   Reset Missle 1 to Player 1
TIA_ADDRESS(HMOVE);	//ds 1// $2A---- ----Apply Horizontal Motion
TIA_ADDRESS(HMCLR);	//ds 1// $2B---- ----Clear Horizontal Move Registers
TIA_ADDRESS(CXCLR);	//ds 1// $2C---- ----Clear Collision Latches
*/

VCS2600::VCS2600()
{
	// reset the memory for the members
	//memset(rom, 0, 4096);
	//memset(ram, 0, 128);
	//memset(&riot, 0, sizeof(riot));
	//memset(&tia, 0, sizeof(tia));
}

void VCS2600::Init6502()
{
	//cpu.load("combat.bin", 0xf000);
	FILE* fp;
	//fopen_s(&fp, "combat.bin", "rb");
	//unsigned size = (unsigned)_filelength(_fileno(fp));
	//fread(rom, size, 1, fp);
	//if (size == 2048)
	//	memcpy(rom + 2048, rom, 2048);
	//fclose(fp);

	//DisassembleRom();
	cpu.reset(0xf000);
	frameCounter = 0;
	lastWSYNC = 0;
}

/*
void VCS2600::DisassembleRom()
{
	FILE* fp;
	fopen_s(&fp, "c:\\code\\2600emu\\combat_timings.asm", "w");
	int len = 0;
	char dis[256];
	for (ushort pc = 0xf000; pc < 0xf5c5; pc += len)
	{
		byte opcode = rom[pc&ROM_ADDRESS_MASK];
		CPU6502::instruction* inst = cpu.instLookup[opcode];
		len = cpu.disassemble(pc, dis, sizeof(dis));
		char bytes[10];
		int off = 0;
		for (int i = 0; i < len; i++)
		{
			byte mem = rom[(pc + i)&ROM_ADDRESS_MASK];
			off += sprintf(bytes + off, "%02X ", mem);
		}
		bytes[off - 1] = 0;
		fprintf(fp, "%0X %-8s %-12s\t%d\t%d\n", pc, bytes, dis, len, inst->cycles);
	}
	fclose(fp);
}
*/

void VCS2600::step()
{
	cpu.step();
}

void VCS2600::scanLine()
{
	while (!tia.wSync)	// && (cpu.cycles - lastWSYNC < 76))
		cpu.step();
	int scanlines = ((cpu.cycles - lastWSYNC) / 76) + 1;
	lastWSYNC = cpu.cycles;
	while (scanlines--)
		tia.DrawScanLine();
}

void VCS2600::runFrame()
{
	// do a loop which runs the cpu until a 'wait' instruction
	// then asks the tia to draw a scanline
	// repeat until a VSYNC command written to the TIA
	// then run the cpu until it is over etc.

	lastWSYNC = cpu.cycles;
	while (tia.vBlank)
	{
		scanLine();
	}
	while (!tia.vBlank)
	{
		scanLine();
	}
	//cpu.resetCycles();
	frameCounter++;
}



// TIA mask = 0x7f, if all the other bits are zero in the address, this is for TIA
// A12 = 0, A7 = 0
// RAM mask = 0x80, if all upper bits are zero, then this is RAM
// ROM mask = 0x1000 if this bit is set, we are addressing ROM
// RIOT	mask = 0x280, hmmm - needs more thought
byte* tiaMemoryMapper(int address)
{
	bool isWrite = (address & WRITE_FLAG) != 0;
	byte data = 0;
	if (isWrite)
		data = address >> 17;
	address &= (WRITE_FLAG - 1);

	if (address & ROM_SELECT)
	{
		return &VCS2600::rom[address & ROM_ADDRESS_MASK];
	}
	else if ((address & TIA_MASK) == TIA_SELECT)
	{
		if (isWrite)	// readwrite)
		{
			// write registers
			address &= TIA_WRITE_ADDRESS_MASK;
			// check here for WSYNC write
			if (address == 2)
			{
				VCS2600::tia.SetWSync();
//				VCS2600::lastWSYNC = VCS2600::cpu.cycles + 20;
			}
			if (address == 0)
				VCS2600::tia.SetVSync();
			if (address == 1)
				VCS2600::tia.SetVBlank( (data & 2) != 0 );
			// here is where we can capture any interesting behavior by TIA
			// as a result of 'strobing' an address etc.

			// GRP0 - implement VDEL behavior
			if (address == 0x1c)
			{
				// copy the old value of GRP1 into the shadow register
				VCS2600::tia.GRP1_del = VCS2600::tia.write_registers.GRP1;
			}
			// GRP1 - implement VDEL behavior
			if (address == 0x1d)
			{
				// copy the old value of GRP0 into the shadow register
				VCS2600::tia.GRP0_del = VCS2600::tia.write_registers.GRP0;
			}

			// HMP0 - move Player0 horizontally
			if (address == 0x20)
			{
				VCS2600::tia.write_registers.HMP0 = 0;
			}

			// HMOVE = 0x2A - apply horizontal motion
			if (address == 0x2a)
			{
				// add the HMxx registers to the current position of the players
				// sign-extend the 4-bit 2s complement values
				int complement = 0;
				if (VCS2600::tia.write_registers.HMP0 & 0x80)
					complement = 0xfffffff0;
				VCS2600::tia.player0 += (complement + (VCS2600::tia.write_registers.HMP0>>4));
				VCS2600::tia.player0 %= 160;

				complement = 0;
				if (VCS2600::tia.write_registers.HMP1 & 0x80)
					complement = 0xfffffff0;
				VCS2600::tia.player1 += (complement + (VCS2600::tia.write_registers.HMP1>>4));
				VCS2600::tia.player1 %= 160;

				complement = 0;
				if (VCS2600::tia.write_registers.HMM0 & 0x80)
					complement = 0xfffffff0;
				VCS2600::tia.missile0 += (complement + (VCS2600::tia.write_registers.HMM0>>4));
				VCS2600::tia.missile0 %= 160;

				complement = 0;
				if (VCS2600::tia.write_registers.HMM1 & 0x80)
					complement = 0xfffffff0;
				VCS2600::tia.missile1 += (complement + (VCS2600::tia.write_registers.HMM1>>4));
				VCS2600::tia.missile1 %= 160;

				complement = 0;
				if (VCS2600::tia.write_registers.HMBL & 0x80)
					complement = 0xfffffff0;
				VCS2600::tia.ball += (complement + (VCS2600::tia.write_registers.HMBL>>4));
				VCS2600::tia.ball %= 160;
			}

			// HMCLR = 0x2B - reset all horizontal motion registers
			if (address == 0x2b)
			{
				VCS2600::tia.write_registers.HMP0 = 0;
				VCS2600::tia.write_registers.HMP1 = 0;
				VCS2600::tia.write_registers.HMM0 = 0;
				VCS2600::tia.write_registers.HMM1 = 0;
				VCS2600::tia.write_registers.HMBL = 0;
			}

			// RESP0 = 0x10
			if (address == 0x10)
			{
				VCS2600::tia.player0 = (VCS2600::cpu.cycles - VCS2600::lastWSYNC) * 3;
				//VCS2600::tia.player0 += 14;
				int multiple15 = VCS2600::tia.player0 / 15;
				VCS2600::tia.player0 = 15 * multiple15;
				VCS2600::tia.player0 &= 0xff;
			}

			// RESP1 = 0x11
			if (address == 0x11)
			{
				VCS2600::tia.player1 = (VCS2600::cpu.cycles - VCS2600::lastWSYNC) * 3;
				//VCS2600::tia.player1 += 14;
				int multiple15 = VCS2600::tia.player1 / 15 - 1;
				VCS2600::tia.player1 = 15 * multiple15;
				VCS2600::tia.player1 &= 0xff;
			}

			// RESM0 = 0x12
			if (address == 0x12)
			{
				VCS2600::tia.missile0 = (VCS2600::cpu.cycles - VCS2600::lastWSYNC) * 3;
			}

			// RESM1 = 0x13
			if (address == 0x13)
			{
				VCS2600::tia.missile1 = (VCS2600::cpu.cycles - VCS2600::lastWSYNC) * 3;
			}

			// RESBL = 0x14
			if (address == 0x14)
			{
				VCS2600::tia.ball = (VCS2600::cpu.cycles - VCS2600::lastWSYNC) * 3;
			}

			// when any of these are written to, TIA will start drawing that object at this same clock next scanline (if enabled)

			// CXCLR = 0x2C - reset all collision bits

			return ((byte*)VCS2600::tia.GetWriteRegisters()) + address;
		}
		else
		{
			// read registers
			address &= TIA_READ_ADDRESS_MASK;
			return ((byte*)VCS2600::tia.GetReadRegisters()) + address;
		}
	}
	else if ((address & PIA_MASK) == PIA_IO_SELECT)
	{
		// PIA registers
		address &= PIA_READ_ADDRESS_MASK;

		// need to intercept SWCHB address and react appropriately
		if (address == 0x02)	// SWCHB
		{
			// TODO: SWCHB needs to reflect console input switches
//			if (!isWrite)
//				VCS2600::riot.SWCHB = 0x0b;
		}

		return ((byte*)&VCS2600::riot) + address;
	}
	else if ((address & PIA_MASK) == PIA_RAM_SELECT)
	{
		// PIA RAM
		// for debugging Combat - catch read/write watches here:
		// TANKY1
		if (address == 0xa5)
		{
			// who's writing to TANKY1?
			if (isWrite)
				int tanky1 = data;
		}

		// TANKY0
		if (address == 0xa4)
		{
			// who's writing to TANKY0?
			if (isWrite)
				int tanky0 = data;
		}
		// StirTimer
		if (address == 0x8a)
		{
			if (isWrite)
				int stirTimer = data;
		}

		return &VCS2600::ram[address & 0x7f];
	}
	return 0;
}

byte VCS2600::rom[4096];
byte VCS2600::ram[128];
RIOT VCS2600::riot;
TIA VCS2600::tia;
CPU6502 VCS2600::cpu(tiaMemoryMapper);
int VCS2600::frameCounter;
int VCS2600::lastWSYNC;

