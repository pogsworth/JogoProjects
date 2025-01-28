#include "Jogo.h"
#include "2600/2600.h"

using namespace Jogo;

#define GUI_W 640
#define GUI_H 480

#define ID_VIDEO 10001
#define VIDEO_X 4
#define VIDEO_Y 4
#define VIDEO_W 320
#define VIDEO_H 220

#define REGISTERS_X 328
#define REGISTERS_Y 4
#define REGISTERS_W 90
#define REGISTERS_H 200
#define REGISTER_SIZE 14
#define ID_REGISTERA 10101
#define ID_REGISTERX 10102
#define ID_REGISTERY 10103
#define ID_REGISTERS 10104
#define ID_REGISTERPC 10105
#define ID_REGISTERFLAGS 10106
#define ID_REGISTEROPCODE 10107
#define ID_REGISTERCLOCK 10108
#define ID_REGISTERFRAMES 10109

#define TIA_REG(t,id) {#t,id}
#define TIA_X 430
#define TIA_Y 4
#define TIA_W REGISTERS_W*2
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

#define ID_SOURCE 10200
#define SOURCE_X 4
#define SOURCE_Y 208
#define SOURCE_W 104
#define SOURCE_H 150
#define SOURCE_ROWS 8

#define ID_BUTTONPANEL 10300
#define BUTTON_SIZE 32
#define BUTTON_GAP 4
#define BUTTON_PANEL_X 4
#define BUTTON_PANEL_Y 360
#define BUTTON_PANEL_W (10*(BUTTON_SIZE+BUTTON_GAP)+BUTTON_GAP)
#define BUTTON_PANEL_H 48
#define ID_RUNPAUSE 10301
#define ID_STEP_OP 10302
#define ID_STEP_LINE 10303
#define ID_STEP_FRAME 10304
#define ID_RESET 10305
#define ID_RESET_CLOCK 10306
#define ID_FRAME_COUNT 10307

#define ID_RAM 10400
#define RAM_X 108
#define RAM_Y 208
#define RAM_W 320
#define RAM_H 152
#define RAM_ROWS 8

struct MMDC : public JogoApp
{
	VCS2600 vcs2600;
	bool paused = false;
	bool step = false;
	bool step_line = false;
	bool step_frame = false;
	bool Done = false;

	MMDC()
	{
		vcs2600.Init6502();
	}

	void ShowVideo()
	{
		Bitmap videoFrame;
		videoFrame.Width = 160;
		videoFrame.Height = 220;
		videoFrame.PixelSize = 4;
		videoFrame.Pixels = vcs2600.tia.frameBuffer;
//		BackBuffer.PasteBitmap(VIDEO_X, VIDEO_Y, videoFrame, 0);
//		BackBuffer.PasteBitmapSelectionScaled({ VIDEO_X, VIDEO_Y, 160, 220 }, videoFrame, { 0,0,160,220 }, 0);
		BackBuffer.PasteBitmapSelectionScaled({ 0,0,640, 440 }, videoFrame, { 0,0, 160,220 }, 0);
//		BackBuffer.PasteBitmapSelection( 0,0, videoFrame, { 0,0, 160,220 }, 0);
	}

	void ShowRegisters()
	{
		//char reg[256];
		//wsprintf(reg, "A: %02X", vcs2600.cpu.a);
		//SetWindowText(registerA, reg);
		//wsprintf(reg, "X: %02X", vcs2600.cpu.x);
		//SetWindowText(registerX, reg);
		//wsprintf(reg, "Y: %02X", vcs2600.cpu.y);
		//SetWindowText(registerY, reg);
		//wsprintf(reg, "S: %02X", vcs2600.cpu.s);
		//SetWindowText(registerS, reg);
		//wsprintf(reg, "PC: %04X", vcs2600.cpu.pc);
		//SetWindowText(registerPC, reg);
		//wsprintf(reg, "FLAGS: %02X", vcs2600.cpu.status);
		//SetWindowText(registerFLAG, reg);
		//wsprintf(reg, "OpCode: %02X", vcs2600.cpu.opcode);
		//SetWindowText(registerOpCode, reg);
		//wsprintf(reg, "Clock: %d", vcs2600.cpu.cycles);
		//SetWindowText(registerClock, reg);
		//wsprintf(reg, "Frame: %d", vcs2600.frameCounter);
		//SetWindowText(registerFrames, reg);

		//for (int i = 0; i < TIA_COUNT; i++)
		//{
		//	wsprintf(reg, "%02X %6s: %02X", i, tiaRefs[i].name, vcs2600.tia.GetWriteRegisters()[i]);
		//	SetWindowText(tiaRefs[i].hwnd, reg);
		//}
	}

