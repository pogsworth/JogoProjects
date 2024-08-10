#pragma once

#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"

namespace UI
{
	Bitmap Target;
	Font DefaultFont;
	const u32 FirstID = 0xf000;
	u32 NextID = FirstID;
	u32 HotID = 0;
	u32 ActiveID = 0;
	u32 CaptureID = 0;
	u32 UncaptureID = 0;
	s32 CursorX;
	s32 CursorY;
	u32 ButtonColor = 0x808080;
	u32 LabelColor = 0x404040;
	u32 EditColor = 0x303030;
	u32 EditColorActive = 0x000000;
	u32 HotColor = 0xa0a0a0;
	u32 HiLight = 0xc0c0c0;
	u32 LoLight = 0x404040;
	u32 TextColor = 0xf0f0f0;
	u32 CurrentColor;
	u32 HiColor;
	u32 LoColor;
	u32 Black = 0;
	u32 White = 0xffffff;
	char EditBuffer[256];
	u32 InsertionPoint = 0;
	s32 RadioButtonGroupStart = -1;
	s32 RadioButtonGroupWidth;
	u32 RadioChoice = -1;
	u32 CurrentRadio = -1;
	// TODO: establish default widths of controls?
	// or require that rects be passed in to establish sizes
	// or follow some kind of layout rules establed by BeginFrame

	inline u32 stringlength(const char* src)
	{
		const char* s = src;
		while (*s) s++;
		return (u32)(s - src);
	}

	void Init(Bitmap& InTarget, Font InDefaultFont)
	{
		Target = InTarget;
		DefaultFont = InDefaultFont;
		// override input handler during capture
	}

	void Char(u32 character)
	{
		if (character >= 32 && character < 128 && InsertionPoint < sizeof(EditBuffer))
		{
			if (EditBuffer[InsertionPoint] != 0)
			{
				// move all characters down to make room for current character
				char* s = EditBuffer + InsertionPoint;
				char* p = s;
				while (*p && p - EditBuffer < sizeof(EditBuffer)) p++;
				while (p > s) *p-- = p[-1];
			}
			bool IsEnd = EditBuffer[InsertionPoint] == 0;
			EditBuffer[InsertionPoint++] = character;
			if (IsEnd)
			{
				EditBuffer[InsertionPoint] = 0;
			}
		}
	}

	void KeyDown(u32 key)
	{
		if (key == Jogo::JogoApp::KEY_BACKSPACE)
		{
			if (InsertionPoint > 0)
			{
				InsertionPoint--;
				char* p = EditBuffer + InsertionPoint;
				while (*p)
				{
					*p = p[1];
					p++;
				}
			}
		}
		if (key == Jogo::JogoApp::KEY_DELETE)
		{
			char* p = EditBuffer + InsertionPoint;
			if (*p)
			{
				do
				{
					*p = p[1];
					p++;
				} while (p[-1]);
			}
		}
		if (key == Jogo::JogoApp::KEY_LEFT)
		{
			if (InsertionPoint > 0)
			{
				InsertionPoint--;
			}
		}
		if (key == Jogo::JogoApp::KEY_RIGHT)
		{
			if (EditBuffer[InsertionPoint] != 0)
			{
				InsertionPoint++;
			}
		}
		if (key == Jogo::JogoApp::KEY_HOME)
		{
			InsertionPoint = 0;
		}
		if (key == Jogo::JogoApp::KEY_END)
		{
			while (EditBuffer[InsertionPoint] != 0)
			{
				InsertionPoint++;
			}
		}
		if (key == Jogo::JogoApp::KEY_ENTER || key == Jogo::JogoApp::KEY_TAB)
		{
			UncaptureID = CaptureID;
			CaptureID = 0;
		}
	}

	u32 GetID()
	{
		return NextID++;
	}

