#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "2600/2600.h"

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
	HWND hwnd;
};

tia_refs g_tiaRefs[] =
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

LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

VCS2600* g_2600 = NULL;
bool g_paused = false;
bool g_step = false;
bool g_step_line = false;
bool g_step_frame = false;

HBITMAP dibSection = 0;
BITMAPINFO videoBitmap = {};
VOID *pbitmapMemory = 0;

HWND g_videoPanel;
HWND g_pauseButton;
HWND g_stepButton;
HWND g_lineButton;
HWND g_frameButton;
HWND g_resetButton;
HWND g_resetClockButton;
HWND g_registerA;
HWND g_registerX;
HWND g_registerY;
HWND g_registerS;
HWND g_registerPC;
HWND g_registerFLAG;
HWND g_registerOpCode;
HWND g_registerClock;
HWND g_registerFrames;
HWND g_sourceWindow[SOURCE_ROWS];
HWND g_ramWindow[RAM_ROWS];

void UpdateWindow();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR pCmdLine, int nCmdShow)
{
	const char CLASS_NAME[] = "2600GUI";

	// register a window class
	WNDCLASS wc = {};	// initialize the struct to zero
	wc.lpfnWndProc = WindProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = (LPCSTR)CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&wc);

	g_2600 = new VCS2600();
	g_2600->Init6502();

	// create a window using that class
	HWND hwnd = CreateWindow(CLASS_NAME, "2600GUI", WS_OVERLAPPEDWINDOW - WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, GUI_W, GUI_H,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nCmdShow);

	//	SetTimer(hwnd, NULL, 16, NULL);
	// event loop
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			static bool last_frame = false;

			// handle running/pausing emulator here
			if (!g_paused)
				g_2600->runFrame();
			else if (g_step)
				g_2600->step();
			else if (g_step_line)
				g_2600->scanLine();
			else if (g_step_frame)
				g_2600->runFrame();

			RECT rect = { VIDEO_X, VIDEO_Y, VIDEO_X + VIDEO_W, VIDEO_Y + VIDEO_H };
			if (!last_frame || g_step || g_step_line || g_step_frame)
			{
				//UpdateWindow();
				InvalidateRect(hwnd, &rect, FALSE);
			}
			last_frame = g_paused;
			if (g_step_frame)
				memset(g_2600->tia.frameBuffer, 0, 160 * 220 * 4);
			g_step = g_step_line = g_step_frame = false;
		}
	}

	return 0;
}

bool CALLBACK SetFont(HWND child, LPARAM font) {
	SendMessage(child, WM_SETFONT, font, true);
	return true;
}


void ShowVideo(HDC hdc)
{
	StretchDIBits(hdc, VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H,
		0, 0, 160, 220, g_2600->tia.frameBuffer,
		&videoBitmap, DIB_RGB_COLORS, SRCCOPY);
}

void ShowRegisters()
{
	char reg[256];
	wsprintf(reg, "A: %02X", g_2600->cpu.a);
	SetWindowText(g_registerA, reg);
	wsprintf(reg, "X: %02X", g_2600->cpu.x);
	SetWindowText(g_registerX, reg);
	wsprintf(reg, "Y: %02X", g_2600->cpu.y);
	SetWindowText(g_registerY, reg);
	wsprintf(reg, "S: %02X", g_2600->cpu.s);
	SetWindowText(g_registerS, reg);
	wsprintf(reg, "PC: %04X", g_2600->cpu.pc);
	SetWindowText(g_registerPC, reg);
	wsprintf(reg, "FLAGS: %02X", g_2600->cpu.status);
	SetWindowText(g_registerFLAG, reg);
	wsprintf(reg, "OpCode: %02X", g_2600->cpu.opcode);
	SetWindowText(g_registerOpCode, reg);
	wsprintf(reg, "Clock: %d", g_2600->cpu.cycles);
	SetWindowText(g_registerClock, reg);
	wsprintf(reg, "Frame: %d", g_2600->frameCounter);
	SetWindowText(g_registerFrames, reg);

	for (int i = 0; i < TIA_COUNT; i++)
	{
		wsprintf(reg, "%02X %6s: %02X", i, g_tiaRefs[i].name, g_2600->tia.GetWriteRegisters()[i]);
		SetWindowText(g_tiaRefs[i].hwnd, reg);
	}
}

