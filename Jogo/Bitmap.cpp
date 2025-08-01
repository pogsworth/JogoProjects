#include <stdio.h>
#include "Arena.h"
#include "Bitmap.h"
#include "JMath.h"

using namespace Jogo;

void Bitmap::FillRect(const Rect& r, u32 color)
{
	Rect clip;

	if (!ClipRect(r, clip))
		return;

	if (PixelSize == 1)
	{
		u8* row = PixelA + (clip.y * Width + clip.x) * PixelSize;
		for (s32 i = 0; i < clip.h; i++)
		{
			__stosb(row, (unsigned char)color, (size_t)clip.w);
			row += Width;
		}
	}
	else if (PixelSize == 4)
	{
		u32* row = PixelBGRA + (clip.y * Width + clip.x);
		for (s32 i = 0; i < clip.h; i++)
		{
			__stosd((unsigned long*)row, (unsigned long)color, (size_t)clip.w);
			row += Width;
		}
	}
}

bool Bitmap::ClipRect(const Rect& r, Rect& Clipped)
{
	Clipped = r;

	// exclude zero area rectangles
	if (r.w == 0 || r.h == 0)
		return false;

	if (r.w > 0 && r.h > 0)
	{
		// test the all included case first
		if (r.x >= 0 && r.x + r.w <= (s32)Width && r.y >= 0 && r.y + r.h <= (s32)Height)
			return true;
	}

	// test all excluded next
	if (r.w > 0 && (r.x >= (s32)Width || (r.x < 0 && r.x + r.w < 0)))
		return false;

	if (r.w < 0 && (r.x < 0 || (r.x >= (s32)Width && r.x + r.w >= (s32)Width)))
		return false;

	if (r.h > 0 && (r.y >= (s32)Height || (r.y < 0 && r.y + r.h < 0)))
		return false;

	if (r.h < 0 && (r.y < 0 || (r.y >= (s32)Height && r.y + r.h >= (s32)Height)))
		return false;

	// now alter any coords that need to be fixed...
	if (r.x < 0)
	{
		Clipped.w += r.x;
		Clipped.x = 0;
	}

	if (Clipped.x + Clipped.w < 0)
	{
		Clipped.w = -Clipped.x - 1;
	}

	if (Clipped.x >= (s32)Width)
	{
		Clipped.h -= Clipped.x - Width;
		Clipped.x = Width - 1;
	}

	if (Clipped.x + Clipped.w > (signed)Width)
	{
		Clipped.w = Width - Clipped.x;
	}

	if (r.y < 0)
	{
		Clipped.h += r.y;
		Clipped.y = 0;
	}

	if (Clipped.y >= (s32)Height)
	{
		Clipped.h -= Clipped.y - Height;
		Clipped.y = Height - 1;
	}

	if (Clipped.y + Clipped.h < 0)
	{
		Clipped.h = -Clipped.y - 1;
	}

	if (Clipped.y + Clipped.h > (signed)Height)
	{
		Clipped.h = Height - Clipped.y;
	}

	return true;
}

