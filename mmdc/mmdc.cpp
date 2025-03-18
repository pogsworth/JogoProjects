#include "Jogo.h"
#include "UI.h"
#include "str8.h"
#include "2600/2600.h"

using namespace Jogo;

#define GUI_W 640
#define GUI_H 480

#define VIDEO_X 4
#define VIDEO_Y 4
#define VIDEO_W 320
#define VIDEO_H 220

#define REGISTERS_X 328
#define REGISTERS_Y 4
#define REGISTERS_W 100
#define REGISTERS_H 226
#define REGISTER_SIZE 14

#define TIA_REG(t,id) {#t,id}
#define TIA_X 590
#define TIA_Y 4
#define TIA_W REGISTERS_W
#define TIA_H 200
#define TIA_SIZE 18
#define TIA_COUNT 45


struct tia_refs {
	char name[8];
	int id;
	int hwnd;
};

tia_refs tiaRefs[] =
{
	TIA_REG(VSYNC, 10401),
	TIA_REG(VBLANK, 10402),
	TIA_REG(WSYNC, 10403),
	TIA_REG(RSYNC, 10404),
	TIA_REG(NUSIZ0, 10405),
	TIA_REG(NUSIZ1, 10406),
	TIA_REG(COLUP0, 10407),
	TIA_REG(COLUP1, 10408),
	TIA_REG(COLUPF, 10409),
	TIA_REG(COLUBK, 10410),
	TIA_REG(CTRLPF, 10411),
	TIA_REG(REFP0, 10412),
	TIA_REG(REFP1, 10413),
	TIA_REG(PF0, 10414),
	TIA_REG(PF1, 10415),
	TIA_REG(PF2, 10416),
	TIA_REG(RESP0, 10417),
	TIA_REG(RESP1, 10418),
	TIA_REG(RESM0, 10419),
	TIA_REG(RESM1, 10420),
	TIA_REG(RESBL, 10421),
	TIA_REG(AUDC0, 10422),
	TIA_REG(AUDC1, 10423),
	TIA_REG(AUDF0, 10424),
	TIA_REG(AUDF1, 10425),
	TIA_REG(AUDV0, 10426),
	TIA_REG(AUDV1, 10427),
	TIA_REG(GRP0, 10428),
	TIA_REG(GRP1, 10429),
	TIA_REG(ENAM0, 10430),
	TIA_REG(ENAM1, 10431),
	TIA_REG(ENABL, 10432),
	TIA_REG(HMP0, 10433),
	TIA_REG(HMP1, 10434),
	TIA_REG(HMM0, 10435),
	TIA_REG(HMM1, 10436),
	TIA_REG(HMBL, 10437),
	TIA_REG(VDELP0, 10438),
	TIA_REG(VDELP1, 10439),
	TIA_REG(VDELBL, 10440),
	TIA_REG(RESMP0, 10441),
	TIA_REG(RESMP1, 10442),
	TIA_REG(HMOVE, 10443),
	TIA_REG(HMCLR, 10444),
	TIA_REG(CXCLR, 10445)
};

#define SOURCE_X 4
#define SOURCE_Y 224
#define SOURCE_W 130
#define SOURCE_H 150
#define SOURCE_ROWS 8

#define BUTTON_SIZE 32
#define BUTTON_GAP 4
#define BUTTON_PANEL_X 4
#define BUTTON_PANEL_Y 400
#define BUTTON_PANEL_W (10*(BUTTON_SIZE+BUTTON_GAP)+BUTTON_GAP)
#define BUTTON_PANEL_H 48

#define RAM_X 138
#define RAM_Y 224
#define RAM_W 432
#define RAM_H 152
#define RAM_ROWS 8

struct MMDC : public Jogo::App
{
	Font AtariFont;
	VCS2600 vcs2600;
	bool paused = false;
	bool step = false;
	bool step_line = false;
	bool step_frame = false;
	bool Done = false;
	u32 paused_frame = 0;
	static const char* Name;

	MMDC()
	{
		vcs2600.Init6502();
		AtariFont = Font::Load("../Jogo/Atari8Tall.fnt", DefaultArena);
		UI::Init(BackBuffer, AtariFont);
	}

	void Resize(int width, int height)
	{
		Jogo::App::Resize(width, height);
		UI::Init(BackBuffer, AtariFont);
	}

	const char* GetName() const override { return (char*)Name; }

	void ShowVideo()
	{
		Bitmap videoFrame;
		videoFrame.Width = 160;
		videoFrame.Height = 220;
		videoFrame.PixelSize = 4;
		videoFrame.Pixels = vcs2600.tia.frameBuffer;
		BackBuffer.PasteBitmapSelectionScaled({ 0,0,320, 220 }, videoFrame, { 0,0, 160,220 }, 0);
	}