	void ShowRam()
	{
		//char ram[2560];
		//int sofar;
		//for (int i = 0; i < RAM_ROWS; i++)
		//{
		//	sofar = wsprintf(ram, "%03X ", i * 16 + 128);
		//	for (int j = 0; j < 8; j++)
		//	{
		//		sofar += wsprintf(ram + sofar, "%02X ", vcs2600.ram[j + i * 16]);
		//	}
		//	sofar += wsprintf(ram + sofar, " ");
		//	for (int j = 0; j < 8; j++)
		//	{
		//		sofar += wsprintf(ram + sofar, "%02X ", vcs2600.ram[j + 8 + i * 16]);
		//	}
		//	SetWindowText(ramWindow[i], ram);
		//}
	}

	void ShowSource()
	{
		//int length = 0;
		//char dis[256];
		//// loop through a few instructions and add them to the text output
		//for (int i = 0; i < SOURCE_ROWS; i++)
		//{
		//	int address = sprintf_s(dis, sizeof(dis), "%04X ", vcs2600.cpu.pc + length);
		//	length += vcs2600.cpu.disassemble(vcs2600.cpu.pc + length, dis, sizeof(dis));
		//	strcat_s(dis, sizeof(dis), "      ");
		//	SetWindowText(sourceWindow[i], dis);
		//}
	}

