#pragma once 

#include "Bitmap.h"
#include "str8.h"

#ifdef max
#undef max
#endif

template<typename T>
T max(T a, T b)
{
	return a >= b ? a : b;
}
/*
template<typename T>
T min(T a, T b)
{
	return a <= b ? a : b;
}
*/
struct Font {

	u32 CharacterMin;
	u32 CharacterCount;
	u32 CharacterWidth;
	u32 CharacterHeight;
	Bitmap::Rect* CharacterRects;
	Bitmap FontBitmap;

	Bitmap::Rect GetTextSize(const Jogo::str8& Text)
	{
		// FixedWidth Font
		if (!CharacterRects)
		{
			// TODO: don't count characters that aren't mapped
			s32 len = (s32)Text.len;
			Bitmap::Rect r = { 0, 0, len * (s32)CharacterWidth, (s32)CharacterHeight };
			return r;
		}
		else
		{
			Bitmap::Rect r = { 0,0 };
			s32 len = (s32)Text.len;
			const char* p = Text.chars;
			for (int i=0; i<len; i++)
			{
				u8 c = *p++ - CharacterMin;
				if (c > 0 && c < CharacterCount)
				{
					r.w += CharacterRects[c].w;
					r.h = max(r.h, CharacterRects[c].h);
				}
			}
			return r;
		}
	}

	u32 GetCursorPos(const Jogo::str8& Text, u32 mousex)
	{
		// find pos index of character in string closest to mousex
		// pos = mousex - character width of each character in Text up to and not exceeding mousex
		// index = mousex - closest character less than mousex
		u32 len = (u32)Text.len;
		if (!CharacterRects)
		{
			u32 pos = mousex / CharacterWidth;
			if (pos > len)
			{
				pos = len;
			}
			else
			{
				u32 rest = mousex % CharacterWidth;
				if (pos < len && rest > CharacterWidth / 2)
				{
					pos++;
				}
			}
			return pos;
		}
		else
		{
			// we have variable width characters
			u32 dist = 0;
			u32 pos = 0;
			u32 len = (u32)Text.len;
			u32 lastwidth = 0;
			const char* p = Text.chars;
			for (u32 i = 0; i < len && dist < mousex; i++)
			{
				u8 c = *p++ - CharacterMin;
				if (c > 0 && c < CharacterCount)
				{
					lastwidth = CharacterRects[c].w;
					dist += lastwidth;
				}
				pos++;
			}
			if (dist - lastwidth / 2 > mousex)
			{
				pos--;
			}
			return pos;
		}
	}

	// TODO: Add support for BGRA fonts and anti-aliased alpha fonts
	void DrawText(s32 x, s32 y, const Jogo::str8& Text, u32 color, u32 bkcolor, Bitmap destination, s32 scale = 1)
	{
		// FixedWidth Font - assume characters are packed Bitmap.Width / Font.CharacterWidth per row
		if (!CharacterRects)
		{
			u32 CharactersPerRow = FontBitmap.Width / CharacterWidth;
			s32 cursor = x;
			s32 DestWidth = CharacterWidth * scale;
			s32 DestHeight = CharacterHeight * scale;
			s32 i = 0;
			for (const char* p = Text.chars; i < Text.len; i++, p++, cursor += DestWidth)
			{
				char c = *p - CharacterMin;
				s32 sx = (c % CharactersPerRow) * CharacterWidth;
				s32 sy = (c / CharactersPerRow) * CharacterHeight;

				destination.PasteBitmapSelectionScaled({ cursor, y, DestWidth, DestHeight }, FontBitmap, { sx, sy, (s32)CharacterWidth, (s32)CharacterHeight }, color, bkcolor);
				//	destination.PasteBitmapSelection( cursor, y, FontBitmap, { sx, sy, (s32)CharacterWidth, (s32)CharacterHeight }, color);
				//	Jogo::Show(destination.PixelBGRA, destination.Width, destination.Height);
			}
		}
		else
		{
			s32 cursor = x;
			s32 i = 0;
			for (const char* p = Text.chars; i<Text.len; p++)
			{
				u8 c = *p - CharacterMin;
				if (c > 0 && c < CharacterCount)
				{
					destination.PasteBitmapSelection(cursor, y, FontBitmap, CharacterRects[c], color);
				}
			}
		}
	}

	static Font Load(const char* filename, Arena& arena);
};