	void ShowRegisters()
	{
		Arena& sa = FrameArena;
		UI::BeginFrame({ REGISTERS_X, REGISTERS_Y, REGISTERS_W, REGISTERS_H });

		UI::Label(str8::format("   A: {:02X}", sa, vcs2600.cpu.a));
		UI::Label(str8::format("   X: {:02X}", sa, vcs2600.cpu.x));
		UI::Label(str8::format("   Y: {:02X}", sa, vcs2600.cpu.y));
		UI::Label(str8::format("   S: {:02X}", sa, vcs2600.cpu.s));
		UI::Label(str8::format("PC: {:04X}", sa, vcs2600.cpu.pc));
		UI::Label(str8::format("FLAGS: {:02X}", sa, vcs2600.cpu.status));
		UI::Label(str8::format("OpCode: {:02X}", sa, vcs2600.cpu.opcode));
		UI::Label(str8::format("Clock: {:04X}", sa, vcs2600.cpu.cycles&0xffff));
		UI::Label(str8::format("Frame: {}", sa, vcs2600.frameCounter));

		UI::EndFrame();

		UI::BeginFrame({ TIA_X, TIA_Y, TIA_W, TIA_H });
		for (int i = 0; i < TIA_COUNT / 2; i++)
		{
			UI::Label(str8::format("{:02X} {:6}: {:02X}", sa, i, tiaRefs[i].name, vcs2600.tia.GetWriteRegisters()[i]));
		}
		UI::EndFrame();

		UI::BeginFrame({ TIA_X+TIA_W+16, TIA_Y, TIA_W, TIA_H });
		for (int i = TIA_COUNT/2; i < TIA_COUNT; i++)
		{
			UI::Label(str8::format("{:02X} {:6}: {:02X}", sa, i, tiaRefs[i].name, vcs2600.tia.GetWriteRegisters()[i]));
		}
		UI::EndFrame();
	}

	void ShowRam()
	{
		Arena& sa = FrameArena;
		size_t savedAlignment = sa.Alignment;
		sa.Alignment = 1;
		UI::BeginFrame({ RAM_X, RAM_Y, RAM_W, RAM_H });
		for (int i = 0; i < RAM_ROWS; i++)
		{
			str8 mem = str8::format("{:03X}: ", sa, i * 16 + 128);
			for (int j = 0; j < 8; j++)
			{
				str8 m = str8::format("{:02X} ", sa, vcs2600.ram[j + i * 16]);
				mem.len += m.len;
			}
			str8 space = str8::format(" ", sa);
			mem.len += space.len;
			for (int j = 0; j < 8; j++)
			{
				str8 m = str8::format("{:02X} ", sa, vcs2600.ram[j + 8 + i * 16]);
				mem.len += m.len;
			}
			UI::Label(mem);
		}
		UI::EndFrame();
		sa.Alignment = savedAlignment;

	}

	void ShowSource()
	{
		int length = 0;
		char dis[256];
		// loop through a few instructions and add them to the text output
		UI::BeginFrame({ SOURCE_X, SOURCE_Y, SOURCE_W, SOURCE_H });
		for (int i = 0; i < SOURCE_ROWS; i++)
		{
			str8 s = str8::format("{:04X} ", FrameArena, vcs2600.cpu.pc + length);
//			int address = sprintf_s(dis, sizeof(dis), "%04X ", vcs2600.cpu.pc + length);
			length += vcs2600.cpu.disassemble(vcs2600.cpu.pc + length, dis, sizeof(dis));
			str8 diss = str8::format("{}{}", FrameArena, s, dis);
			UI::Label(diss);
		}
		UI::EndFrame();
	}

	void KeyDown(u32 key)
	{
		if (key == Input::KEY_ESC)
		{
			Done = true;
		}

		if (key == ' ')
		{
			paused = !paused;
			if (paused)
				paused_frame = vcs2600.frameCounter;
		}

		if (key == 'J')
		{
			step = true;
		}

		if (key == 'K')
		{
			step_line = true;
		}

		if (key == 'L')
		{
			step_frame = true;
		}
	}

	bool Tick(float dt) override
	{
		// handle running/pausing emulator here
		if (!paused)
			vcs2600.runFrame();
		else if (step)
			vcs2600.step();
		else if (step_line)
		{
			vcs2600.scanLine();
			if (paused_frame != vcs2600.frameCounter)
			{
				memset(vcs2600.tia.frameBuffer, 0, 160 * 220 * 4);
				paused_frame = vcs2600.frameCounter;
			}
		}
		else if (step_frame)
			vcs2600.runFrame();

		return Done;
	}

	void DoButtons()
	{
		UI::BeginFrame({ BUTTON_PANEL_X, BUTTON_PANEL_Y, BUTTON_PANEL_W, BUTTON_PANEL_H }, 1);
		if (paused)
		{
			if (UI::Button(" >  "))
				paused = !paused;
		}
		else if (UI::Button(" || "))
		{
			paused = !paused;
			paused_frame = vcs2600.frameCounter;
		}
		if (UI::Button(" >| "))
		{
			step = true;
		}
		if (UI::Button(" >>|"))
		{
			step_line = true;
		}
		if (UI::Button(">>>|"))
		{
			step_frame = true;
		}
		UI::EndFrame();
	}

	void Draw() override
	{
		BackBuffer.Erase(0);
		AtariFont.DrawText(100, 400, "MMDC", 0x400000, BackBuffer, 25);
		FrameArena.Clear();
		ShowVideo();
		ShowRegisters();
		ShowSource();
		ShowRam();
		step = step_line = step_frame = false;
		DoButtons();
		Show(BackBuffer.PixelBGRA, BackBuffer.Width, BackBuffer.Height);

	}

};

const char* MMDC::Name = "MMDC";

int main(int argc, char* argv[])
{
	MMDC mmdc;
	Jogo::Run(mmdc, 60);
	return 0;
}