void Bitmap::PasteBitmapSelectionScaled(const Rect& dest, Bitmap source, const Rect& srcRect, u32 color, u32 bkcolor)
{
	if (!source.Pixels)
		return;

	Rect DstClip;
	if (!ClipRect(dest, DstClip))
		return;

	float sxdx = (float)srcRect.w / abs(dest.w);
	float sydy = (float)srcRect.h / abs(dest.h);

	float sourceX = srcRect.x + (DstClip.x - dest.x) * sxdx;
	if (sourceX < 0 || sourceX > source.Width)
	{
		sourceX = (float)(srcRect.x % source.Width);
		if (sourceX < 0)
			sourceX += source.Width;
	}

	float sourceY = srcRect.y + (DstClip.y - dest.y) * sydy;
	if (sourceY < 0 || sourceY > source.Height)
	{
		sourceY = (float)(srcRect.y % source.Height);
		if (sourceY < 0)
			sourceY += source.Width;
	}

	u8* DstRow = PixelA + DstClip.y * Width * PixelSize + DstClip.x * PixelSize;
	s32 PixelStep = PixelSize;
	s32 VertStep = Width;
	if (DstClip.w < 0)
	{
		PixelStep = -PixelStep;
		DstClip.w = -DstClip.w;
	}
	if (DstClip.h < 0)
	{
		DstClip.h = -DstClip.h;
		VertStep = -VertStep;
	}
	for (s32 j = 0; j < DstClip.h; j++)
	{
		u8* Dest = DstRow;
		float x = sourceX;

		if (PixelSize == source.PixelSize)
		{
			if (PixelSize == 1)
			{
				u8* SrcRow = source.PixelA + (u32)sourceY * source.Width;
				for (s32 i = 0; i < DstClip.w; i++)
				{
					x += sxdx;
					if (x < 0)
						x += source.Width;
					if (x >= source.Width)
						x -= source.Width;
					*Dest = *(SrcRow + (u32)x);
					Dest += PixelStep;
				}
			}
			else
			{
				u32* SrcRow = source.PixelBGRA + (u32)sourceY * source.Width;
				for (s32 i = 0; i < DstClip.w; i++)
				{
					x += sxdx;
					if (x < 0)
						x += source.Width;
					if (x >= source.Width)
						x -= source.Width;
					*((u32*)Dest) = *((u32*)SrcRow + (u32)x);
					Dest += PixelStep;
				}
			}
		}
		else if (PixelSize == 4)
		{
			if (bkcolor & 0xff000000)
			{
				u8* SrcRow = source.PixelA + (u32)sourceY * source.Width;
				for (s32 i = 0; i < DstClip.w; i++)
				{
					if (x < 0) x += source.Width;
					if (x >= source.Width) x -= source.Width;
					*((u32*)Dest) = (*(SrcRow + (u32)x)) ? color : bkcolor;
					Dest += PixelStep;
					x += sxdx;
				}
			}
			else
			{
				u8* SrcRow = source.PixelA + (u32)sourceY * source.Width;
				for (s32 i = 0; i < DstClip.w; i++)
				{
					if (x < 0) x += source.Width;
					if (x >= source.Width) x -= source.Width;
					if (*(SrcRow + (u32)x))
					{
						*((u32*)Dest) = color;
					}
					Dest += PixelStep;
					x += sxdx;
				}
			}
		}
		sourceY += sydy;
		if (sourceY < 0)
			sourceY += source.Height;
		if (sourceY >= source.Height)
			sourceY -= source.Height;
		DstRow += (s64)VertStep * PixelSize;
	}
}

void Bitmap::PasteBitmapSelection(int x, int y, Bitmap source, const Rect& srcRect, u32 color, u32 bkcolor)
{
	if (!source.Pixels)
		return;

	Rect SrcClip;
	if (!source.ClipRect(srcRect, SrcClip))
		return;

	Rect DstClip;
	if (!ClipRect({ x, y, SrcClip.w, SrcClip.h }, DstClip))
		return;

	SrcClip.x += DstClip.x - x;
	SrcClip.y += DstClip.y - y;
	u8* SrcRow = source.PixelA + SrcClip.y * source.Width * source.PixelSize + SrcClip.x * source.PixelSize;
	u8* DstRow = PixelA + DstClip.y * Width * PixelSize + DstClip.x * PixelSize;

	if (PixelSize == source.PixelSize)
	{
		for (s32 j = 0; j < DstClip.h; j++)
		{
			__movsb(DstRow, SrcRow, DstClip.w * PixelSize);
			SrcRow += source.Width * PixelSize;
			DstRow += Width * PixelSize;
		}
	}
	else if (PixelSize == 4)
	{
		for (s32 j = 0; j < DstClip.h; j++)
		{
			u8* src = SrcRow;
			u32* dst = (u32*)DstRow;
			if (bkcolor & 0xff000000)
			{
				for (s32 i = 0; i < DstClip.w; i++)
				{
					*dst = *src ? color : bkcolor;
					src++;
					dst++;
				}
			}
			else
			{
				for (s32 i = 0; i < DstClip.w; i++)
				{
					if (*src)
					{
						*dst = color;
					}
					src++;
					dst++;
				}
			}
			SrcRow += source.Width;
			DstRow += Width * PixelSize;
		}
	}
}

