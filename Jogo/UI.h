#pragma once

#include "Jogo.h"
#include "Bitmap.h"
#include "str8.h"
#include "Input.h"

struct Font;

namespace UI
{
	const int CONTAINER_FLOW = 1;
	const int CONTAINER_RELATIVE = 2;

	struct Container
	{
		Bitmap::Rect ContainerRect;
		u32 CursorX;
		u32 CursorY;
		u32 NextID;
		u32 FlowDir;
		u32 Relative;

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
		virtual bool KeyDown(Input::Keys key) override;
		virtual bool Char(char c) override;
		virtual bool MouseDown(s32 x, s32 y, Input::Keys button) override;
		virtual bool MouseUp(s32 x, s32 y, Input::Keys button) override;
		virtual bool MouseDoubleClick(s32 x, s32 y, Input::Keys button) override;

		u32 InputLimit;
		u32 InputFilter;
		static const u32 FILTER_NONE = 0;
		static const u32 FILTER_NUMERIC = 1;
		static const u32 FILTER_ALPHA = 2;
		static const u32 FILTER_ALPHANUMERIC = 3;
		static const u32 FILTER_INTEGER = 4;	// NUMERIC plus allow one leading '-'
		static const u32 FILTER_REAL = 8; // INTEGER plus one '.' allowed
		static const u32 FILTER_HEX = 16;
	};

	void Init(const Bitmap& InTarget, const Font& InDefaultFont);
	Input::InputHandler GetHandler();

	u32 GetID();
	bool Interact(u32 Id, const Bitmap::Rect& r);
	bool Button(const Jogo::str8& Text);
	void Label(const Jogo::str8& Text);
	const Jogo::str8 EditBox(const Jogo::str8& Text, const Jogo::str8& format="");
	void RadioButton(const Jogo::str8& Text);
	u32 RadioButtons(u32 choice, const Jogo::str8 strings[], u32 count);
	bool CheckBox(const Jogo::str8& label, bool checked);

	// TODO: fix potential buffer overrun with unpaired BeginContainer/EndContainer
	void PushContainer(const Bitmap::Rect& ThisContainer, u32 Flags = 0);
	void PopContainer();

	// need to pass in input state to BeginContainer
	// TODO: reset and establish layout rules within this Container
	void BeginFrame();
	void EndFrame();

	Bitmap::Rect MenuContainer();
	void BeginMenu();
	void EndMenu();
	bool MenuButton(const Jogo::str8& label, bool open);
	bool MenuItem(const Jogo::str8& Item, bool checked);

	void PrintDebug(Arena arena, s32, s32, u32);
}