	bool Interact(u32 Id, const Bitmap::Rect& r, s32 mousex, s32 mousey)
	{
		bool clicked = false;

		if (ActiveID == Id)
		{
			HiColor = LoLight;
			LoColor = HiLight;
			if (!Jogo::IsKeyPressed(Jogo::JogoApp::BUTTON_LEFT))
			{
				if (HotID == Id)
				{
					clicked = true;
				}
				ActiveID = 0;
			}
		}
		else if (Id == HotID)
		{
			if (Jogo::IsKeyPressed(Jogo::JogoApp::BUTTON_LEFT))
			{
				ActiveID = Id;
			}
		}

		// SetHot
		if (mousex >= r.x && mousex < r.x + r.w && mousey >= r.y && mousey < r.y + r.h)
		{
			if (ActiveID == Id || (ActiveID == 0 && !Jogo::IsKeyPressed(Jogo::JogoApp::BUTTON_LEFT)))
			{
				HotID = Id;
				CurrentColor = HotColor;
			}
		}
		else if (HotID == Id)
		{
			HotID = 0;
		}

		return clicked;
	}

	void DrawButton(Bitmap::Rect& r, const char* Text)
	{
		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(CursorY, r.x, r.x + r.w - 1, HiColor);
		Target.DrawHLine(CursorY + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(CursorX, r.y, r.y + r.h - 1, HiColor);
		Target.DrawVLine(CursorX + r.w, r.y, r.y + r.h - 1, LoColor);
		DefaultFont.DrawText(CursorX + 4, CursorY + 4, Text, TextColor, UI::Target);
	}

	bool Button(const char* Text)
	{
		bool clicked = false;

		u32 ButtonID = GetID();
		// get an id for this button
		// compare to hot and active
		// calc return value (based on mouse in rect for this button)
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(Text);
		TextSize.x = CursorX;
		TextSize.y = CursorY;
		TextSize.w += 8;
		TextSize.h += 8;
		CurrentColor = ButtonColor;
		HiColor = HiLight;
		LoColor = LoLight;

		int x, y;
		Jogo::GetMousePos(x, y);

		// TODO: call Layout here?  to give a chance for UI layout to arrange things...

		clicked = Interact(ButtonID, TextSize, x, y);

		DrawButton(TextSize, Text);

		// advance layout cursor
		// TODO: maybe this belongs in a UI::Layout function?
		CursorY += TextSize.h + 1;

		return clicked;
	}

	void DrawLabel(Bitmap::Rect& r, const char* Text)
	{
		u32 LabelID = GetID();

		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(CursorY, r.x, r.x + r.w - 1, LoColor);
		Target.DrawHLine(CursorY + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(CursorX, r.y, r.y + r.h - 1, LoColor);
		Target.DrawVLine(CursorX + r.w, r.y, r.y + r.h - 1, LoColor);

		DefaultFont.DrawText(CursorX + 4, CursorY + 4, Text, TextColor, UI::Target);
	}

	void Label(const char* Text)
	{
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(Text);
		TextSize.x = CursorX;
		TextSize.y = CursorY;
		TextSize.w += 8;
		TextSize.h += 8;
		CurrentColor = LabelColor;
		HiColor = HiLight;
 
		DrawLabel(TextSize, Text);

		CursorY += TextSize.h + 1;
	}

	void DrawEditBox(Bitmap::Rect& r, const char* Text)
	{
		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(CursorY, r.x, r.x + r.w - 1, HiColor);
		Target.DrawHLine(CursorY + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(CursorX, r.y, r.y + r.h - 1, HiColor);
		Target.DrawVLine(CursorX + r.w, r.y, r.y + r.h - 1, LoColor);
		DefaultFont.DrawText(CursorX + 4, CursorY + 4, Text, TextColor, UI::Target);
	}

	const char* EditBox(const char* Text)
	{
		u32 EditID = GetID();
		int x, y;
		Jogo::GetMousePos(x, y);
		Bitmap::Rect TextSize = DefaultFont.GetTextSize("DefaultSize");
		const char* result = Text;

		if (Text)
		{
			TextSize = DefaultFont.GetTextSize(Text);
		}
		TextSize.x = CursorX;
		TextSize.y = CursorY;
		TextSize.w += 8;
		TextSize.h += 8;

		CurrentColor = EditColor;
		if (!CaptureID)
		{
			if (Interact(EditID, TextSize, x, y))
			{
				CaptureID = EditID;
				u32 TextLen = stringlength(Text);
				Jogo::copystring(Text, EditBuffer, TextLen, sizeof(EditBuffer));
				InsertionPoint = TextLen;
			}
		}
		if (CaptureID == EditID || UncaptureID == EditID)
		{
			CurrentColor = EditColorActive;
			// draw the current EditBuffer
			DrawEditBox(TextSize, EditBuffer);
			// draw the current insertion point
			char PartialText[256];
			Jogo::copystring(EditBuffer, PartialText, InsertionPoint, sizeof(PartialText));
			Bitmap::Rect PartialSize = DefaultFont.GetTextSize(PartialText);
			s32 CaretX = TextSize.x + PartialSize.w + 4;
			s32 CaretY = TextSize.y+2;
			Target.DrawLine(CaretX, CaretY, CaretX, CaretY + PartialSize.h + 2, 0xff0000);
			// and manage to blink it...
			result = EditBuffer;
			UncaptureID = 0;
		}
		else
		{
			DrawEditBox(TextSize, Text);
		}

		CursorY += TextSize.h + 1;

		return result;
	}

	void BeginRadioButtons(u32 choice)
	{
		RadioChoice = choice;
		RadioButtonGroupStart = CursorY;
		RadioButtonGroupWidth = DefaultFont.CharacterHeight;
		CurrentRadio = 0;
		// make space to surround RadioButtonGroup
		CursorY ++;
	}

	u32 EndRadioButtons()
	{
		if (RadioButtonGroupStart != -1)
		{
			Bitmap::Rect Box = { CursorX, RadioButtonGroupStart, CursorX + RadioButtonGroupWidth, CursorY - RadioButtonGroupStart };
			Target.DrawRect(Box, Black);
		}
		RadioButtonGroupStart = -1;
		return RadioChoice;
	}

	void DrawRadioButton(Bitmap::Rect& r, const char* Text, bool clicked)
	{
		u32 OuterRadius = (r.h - 10) / 2;
		Target.FillRect(r, CurrentColor);
		Target.DrawCircle(r.x + r.h / 2, r.y + r.h / 2 - 1, OuterRadius, Black);
		if (clicked)
		{
			Target.FillCircle(r.x + r.h / 2, r.y + r.h /2 - 1, OuterRadius - 3, Black);
		}
		DefaultFont.DrawText(r.x + r.h + 4, r.y + 4, Text, TextColor, UI::Target);
	}

	void RadioButton(const char* Text)
	{
		u32 ButtonID = GetID();
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(Text);
		TextSize.x = CursorX;
		TextSize.y = CursorY;
		TextSize.w += 8 + TextSize.h + 8;
		TextSize.h += 8;
		CurrentColor = ButtonColor;

		int x, y;
		Jogo::GetMousePos(x, y);

		// TODO: call Layout here?  to give a chance for UI layout to arrange things...

		if (Interact(ButtonID, TextSize, x, y))
		{
			RadioChoice = CurrentRadio;
		}

		DrawRadioButton(TextSize, Text, RadioChoice == CurrentRadio);

		RadioButtonGroupWidth = max(RadioButtonGroupWidth, TextSize.w);
		CurrentRadio++;
		CursorY += TextSize.h;
	}
	
	// need to pass in input state to BeginFrame
	// TODO: reset and establish layout rules within this frame
	void BeginFrame(Bitmap::Rect Frame)
	{
		NextID = FirstID;
		CursorX = Frame.x;
		CursorY = Frame.y;
	}
}