bool Bitmap::ClipLine(s32& x1, s32& y1, s32& x2, s32& y2, Rect clipRect)
{
	s32 dx = x2 - x1;
	s32 dy = y2 - y1;
	s32 x;
	s32 y;
	s32 right = clipRect.x + clipRect.w;
	s32 bottom = clipRect.y + clipRect.h;

	// clip the line to the dimensions of the Bitmap
	u32 code1 = ((x1 < clipRect.x) << 3) | ((y1 < clipRect.y) << 2) | ((x1 >= clipRect.x + clipRect.w) << 1) | (y1 >= clipRect.y + clipRect.h);
	u32 code2 = ((x2 < clipRect.x) << 3) | ((y2 < clipRect.y) << 2) | ((x2 >= clipRect.x + clipRect.w) << 1) | (y2 >= clipRect.y + clipRect.h);

	// if both endpoints are outside of any of the same edge, then the whole line is out
	if (code1 & code2)
		return false;

	while (code1 | code2)
	{
		u32 outcode = code1 > code2 ? code1 : code2;

		if (outcode & 8)
		{
			y = y2 - dy * (x2 - clipRect.x) / dx;
			x = clipRect.x;
		}
		else if (outcode & 4)
		{
			x = x2 - dx * (y2 - clipRect.y) / dy;
			y = clipRect.y;
		}
		else if (outcode & 2)
		{
			y = y2 - dy * (x2 - right + 1) / dx;
			x = right - 1;
		}
		else if (outcode & 1)
		{
			x = x2 - dx * (y2 - bottom + 1) / dy;
			y = bottom - 1;
		}

		if (outcode == code1)
		{
			x1 = x;
			y1 = y;
			code1 = ((x1 < clipRect.x) << 3) | ((y1 < clipRect.y) << 2) | ((x1 >= clipRect.x + clipRect.w) << 1) | (y1 >= clipRect.y + clipRect.h);
		}
		else
		{
			x2 = x;
			y2 = y;
			code2 = ((x2 < clipRect.x) << 3) | ((y2 < clipRect.y) << 2) | ((x2 >= clipRect.x + clipRect.w) << 1) | (y2 >= clipRect.y + clipRect.h);
		}
		if (code1 & code2)
			return false;
	}
	return true;
}

void Bitmap::DrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u32 color)
{
	if (ClipLine(x1, y1, x2, y2, { 0,0,(s32)Width,(s32)Height }))
	{
		s32 dx = abs(x2 - x1);
		s32 sx = x1 < x2 ? 1 : -1;
		s32 dy = -abs(y2 - y1);
		s32 sy = y1 < y2 ? 1 : -1;
		s32 error = dx + dy;
		s32 x = x1;
		s32 y = y1;

		while (true)
		{
			SetPixel(x, y, color);
			if (x == x2 && y == y2)
				break;

			s32 e2 = error + error;
			if (e2 >= dy)
			{
				if (x == x2)
					break;
				error += dy;
				x += sx;
			}
			if (e2 <= dx)
			{
				if (y == y2)
					break;
				error += dx;
				y += sy;
			}
		}
	}
}

struct CircleDDA
{
	s32 x;
	s32 y;
	s32 f;
	s32 dx;
	s32 dy;

	void Start(s32 r)
	{
		f = 1 - r;
		dx = 0;
		dy = -2 * r;
		x = 0;
		y = r;
	}

	void Step()
	{
		if (f >= 0)
		{
			y--;
			dy += 2;
			f += dy;
		}
		x++;
		dx += 2;
		f += dx + 1;
	}

	bool Done()
	{
		return x >= y;
	}
};


void Bitmap::DrawCircle(s32 cx, s32 cy, s32 r, u32 color)
{
	CircleDDA Octant = {};
	Octant.Start(r);

	SetPixel(cx, cy + r, color);
	SetPixel(cx, cy - r, color);
	SetPixel(cx + r, cy, color);
	SetPixel(cx - r, cy, color);

	while (!Octant.Done())
	{
		Octant.Step();
		
		s32 x = Octant.x;
		s32 y = Octant.y;
		if (x <= y)
		{
			SetPixel(cx + x, cy + y, color);
			SetPixel(cx - x, cy + y, color);
			SetPixel(cx + x, cy - y, color);
			SetPixel(cx - x, cy - y, color);
		}
		if (x < y)
		{
			SetPixel(cx + y, cy + x, color);
			SetPixel(cx - y, cy + x, color);
			SetPixel(cx + y, cy - x, color);
			SetPixel(cx - y, cy - x, color);
		}
	}
}

