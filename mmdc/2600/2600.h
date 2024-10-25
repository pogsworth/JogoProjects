#include "vcs.h"
#include "6502/6502.h"
#include "tia.h"

struct VCS2600
{
	static byte rom[4096];
	static byte ram[128];
	static RIOT riot;
	static TIA tia;
	static CPU6502 cpu;
	static int frameCounter;
	static int lastWSYNC;

	VCS2600();
	void Init6502();
	void DisassembleRom();
	void step();
	void scanLine();
	void runFrame();
};




