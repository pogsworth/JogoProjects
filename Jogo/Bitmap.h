#pragma once

#include <intrin.h>
#include "int_types.h"
#include "Arena.h"
#include "JMath.h"

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
	void PasteBitmap(int x, int y, Bitmap source, u32 color, u32 bkcolor = 0)
	{
		PasteBitmapSelection(x, y, source, { 0, 0, (s32)source.Width, (s32)source.Height }, color, bkcolor);
	}
	void PasteBitmapSelectionScaled(const Rect& dest, Bitmap source, const Rect& srcRect, u32 color, u32 bkcolor = 0);
	void PasteBitmapSelection(int x, int y, Bitmap source, const Rect& srcRect, u32 color, u32 bkcolor = 0);
	void SetPixel(s32 x, s32 y, u32 color)
	{
		if (x < 0 || x >= (s32)Width || y < 0 || y >= (s32)Height)
			return;

		if (PixelSize == 1)
			*(PixelA + y * Width + x) = color;
		else
			*(PixelBGRA + y * Width + x) = color;
	}

	u32 GetPixel(s32 x, s32 y) const
	{
		if (PixelSize == 4)
			return *(PixelBGRA + y * Width + x);
		return *(PixelA + y * Width + x);
	}

	u32 GetTexel(float u, float v) const
	{
		s32 x = (s32)(u * Width) & (Width-1);
		s32 y = (s32)(v * Height) & (Height - 1);
		if (PixelSize == 4)
			return *(PixelBGRA + y * Width + x);
		return *(PixelA + y * Width + x);
	}

	bool ClipLine(s32& x1, s32& y1, s32& x2, s32& y2, Rect clipRect);
	void DrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u32 color);
	void DrawHLine(s32 y, s32 x1, s32 x2, u32 color)
	{
		if (ClipLine(x1, y, x2, y, { 0,0,(s32)Width,(s32)Height }))
		{
			if (x1 > x2)
				Jogo::swap(x1, x2);

			if (PixelSize == 1)
			{
				u8* row = PixelA + y * Width + x1;
				__stosb(row, (u8)color, (size_t)(x2 - x1 + 1));
			}
			else
			{
				u32* row = PixelBGRA + y * Width + x1;
				__stosd((unsigned long*)row, color, (size_t)(x2 - x1 + 1));
			}
		}
	}

	void DrawVLine(s32 x, s32 y1, s32 y2, u32 color)
	{
		if (ClipLine(x, y1, x, y2, { 0,0,(s32)Width,(s32)Height }))
		{
			if (PixelSize == 1)
			{
				u8* Pixel = PixelA + y1 * Width + x;
				for (s32 y = y1; y <= y2; y++)
				{
					*Pixel = color;
					Pixel += Width;
				}
			}
			else
			{
				u32* Pixel = PixelBGRA + y1 * Width + x;
				for (s32 y = y1; y <= y2; y++)
				{
					*Pixel = color;
					Pixel += Width;
				}
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
	void DrawRoundedRect(const Rect& box, s32 radius, u32 thickness, u32 color);
	void FillRoundedRect(const Rect& box, s32 radius, u32 color);
#ifdef RGB
#undef RGB
#endif

	static u32 RGB(u8 r, u8 g, u8 b)
	{
		return (r << 16) + (g << 8) + b;
	}

	static u32 RGBA(u8 r, u8 g, u8 b, u8 a)
	{
		return (a<<24) + (r << 16) + (g << 8) + b;
	}

	static u8 GetR(u32 rgb)
	{
		return (rgb >> 16) & 0xff;
	}

	static u8 GetG(u32 rgb)
	{
		return (rgb >> 8) & 0xff;
	}

	static u8 GetB(u32 rgb)
	{
		return rgb & 0xff;
	}

	static u8 GetA(u32 rgba)
	{
		return (rgba >> 24) & 0xff;
	}

	static float GetRfloat(u32 rgb)
	{
		return ((rgb >> 16)&0xff) / 255.0f;
	}

	static float GetGfloat(u32 rgb)
	{
		return ((rgb >> 8)&0xff) / 255.0f;
	}

	static float GetBfloat(u32 rgb)
	{
		return (rgb & 0xff) / 255.0f;
	}

	static float GetAfloat(u32 rgba)
	{
		return ((rgba >> 24) & 0xff) / 255.0f;
	}

	struct FloatRGBA
	{
		float r;
		float g;
		float b;
		float a;
	};

	static FloatRGBA GetFloatColor(u32 c)
	{
		return FloatRGBA{ GetRfloat(c), GetGfloat(c), GetBfloat(c),1.0f };
	}

	static u32 GetColorFromFloatRGBA(const FloatRGBA& frgba)
	{
		return RGBA((u8)(frgba.r * 255), (u8)(frgba.g * 255), (u8)(frgba.b * 255), (u8)(frgba.a * 255));
	}

	static u32 GetColorFromFloatRGB(const FloatRGBA& frgba)
	{
		return RGB((u8)(frgba.r * 255), (u8)(frgba.g * 255), (u8)(frgba.b * 255));
	}

	static u32 LerpRGB(u32 a, u32 b, float t)
	{
		FloatRGBA p = GetFloatColor(a);
		FloatRGBA q = GetFloatColor(b);
		FloatRGBA r{ Jogo::lerp(p.r, q.r, t), Jogo::lerp(p.g, q.g, t), Jogo::lerp(p.b, q.b, t), Jogo::lerp(p.a, q.a, t) };
		return GetColorFromFloatRGBA(r);
	}

	struct VertexLit
	{
		float x;
		float y;
		u32 c;
	};

	struct VertexTexLit : public Jogo::Vector4
	{
		u32 c;
		float u;
		float v;
	};

	struct Gradient
	{
		float drdy;
		float drdx;
		float dgdy;
		float dgdx;
		float dbdy;
		float dbdx;

		s32 fixdr;
		s32 fixdg;
		s32 fixdb;

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

		void Step(Gradient& grad)
		{
			x += dxdy;
			r += grad.drdy + grad.drdx * dxdy;
			g += grad.dgdy + grad.dgdx * dxdy;
			b += grad.dbdy + grad.dbdx * dxdy;
		}
	};

	Edge MakeEdge(const VertexLit& a, const VertexLit& b, Gradient& g);
	Gradient MakeGradient(VertexLit corners[]);
	void FillTriangle(VertexLit corners[]);
	void TriangleScanLine(s32 y, Edge&, Edge&, Gradient&);
	void FillTriangle(const VertexTexLit& a, const VertexTexLit& b, const VertexTexLit& c, const Bitmap& texture) const;
	void FillTriangleTL(const VertexTexLit& a, const VertexTexLit& b, const VertexTexLit& c, const Bitmap& texture) const;
	//void FillTexLitTriangle(VertexTexLit);

	static Bitmap Load(const char* filename, Arena& arena);
	static Bitmap Create(u32 Width, u32 Height, u32 PixelSize, Arena& arena)
	{
		Bitmap bitmap = { Width, Height, PixelSize };
		bitmap.Pixels = arena.Allocate(Width * Height *PixelSize);
		return bitmap;
	}
};