void Bitmap::FillCircle(s32 cx, s32 cy, s32 r, u32 color)
{
	CircleDDA Octant = {};
	Octant.Start(r);

	DrawHLine(cy, cx - r, cx + r, color);
	while (!Octant.Done())
	{
		s32 x = Octant.x;
		s32 y = Octant.y;
		if (Octant.f >= 0)
		{
			if (x <= y)
			{
				DrawHLine(cy - y, cx - x, cx + x, color);
				DrawHLine(cy + y, cx - x, cx + x, color);
			}
		}
		Octant.Step();
		x = Octant.x;
		y = Octant.y;
		if (x <= y)
		{
			DrawHLine(cy - x, cx - y, cx + y, color);
			DrawHLine(cy + x, cx - y, cx + y, color);
		}
	}
}

void GetRoundedRWH(const Bitmap::Rect& box, s32 radius, s32& r, s32& w, s32& h)
{
	r = radius;
	w = box.w - 2 * r - 1;
	if (w < 0)
	{
		r = box.w / 2;
		w = box.w - 2 * r - 1;
	}
	h = box.h - 2 * r - 1;
	if (h < 0)
	{
		r = min(r, box.h / 2);
		h = box.h - 2 * r - 1;
		w = box.w - 2 * r - 1;
	}
}

void Bitmap::DrawRoundedRect(const Rect& box, s32 radius, u32 color)
{
	// how to clip this?
	if (radius <= 0)
	{
		DrawRect(box, color);
		return;
	}
	s32 right = box.x + box.w - 1;
	s32 bottom = box.y + box.h - 1;

	s32 r, w, h;
	GetRoundedRWH(box, radius, r, w, h);

	s32 lx = box.x + r;
	s32 uy = box.y + r;
	s32 rx = right - r;
	s32 ly = bottom - r;

	// draw the straight edges
	if (w)
	{
		DrawHLine(box.y, lx, rx, color);
		DrawHLine(bottom, lx, rx, color);
	}
	if (h)
	{
		DrawVLine(box.x, uy, ly, color);
		DrawVLine(right, uy, ly, color);
	}

	// draw the rounded corners
	CircleDDA Octant = {};
	Octant.Start(r);
	
	while (!Octant.Done())
	{
		Octant.Step();
		s32 x = Octant.x;
		s32 y = Octant.y;
		if (x <= y)
		{
			SetPixel(rx + x, uy - y, color);
			SetPixel(lx - x, uy - y, color);
			SetPixel(rx + x, ly + y, color);
			SetPixel(lx - x, ly + y, color);
		}
		if (x < y)
		{
			SetPixel(rx + y, uy - x, color);
			SetPixel(lx - y, uy - x, color);
			SetPixel(rx + y, ly + x, color);
			SetPixel(lx - y, ly + x, color);
		}
	}
}

void Bitmap::DrawRoundedRect(const Rect& box, s32 radius, u32 thickness, u32 color)
{
	// how to clip this?
	if (radius <= 0)
	{
		for (s32 t = 0; t < (s32)thickness; t++)
		{
			Rect r = { box.x + t, box.y + t, box.w - t * 2, box.h - t * 2};
			DrawRect(r, color);
		}
		return;
	}
	s32 right = box.x + box.w - 1;
	s32 bottom = box.y + box.h - 1;

	s32 r, w, h;
	GetRoundedRWH(box, radius, r, w, h);

	s32 lx = box.x + r;
	s32 uy = box.y + r;
	s32 rx = right - r;
	s32 ly = bottom - r;

	// draw the straight edges
	if (w)
	{
		for (u32 t = 0; t < thickness; t++)
		{
			DrawHLine(box.y + t, lx, rx, color);
			DrawHLine(bottom - t, lx, rx, color);
		}
	}
	if (h)
	{
		for (u32 t = 0; t < thickness; t++)
		{
			DrawVLine(box.x + t, uy, ly, color);
			DrawVLine(right - t, uy, ly, color);
		}
	}

	// draw the rounded corners
	CircleDDA BigR = {};
	CircleDDA LittleR = {};

	BigR.Start(r);
	s32 lr = r - thickness;
	lr = lr < 0 ? 0 : lr;
	LittleR.Start(lr);

	while (!BigR.Done())
	{
		BigR.Step();
		LittleR.Step();
		
		s32 x = BigR.x;
		s32 y = BigR.y;
		s32 span = Jogo::min(y - x + 1, y - LittleR.y);
		if (x <= y)
		{
			for (u32 t = 0; t < (u32)span; t++)
			{
				SetPixel(rx + x, uy - y + t, color);
				SetPixel(lx - x, uy - y + t, color);
				SetPixel(rx + x, ly + y - t, color);
				SetPixel(lx - x, ly + y - t, color);
			}
		}
		if (x < y)
		{
			for (u32 t = 0; t < (u32)span; t++)
			{
				SetPixel(rx + y - t, uy - x, color);
				SetPixel(lx - y + t, uy - x, color);
				SetPixel(rx + y - t, ly + x, color);
				SetPixel(lx - y + t, ly + x, color);
			}
		}
	}
}

