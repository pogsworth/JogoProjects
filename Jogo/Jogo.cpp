#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <timeapi.h>
#include "Jogo.h"

namespace Jogo
{
	const char* App::JogoName = "Jogo Default";

	HDC hdc;
	HWND hwnd;
	bool Pause = false;
	Input::InputHandler* UIHandler = nullptr;
	u32 NumTickHandlers = 0;
	const u32 MaxTickHandlers = 8;
	TickHandler* TickHandlers[MaxTickHandlers];

	App::App()
	{
		DefaultArena = Arena::Create(DefaultArenaSize);
		FrameArena = Arena::Create(1024*1024);
		BackBuffer = { (u32)Width, (u32)Height, sizeof(u32) };
		BackBuffer.Pixels = Allocate(Width * Height * sizeof(u32));
		DefaultFont = Font::Load("../Jogo/Font16.fnt", DefaultArena);
	}

	void App::Resize(int width, int height)
	{
		if (width * height > Width * Height)
		{
			Free(BackBuffer.Pixels);
			BackBuffer.Pixels = Allocate(width * height * 4);
		}
		BackBuffer.Width = width;
		BackBuffer.Height = height;
		Width = width;
		Height = height;
	}

	void SetUIHandler(Input::InputHandler* UIHandler)
	{
		Jogo::UIHandler = UIHandler;
	}

	void SetTickHandler(TickHandler Handler)
	{
		if (NumTickHandlers < MaxTickHandlers)
		{
			TickHandlers[NumTickHandlers++] = Handler;
		}
	}

	LRESULT CALLBACK JogoWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// get current input handler
		App* app = (App*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		Input::Keys key = (Input::Keys)wParam;
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (!UIHandler || (UIHandler && !UIHandler->KeyDown(key)))
			{
				if (app)
					app->KeyDown(key);
			}
			break;

		case WM_KEYUP:
			if (app)
				app->KeyUp(key);
			break;

		case WM_CHAR:
			if (!UIHandler || (UIHandler && !UIHandler->Char((char)wParam)))
			{
				if (app)
					app->Char((char)wParam);
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			SetCapture(hwnd);
			if (!UIHandler || (UIHandler && !UIHandler->MouseDown((s32)LOWORD(lParam), (s32)HIWORD(lParam), key)))
			{
				if (app)
					app->MouseDown((s32)LOWORD(lParam), (s32)HIWORD(lParam), key);
			}
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			ReleaseCapture();
			if (!UIHandler || (UIHandler && !UIHandler->MouseUp((s32)LOWORD(lParam), (s32)HIWORD(lParam), key)))
			{
				if (app)
					app->MouseUp((s32)LOWORD(lParam), (s32)HIWORD(lParam), key);
			}
			break;

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
			if (!UIHandler || (UIHandler && !UIHandler->MouseDoubleClick((s32)LOWORD(lParam), (s32)HIWORD(lParam), key)))
			{
				if (app)
					app->MouseDoubleClick((s32)LOWORD(lParam), (s32)HIWORD(lParam), key);
			}
			break;

		case WM_MOUSEMOVE:
			if (app)
				app->MouseMove((s32)LOWORD(lParam), (s32)HIWORD(lParam));
			break;

		case WM_MOUSEWHEEL:
			if (app)
				app->MouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZE:
			{
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);
				if (app)
					app->Resize(width, height);
				InvalidateRect(hwnd, NULL, FALSE);
			}
			break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				if (app)
					app->Draw();
				EndPaint(hwnd, &ps);
			}
			return 0;

		case WM_ACTIVATE:
			if (wParam == WA_INACTIVE)
				Pause = true;
			else
				Pause = false;
			return 0;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	template<class T>
	T Clamp(T input, T min, T max)
	{
		return input < min
			? min
			: input > max
			? max
			: input;
	}

	Timer::Timer()
	{
		if (!Frequency)
		{
			QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&Last);
	}

