#include <memory.h>
#include <string.h>
#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short ushort;

#define WRITE_FLAG (1<<16)

byte* defaultMapper(int);

struct CPU6502
{
	ushort pc;
	byte	s;
	byte	a;
	byte	x;
	byte	y;
	union
	{
		struct
		{
			byte	status;
		};
		struct
		{
			byte	C : 1;
			byte	Z : 1;
			byte	I : 1;
			byte	D : 1;
			byte	B : 1;
			byte	Q : 1;
			byte	V : 1;
			byte	N : 1;
		};
	};
	bool paused;	// simulate the RDY line
	int cycles;

	CPU6502(byte*(*mapper)(int) = NULL)
	{
		if (mapper)
			mapAddress = mapper;
		else
			mapAddress = defaultMapper;

		paused = false;
		cycles = 0;
	}

	// replace every direct reference into systemRam
	// with a call to memory interface, that examines addresses to divert for io interfaces
	// handle ROM writes etc. and register updates through io writes.

	byte* (*mapAddress)(int);

	byte opcode;

	ushort readWord(ushort address)
	{
		return *(ushort*)mapAddress(address);
	}

	void writeWord(ushort address, ushort word)
	{
		*(ushort*)mapAddress(address|WRITE_FLAG) = word;
	}

	byte readByte(ushort address)
	{
		return *mapAddress(address);
	}

	// if we write to an address that pauses the processor, return true
	bool writeByte(ushort address, byte b)
	{
		// we need to pass a special address to mapAddress to tell it we are going to write to this address
		// so the emulator can react somehow
		*mapAddress(address|WRITE_FLAG|(b<<17)) = b;

		return false;
	}


	byte fetchByte()
	{
		byte b = readByte(pc);
		pc++;
		pc &= 0xffff;
		return b;
	}

	ushort fetchWord()
	{
		// TODO: handle straddling last byte of ram
		ushort word = readWord(pc);
		pc += 2;
		pc &= 0xffff;
		return word;
	}

	byte getMode()
	{
		return ((opcode >> 2) & 7);
	}

	byte getSourceByte()
	{
		// handle immediate case
		if ((opcode & 0x1f) == 0x9)
			return fetchByte();
		ushort address = getDestAddress();
		return readByte(address);
	}

	ushort getDestAddress()
	{
		switch (opcode & 3)
		{
		case 0:
		case 2:
			//000	#immediate
			//001	zero page
			//010	accumulator, except when opcode&3 == 0
			//011	absolute
			//
			//101	zero page,X
			//
			//111	absolute,X
			switch (getMode())
			{
			case 0:
				break;
				//return fetchByte();	// NEED TO RETURN THIS IMMEDIATE BYTE AS THE OPERAND

			case 1:
				return fetchByte();

			case 2:
				// this is handled in the individual instructions ASR, LSL, ROR, ROL
				// if (opcode & 3) return a;	// NEED TO RETURN THE REGISTER A VALUE INSTEAD OF AN ADDRESS
				break;


			case 3:
				return fetchWord();

			case 5:
				return (fetchByte() + x)&0xff;

			case 7:
				return fetchWord() + x;
			}
			break;

		case 1:
			//000	(zero page,X)
			//001	zero page
			//010	#immediate
			//011	absolute
			//100	(zero page),Y
			//101	zero page,X
			//110	absolute,Y
			//111	absolute,X
			switch (getMode())
			{
			case 0:
				return readByte(fetchByte() + x)&0xffff;

			case 1:
				return fetchByte();

			case 2:
				return fetchByte();	// RETURN THIS BYTE AS OPERAND

			case 3:
				return fetchWord();

			case 4:
				return (readWord(fetchByte()) + y)&0xffff;

			case 5:
				return (fetchByte() + x)&0xff;

			case 6:
				return fetchWord() + y;

			case 7:
				return fetchWord() + x;
			}
		}
		// error - unknown opcode!
		return 0;
	}

	void setStatus(byte value)
	{
		Z = value == 0;
		N = value >> 7;
	}

	void push(byte val)
	{
		writeByte(0x100 + s, val);
		s--;
	}

	byte pop()
	{
		s++;
		return readByte(0x100 + s);
	}

	void load(byte* program, ushort address, ushort length)
	{
		ushort max = (address + length) < 65536 ? length : 65536 - address;
		memcpy(mapAddress(address), program, length);
	}

	void load(const char* filename, ushort address)
	{
/*
		FILE* fp;
		fopen_s(&fp, filename, "rb");
		unsigned size = (unsigned)_filelength(_fileno(fp));
		unsigned max = (address + size) < 65536 ? size : 65536 - address;
		fread(systemRam + address, max, 1, fp);
		fclose(fp);
*/
	}

	void reset(ushort vector)
	{
		writeWord(0xfffc, vector);

		pc = vector;
	}

