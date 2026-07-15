#include "UI.h"
#include "Input.h"
#include "Font.h"

namespace UI
{
	const u32 MaxContainerStack = 15;
	Container ContainerStack[MaxContainerStack];
	u32 CurrentContainer = 0;
	u32 NextContainer = 0;
	Bitmap Target;
	Font DefaultFont;
	u32 HotID = 0;
	u32 ActiveID = 0;
	u32 FocusID = 0;
	u32 LastFocusID = 0;
	u32 NextFocusID = 0;
	u32 PrevFocusID = 0;
	u32 ButtonColor = 0x808080;
	u32 LabelColor = 0x404040;
	u32 EditColor = 0x303030;
	u32 EditColorActive = 0x000000;
	u32 HotColor = 0xa0a0a0;
	u32 HiLight = 0xc0c0c0;
	u32 LoLight = 0x404040;
	u32 TextColor = 0xf0f0f0;
	u32 BackColor = 0;
	u32 SelectColor = 0xff4040c0;
	u32 CurrentColor;
	u32 HiColor;
	u32 LoColor;
	u32 Black = 0;
	u32 White = 0xffffff;
	char EditBuffer[256] = { '0' };
	size_t EditBufferLen = 0;
	u32 InsertionPoint = 0;
	s32 SelectionBegin = -1;
	s32 SelectionEnd = -1;
	s32 SelectionAnchor = -1;
	s32 RadioButtonGroupStart = -1;
	s32 RadioButtonGroupWidth;
	u32 RadioChoice = -1;
	u32 CurrentRadio = -1;
	UIInputHandler UIHandler;
	float CursorBlinkInterval = .5f;
	float CursorTime = 0;
	bool bTextEditCursorVisible = false;
	float DoubleClickTime = 0;
	float TripleClickTime = 0;
	bool bSelecting = false;

	// TODO: establish default widths of controls?
	// or require that rects be passed in to establish sizes
	// or follow some kind of layout rules establed by PushContainer
	// TODO: need to be able to specify:
	// container
	// flow direction
	// Colors
	// Spacing
	// 

	Jogo::TickHandler UITick;

	void Init(const Bitmap& InTarget, const Font& InDefaultFont)
	{
		Target = InTarget;
		DefaultFont = InDefaultFont;
		// override input handler during capture
		Jogo::SetUIHandler(&UIHandler);
		Jogo::SetTickHandler(UITick);
	}

	void UITick(float DeltaTime)
	{
		CursorTime += DeltaTime;
		if (CursorTime > CursorBlinkInterval)
		{
			bTextEditCursorVisible = !bTextEditCursorVisible;
			CursorTime -= CursorBlinkInterval;
		}
		DoubleClickTime += DeltaTime;
		TripleClickTime += DeltaTime;
	}

	void ClearSelection()
	{
		SelectionBegin = SelectionEnd = -1;
	}

	void DeleteSelection(u32 bInputLimited = false)
	{
		if (SelectionEnd > SelectionBegin)
		{
			char* s = EditBuffer + SelectionEnd;
			char* d = EditBuffer + SelectionBegin;
			size_t l = EditBufferLen - SelectionEnd;
			if (bInputLimited)
			{
				while (l--)
					*d++ = ' ';
			}
			else
			{
				while (l--)
				{
					*d++ = *s++;
				}
				EditBufferLen -= SelectionEnd - SelectionBegin;
			}
		}
		ClearSelection();
	}

	void InsertChars(Jogo::str8 insert)
	{

	}

	void InsertChar(char character)
	{
		if (InsertionPoint <= EditBufferLen && EditBufferLen < sizeof(EditBuffer))
		{
			// move all characters down to make room for current character
			char* s = EditBuffer + InsertionPoint;
			char* p = EditBuffer + EditBufferLen;
			while (p > s)
			{
				*p-- = p[-1];
			}
			EditBuffer[InsertionPoint++] = character;
			EditBufferLen++;
		}
	}

	void DeleteChar()
	{
		char* p = EditBuffer + InsertionPoint;
		size_t l = EditBufferLen - InsertionPoint;
		while (l--)
		{
			*p = p[1];
			p++;
		}
		EditBufferLen--;
	}