	void KeyDown(u32 key)
	{
		if (key == KEY_ESC)
		{
			Done = true;
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
			vcs2600.scanLine();
		else if (step_frame)
			vcs2600.runFrame();

		if (step_frame)
			memset(vcs2600.tia.frameBuffer, 0, 160 * 220 * 4);
		step = step_line = step_frame = false;

		return Done;
	}

	void Draw() override
	{
		ShowVideo();
		ShowRegisters();
		ShowSource();
		ShowRam();
		Show(BackBuffer.PixelBGRA, BackBuffer.Width, BackBuffer.Height);
	}

			// create all the child panels we need
			// video panel
			// registers panel - with multiple edit controls and labels
			// disassembly panel - edit control with multiple rows of text
			// button panel, for pausing, stepping program control
			//videoPanel = CreateWindowEx(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H, hwnd, (HMENU)ID_VIDEO, NULL, NULL);
			//HDC hdc = GetDC(hwnd);
			//videoBitmap.bmiHeader.biSize = sizeof(videoBitmap.bmiHeader);
			//videoBitmap.bmiHeader.biWidth = 160;
			//videoBitmap.bmiHeader.biHeight = -220;
			//videoBitmap.bmiHeader.biPlanes = 1;
			//videoBitmap.bmiHeader.biBitCount = 32;
			//videoBitmap.bmiHeader.biCompression = BI_RGB;
			//videoBitmap.bmiHeader.biSizeImage = VIDEO_W * VIDEO_H * 4;
			//dibSection = CreateDIBSection(hdc, &videoBitmap, 0, &pbitmapMemory, NULL, 0);
			//*(int*)pbitmapMemory = 0xffffffff;
			//vcs2600.tia.setVideoMemory((int*)pbitmapMemory);
			//pauseButton = CreateWindowEx(0, "BUTTON", "||", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP, BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_RUNPAUSE, NULL, NULL);
			//stepButton = CreateWindowEx(0, "BUTTON", ">|", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + 2 * BUTTON_GAP + BUTTON_SIZE, BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_STEP_OP, NULL, NULL);
			//lineButton = CreateWindowEx(0, "BUTTON", ">>|", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 2 * (BUTTON_GAP + BUTTON_SIZE), BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_STEP_LINE, NULL, NULL);
			//frameButton = CreateWindowEx(0, "BUTTON", ">>>|", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 3 * (BUTTON_GAP + BUTTON_SIZE), BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_STEP_FRAME, NULL, NULL);
			//resetButton = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 4 * (BUTTON_GAP + BUTTON_SIZE), BUTTON_PANEL_Y + BUTTON_GAP, 2 * BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_RESET, NULL, NULL);
			//resetClockButton = CreateWindowEx(0, "BUTTON", "Reset Clock", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 5 * (BUTTON_GAP + BUTTON_SIZE) + BUTTON_SIZE, BUTTON_PANEL_Y + BUTTON_GAP, 3 * BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_RESET_CLOCK, NULL, NULL);

			//registerA = CreateWindowEx(0, "STATIC", "A: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERA, NULL, NULL);
			//registerX = CreateWindowEx(0, "STATIC", "X: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERX, NULL, NULL);
			//registerY = CreateWindowEx(0, "STATIC", "Y: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 2 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERY, NULL, NULL);
			//registerS = CreateWindowEx(0, "STATIC", "S: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 3 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERS, NULL, NULL);
			//registerPC = CreateWindowEx(0, "STATIC", "PC: 0000", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 4 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERPC, NULL, NULL);
			//registerFLAG = CreateWindowEx(0, "STATIC", "FLAGS: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 5 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERFLAGS, NULL, NULL);
			//registerOpCode = CreateWindowEx(0, "STATIC", "OpCode: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 6 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTEROPCODE, NULL, NULL);
			//registerClock = CreateWindowEx(0, "STATIC", "Clock: 0", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 7 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERCLOCK, NULL, NULL);
			//registerFrames = CreateWindowEx(0, "STATIC", "Frame: 0", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 8 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERFRAMES, NULL, NULL);

			//for (int i = 0; i < SOURCE_ROWS; i++)
			//{
			//	sourceWindow[i] = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX, SOURCE_X, SOURCE_Y + i * REGISTER_SIZE, SOURCE_W, REGISTER_SIZE, hwnd, (HMENU)(ID_SOURCE + i), NULL, NULL);
			//}
			//for (int i = 0; i < TIA_COUNT; i++)
			//{
			//	tiaRefs[i].hwnd = CreateWindowEx(0, "STATIC", tiaRefs[i].name, WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX, TIA_X + (i / 23) * REGISTERS_W, TIA_Y + (i % 23) * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)tiaRefs[i].id, NULL, NULL);
			//}
			//for (int i = 0; i < RAM_ROWS; i++)
			//{
			//	ramWindow[i] = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX, RAM_X, RAM_Y + i * REGISTER_SIZE, RAM_W, REGISTER_SIZE, hwnd, (HMENU)(ID_RAM + i), NULL, NULL);
			//}

			//HFONT hf = CreateFont(8, 0, 0, 0, 0, FALSE, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, "Atari Classic Chunky");
			//EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)hf);
			//	switch (LOWORD(wParam))
			//	{
			//	case ID_RUNPAUSE:
			//		if (paused)
			//		{
			//			SetWindowText(pauseButton, "||");
			//			paused = false;
			//		}
			//		else
			//		{
			//			SetWindowText(pauseButton, ">");
			//			paused = true;
			//		}
			//		break;

			//	case ID_STEP_OP:
			//		step = true;
			//		break;

			//	case ID_STEP_LINE:
			//		step_line = true;
			//		break;

			//	case ID_STEP_FRAME:
			//		step_frame = true;
			//		break;

			//	case ID_RESET:
			//		break;

			//	case ID_RESET_CLOCK:
			//		vcs2600.cpu.resetCycles();
			//		break;

			//	}
			//break;

		//case WM_PAINT:
		//{
		//	PAINTSTRUCT ps;
		//	HDC hdc = BeginPaint(hwnd, &ps);
		//	//FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
		//	ShowVideo(hdc);
		//	ShowRegisters();
		//	ShowSource();
		//	ShowRam();
		//	EndPaint(hwnd, &ps);
		//}
		//break;

				//case VK_F1:
				//	// turn off low bit of console switches - reset button
				//	vcs2600.riot.SWCHB &= 0xfe;
				//	break;

};


int main(int argc, char* argv[])
{
	MMDC mmdc;
	Jogo::Run(mmdc, 60);
	return 0;
}