void ShowRam()
{
	char ram[2560];
	int sofar;
	for (int i = 0; i < RAM_ROWS; i++)
	{
		sofar = wsprintf(ram, "%03X ", i * 16 + 128);
		for (int j = 0; j < 8; j++)
		{
			sofar += wsprintf(ram + sofar, "%02X ", g_2600->ram[j + i * 16]);
		}
		sofar += wsprintf(ram + sofar, " ");
		for (int j = 0; j < 8; j++)
		{
			sofar += wsprintf(ram + sofar, "%02X ", g_2600->ram[j + 8  + i * 16]);
		}
		SetWindowText(g_ramWindow[i], ram);
	}
}

void ShowSource()
{
	int length = 0;
	char dis[256];
	// loop through a few instructions and add them to the text output
	for (int i = 0; i < SOURCE_ROWS; i++)
	{
		int address = sprintf_s(dis, sizeof(dis), "%04X ", g_2600->cpu.pc + length);
		length += g_2600->cpu.disassemble(g_2600->cpu.pc + length, dis, sizeof(dis));
		strcat_s(dis, sizeof(dis), "      ");
		SetWindowText(g_sourceWindow[i], dis);
	}
}

void UpdateWindow()
{
	ShowRegisters();
	ShowSource();
	ShowRam();
}

WNDPROC origStaticProc;