	void MoveWordLeft()
	{
		// search left to find next whitespace and stop on character to the right of it
		while (InsertionPoint > 0 && EditBuffer[InsertionPoint] == ' ')
			InsertionPoint--;
		while (InsertionPoint > 0 && EditBuffer[InsertionPoint - 1] != ' ')
			InsertionPoint--;
	}

	void MoveWordRight()
	{
		// skip all whitespace to the right until non-whitespace then stop on next whitespace
		while (InsertionPoint < EditBufferLen && EditBuffer[InsertionPoint] == ' ')
			InsertionPoint++;
		while (InsertionPoint < EditBufferLen && EditBuffer[InsertionPoint] != ' ')
			InsertionPoint++;
	}

	void ExpandSelection()
	{
		if (InsertionPoint < (u32)SelectionBegin)
		{
			SelectionBegin = InsertionPoint;
		}
		else
		{
			SelectionEnd = InsertionPoint;
		}
		if (SelectionBegin == SelectionEnd)
		{
			ClearSelection();
		}
	}

	bool UIInputHandler::KeyDown(Input::Keys key)
	{
		if (!FocusID)
			return false;

		// keep cursor visible while typing
		bTextEditCursorVisible = true;
		CursorTime = 0.0f;

		if (key == Input::KEY_BACKSPACE)
		{
			bool bSelection = SelectionBegin > -1;
			if (InsertionPoint == SelectionEnd)
			{
				InsertionPoint = SelectionBegin;
			}
			DeleteSelection(InputLimit < sizeof(EditBuffer));
			if (!bSelection && InsertionPoint > 0)
			{
				InsertionPoint--;
				DeleteChar();
				if (InputLimit < sizeof(EditBuffer))
				{
					EditBufferLen++;
				}
			}
			return true;
		}
		if (key == Input::KEY_DELETE)
		{
			bool bSelection = SelectionBegin > -1;
			if (InsertionPoint == SelectionEnd)
			{
				InsertionPoint = SelectionBegin;
			}
			DeleteSelection(InputLimit < sizeof(EditBuffer));
			if (!bSelection && InsertionPoint < EditBufferLen)
			{
				DeleteChar();
				if (InputLimit < sizeof(EditBuffer))
				{
					EditBufferLen++;
				}
			}
			return true;
		}
		if (key == Input::KEY_LEFT)
		{
			if (InsertionPoint > 0)
			{
				u32 StartSelection = InsertionPoint;
				InsertionPoint--;
				if (Input::IsKeyPressed(Input::KEY_CONTROL))
				{
					MoveWordLeft();
				}
				if (Input::IsKeyPressed(Input::KEY_SHIFT))
				{
					if (SelectionBegin == -1)
					{
						SelectionBegin = InsertionPoint;
						SelectionEnd = StartSelection;
					}
					else
					{
						ExpandSelection();
					}
				}
				else
				{
					ClearSelection();
				}
			}
			else
			{
				if (!Input::IsKeyPressed(Input::KEY_SHIFT))
					ClearSelection();
			}
			return true;
		}
		if (key == Input::KEY_RIGHT)
		{
			if (InsertionPoint < EditBufferLen)
			{
				u32 StartSelection = InsertionPoint;
				if (Input::IsKeyPressed(Input::KEY_CONTROL))
				{
					MoveWordRight();
				}
				else
				{
					InsertionPoint++;
				}
				if (Input::IsKeyPressed(Input::KEY_SHIFT))
				{
					if (SelectionBegin == -1)
					{
						SelectionBegin = StartSelection;
						SelectionEnd = InsertionPoint;
					}
					else
					{
						ExpandSelection();
					}
				}
				else
				{
					ClearSelection();
				}
			}
			else
			{
				if (!Input::IsKeyPressed(Input::KEY_SHIFT))
					ClearSelection();
			}
			return true;
		}
		if (key == Input::KEY_HOME)
		{
			if (Input::IsKeyPressed(Input::KEY_SHIFT))
			{
				if (SelectionBegin == -1)
				{
					SelectionEnd = InsertionPoint;
				}
				else if (InsertionPoint == SelectionEnd)
				{
					SelectionEnd = SelectionBegin;
				}
				SelectionBegin = 0;
				if (SelectionBegin == SelectionEnd)
				{
					ClearSelection();
				}
			}
			else
			{
				ClearSelection();
			}
			InsertionPoint = 0;
			return true;
		}
		if (key == Input::KEY_END)
		{
			if (Input::IsKeyPressed(Input::KEY_SHIFT))
			{
				if (SelectionBegin == -1)
				{
					SelectionBegin = InsertionPoint;;
				}
				else if (InsertionPoint == SelectionBegin)
				{
					SelectionBegin = SelectionEnd;
				}
				SelectionEnd = (s32)EditBufferLen;
				if (SelectionBegin == SelectionEnd)
				{
					ClearSelection();
				}
			}
			else
			{
				ClearSelection();
			}
			InsertionPoint = (u32)EditBufferLen;
			return true;
		}
		if (key == Input::KEY_ENTER || key == Input::KEY_TAB)
		{
			ClearSelection();
			LastFocusID = FocusID;
//			NextFocusID = FocusID;
			FocusID = 0;
			return true;
		}

		// TODO: we need to determine during Focus capture which keystrokes can be passed on
		// such as: Escape
		return false;
	}