void Bitmap::FillRoundedRect(const Rect& box, s32 radius, u32 color)
{
	// how to clip this?
	if (box.w < 1 || box.h < 1)
		return;
	if (radius <= 0)
	{
		FillRect(box, color);
		return;
	}
	s32 right = box.x + box.w - 1;
	s32 bottom = box.y + box.h - 1;

	s32 r, w, h;
	GetRoundedRWH(box, radius, r, w, h);

	s32 lx = box.x + r;
	s32 uy = box.y + r;
	s32 rx = right - r - 1;
	s32 ly = bottom - r - 1;

	// draw the straight edges
	if (h)
	{
		FillRect({ box.x, uy, box.w - 1, h }, color);
	}

	// draw the rounded corners
	CircleDDA Octant = {};
	Octant.Start(r);

	while (!Octant.Done())
	{
		s32 x = Octant.x;
		s32 y = Octant.y;
		if (Octant.f >= 0)
		{
			if (x <= y)
			{
				DrawHLine(uy - y, lx - x, rx + x, color);
				DrawHLine(ly + y, lx - x, rx + x, color);
			}
		}
		Octant.Step();

		x = Octant.x;
		y = Octant.y;

		if (x <= y)
		{
			DrawHLine(uy - x, lx - y, rx + y, color);
			DrawHLine(ly + x, lx - y, rx + y, color);
		}
	}
}

Bitmap::Edge Bitmap::MakeEdge(const Vertex& a, const Vertex& b, Gradient& g)
{
	Edge e;
	if (a.y <= b.y)
	{
		e.x1 = a.x;
		e.y1 = a.y;
		e.y2 = b.y;
		e.dy = b.y - a.y;
		e.dx = b.x - a.x;
		e.dxdy = e.dx / e.dy;
		e.r = GetRfloat(a.c);
		e.g = GetGfloat(a.c);
		e.b = GetBfloat(a.c);
	}
	else
	{
		e.x1 = b.x;
		e.y1 = b.y;
		e.y2 = a.y;
		e.dy = a.y - b.y;
		e.dx = a.x - b.x;
		e.dxdy = e.dx / e.dy;
		e.r = GetRfloat(b.c);
		e.g = GetGfloat(b.c);
		e.b = GetBfloat(b.c);
	}

	e.y = Jogo::ceil(e.y1);
	float dy = e.y - e.y1;
	float dx = dy * e.dxdy;
	e.x = e.x1 + dx;
	e.r += dy * g.drdy + dx * g.drdx;
	e.g += dy * g.dgdy + dx * g.dgdx;
	e.b += dy * g.dbdy + dx * g.dbdx;

	return e;
}

Bitmap::Gradient Bitmap::MakeGradient(Vertex corners[])
{
	Gradient g;

	float dx1 = corners[0].x - corners[2].x;
	float dx2 = corners[1].x - corners[2].x;
	float dy1 = corners[0].y - corners[2].y;
	float dy2 = corners[1].y - corners[2].y;

	float dr1 = GetRfloat(corners[0].c) - GetRfloat(corners[2].c);
	float dr2 = GetRfloat(corners[1].c) - GetRfloat(corners[2].c);
	float dg1 = GetGfloat(corners[0].c) - GetGfloat(corners[2].c);
	float dg2 = GetGfloat(corners[1].c) - GetGfloat(corners[2].c);
	float db1 = GetBfloat(corners[0].c) - GetBfloat(corners[2].c);
	float db2 = GetBfloat(corners[1].c) - GetBfloat(corners[2].c);

	float OneOverdx = 1.0f / (dx2 * dy1 - dx1 * dy2);
	float OneOverdy = -OneOverdx;
	g.drdx = (dr2 * dy1 - dr1 * dy2) * OneOverdx;
	g.drdy = (dr2 * dx1 - dr1 * dx2) * OneOverdy;
	g.dgdx = (dg2 * dy1 - dg1 * dy2) * OneOverdx;
	g.dgdy = (dg2 * dx1 - dg1 * dx2) * OneOverdy;
	g.dbdx = (db2 * dy1 - db1 * dy2) * OneOverdx;
	g.dbdy = (db2 * dx1 - db1 * dx2) * OneOverdy;

	return g;
}