	// returns whether the processor hit a BRK instruction
	bool step()
	{
		static ushort breakpoint = 0xf508;
		if (pc == breakpoint)
		{
			int stop = breakpoint;
		}

		opcode = fetchByte();

  		CPU6502::instruction* inst = instLookup[opcode];
		if (inst != 0)
		{
			(this->*inst->op)();
			cycles += inst->cycles;
			return false;
		}
		return true;
	}

	void pause(bool value)
	{
		paused = value;
	}

	void resetCycles()
	{
		cycles = 0;
	}

	bool run()
	{
		paused = false;
		bool stop = false;

		while (!stop && !paused)
		{
			opcode = fetchByte();

			instruction* inst = instLookup[opcode];
			if (inst != 0)
			{
				(this->*inst->op)();
				cycles += inst->cycles;
			}
			else
				stop = true;
		}

		return paused;
	}

	// fills in buffer with text of disassembly, returns instruction length in bytes
	int disassemble(int pc, char* buffer, int maxlen)
	{
		byte opcode = readByte(pc);
		instruction* inst = instLookup[opcode];
		strcpy_s(buffer, maxlen, inst->name);
		strcat_s(buffer, maxlen, " ");
		ushort operand = 0;
		if (inst->length > 1)
		{
			operand = readByte(pc + 1);
			if (inst->length > 2)
			{
				operand += (readByte(pc + 2) << 8);
			}
		}
		sprintf_s(buffer + 4, maxlen-4, inst->format, operand);

		return inst->length;
	}

	void ADC()
	{
		byte source = getSourceByte();
		ushort sum = a + source + C;
		C = (sum >> 8) & 1;
		V = ((a^source) & 0x80) && ((a^sum) & 0x80);
		a = (byte)sum;
		setStatus(a);
		// todo: deal with D flag
	}

	void AND()
	{
		a &= getSourceByte();
		setStatus(a);
	}

	void ASL()
	{
		// handle register case
		if (opcode == 0x0A)
		{
			C = (a & 0x80) == 0x80;
			a <<= 1;
			setStatus(a);
		}
		else
		{
			ushort mem = getDestAddress();
			byte val = readByte(mem);
			C = (val & 0x80) == 0x80;
			val <<= 1;
			setStatus(val);
			writeByte(mem, val);		// potentially writing to an address (could be register)
		}
	}

