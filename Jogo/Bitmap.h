#pragma once

#include <intrin.h>
#include "int_types.h"
#include "Arena.h"

struct Bitmap
{
	u32 Width;
	u32 Height;
	u32 PixelSize;		// in bytes
	union
	{
		void* Pixels;
		u8* PixelA;
		u32* PixelBGRA;
	};

	struct Rect
	{
		s32 x, y, w, h;
	};

	void Erase(u32 color)
	{
		FillRect({ 0,0,(s32)Width,(s32)Height }, color);
	}

	void FillRect(const Rect& r, u32 color);
	bool ClipRect(const Rect& r, Rect& Clipped);
	void PasteBitmap(int x, int y, Bitmap source, u32 color)
	{
		PasteBitmapSelection(x, y, source, { 0, 0, (s32)source.Width, (s32)source.Height }, color);
	}
	void PasteBitmapSelectionScaled(const Rect& dest, Bitmap source, const Rect& srcRect, u32 color);
	void PasteBitmapSelection(int x, int y, Bitmap source, const Rect& srcRect, u32 color);
	void SetPixel(s32 x, s32 y, u32 color)
	{
		if (x < 0 || x >= (s32)Width || y < 0 || y >= (s32)Height)
			return;

		if (PixelSize == 1)
			*(PixelA + y * Width + x) = color;
		else
			*(PixelBGRA + y * Width + x) = color;
	}

	bool ClipLine(s32& x1, s32& y1, s32& x2, s32& y2, Rect clipRect);
	void DrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u32 color);
	void DrawHLine(s32 y, s32 x1, s32 x2, u32 color)
	{
		if (ClipLine(x1, y, x2, y, { 0,0,(s32)Width,(s32)Height }))
		{
			for (s32 x = x1; x <= x2; x++)
			{
				SetPixel(x, y, color);
			}
		}
	}

	void DrawVLine(s32 x, s32 y1, s32 y2, u32 color)
	{
		if (ClipLine(x, y1, x, y2, { 0,0,(s32)Width,(s32)Height }))
		{
			for (s32 y = y1; y <= y2; y++)
			{
				SetPixel(x, y, color);
			}
		}
	}

	void DrawRect(const Rect& box, u32 color)
	{
		s32 right = box.x + box.w - 1;
		s32 bottom = box.y + box.h - 1;

		DrawHLine(box.y, box.x, right, color);
		DrawVLine(right, box.y, bottom, color);
		DrawHLine(bottom, box.x, right, color);
		DrawVLine(box.x, box.y, bottom, color);
	}

	void DrawCircle(s32 cx, s32 cy, s32 r, u32 color);
	void FillCircle(s32 cx, s32 cy, s32 r, u32 color);
	void DrawRoundedRect(const Rect& box, s32 radius, u32 color);
	void FillRoundedRect(const Rect& box, s32 radius, u32 color);
#ifdef RGB
#undef RGB
#endif

	u32 RGB(u8 r, u8 g, u8 b)
	{
		return (r << 16) + (g << 8) + b;
	}

	u8 GetR(u32 rgb)
	{
		return (rgb >> 16) & 0xff;
	}

	u8 GetG(u32 rgb)
	{
		return (rgb >> 8) & 0xff;
	}

	u8 GetB(u32 rgb)
	{
		return rgb & 0xff;
	}

	float GetRfloat(u32 rgb)
	{
		return ((rgb >> 16)&0xff) / 255.0f;
	}

	float GetGfloat(u32 rgb)
	{
		return ((rgb >> 8)&0xff) / 255.0f;
	}

	float GetBfloat(u32 rgb)
	{
		return (rgb & 0xff) / 255.0f;
	}

	struct FloatRGBA
	{
		float r;
		float g;
		float b;
		float a;
	};

	struct Vertex
	{
		float x;
		float y;
		u32 c;
	};

	struct Gradient
	{
		float drdy;
		float drdx;
		float dgdy;
		float dgdx;
		float dbdy;
		float dbdx;
	};

	struct Edge
	{
		float x;
		float y;
		float x1;
		float dx;
		float dy;
		float dxdy;
		float y1;
		float y2;
		float r;
		float g;
		float b;
	};

	Edge MakeEdge(const Vertex& a, const Vertex& b);
	Gradient MakeGradient(Vertex corners[]);
	void FillTriangle(Vertex corners[]);

	static Bitmap Load(const char* filename, Arena& arena);
	static Bitmap Create(u32 Width, u32 Height, u32 PixelSize, Arena& arena)
	{
		Bitmap bitmap = { Width, Height, PixelSize };
		bitmap.Pixels = arena.Allocate(Width * Height *PixelSize);
		return bitmap;
	}
};