LRESULT CALLBACK WindProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		// create all the child panels we need
		// video panel
		// registers panel - with multiple edit controls and labels
		// disassembly panel - edit control with multiple rows of text
		// button panel, for pausing, stepping program control
		g_videoPanel = CreateWindowEx(0, "Static", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, VIDEO_X, VIDEO_Y, VIDEO_W, VIDEO_H, hwnd, (HMENU)ID_VIDEO, NULL, NULL);
		HDC hdc = GetDC(hwnd);
		videoBitmap.bmiHeader.biSize = sizeof(videoBitmap.bmiHeader);
		videoBitmap.bmiHeader.biWidth = 160;
		videoBitmap.bmiHeader.biHeight = -220;
		videoBitmap.bmiHeader.biPlanes = 1;
		videoBitmap.bmiHeader.biBitCount = 32; 
		videoBitmap.bmiHeader.biCompression = BI_RGB;
		videoBitmap.bmiHeader.biSizeImage = VIDEO_W * VIDEO_H * 4;
		dibSection = CreateDIBSection(hdc, &videoBitmap, 0, &pbitmapMemory, NULL, 0);
		*(int*)pbitmapMemory = 0xffffffff;
		g_2600->tia.setVideoMemory((int*)pbitmapMemory);
		g_pauseButton = CreateWindowEx(0, "BUTTON", "||", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP, BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_RUNPAUSE, NULL, NULL);
		g_stepButton = CreateWindowEx(0, "BUTTON", ">|", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + 2 * BUTTON_GAP + BUTTON_SIZE, BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_STEP_OP, NULL, NULL);
		g_lineButton = CreateWindowEx(0, "BUTTON", ">>|", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 2 * (BUTTON_GAP + BUTTON_SIZE), BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_STEP_LINE, NULL, NULL);
		g_frameButton = CreateWindowEx(0, "BUTTON", ">>>|", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 3 * (BUTTON_GAP + BUTTON_SIZE), BUTTON_PANEL_Y + BUTTON_GAP, BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_STEP_FRAME, NULL, NULL);
		g_resetButton = CreateWindowEx(0, "BUTTON", "Reset", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 4 * (BUTTON_GAP + BUTTON_SIZE), BUTTON_PANEL_Y + BUTTON_GAP, 2*BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_RESET, NULL, NULL);
		g_resetClockButton = CreateWindowEx(0, "BUTTON", "Reset Clock", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, BUTTON_PANEL_X + BUTTON_GAP + 5 * (BUTTON_GAP + BUTTON_SIZE)+BUTTON_SIZE, BUTTON_PANEL_Y + BUTTON_GAP, 3*BUTTON_SIZE, BUTTON_SIZE, hwnd, (HMENU)ID_RESET_CLOCK, NULL, NULL);

		g_registerA = CreateWindowEx(0, "STATIC", "A: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERA, NULL, NULL);
		g_registerX = CreateWindowEx(0, "STATIC", "X: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERX, NULL, NULL);
		g_registerY = CreateWindowEx(0, "STATIC", "Y: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 2*REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERY, NULL, NULL);
		g_registerS = CreateWindowEx(0, "STATIC", "S: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 3*REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERS, NULL, NULL);
		g_registerPC = CreateWindowEx(0, "STATIC", "PC: 0000", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 4*REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERPC, NULL, NULL);
		g_registerFLAG = CreateWindowEx(0, "STATIC", "FLAGS: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 5 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERFLAGS, NULL, NULL);
		g_registerOpCode = CreateWindowEx(0, "STATIC", "OpCode: 00", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 6 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTEROPCODE, NULL, NULL);
		g_registerClock = CreateWindowEx(0, "STATIC", "Clock: 0", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 7 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERCLOCK, NULL, NULL);
		g_registerFrames = CreateWindowEx(0, "STATIC", "Frame: 0", WS_CHILD | WS_VISIBLE | SS_SIMPLE, REGISTERS_X, REGISTERS_Y + 8 * REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)ID_REGISTERFRAMES, NULL, NULL);

		for (int i = 0; i < SOURCE_ROWS; i++)
		{
			g_sourceWindow[i] = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX, SOURCE_X, SOURCE_Y + i*REGISTER_SIZE, SOURCE_W, REGISTER_SIZE, hwnd, (HMENU)(ID_SOURCE+i), NULL, NULL);
		}
		for (int i = 0; i < TIA_COUNT; i++)
		{
			g_tiaRefs[i].hwnd = CreateWindowEx(0, "STATIC", g_tiaRefs[i].name, WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX, TIA_X + (i/23)*REGISTERS_W, TIA_Y + (i%23)* REGISTER_SIZE, REGISTERS_W, REGISTER_SIZE, hwnd, (HMENU)g_tiaRefs[i].id, NULL, NULL);
		}
		for (int i = 0; i < RAM_ROWS; i++)
		{
			g_ramWindow[i] = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_SIMPLE | SS_NOPREFIX, RAM_X, RAM_Y + i*REGISTER_SIZE, RAM_W, REGISTER_SIZE, hwnd, (HMENU)(ID_RAM+i), NULL, NULL);
		}

		HFONT hf = CreateFont(6, 0, 0, 0, 0, FALSE, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, "Atari Classic Chunky");
		EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)hf);
	}
	break;

	case WM_COMMAND:
		if (lParam && (HIWORD(wParam) == BN_CLICKED))
		switch (LOWORD(wParam))
		{
		case ID_RUNPAUSE:
			if (g_paused)
			{
				SetWindowText(g_pauseButton, "||");
				g_paused = false;
			}
			else
			{
				SetWindowText(g_pauseButton, ">");
				g_paused = true;
			}
			break;

		case ID_STEP_OP:
			g_step = true;
			break;

		case ID_STEP_LINE:
			g_step_line = true;
			break;

		case ID_STEP_FRAME:
			g_step_frame = true;
			break;

		case ID_RESET:
			break;

		case ID_RESET_CLOCK:
			g_2600->cpu.resetCycles();
			break;

		}
		break;

	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	//case WM_ERASEBKGND:
	//	return (LRESULT)1;
	//	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		//FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
		ShowVideo(hdc);
		ShowRegisters();
		ShowSource();
		ShowRam();
		EndPaint(hwnd, &ps);
	}
	break;

	case WM_TIMER:
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else
		{
			switch (wParam)
			{
			case VK_F1:
				// turn off low bit of console switches - reset button
				g_2600->riot.SWCHB &= 0xfe;
				break;

			}
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}