void Bitmap::FillTriangle(Vertex corners[])
{
	Gradient grad = MakeGradient(corners);

	Edge edges[3];
	for (int i = 0; i < 3; i++)
	{
		edges[i] = MakeEdge(corners[i], corners[(i + 1) % 3], grad);
	}
	// sort edges on beginning y value
	// and first edge is the leftmost
	if (edges[0].y1 > edges[1].y1)
	{
		Edge e = edges[0];
		edges[0] = edges[1];
		edges[1] = e;
	}
	if (edges[1].y1 > edges[2].y1)
	{
		Edge e = edges[1];
		edges[1] = edges[2];
		edges[2] = e;
	}
	if (edges[0].dx > edges[1].dx)
	{
		Edge e = edges[0];
		edges[0] = edges[1];
		edges[1] = e;
	}

	bool bThirdEdgeUsed = false;
	Edge leftEdge = edges[0];
	Edge rightEdge = edges[1];
	if (Jogo::ceil(leftEdge.y1) == Jogo::ceil(leftEdge.y2))
	{
		leftEdge = edges[2];
		bThirdEdgeUsed = true;
	}
	if (Jogo::ceil(rightEdge.y1) == Jogo::ceil(rightEdge.y2))
	{
		if (bThirdEdgeUsed)
			return;
		rightEdge = edges[2];
		bThirdEdgeUsed = true;
	}

	s32 top = (s32)Jogo::ceil(edges[0].y1);
	s32 bottom = (s32)Jogo::ceil(edges[2].y2);

	for (s32 y = top; y < bottom; y++)
	{
		if (y > leftEdge.y2)
		{
			if (bThirdEdgeUsed)
				break;

			// replace leftEdge
			leftEdge = edges[2];
			bThirdEdgeUsed = true;

			if (Jogo::ceil(leftEdge.y1) == Jogo::ceil(leftEdge.y2))
				return;
		}
		if (y > rightEdge.y2)
		{
			if (bThirdEdgeUsed)
				break;

			// replace rightEdge
			rightEdge = edges[2];
			bThirdEdgeUsed = true;

			if (Jogo::ceil(rightEdge.y1) == Jogo::ceil(rightEdge.y2))
				return;
		}
		s32 fixdr = (s32)(grad.drdx * 65535.0f);
		s32 fixdg = (s32)(grad.dgdx * 65535.0f);
		s32 fixdb = (s32)(grad.dbdx * 65535.0f);

		// scissor clip y
		if (y >= 0 && y < (s32)Height)
		{
			// scissor clip x
			s32 x1 = (s32)Jogo::ceil(leftEdge.x);
			x1 = max(min(x1, (s32)Width - 1), 0);
			s32 x2 = (s32)Jogo::ceil(rightEdge.x);
			x2 = max(min(x2, (s32)Height - 1), 0);
			float dx = x1 - leftEdge.x;
			float r = leftEdge.r + dx * grad.drdx;
			float g = leftEdge.g + dx * grad.dgdx;
			float b = leftEdge.b + dx * grad.dbdx;

			s32 fixr = (s32)(r * 65535.0f);
			s32 fixg = (s32)(g * 65535.0f);
			s32 fixb = (s32)(b * 65535.0f);

			u32* pixel = PixelBGRA + y * Width + x1;

			for (s32 x = x1; x < x2; x++)
			{
				//_mm_shufflehi_epi16
				//_mm_srliepi16(fixrgb, 8);
				//_mm_packus_epi16(highbytes);
				*pixel++ = ((fixr & 0xff00) << 8) + (fixg & 0xff00) + ((fixb & 0xff00) >> 8);;
				fixr += fixdr;
				fixg += fixdg;
				fixb += fixdb;
			}

		}
		// update left and right edge x
		leftEdge.x += leftEdge.dxdy;
		leftEdge.r += grad.drdy + grad.drdx * leftEdge.dxdy;
		leftEdge.g += grad.dgdy + grad.dgdx * leftEdge.dxdy;
		leftEdge.b += grad.dbdy + grad.dbdx * leftEdge.dxdy;
		rightEdge.x += rightEdge.dxdy;
		rightEdge.r += grad.drdy + grad.drdx * rightEdge.dxdy;
		rightEdge.g += grad.dgdy + grad.dgdx * rightEdge.dxdy;
		rightEdge.b += grad.dbdy + grad.dbdx * rightEdge.dxdy;
	}
}