	bool UIInputHandler::MouseDown(s32 mousex, s32 mousey, Input::Keys button)
	{
		if (HotID)
		{
			if (HotID != FocusID)
			{
//				FocusID = HotID;
			}
			else
				bSelecting = true;
			return true;
		}
		// we have clicked in an area without UI, so disable focus capture
		FocusID = 0;

		return false;
	}

	bool UIInputHandler::MouseUp(s32 mousex, s32 mousey, Input::Keys button)
	{
		bSelecting = false;
		SelectionAnchor = -1;

		if (ActiveID)
		{
			return true;
		}
		return false;
	}

	bool UIInputHandler::MouseDoubleClick(s32 mousex, s32 mousey, Input::Keys button)
	{
		return false;
	}

	bool UIInputHandler::Char(char character)
	{
		if (!FocusID)
			return false;

		// TODO: implement input filter here
		// for instance, an editbox that only takes numbers or hex digits etc.
		switch (InputFilter)
		{
		case UIInputHandler::FILTER_NONE:
			if (!Jogo::str8::isprintable(character))
				return true;
			break;

		case UIInputHandler::FILTER_NUMERIC:
			if (!Jogo::str8::isdigit(character))
				return true;
			break;

		case UIInputHandler::FILTER_ALPHA:
			if (!Jogo::str8::isalpha(character))
				return true;
			break;

		case UIInputHandler::FILTER_ALPHANUMERIC:
			if (!Jogo::str8::isalpha(character) && !Jogo::str8::isdigit(character))
				return true;
			break;

		case UIInputHandler::FILTER_HEX:
			if (!Jogo::str8::ishex(character))
				return true;
			break;

		case UIInputHandler::FILTER_REAL:
			if (!Jogo::str8::isdigit(character) && (character != '.'))
				return true;
			break;
		}


		if (SelectionBegin != -1)
		{
			InsertionPoint = SelectionBegin;
			DeleteSelection(InputLimit < sizeof(EditBuffer));
		}

		if (InsertionPoint >= InputLimit)
		{
			InsertionPoint = Jogo::max(InputLimit - 1, (u32)0);
		}
		if (InputLimit < sizeof(EditBuffer))
		{
			EditBuffer[InsertionPoint++] = character;
		}
		else
		{
			InsertChar(character);
		}

		return true;
	}

	u32 GetID()
	{
		u32 ID = ContainerStack[CurrentContainer].NextID++;
		//if (PrevFocusID == ID)
		//{
		//	FocusID = ID;
		//	PrevFocusID = 0;
		//}
		//if (NextFocusID == -1)
		//{
		//	FocusID = ID;
		//	NextFocusID = 0;
		//}
		//if (NextFocusID == ID)
		//{
		//	NextFocusID = -1;
		//}
		return ID;
	}

