#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"
#include "Arena.h"
#include "UI.h"
#include "str8.h"

using namespace Jogo;

class UIExample : public Jogo::App
{
	static const char* Name;

	int DesiredFrameRate = 60;
	float LastFrameDT = 0.0f;
	float LastFrameRate = (float)DesiredFrameRate;
	float AvgFrameRate = LastFrameRate;

	float Step = 0.0f;
	float StepTime = 0.5;

	bool GameOver = true;
	u32 HeartBeatCount = 0;

	s32 mouseX, mouseY;
	s32 deltaX = 0, deltaY = 0;
	bool dragging = false;

	static const size_t GameArenaSize = 128 * 1024 * 1024;
	Arena GameArena;
	Bitmap TestBitmap;
	Jogo::Random RandomNumber;
	Jogo::Timer GameTimer;
	char LastChar[2] = {};
	bool QuitApp = false;
public:

	UIExample()
	{
		RandomNumber.State = (u32)GameTimer.Start();
		GameArena = Arena::Create(GameArenaSize);
		UI::Init(BackBuffer, DefaultFont);
	}

	~UIExample()
	{
		GameArena.ReleaseMemory();
	}

	const char* GetName() const override { return (char*)Name; }

	void Init() override
	{
	}

	bool KeyDown(Input::Keys key) override
	{
		LastChar[0] = key;

		return true;
	}

	bool MouseDown(s32 x, s32 y, Input::Keys buttons) override
	{
		mouseX = x;
		mouseY = y;
		dragging = true;

		return true;
	}

	bool MouseMove(s32 x, s32 y) override
	{
		if (dragging)
		{
			deltaX = x - mouseX;
			deltaY = y - mouseY;
		}

		return true;
	}

	bool MouseUp(s32 x, s32 y, Input::Keys buttons) override
	{
		if (dragging)
		{
			dragging = false;
		}

		return true;
	}

	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		if (Input::IsKeyPressed(Input::KEY_ESC))
		{
			return true;
		}

		bool HeartBeat = false;
		Step += DT;
		if (Step >= StepTime)
		{
			HeartBeat = true;
			HeartBeatCount++;
			Step -= StepTime;
		}

		LastFrameDT = DT;
		LastFrameRate = 1.0f / DT;
		AvgFrameRate = (31.0f * AvgFrameRate + LastFrameRate) / 32.0f;
		return QuitApp;
	}

	void ClearBuffer(u32 color)
	{
		BackBuffer.Erase(color);
	}

	void Draw() override
	{
		u32 black = 0x00000000;
		u32 white = 0xffffffff;
		ClearBuffer(white);

		UI::BeginFrame({ 100,0,Width,Height });

		char ButtonText[] = "Button0";
		static char Clicked[] = "Nothing Clicked.";
		for (u32 i = 0; i < 5; i++)
		{
			ButtonText[6] = '1' + i;
			if (UI::Button(ButtonText))
			{
				char* p = ButtonText;
				char* d = Clicked;
				while (*p)
					*d++ = *p++;
			}
		}
		static char buffer[256] = "Test";
		const Jogo::str8 bufferStr(buffer, Jogo::str8::cstringlength(buffer));
		const Jogo::str8 newstring = UI::EditBox(bufferStr);
		static u32 listofnumbers[10] = { 3,1,4,5,9,2,6,8,7,0 };
		for (u32 i = 0; i < 10; i++)
		{
			listofnumbers[i] = UI::EditBox(Jogo::str8::format(FrameArena, "{:02}", listofnumbers[i])).atoi();
		}
		DefaultFont.DrawText(0, 200, Clicked, 0, 0, BackBuffer);

		static u32 choice = -1;
		const str8 RadioButtons[] =
		{
			"Banana",
			"Apple",
			"Orange",
			"Grape"
		};
		choice = UI::RadioButtons(choice, RadioButtons, 4);

		static bool IsChecked = false;
		IsChecked = UI::CheckBox("Checked", IsChecked);
		static bool FileMenu = false;
		FileMenu = UI::MenuButton("File", FileMenu);
		if (FileMenu)
		{
			UI::BeginMenu();
			static bool OpenMenu = false;
			OpenMenu = UI::MenuButton("Open", OpenMenu);
			if (OpenMenu)
			{
				UI::BeginMenu();
				static bool TestItem = false;
				TestItem = UI::MenuButton("Test", TestItem);
				if (TestItem)
				{
					// print a dialog that says "Test"
				}
				UI::EndMenu();
			}
			static bool ExitItem = false;
			ExitItem = UI::MenuItem("Exit", ExitItem);
			if (ExitItem)
			{
				QuitApp = true;
			}
			char ButtonText[] = "Item 1";
			for (s32 i = 0; i < 4; i++)
			{
				ButtonText[5] = '1' + i;
				UI::MenuItem(ButtonText, false);
			}
			UI::EndMenu();
		}
		UI::EndFrame();
		UI::PrintDebug(FrameArena);

#define TEST_ROUNDEDRECT
#ifdef TEST_ROUNDEDRECT
		static s32 rad = 40;
		static s32 dr = -1;
		Bitmap::Rect rounded = { 150, 400, 400, 400 };
		BackBuffer.DrawRoundedRect(rounded, 2 * rad, rad, 0x4080a0);

		rad += dr;
		if (rad <= 0)
		{
			rad = 0;
			dr = -dr;
		}
		if (rad > 100)
		{
			rad = 100;
			dr = -dr;
		}
#endif		

#ifdef DEBUG_UI
		char HotID[] = "HotID: ";
		char ActiveID[] = "ActiveID: ";
		char HotIDString[8];
		char ActiveIDString[8];
		DefaultFont.DrawText(0, 220, HotID, 0, BackBuffer);
		DefaultFont.DrawText(0, 240, ActiveID, 0, BackBuffer);
		HotIDString[0] = 0;
		ActiveIDString[0] = 0;
		str8::itoa(UI::HotID & 0xfff, HotIDString, sizeof(HotIDString));
		str8::itoa(UI::ActiveID & 0xfff, ActiveIDString, sizeof(ActiveIDString));
		DefaultFont.DrawText(70, 220, HotIDString, 0, BackBuffer);
		DefaultFont.DrawText(100, 240, ActiveIDString, 0, BackBuffer);
#endif // DEBUG_UI
		Jogo::Show(BackBuffer.PixelBGRA, Width, Height);
	}

	// TODO: handle resizing BackBuffer here
	void Resize(int width, int height) override {}
};
const char* UIExample::Name = "Tetris";

int main(int argc, char* argv[])
{
	UIExample ui;
	Jogo::Run(ui, 60);
	return 0;
}