// ---- triangle rasterizer, from https://gist.github.com/rygorous/9b793cd21d876da928bf4c7f3e625908

#define SUBPIXEL_SHIFT  8
#define SUBPIXEL_SCALE  (1 << SUBPIXEL_SHIFT)

s64 det2x2(s32 a, s32 b, s32 c, s32 d)
{
	s64 r = (s64)a * d - (s64)b * c;
	return r >> SUBPIXEL_SHIFT;
}

s64 det2x2_fill_convention(s32 a, s32 b, s32 c, s32 d)
{
	s64 r = (s64)a * d - (s64)b * c;         // the determinant
	if (c > 0 || c == 0 && a <= 0) r--;    // this implements the top-left fill convention
	return r >> SUBPIXEL_SHIFT;
}

s32 fixed(f32 x)
{
	// -0.5f to place pixel centers at integer coords + 0.5
	// +0.5f afterwards is rounding factor
	return (s32)((x - 0.5f) * SUBPIXEL_SCALE + 0.5f);
}

static s32 fixed_ceil(s32 x)
{
	return (x + SUBPIXEL_SCALE - 1) >> SUBPIXEL_SHIFT;
}

typedef s32 EdgeDist; // switch to s64 if there are overflows

static void point_to_fixed_xform(s32* ox, s32* oy, Bitmap::Vertex* in)	// , gswf_matrix* m)
{
	//f32 x = in->x * m->m00 + (in->y * m->m01 + m->trans[0]);
	//f32 y = in->x * m->m10 + (in->y * m->m11 + m->trans[1]);
	*ox = fixed(in->x);
	*oy = fixed(in->y);
}

void Bitmap::FillTriangle(Vertex* a, Vertex* b, Vertex* c)	//, gswf_matrix* m, int allow_neg_winding)
{
	s32 x0, x1, x2;
	s32 y0, y1, y2;
	s32 minx, miny, maxx, maxy;
	s32 dx10, dx21, dx02;
	s32 dy10, dy21, dy02;
	EdgeDist e1, e2, e3;
	s32 t;
	s32 x, y;
	s64 det;
	int incr;

	u32 color = 0xffff;

	// convert coordinates to fixed point
	point_to_fixed_xform(&x0, &y0, a);	// , m);
	point_to_fixed_xform(&x1, &y1, b);	// , m);
	point_to_fixed_xform(&x2, &y2, c);	// , m);

	// check triangle winding order
	det = det2x2(x1 - x0, x2 - x0, y1 - y0, y2 - y0);
	if (det > 0)
		incr = 1;
	else if (det < 0) {
		incr = 1;	// allow_neg_winding ? -1 : 1;
		t = x0; x0 = x1; x1 = t;
		t = y0; y0 = y1; y1 = t;
	}
	else // zero-area triangle
		return;

	// bounding box / clipping
	minx = max(fixed_ceil(min3(x0, x1, x2)), (s32)0);
	miny = max(fixed_ceil(min3(y0, y1, y2)), (s32)0);
	maxx = min(fixed_ceil(max3(x0, x1, x2)), (s32)Width);	// tgt->w);
	maxy = min(fixed_ceil(max3(y0, y1, y2)), (s32)Height);	// tgt->h);
	if (minx >= maxx || miny >= maxy)
		return;

	// edge vectors
	dx10 = x1 - x0; dy10 = y1 - y0;
	dx21 = x2 - x1; dy21 = y2 - y1;
	dx02 = x0 - x2; dy02 = y0 - y2;

	// edge functions
	e1 = (EdgeDist)det2x2_fill_convention(dx10, (minx << SUBPIXEL_SHIFT) - x0, dy10, (miny << SUBPIXEL_SHIFT) - y0);
	e2 = (EdgeDist)det2x2_fill_convention(dx21, (minx << SUBPIXEL_SHIFT) - x1, dy21, (miny << SUBPIXEL_SHIFT) - y1);
	e3 = (EdgeDist)det2x2_fill_convention(dx02, (minx << SUBPIXEL_SHIFT) - x2, dy02, (miny << SUBPIXEL_SHIFT) - y2);

	// rasterize
	for (y = miny; y < maxy; y++) {
		u32* line = PixelBGRA + y * Width;	// tgt->data + y * tgt->w;
		EdgeDist ei1 = e1, ei2 = e2, ei3 = e3;

		for (x = minx; x < maxx; x++) {
			if ((ei1 | ei2 | ei3) >= 0) // pixel in triangle
			{
				line[x] = color;	// line[x] + incr;
			}
			ei1 -= dy10;
			ei2 -= dy21;
			ei3 -= dy02;
		}

		e1 += dx10;
		e2 += dx21;
		e3 += dx02;
	}
}