	bool Interact(u32 Id, const Bitmap::Rect& r)
	{
		bool clicked = false;
		s32 mousex;
		s32 mousey;

		Input::GetMousePos(mousex, mousey);

		if (ActiveID == Id)
		{
			HiColor = LoLight;
			LoColor = HiLight;
			if (!Input::IsKeyPressed(Input::BUTTON_LEFT))
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
			if (Input::IsKeyPressed(Input::BUTTON_LEFT))
			{
				ActiveID = Id;
			}
		}

		// SetHot
		if (mousex >= r.x && mousex < r.x + r.w && mousey >= r.y && mousey < r.y + r.h)
		{
			if (ActiveID == Id || (ActiveID == 0 && !Input::IsKeyPressed(Input::BUTTON_LEFT)))
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

	void DrawButton(Bitmap::Rect& r, const Jogo::str8& Text)
	{
		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(r.y, r.x, r.x + r.w - 1, HiColor);
		Target.DrawHLine(r.y + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(r.x, r.y, r.y + r.h - 1, HiColor);
		Target.DrawVLine(r.x + r.w, r.y, r.y + r.h - 1, LoColor);
		DefaultFont.DrawText(r.x + 4, r.y + 4, Text, TextColor, BackColor, UI::Target);
	}

	Bitmap::Rect GetButtonSize(const Jogo::str8& Text)
	{
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(Text);
		TextSize.w += 8;
		TextSize.h += 8;
		return TextSize;
	}

	bool Button(const Jogo::str8& Text)
	{
		bool clicked = false;

		u32 ButtonID = GetID();
		// get an id for this button
		// compare to hot and active
		// calc return value (based on mouse in rect for this button)
		Bitmap::Rect ButtonSize = GetButtonSize(Text);
		Bitmap::Rect location = ContainerStack[CurrentContainer].PrimitiveBegin();
		ButtonSize.x = location.x;
		ButtonSize.y = location.y;
		CurrentColor = ButtonColor;
		HiColor = HiLight;
		LoColor = LoLight;


		// TODO: call Layout here?  to give a chance for UI layout to arrange things...

		clicked = Interact(ButtonID, ButtonSize);

		DrawButton(ButtonSize, Text);

		ContainerStack[CurrentContainer].PrimitiveEnd(ButtonSize);

		return clicked;
	}

	void DrawLabel(Bitmap::Rect& r, const Jogo::str8& Text)
	{
		u32 LabelID = GetID();

		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(r.y, r.x, r.x + r.w - 1, LoColor);
		Target.DrawHLine(r.y + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(r.x, r.y, r.y + r.h - 1, LoColor);
		Target.DrawVLine(r.x + r.w, r.y, r.y + r.h - 1, LoColor);

		DefaultFont.DrawText(r.x + 4, r.y + 4, Text, TextColor, BackColor, UI::Target);
	}

	void Label(const Jogo::str8& Text)
	{
		Bitmap::Rect TextSize = GetButtonSize(Text);
		TextSize.x = ContainerStack[CurrentContainer].CursorX;
		TextSize.y = ContainerStack[CurrentContainer].CursorY;
		CurrentColor = LabelColor;
		HiColor = HiLight;

		DrawLabel(TextSize, Text);

		ContainerStack[CurrentContainer].PrimitiveEnd(TextSize);
	}

	void DrawEditBox(Bitmap::Rect& r, const Jogo::str8& Text, bool bFocused = false)
	{
		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(r.y, r.x, r.x + r.w - 1, HiColor);
		Target.DrawHLine(r.y + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(r.x, r.y, r.y + r.h - 1, HiColor);
		Target.DrawVLine(r.x + r.w, r.y, r.y + r.h - 1, LoColor);
		if (bFocused && SelectionBegin != -1)
		{
			if (SelectionEnd > EditBufferLen) SelectionEnd = (u32)EditBufferLen;
			u32 SelectionX = DefaultFont.GetTextSize(Text.substr(0, SelectionBegin)).w;
			u32 SelectionLen = (u32)(SelectionEnd - SelectionBegin);
			u32 SelectionEndX = SelectionX + DefaultFont.GetTextSize(Text.substr(SelectionBegin, SelectionLen)).w;
			DefaultFont.DrawText(r.x + 4, r.y + 4, Text.substr(0, (u32)SelectionBegin), TextColor, BackColor, UI::Target);
			DefaultFont.DrawText(r.x + 4 + SelectionX, r.y + 4, Text.substr((u32)SelectionBegin, SelectionLen), TextColor, SelectColor, UI::Target);
			DefaultFont.DrawText(r.x + 4 + SelectionEndX, r.y + 4, Text.substr((u32)SelectionEnd, (u32)(EditBufferLen - (u32)SelectionEnd)), TextColor, BackColor, UI::Target);
		}
		else
		{
			DefaultFont.DrawText(r.x + 4, r.y + 4, Text, TextColor, BackColor, UI::Target);
		}
	}

	const Jogo::str8 EditBox(const Jogo::str8& Text, const Jogo::str8& format)
	{
		u32 EditID = GetID();
		Jogo::str8 result = Text;
		Bitmap::Rect TextSize = GetButtonSize(result);

		TextSize = GetButtonSize(result);
		TextSize.x = ContainerStack[CurrentContainer].CursorX;
		TextSize.y = ContainerStack[CurrentContainer].CursorY;

		CurrentColor = EditColor;
		HiColor = HiLight;
		LoColor = LoLight;
		
		if (!FocusID)
		{
			if (Interact(EditID, TextSize))
			{
				// start the cursor visible when we start interacting
				FocusID = EditID;
				bTextEditCursorVisible = true;
				CursorTime = 0.0f;
				Jogo::str8::copystring(Text.chars, EditBuffer, (u32)Text.len, sizeof(EditBuffer));
				s32 mousex;
				s32 mousey;
				Input::GetMousePos(mousex, mousey);
				InsertionPoint = (u32)DefaultFont.GetCursorPos(Text, mousex - TextSize.x);
				EditBufferLen = Text.len;
				DoubleClickTime = 0;

				// parse the format spec:
				u32 spec = Jogo::str8::parseSpec(format);
				// set up the UIInputHandler constraints
				UIHandler.InputLimit = sizeof(EditBuffer);
				UIHandler.InputFilter = UIInputHandler::FILTER_NONE;
				if (spec & Jogo::str8::SPEC_WIDTH)
				{
					UIHandler.InputLimit = (spec >> Jogo::str8::SPEC_WIDTH_SHIFT) & Jogo::str8::SPEC_WIDTH_MASK;
					if (UIHandler.InputLimit < sizeof(EditBuffer))
					{
						// pad out the remainder with spaces
						for (u32 i = (u32)Text.len; i < sizeof(EditBuffer); i++)
						{
							EditBuffer[i] = ' ';
						}
					}
				}
				if (spec & Jogo::str8::SPEC_HEX)
				{
					UIHandler.InputFilter = UIInputHandler::FILTER_HEX;
				}
				if (spec & Jogo::str8::SPEC_NUMERIC | Jogo::str8::SPEC_ALPHA)
				{
					UIHandler.InputFilter = UIInputHandler::FILTER_ALPHANUMERIC;
				}
				if (spec & Jogo::str8::SPEC_NUMERIC)
				{
					UIHandler.InputFilter = UIInputHandler::FILTER_NUMERIC;
				}
				if (spec & Jogo::str8::SPEC_ALPHA)
				{
					UIHandler.InputFilter = UIInputHandler::FILTER_ALPHA;
				}
				if (spec & Jogo::str8::SPEC_DECIMAL)
				{
					UIHandler.InputFilter = UIInputHandler::FILTER_REAL;
				}
			}
		}
		if (FocusID == EditID)
		{
			CurrentColor = EditColorActive;
			if (Interact(EditID, TextSize))
			{
				// start the cursor visible when we start interacting
				bTextEditCursorVisible = true;
				CursorTime = 0.0f;

				s32 mousex;
				s32 mousey;
				Input::GetMousePos(mousex, mousey);
				InsertionPoint = (u32)DefaultFont.GetCursorPos(Text, mousex - TextSize.x);

				// detect double and triple click here to select word and entire textbox
				if (TripleClickTime < 0.5)
				{
					SelectionBegin = 0;
					SelectionEnd = (u32)EditBufferLen;
					InsertionPoint = 0;
				}
				else if (DoubleClickTime < 0.5)
				{
					// select current word
					u32 WordBegin = InsertionPoint;
					while (WordBegin > 0 && EditBuffer[WordBegin - 1] == ' ')
						WordBegin--;

					u32 WordEnd = WordBegin;
					while (WordBegin > 0 && EditBuffer[WordBegin - 1] != ' ')
						WordBegin--;
					
					while (WordEnd < EditBufferLen && EditBuffer[WordEnd] != ' ')
						WordEnd++;
					
					// mark selection of the word
					SelectionBegin = WordBegin;
					SelectionEnd = WordEnd;
					InsertionPoint = WordEnd;
					TripleClickTime = 0;
				}
				else
				{
					SelectionAnchor = -1;
					DoubleClickTime = 0;
				}
			}
			else if (bSelecting)
			{
				s32 mousex;
				s32 mousey;
				Input::GetMousePos(mousex, mousey);
				InsertionPoint = (u32)DefaultFont.GetCursorPos(Text, mousex - TextSize.x);
				if (SelectionAnchor == -1)
				{
					SelectionAnchor = InsertionPoint;
				}
				if (InsertionPoint == SelectionAnchor)
				{
					ClearSelection();
				}
				else
				{
					if ((s32)InsertionPoint < SelectionAnchor)
					{
						SelectionBegin = InsertionPoint;
						SelectionEnd = SelectionAnchor;
					}
					else
					{
						SelectionBegin = SelectionAnchor;
						SelectionEnd = InsertionPoint;
					}
				}
			}

			// draw the current EditBuffer
			const Jogo::str8 EditText(EditBuffer, EditBufferLen);
			Bitmap::Rect Size = GetButtonSize(EditText);
			TextSize.w = Size.w;
			TextSize.h = Size.h;
			DrawEditBox(TextSize, EditText, SelectionBegin > -1);

			// draw the current insertion point
			if (bTextEditCursorVisible)
			{
				Jogo::str8 PartialText = Jogo::str8(EditBuffer).substr(0, InsertionPoint);
				//Jogo::str8::copystring(EditBuffer, PartialText, InsertionPoint, sizeof(PartialText));
				Bitmap::Rect PartialSize = DefaultFont.GetTextSize(PartialText);
				s32 CaretX = TextSize.x + PartialSize.w + 4;
				s32 CaretY = TextSize.y + 2;
				Target.DrawLine(CaretX, CaretY, CaretX, CaretY + PartialSize.h + 2, 0xff0000);
			}
		}
		else
		{
			// did user just tab away or change focus away from this control?
			if (LastFocusID == EditID)
			{
				result = Jogo::str8(EditBuffer, EditBufferLen);
				LastFocusID = 0;
			}
			DrawEditBox(TextSize, result);
		}

		ContainerStack[CurrentContainer].PrimitiveEnd(TextSize);

		return result;
	}

	void DrawRadioButton(Bitmap::Rect& r, const Jogo::str8& Text, bool clicked)
	{
		u32 OuterRadius = (r.h - 10) / 2;
		Target.FillRect(r, CurrentColor);
		Target.DrawCircle(r.x + r.h / 2, r.y + r.h / 2 - 1, OuterRadius, Black);
		if (clicked)
		{
			Target.FillCircle(r.x + r.h / 2, r.y + r.h / 2 - 1, OuterRadius - 3, Black);
		}
		DefaultFont.DrawText(r.x + r.h + 4, r.y + 4, Text, TextColor, BackColor, UI::Target);
	}

	void RadioButton(const Jogo::str8& Text)
	{
		u32 ButtonID = GetID();
		Bitmap::Rect TextSize = GetButtonSize(Text);
		TextSize.x = ContainerStack[CurrentContainer].CursorX;
		TextSize.y = ContainerStack[CurrentContainer].CursorY;
		TextSize.w += TextSize.h;

		CurrentColor = ButtonColor;

		if (Interact(ButtonID, TextSize))
		{
			RadioChoice = CurrentRadio;
		}
		DrawRadioButton(TextSize, Text, RadioChoice == CurrentRadio);
		CurrentRadio++;
		ContainerStack[CurrentContainer].PrimitiveEnd(TextSize);
	}

	u32 RadioButtons(u32 choice, const Jogo::str8 strings[], u32 count)
	{
		Bitmap::Rect MaxRadio{ 0,0,0,0 };
		for (u32 i = 0; i < count; i++)
		{
			Bitmap::Rect RadioSize = GetButtonSize(strings[i]);
			if (RadioSize.w > MaxRadio.w)
			{
				MaxRadio = RadioSize;
			}
		}
		MaxRadio.w += MaxRadio.h + 1;
		Bitmap::Rect AllRadios = { (s32)ContainerStack[CurrentContainer].CursorX, (s32)ContainerStack[CurrentContainer].CursorY, (s32)MaxRadio.w, (s32)(count * MaxRadio.h) };
		Target.FillRect(AllRadios, ButtonColor);

		RadioChoice = choice;
		CurrentRadio = 0;
		for (u32 i = 0; i < count; i++)
		{
			RadioButton(strings[i]);
		}
		Target.DrawRect(AllRadios, Black);
		ContainerStack[CurrentContainer].CursorY++;
		return RadioChoice;
	}

	void DrawCheckBox(Bitmap::Rect& r, const Jogo::str8& label, bool checked)
	{
		Bitmap::Rect DrawBox = { r.x + 4, r.y + 4, r.h - 7, r.h - 8 };
		Target.FillRect(r, CurrentColor);
		Target.DrawRect(DrawBox, Black);
		if (checked)
		{
			s32 x1 = DrawBox.x + 3;
			s32 y1 = DrawBox.y + 3;
			s32 x2 = DrawBox.x + DrawBox.h - 4;
			s32 y2 = DrawBox.y + DrawBox.h - 4;

			Target.DrawLine(x1, y1, x2, y2, Black);
			Target.DrawLine(x1 + 1, y1, x2, y2 - 1, Black);
			Target.DrawLine(x1, y1 + 1, x2 - 1, y2, Black);
			Target.DrawLine(x2, y1, x1, y2, Black);
			Target.DrawLine(x2 - 1, y1, x1, y2 - 1, Black);
			Target.DrawLine(x2, y1 + 1, x1 + 1, y2, Black);
		}
		DefaultFont.DrawText(r.x + r.h + 4, r.y + 4, label, TextColor, BackColor, Target);
	}

	bool CheckBox(const Jogo::str8& label, bool checked)
	{
		u32 ButtonID = GetID();
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(label);
		TextSize.x = ContainerStack[CurrentContainer].CursorX;
		TextSize.y = ContainerStack[CurrentContainer].CursorY;
		TextSize.w += 8 + TextSize.h + 8;
		TextSize.h += 8;
		CurrentColor = ButtonColor;

		if (Interact(ButtonID, TextSize))
		{
			checked = !checked;
		}
		ContainerStack[CurrentContainer].PrimitiveEnd(TextSize);
		DrawCheckBox(TextSize, label, checked);

		return checked;
	}

	// TODO: fix potential buffer overrun with unpaired PushContainer/PopContainer
	void PushContainer(const Bitmap::Rect& ThisContainer, u32 Flags)
	{
		u32 ContainerOffsetX = 0;
		u32 ContainerOffsetY = 0;
		if (Flags & CONTAINER_RELATIVE && CurrentContainer)
		{
			ContainerOffsetX = ContainerStack[CurrentContainer].CursorX;
			ContainerOffsetY = ContainerStack[CurrentContainer].CursorY;
		}
		CurrentContainer++;
		NextContainer++;
		Jogo::Assert(CurrentContainer < MaxContainerStack);
		ContainerStack[CurrentContainer].ContainerRect = ThisContainer;
		ContainerStack[CurrentContainer].ContainerRect.x += ContainerOffsetX;
		ContainerStack[CurrentContainer].ContainerRect.y += ContainerOffsetY;
		ContainerStack[CurrentContainer].CursorX = ContainerOffsetX + ThisContainer.x;
		ContainerStack[CurrentContainer].CursorY = ContainerOffsetY + ThisContainer.y;
		ContainerStack[CurrentContainer].NextID = NextContainer << 12;
		u32 FlowDir = Flags & CONTAINER_FLOW;
		ContainerStack[CurrentContainer].FlowDir = FlowDir;
		ContainerStack[CurrentContainer].Relative = (Flags & CONTAINER_RELATIVE) == CONTAINER_RELATIVE;
	}

	void PopContainer()
	{
		Jogo::Assert(CurrentContainer > 0);
		CurrentContainer--;
		if (ContainerStack[CurrentContainer + 1].Relative)
		{
			if (ContainerStack[CurrentContainer].FlowDir)
				ContainerStack[CurrentContainer].CursorX = ContainerStack[CurrentContainer + 1].CursorX;
			else
				ContainerStack[CurrentContainer].CursorY = ContainerStack[CurrentContainer + 1].CursorY;
		}
		ContainerStack[CurrentContainer].PrimitiveEnd({ 0,0,(s32)DefaultFont.CharacterHeight+8, (s32)DefaultFont.CharacterHeight+8});
	}

	// need to pass in input state to BeginContainer
	// TODO: reset and establish layout rules within this Container
	void BeginFrame()
	{
		CurrentContainer = 0;
		NextContainer = 0;
	}

	void EndFrame()
	{
	}

	Bitmap::Rect MenuContainer()
	{
		Bitmap::Rect Container = { (s32)ContainerStack[CurrentContainer].CursorX
							, (s32)ContainerStack[CurrentContainer].CursorY
							, (s32)DefaultFont.CharacterWidth * 32, (s32)DefaultFont.CharacterHeight };
		return Container;
	}

	void BeginMenu()
	{
		PushContainer(MenuContainer());
	}

	void EndMenu()
	{
		// draw the menu items here?
		PopContainer();
	}

	void DrawMenuButton(Bitmap::Rect& r, const Jogo::str8& Text, bool open)
	{
		if (open)
		{
			HiColor = HiLight;
			LoColor = LoLight;
		}
		Target.FillRect(r, CurrentColor);
		Target.DrawHLine(r.y, r.x, r.x + r.w - 1, HiColor);
		Target.DrawHLine(r.y + r.h - 1, r.x, r.x + r.w - 1, LoColor);
		Target.DrawVLine(r.x, r.y, r.y + r.h - 1, HiColor);
		Target.DrawVLine(r.x + r.w, r.y, r.y + r.h - 1, LoColor);
		DefaultFont.DrawText(r.x + 4, r.y + 4, Text, TextColor, BackColor, UI::Target);
	}

	bool MenuButton(const Jogo::str8& label, bool open)
	{
		u32 ButtonID = GetID();
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(label);
		TextSize.x = ContainerStack[CurrentContainer].CursorX;
		TextSize.y = ContainerStack[CurrentContainer].CursorY;
		TextSize.w += 8;
		TextSize.h += 8;
		CurrentColor = ButtonColor;
		HiColor = HiLight;
		LoColor = LoLight;

		if (Interact(ButtonID, TextSize))
		{
			open = !open;
		}
		ContainerStack[CurrentContainer].PrimitiveEnd(TextSize);
		DrawMenuButton(TextSize, label, open);

		return open;
	}

	bool MenuItem(const Jogo::str8& Item, bool checked)
	{
		u32 MenuItemID = GetID();
		Bitmap::Rect TextSize = DefaultFont.GetTextSize(Item);
		TextSize.x = ContainerStack[CurrentContainer].CursorX;
		TextSize.y = ContainerStack[CurrentContainer].CursorY;
		TextSize.w += 8;
		TextSize.h += 8;
		CurrentColor = ButtonColor;
		HiColor = HiLight;
		LoColor = LoLight;
		ContainerStack[CurrentContainer].PrimitiveEnd(TextSize);

		if (Interact(MenuItemID, TextSize))
		{
			checked = !checked;
		}
		DrawButton(TextSize, Item);

		return checked;
	}

	void PrintDebug(Arena scratch, s32 x, s32 y, u32 color)
	{
		DefaultFont.DrawText(x, y, Jogo::str8::format(scratch, "Hot: {:}", HotID), color, BackColor, Target);
		DefaultFont.DrawText(x, y + DefaultFont.CharacterHeight+8, Jogo::str8::format(scratch, "Act: {:}", ActiveID), color, BackColor, Target);
		DefaultFont.DrawText(x, y + 2*(DefaultFont.CharacterHeight+8) , Jogo::str8::format(scratch, "Foc: {:}", FocusID), color, BackColor, Target);
	}
}
