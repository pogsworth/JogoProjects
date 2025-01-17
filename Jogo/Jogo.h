#pragma once
#include "int_types.h"
#include "Arena.h"
#include "Bitmap.h"
#include "Font.h"
#include "JMath.h"

namespace Jogo
{

	class JogoApp
	{
		static const char* JogoName;
	public:
		int Width = 1024;
		int Height = 1024;
		int DefaultArenaSize = 64 * 1024 * 1024;
		Arena DefaultArena;
		Arena FrameArena;
		Bitmap BackBuffer;
		Font DefaultFont;

		JogoApp();

		virtual const char* GetName() const { return JogoName; }

		virtual void Init() {}
		virtual bool Tick(float dt) { return false; }
		virtual void Draw() {}
		virtual void Resize(int width, int height) { Width = width, Height = height; }

		//input
		virtual void KeyDown(u32 key) {}
		virtual void KeyUp(u32 key) {}
		virtual void Char(u32 character) {}
		virtual void MouseDown(s32 x, s32 y, u32 buttons) {}
		virtual void MouseUp(s32 x, s32 y, u32 buttons) {}
		virtual void MouseMove(s32 x, s32 y, u32 buttons) {}
		virtual void MouseWheel(s32 wheelScroll) {}

		static int KEY_LEFT;
		static int KEY_RIGHT;
		static int KEY_UP;
		static int KEY_DOWN;
		static int KEY_ESC;
		static int KEY_ENTER;
		static int KEY_TAB;
		static int KEY_BACKSPACE;
		static int KEY_DELETE;
		static int KEY_HOME;
		static int KEY_END;
		static int BUTTON_LEFT;
		static int BUTTON_RIGHT;
		static int BUTTON_MIDDLE;
	};

	// event loop
	void Run(JogoApp& App, int TargetFPS);

	// memory
	void* Allocate(size_t Size);
	void Free(void* Memory);

	// graphics
	void Show(u32* Buffer, int Width, int Height);
	void DrawString(int x, int y, const char* string);

	// input
	bool IsKeyPressed(int key);
	void GetMousePos(int& x, int& y);

	// debug
	void DebugOut(char* message);

	u32 copystring(const char* src, char* dst, u32 len, u32 destmax);

	struct Timer
	{
		static u64 Frequency;
		u64 Last = 0;

		Timer();
		u64 Start();
		double GetSecondsSinceLast();
	};

	struct Random
	{
		u32 State;

		u32 GetNext();
	};

	inline void Assert(bool expression)
	{
		if (!expression)
		{
			__debugbreak();
		}
	}
};