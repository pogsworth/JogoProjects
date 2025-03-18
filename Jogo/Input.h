#pragma once

#include "int_types.h"

namespace Input
{

	enum Keys
	{
		BUTTON_LEFT = 0x01,
		BUTTON_RIGHT = 0x02,
		BUTTON_MIDDLE = 0x04,
		KEY_BACKSPACE = 0x08,
		KEY_TAB = 0x09,
		KEY_ENTER = 0x0D,
		KEY_ESC = 0x1B,
		KEY_PAGEUP = 0x21,
		KEY_PAGEDOWN = 0x22,
		KEY_END = 0x23,
		KEY_HOME = 0x24,
		KEY_LEFT = 0x25,
		KEY_UP = 0x26,
		KEY_RIGHT = 0x27,
		KEY_DOWN = 0x28,
		KEY_DELETE = 0x2E,
	};

	struct InputHandler
	{
		//virtual bool KeyDown(Keys key) = 0;
		//virtual bool KeyUp(Keys key) = 0;
		//virtual bool Char(char c) = 0;
		//virtual bool MouseDown(s32 x, s32 y, Keys button) = 0;
		//virtual bool MouseUp(s32 x, s32 y, Keys button) = 0;
		//virtual bool MouseMove(s32 x, s32 y) = 0;
		//virtual bool MouseWheel(s32 wheelScroll) = 0;

		virtual bool KeyDown(Keys key) { return false; }
		virtual bool KeyUp(Keys key) { return false; }
		virtual bool Char(char c) { return false; }
		virtual bool MouseDown(s32 x, s32 y, Keys button) { return false; }
		virtual bool MouseUp(s32 x, s32 y, Keys button) { return false; }
		virtual bool MouseMove(s32 x, s32 y) { return false; }
		virtual bool MouseWheel(s32 wheelScroll) { return false; }
	};

	// input
	bool IsKeyPressed(int key);
	void GetMousePos(int& x, int& y);

};