	void BCC()
	{
		byte offset = fetchByte();
		if (!C)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BCS()
	{
		byte offset = fetchByte();
		if (C)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BEQ()
	{
		byte offset = fetchByte();
		if (Z)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BIT()
	{
		byte source = getSourceByte();
		N = (source >> 7) & 1;
		V = (source >> 6) & 1;
		Z = (a & source) == 0;
	}

	void BMI()
	{
		byte offset = fetchByte();
		if (N)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BNE()
	{
		byte offset = fetchByte();
		if (!Z)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BPL()
	{
		byte offset = fetchByte();
		if (!N)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BRK()
	{
		push(pc & 0xff);
		push(pc >> 8);
		byte p = status;
		p |= 0x30;	// turn on B flag and reserved (reserved should already be 1)
		push(status);
	}

	void BVC()
	{
		byte offset = fetchByte();
		if (!V)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void BVS()
	{
		byte offset = fetchByte();
		if (V)
		{
			short signedOffset = (signed char)offset;
			pc += signedOffset;
			cycles++;
		}
	}

	void CLC()
	{
		C = 0;
	}

	void CLD()
	{
		D = 0;
	}

	void CLI()
	{
		I = 0;
	}

	void CLV()
	{
		V = 0;
	}

	void CMP()
	{
		short result = (signed char)a - (signed char)getSourceByte();
		setStatus(result & 0xff);
		C = result >= 0;
	}

	void CPX()
	{
		short result;
		if (opcode == 0xE0)
			result = (signed char)x - (signed char)fetchByte();
		else
			short result = (signed char)x - (signed char)getSourceByte();
		setStatus(result & 0xff);
		C = result >= 0;
	}

	void CPY()
	{
		short result;
		if (opcode == 0xC0)
			result = (signed char)y - (signed char)fetchByte();
		else
			short result = (signed char)y - (signed char)getSourceByte();
		setStatus(result & 0xff);
		C = result >= 0;
	}

	void DEC()
	{
		ushort mem = getDestAddress();
		byte val = readByte(mem);
		val--;
		setStatus(val);
		writeByte(mem, val);		// potentially writing to an address (could be register)
	}

	void DEX()
	{
		x--;
		setStatus(x);
	}

	void DEY()
	{
		y--;
		setStatus(y);
	}

	void EOR()
	{
		a ^= getSourceByte();
		setStatus(a);
	}

	void INC()
	{
		ushort mem = getDestAddress();
		byte val = readByte(mem);
		val++;
		setStatus(val);
		writeByte(mem, val);		// potentially writing to an address (could be register)
	}

	void INX()
	{
		x++;
		setStatus(x);
	}

	void INY()
	{
		y++;
		setStatus(y);
	}

	void JMP()
	{
		// todo: figure out indirect jmp
		if (opcode == 0x6C)
			pc = readWord(fetchWord());
		else
			pc = fetchWord();
	}

	void JSR()
	{
		ushort address = pc + 2;
		writeByte(0x100 + s, address & 0xff);
		s--;
		writeByte(0x100 + s, address >> 8);
		s--;
		pc = fetchWord();
	}

	void LDA()
	{
		a = getSourceByte();
		setStatus(a);
	}

	void LDX()
	{
		if (opcode == 0xA2)
			x = fetchByte();
		else
			x = getSourceByte();
		setStatus(x);
	}

	void LDY()
	{
		if (opcode == 0xA0)
			y = fetchByte();
		else
			y = getSourceByte();
		setStatus(y);
	}

	void LSR()
	{
		// handle register case
		if (opcode == 0x4A)
		{
			C = a & 1;
			a >>= 1;
			setStatus(a);
		}
		else
		{
			ushort mem = getDestAddress();
			byte val = readByte(mem);
			C = val & 1;
			val >>= 1;
			setStatus(val);
			writeByte(mem, val);		// potentially writing to an address (could be register)
		}
	}

	void NOP()
	{

	}

	void ORA()
	{
		a |= getSourceByte();
		setStatus(a);
	}

	void PHA()
	{
		push(a);
	}

	void PHP()
	{
		byte p = status;
		p |= 0x30;	// turn on the B flag
		push(p);
	}

	void PLA()
	{
		a = pop();
		setStatus(a);
	}

	void PLP()
	{
		status = pop();
		Q = 1;	// reserved flag is always 1
	}

	void ROL()
	{
		// handle register case
		if (opcode == 0x2A)
		{
			byte saveC = C;
			C = (a >> 7) & 1;
			a <<= 1;
			a |= saveC;
			setStatus(a);
		}
		else
		{
			ushort mem = getDestAddress();
			byte val = readByte(mem);
			byte saveC = C;
			C = (val >> 7) & 1;
			val <<= 1;
			val |= saveC;
			writeByte(mem, val);		// potentially writing to an address (could be register)
			setStatus(val);
		}
	}

	void ROR()
	{
		// handle register case
		if (opcode == 0x6A)
		{
			byte saveC = C;
			C = a & 1;
			a >>= 1;
			a |= saveC << 7;
			setStatus(a);
		}
		else
		{
			ushort mem = getDestAddress();
			byte val = readByte(mem);
			byte saveC = C;
			C = val & 1;
			val >>= 1;
			val |= saveC << 7;
			writeByte(mem, val);		// potentially writing to an address (could be register)
			setStatus(val);
		}
	}

	void RTI()
	{
		status = pop();
		byte pcHi = pop();
		byte pcLo = pop();
		pc = pcHi * 256 + pcLo;
	}

	void RTS()
	{
		byte hi = pop();
		byte lo = pop();
		pc = lo + hi * 256;
	}

	void SBC()
	{
		// invert the bits of the incoming operand
		byte source = getSourceByte();
		ushort sum = a + (~source) + C;
		C = (sum >> 8) & 1;
		V = ((a^source) & 0x80) && ((a^sum) & 0x80);
		a = (byte)sum;
		setStatus(a);
		// todo: deal with D flag
	}

	void SEC()
	{
		C = 1;
	}

	void SED()
	{
		D = 1;
	}

	void SEI()
	{
		I = 1;
	}

	void STA()
	{
		writeByte(getDestAddress(), a);	//write to address

	}

	void STX()
	{
		writeByte(getDestAddress(), x);	//write to address
	}

	void STY()
	{
		writeByte(getDestAddress(), y);	//write to address
	}

	void TAX()
	{
		x = a;
		setStatus(x);
	}

	void TAY()
	{
		y = a;
		setStatus(y);
	}

	void TSX()
	{
		x = s;
		setStatus(x);
	}

	void TXA()
	{
		a = x;
		setStatus(a);
	}

	void TXS()
	{
		s = x;
	}

	void TYA()
	{
		a = y;
		setStatus(a);
	}

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
	typedef void (CPU6502::*opFunction)();
	struct instruction
	{
		instruction(opFunction op)
		{
			this->op = op;
			this->length = 1;
			this->cycles = 1;
			this->name = "";
			this->format = "";
		}
		instruction(opFunction op, int length, int cycles, char* name, char* format)
		{
			this->op = op;
			this->length = length;
			this->cycles = cycles;
			this->name = name;
			this->format = format;
		}
		opFunction op;
		int length;
		int cycles;
		char* name;
		char* format;
	};
	static instruction* instLookup[];
};