	u64 Timer::Start() 
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&Last);
		return Last;
	}

	double Timer::GetSecondsSinceLast()
	{
		u64 Now;
		QueryPerformanceCounter((LARGE_INTEGER*)&Now);
		return ((double)Now - Last) / Frequency;
		Last = Now;
	}

	u64 Timer::Frequency = 0;

	u32 Random::GetNext()
	{
		State ^= State << 13;
		State ^= State >> 17;
		State ^= State << 5;

		return State;
	}

	void Run(App& App, int TargetFPS)
	{
		// register window class
		WNDCLASS wc = {};
		wc.style = WS_OVERLAPPED | CS_DBLCLKS;
		wc.lpfnWndProc = JogoWndProc;
		HINSTANCE hInstance = GetModuleHandle(nullptr);
		wc.hInstance = hInstance;
		wc.lpszClassName = App.GetName();
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);

		if (RegisterClass(&wc))
		{
			RECT appRect = { 0, 0, App.Width, App.Height };
			AdjustWindowRect(&appRect, WS_OVERLAPPEDWINDOW, 0);
			u32 WindowWidth = appRect.right - appRect.left;
			u32 WindowHeight = appRect.bottom - appRect.top;

			// create window
			if (hwnd = CreateWindowEx(0, App.GetName(), App.GetName(),
				WS_OVERLAPPEDWINDOW|CS_DBLCLKS, CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight,
				nullptr, nullptr, hInstance, nullptr))
			{
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&App);
				hdc = GetDC(hwnd);
				ShowWindow(hwnd, SW_SHOWNORMAL);
				UpdateWindow(hwnd);

				// default 60Hz
				float TargetFrameTimeMS = 16.6666f;
				TargetFPS = Clamp(TargetFPS, 10, 1000);
				TargetFrameTimeMS = 1000.0f / TargetFPS;

				UINT DesiredSchedulerMS = 1;
				bool SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

				bool Done = false;
				Timer timer;
				timer.Start();
				while (!Done)
				{
					double Seconds = timer.GetSecondsSinceLast();
					float MSeconds = (float)(Seconds * 1000.0);
					int MSecondsLeft = (int)(TargetFrameTimeMS - MSeconds);

					if (MSecondsLeft > 0)
					{
						Sleep(MSecondsLeft);
					}
					float DT = (float)timer.GetSecondsSinceLast();

					if (!Pause)
					{
						timer.Start();

						DT = Clamp(DT, 0.001f, 0.1f);
						Done = App.Tick(DT);
						for (u32 t = 0; t < NumTickHandlers; t++)
						{
							TickHandlers[t](DT);
						}
						App.Draw();
					}
					// message pump
					MSG msg = {};
					while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0)
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);

						if (msg.message == WM_QUIT)
						{
							Done = true;
						}
					}
				}
			}
		}
	}

	void* Allocate(size_t Size)
	{
		return VirtualAlloc(nullptr, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	}

	void Free(void* Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}

	void Show(u32* Buffer, int Width, int Height)
	{
		BITMAPINFO Info = {};
		Info.bmiHeader.biSize = sizeof(Info.bmiHeader);
		Info.bmiHeader.biWidth = Width;
		Info.bmiHeader.biHeight = -Height;
		Info.bmiHeader.biPlanes = 1;
		Info.bmiHeader.biBitCount = 32;
		Info.bmiHeader.biCompression = BI_RGB;


		StretchDIBits(hdc,
			0, 0, Width, Height,
//			128, 768, Width/8, Height/8,
			0, 0, Width, Height,
			Buffer,
			&Info,
			DIB_RGB_COLORS, SRCCOPY);
	}

	void DrawString(int x, int y, const char* string)
	{
		RECT r = { x,y,0,0 };
		DrawText(hdc, string, -1, &r, DT_NOCLIP);
	}

	void DebugOut(const str8& message)
	{
		char* localstring = (char*)_alloca(message.len+1);
		str8::copystring(message.chars, localstring, (u32)message.len, (u32)message.len);
		localstring[message.len] = 0;

		OutputDebugString(localstring);
	}

};

namespace Input
{
	bool IsKeyPressed(int key)
	{
		return (GetAsyncKeyState(key) & 0x8000) != 0;
	}

	void GetMousePos(int& x, int& y)
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(Jogo::hwnd, &point);
		x = point.x;
		y = point.y;
	}
};