Bitmap Bitmap::Load(const char* filename, Arena& arena)
{
#pragma pack(push,2)
	struct BitmapHeader
	{
		u16 BMPSignature;	// usually 'BM'
		u32 FileSize;
		u16 Reserved;
		u16 Reserved2;
		u32 ImageOffset;

		// BitmapInfoHeader
		u32 StructureSize;
		s32 Width;
		s32 Height;
		u16 Planes;
		u16 BitCount;
		u32 Compression;
		u32 ImageSize;
		u32 XPixelsPerMeter;
		u32 YPixelsPerMeter;
		u32 ColorsUsed;
		u32 ColorImportant;
	} Header = {};
#pragma pack(pop)

	FILE* fp = nullptr;
	if (!fopen_s(&fp, filename, "rb"))
	{
		fread(&Header, sizeof(BitmapHeader), 1, fp);
		if (Header.ImageOffset != sizeof(BitmapHeader))
		{
			fseek(fp, Header.ImageOffset, SEEK_SET);
		}

		if (Header.BitCount == 8 || Header.BitCount == 24 || Header.BitCount == 32)
		{
			u32 FlipImage = Header.Height > 0;
			u32 ImageHeight = Header.Height < 0 ? -Header.Height : Header.Height;

			if (Header.Width <= 65536 && ImageHeight <= 65536)
			{
				u32 FilePixelSize = Header.BitCount / 8;
				// convert 24-bit bitmaps to 32-bit
				u32 ImagePixelSize = FilePixelSize;
				if (ImagePixelSize == 3)
					ImagePixelSize = 4;

				Bitmap Image = { (u32)Header.Width, ImageHeight, ImagePixelSize };
				Image.Pixels = arena.Allocate(Header.Width * ImageHeight * ImagePixelSize);
				if (Image.Pixels != nullptr)
				{
					u8* RowPtr = Image.PixelA;
					s32 ImageStride = Image.Width * ImagePixelSize;
					if (FlipImage)
					{
						RowPtr = Image.PixelA + (ImageHeight - 1) * Header.Width * ImagePixelSize;
						ImageStride = -ImageStride;
					}
					u32 LeftOver = Image.Width * FilePixelSize & 0x3;
					LeftOver = LeftOver ? 4 - LeftOver : 0;
					for (u32 i = 0; i < ImageHeight; i++)
					{
						fread(RowPtr, Image.Width * FilePixelSize, 1, fp);
						if (LeftOver)
						{
							fseek(fp, LeftOver, SEEK_CUR);
						}

						// convert 24-bit bitmaps to 32-bit
						if (FilePixelSize == 3)
						{
							u8* EndOfFileRow = RowPtr + Image.Width * FilePixelSize - 1;
							u8* EndOfImageRow = RowPtr + Image.Width * ImagePixelSize - 1;
							for (u32 j = 0; j < Image.Width; j++)
							{
								*EndOfImageRow-- = 0xff;
								*EndOfImageRow-- = *EndOfFileRow--;
								*EndOfImageRow-- = *EndOfFileRow--;
								*EndOfImageRow-- = *EndOfFileRow--;
							}
						}
						RowPtr += ImageStride;
					}

					fclose(fp);
					return Image;
				}
			}
		}
		fclose(fp);
	}

	Bitmap EmptyBitmap = {};
	return EmptyBitmap;
}
