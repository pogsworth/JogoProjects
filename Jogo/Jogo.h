#pragma once
#include "int_types.h"
#include "Arena.h"
#include "Bitmap.h"
#include "Font.h"
#include "JMath.h"
#include "Input.h"

namespace Jogo
{
	class App : public Input::InputHandler
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

		App();

		virtual const char* GetName() const { return JogoName; }

		virtual void Init() {}
		virtual bool Tick(float dt) { return false; }
		virtual void Draw() {}
		virtual void Resize(int width, int height);
	};

	// event loop
	void Run(Jogo::App& App, int TargetFPS);

	void SetUIHandler(Input::InputHandler* UIHandler);
	typedef void TickHandler(float DeltaTime);
	void SetTickHandler(TickHandler);

	// memory
	void* Allocate(size_t Size);
	void Free(void* Memory);

	// graphics
	void Show(u32* Buffer, int Width, int Height);
	void DrawString(int x, int y, const str8& string);

	// debug
	void DebugOut(const str8& message);
	void Print(const str8& message);
	template <typename... Args>
	void Printf(Arena& arena, const str8& fmt, const Args&... args)
	{
		Print(str8::format(arena, fmt, args...));
	}

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