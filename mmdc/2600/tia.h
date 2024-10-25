#include "vcs.h"

int tia_colors[];
typedef unsigned char byte;
int defaultFrameBuffer[];


struct TIA
{
	TIA_WRITE write_registers;
	TIA_READ read_registers;
	int* frameBuffer;
	int current_scanline;
	bool wSync;
	bool vSync;
	bool vBlank;
	// cached horizontal positions:
	unsigned int player0;
	unsigned int player1;
	unsigned int missile0;
	unsigned int missile1;
	unsigned int ball;
	byte GRP0_del;
	byte GRP1_del;

	// we need a palette of RGB colors to draw from

	TIA(int* videoMemory = NULL)
	{
		if (videoMemory != NULL)
			frameBuffer = videoMemory;
		else
			frameBuffer = defaultFrameBuffer;
	}

	void setVideoMemory(int* videoBuffer)
	{
		frameBuffer = videoBuffer;
	}

	byte* GetWriteRegisters()
	{
		return (byte*)&write_registers;
	}

	byte* GetReadRegisters()
	{
		return (byte*)&read_registers;
	}

	// When WSYNC is written to, pause the processor
	void SetWSync()
	{
		wSync = true;
	}

	// We need to do something when VSYNC is written to
	void SetVSync()
	{
		vSync = true;
		current_scanline = 0;
	}

	void SetVBlank(bool vBlank)
	{
		this->vBlank = vBlank;
		current_scanline = 0;
	}

	void DrawScanLine()
	{
		if (!vBlank && (current_scanline < 220))
		{
			// take the registers that have been set and draw an scan of the image
			int* pixel = frameBuffer + current_scanline * 160;
			unsigned char bkindex = write_registers.COLUBK;
			unsigned char pfindex = write_registers.COLUPF;
			int bkcolor = tia_colors[bkindex >> 1];
			int pfcolor = tia_colors[pfindex >> 1];
			int pf1 = 0;
			for (int i = 0; i < 8; i++, pf1 <<= 1)
				if ((1 << i) & write_registers.PF1)
					pf1 |= 1;
			pf1 >>= 1;
			int playfield = ((write_registers.PF0 >> 4) & 0xf) + (pf1 << 4) + (write_registers.PF2 << 12);
			for (int i = 0; i < 20; i++)
			{
				if (playfield & (1 << i))
				{
					*pixel++ = pfcolor;
					*pixel++ = pfcolor;
					*pixel++ = pfcolor;
					*pixel++ = pfcolor;
				}
				else
				{
					*pixel++ = bkcolor;
					*pixel++ = bkcolor;
					*pixel++ = bkcolor;
					*pixel++ = bkcolor;
				}
			}
			// if reflect bit is set, reverse the playfield bits
			if (write_registers.CTRLPF & 1)
			{
				int temp = 0;
				for (int i = 0; i < 20; i++, temp <<= 1)
				{
					if ((1 << i) & playfield)
						temp++;
				}
				playfield = temp >> 1;
			}

			for (int i = 0; i < 20; i++)
			{
				if (playfield & (1 << i))
				{
					*pixel++ = pfcolor;
					*pixel++ = pfcolor;
					*pixel++ = pfcolor;
					*pixel++ = pfcolor;
				}
				else
				{
					*pixel++ = bkcolor;
					*pixel++ = bkcolor;
					*pixel++ = bkcolor;
					*pixel++ = bkcolor;
				}
			}
			// draw player0
			byte player0_byte = write_registers.GRP0;
			if (write_registers.VDELP0 & 1)
				player0_byte = GRP0_del;
			if (player0_byte)
			{
				int* pixel = frameBuffer + current_scanline * 160 + player0;
				byte p0index = write_registers.COLUP0;
				int p0color = tia_colors[p0index >> 1];
				if (write_registers.REFP0 & 0x8)
				{
					for (int i = 0; i < 8; i++, pixel++)
						if (player0_byte & (1 << i))
							*pixel = p0color;
				}
				else
				{
					for (int i = 0; i < 8; i++, pixel++)
						if (player0_byte & (0x80 >> i))
							*pixel = p0color;
				}
			}
			byte player1_byte = write_registers.GRP1;
			if (write_registers.VDELP1 & 1)
				player1_byte = GRP1_del;
			if (player1_byte)
			{
				int* pixel = frameBuffer + current_scanline * 160 + player1;
				byte p1index = write_registers.COLUP1;
				int p1color = tia_colors[p1index >> 1];
				if (write_registers.REFP1 & 0x8)
				{
					for (int i = 0; i < 8; i++, pixel++)
						if (player1_byte & (1 << i))
							*pixel = p1color;
				}
				else
				{
					for (int i = 0; i < 8; i++, pixel++)
						if (player1_byte & (0x80 >> i))
							*pixel = p1color;
				}
			}
			current_scanline++;
		}
		wSync = false;
	}

};