#pragma once

#include "Jogo.h"
#include "Bitmap.h"
#include "str8.h"
#include "Input.h"

struct Font;

namespace UI
{
	struct Frame
	{
		Bitmap::Rect FrameRect;
		u32 CursorX;
		u32 CursorY;
		u32 NextID;
		u32 FlowDir;

		Bitmap::Rect PrimitiveBegin()
		{
			Bitmap::Rect loc { (s32)CursorX, (s32)CursorY };
			return loc;
		}
		
		void PrimitiveEnd(const Bitmap::Rect& size)
		{
			if (FlowDir)
			{
				CursorX += size.w + 1;
			}
			else
			{
				CursorY += size.h + 1;
			}
		}
	};

	struct UIInputHandler : public Input::InputHandler
	{
		bool KeyDown(Input::Keys key) override;
		bool KeyUp(Input::Keys key) override { return false; }
		virtual bool Char(char c) override;
		virtual bool MouseDown(s32 x, s32 y, Input::Keys button) override { return false; }
		virtual bool MouseUp(s32 x, s32 y, Input::Keys button) override { return false; }
		virtual bool MouseMove(s32 x, s32 y) override { return false; }
		virtual bool MouseWheel(s32 wheelScroll) override { return false; }
	};

	void Init(const Bitmap& InTarget, const Font& InDefaultFont);
	Input::InputHandler GetHandler();

	u32 GetID();
	u32 GetFocusID();
	bool Interact(u32 Id, const Bitmap::Rect& r);
	bool Button(const Jogo::str8& Text);
	void Label(const Jogo::str8& Text);
	const char* EditBox(const Jogo::str8& Text);
	void RadioButton(const Jogo::str8& Text);
	u32 RadioButtons(u32 choice, const Jogo::str8 strings[], u32 count);
	bool CheckBox(const Jogo::str8& label, bool checked);

	// TODO: fix potential buffer overrun with unpaired BeginFrame/EndFrame
	void PushFrame(const Bitmap::Rect& ThisFrame, u32 FlowDir);
	void PopFrame();

	// need to pass in input state to BeginFrame
	// TODO: reset and establish layout rules within this frame
	void BeginFrame(const Bitmap::Rect& r, u32 Flow = 0);
	void EndFrame();

	Bitmap::Rect MenuFrame();
	void BeginMenu();
	void EndMenu();
	bool MenuButton(const Jogo::str8& label, bool open);
	bool MenuItem(const Jogo::str8& Item, bool checked);
}

