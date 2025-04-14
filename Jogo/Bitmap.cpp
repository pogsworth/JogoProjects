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

void Bitmap::DrawCircle(s32 cx, s32 cy, s32 r, u32 color)
{
	s32 f = 1 - r;
	s32 ddx = 0;
	s32 ddy = -2 * r;
	s32 x = 0;
	s32 y = r;

	SetPixel(cx, cy + r, color);
	SetPixel(cx, cy - r, color);
	SetPixel(cx + r, cy, color);
	SetPixel(cx - r, cy, color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddy += 2;
			f += ddy;
		}
		x++;
		ddx += 2;
		f += ddx + 1;
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
	s32 f = 1 - r;
	s32 ddx = 0;
	s32 ddy = -2 * r;
	s32 x = 0;
	s32 y = r;

	DrawHLine(cy, cx - r, cx + r, color);
	while (x < y)
	{
		if (f >= 0)
		{
			if (x <= y)
			{
				DrawHLine(cy - y, cx - x, cx + x, color);
				DrawHLine(cy + y, cx - x, cx + x, color);
			}
			y--;
			ddy += 2;
			f += ddy;
		}
		x++;
		ddx += 2;
		f += ddx + 1;
		if (x <= y)
		{
			DrawHLine(cy - x, cx - y, cx + y, color);
			DrawHLine(cy + x, cx - y, cx + y, color);
		}
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

	s32 r = radius;
	s32 w = box.w - 2 * r - 1;
	if (w < 0)
	{
		r = box.w / 2;
		w = box.w - 2 * r - 1;
	}
	s32 h = box.h - 2 * r - 1;
	if (h < 0)
	{
		r = min(r, box.h / 2);
		h = box.h - 2 * r - 1;
		w = box.w - 2 * r - 1;
	}
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
	s32 f = 1 - r;
	s32 ddx = 0;
	s32 ddy = -2 * r;
	s32 x = 0;
	s32 y = r;

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddy += 2;
			f += ddy;
		}
		x++;
		ddx += 2;
		f += ddx + 1;
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

	s32 r = radius;
	s32 w = box.w - 2 * r - 1;
	if (w < 0)
	{
		r = box.w / 2;
		w = max(0, box.w - 2 * r - 1);
	}
	s32 h = box.h - 2 * r - 1;
	if (h < 0)
	{
		r = min(r, box.h / 2);
		h = max(0, box.h - 2 * r - 1);
		w = max(0, box.w - 2 * r - 1);
	}
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
	s32 f = 1 - r;
	s32 ddx = 0;
	s32 ddy = -2 * r;
	s32 x = 0;
	s32 y = r;

	while (x < y)
	{
		if (f >= 0)
		{
			if (x <= y)
			{
				DrawHLine(uy - y, lx - x, rx + x, color);
				DrawHLine(ly + y, lx - x, rx + x, color);
			}
			y--;
			ddy += 2;
			f += ddy;
		}
		x++;
		ddx += 2;
		f += ddx + 1;
		if (x <= y)
		{
			DrawHLine(uy - x, lx - y, rx + y, color);
			DrawHLine(ly + x, lx - y, rx + y, color);
		}
	}
}

Bitmap::Edge Bitmap::MakeEdge(const Vertex& a, const Vertex& b)
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
		edges[i] = MakeEdge(corners[i], corners[(i + 1) % 3]);
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
	s32 y1 = (s32)Jogo::ceil(edges[0].y1);
	s32 y2 = (s32)Jogo::ceil(edges[2].y2);
	float dy = y1 - leftEdge.y1;
	float dx = dy * leftEdge.dxdy;
	leftEdge.x = leftEdge.x1 + dx;
	dy = y1 - rightEdge.y1;
	dx = dy * rightEdge.dxdy;
	rightEdge.x = rightEdge.x1 + dx;

	u32 color = 0xffffff;
	for (s32 y = y1; y < y2; y++)
	{
		if (y > leftEdge.y2)
		{
			if (bThirdEdgeUsed)
				break;

			// replace leftEdge
			leftEdge = edges[2];
			leftEdge.x = leftEdge.x1 + (leftEdge.y1 - y) * leftEdge.dxdy;
			float dy = y - leftEdge.y1;
			float dx = dy * leftEdge.dxdy;
			leftEdge.x = leftEdge.x1 + dx;
			leftEdge.r += dy * grad.drdy + dx * grad.drdx;
			leftEdge.g += dy * grad.dgdy + dx * grad.dgdx;
			leftEdge.b += dy * grad.dbdy + dx * grad.dbdx;
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
			float dy = y - rightEdge.y1;
			float dx = dy * rightEdge.dxdy;
			rightEdge.x = rightEdge.x1 + dx;
			rightEdge.r += dy * grad.drdy + dx * grad.drdx;
			rightEdge.g += dy * grad.dgdy + dx * grad.dgdx;
			rightEdge.b += dy * grad.dbdy + dx * grad.dbdx;
			bThirdEdgeUsed = true;

			if (Jogo::ceil(rightEdge.y1) == Jogo::ceil(rightEdge.y2))
				return;
		}
		s32 x1 = (s32)Jogo::ceil(leftEdge.x);
		s32 x2 = (s32)Jogo::ceil(rightEdge.x);
		float dx = x1 - leftEdge.x;
		float r = leftEdge.r + dx * grad.drdx;
		float g = leftEdge.g + dx * grad.dgdx;
		float b = leftEdge.b + dx * grad.dbdx;

		for (s32 x = x1; x < x2; x++)
		{
			color = RGB((u8)(r * 255.0f), (u8)(g * 255.0f), (u8)(b * 255.0f));
			SetPixel(x, y, color);
			r += grad.drdx;
			g += grad.dgdx;
			b += grad.dbdx;
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
