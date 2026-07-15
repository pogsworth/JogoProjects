#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"
#include "Arena.h"
#include <stdio.h>
#include "str8.h"
#include "Jobs.h"

using namespace Jogo;


struct PointF
{
	PointF() {}
	PointF(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
	PointF(s32 x, s32 y)
	{
		this->x = (float)x;
		this->y = (float)y;
	}
	float x;
	float y;
};

Bitmap::Rect JobList[2024];
u32 numJobs = 0;
Random random;

class JuliaSet: public Jogo::App
{
	static const char* Name;
	static const u32 MAX_POINTS = 100;
	PointF Points[MAX_POINTS];
	bool Done = false;
	bool Dragging = false;
	PointF Down = { 0,0 };
	PointF Up = { 0,0 };
	u32 NumPoints = 0;
	JobSystem jobs;

	static float g_Time;

public:

	JuliaSet() {
		const u32 numThreads = 8;
		jobs.Initialize(numThreads);
		Timer timer;
		random = { (u32)timer.Start() };
	}

	const char* GetName() const override { return Name; }

	virtual bool KeyDown(Input::Keys key) override
	{
		if (key == Input::KEY_ESC)
		{
			Done = true;
		}

		return true;
	}

	virtual bool MouseDown(s32 x, s32 y, Input::Keys buttons) override
	{
		if (buttons & 1)
		{
			Down = PointF(x, y);
			Dragging = true;
		}

		return true;
	}

	virtual bool MouseUp(s32 x, s32 y, Input::Keys buttons) override
	{
		if (Dragging)
		{
			Points[NumPoints++] = Down;
			Dragging = false;
		}

		return true;
	}

	virtual bool MouseMove(s32 x, s32 y) override
	{
		if (Dragging)
		{
			Down = PointF(x, y);
		}

		return true;
	}

	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		//char output[32];
		//float num = 623.456f;
		//for (s32 i = 1; i < 32; i++)
		//{
		//	str8::ftoa(num, output, -1);
		//	printf("%s\n", output);
		//	num += 1.0f;
		//	if (i % 10 == 0)
		//	{
		//		num *= 10;
		//	}
		//}

		g_Time += DT;

		u32 jobIndex = 0;
		for (s32 j = 0; j < Height; j += 32)
		{
			s32 h = min(Height - j, 32);
			for (s32 i = 0; i < Width; i += 32)
			{
				s32 w = min(Width - i, 32);
				Bitmap::Rect r{ i, j, w, h};
				JobList[jobIndex++] = r;
			}
		}
		numJobs = jobIndex;
		return Done;
	}

	void DrawParametricCurve()
	{
		if (NumPoints > 2)
		{
			for (u32 i = 0; i < NumPoints-1; i++)
			{
				float t0 = (float)i;
				float t1 = (float)(i + 1);
				float t2 = (float)(i + 2);
				PointF p0 = Points[i];
				PointF p1 = Points[i + 1];
				PointF p2 = Points[i + 2];
				float x1, y1, x2, y2;
				x1 = p0.x;
				y1 = p0.y;
				for (float t = t0; t < t1; t += 0.01f)
				{
					x2 = p0.x * (t1 - t) * (t2 - t) / ((t1 - t0) * (t2 - t0)) +
						p1.x * (t0 - t) * (t2 - t) / ((t0 - t1) * (t2 - t1)) +
						p2.x * (t0 - t) * (t1 - t) / ((t0 - t2) * (t1 - t2));
					y2 = p0.y * (t1 - t) * (t2 - t) / ((t1 - t0) * (t2 - t0)) +
						p1.y * (t0 - t) * (t2 - t) / ((t0 - t1) * (t2 - t1)) +
						p2.y * (t0 - t) * (t1 - t) / ((t0 - t2) * (t1 - t2));

					BackBuffer.DrawLine((s32)x1, (s32)y1, (s32)x2, (s32)y2, 0xff00ff);
					x1 = x2;
					y1 = y2;
				}
			}
		}
	}


	static void RenderJuliaTile(const Bitmap::Rect& tile, Bitmap& bitmap) {
		// These constants define the shape of the fractal.
		// Changing these slightly changes the entire pattern!
		const float cRe = -0.7f;	// +sine(g_Time * 2) * 0.05f;	// -0.7f;
		const float cIm = 0.27015f + sine(g_Time ) * 0.05f;

		// Zoom and pan offsets to fit the fractal nicely on screen
		const float zoom = .75f;
		const float moveX = 0.0f;
		const float moveY = 0.0f;

		const int maxIterations = 200; // Higher = more detail, heavier CPU load

		// Iterate through the 32x32 pixel bounds of this specific job tile
		for (int y = 0; y < tile.h;  ++y) {
			for (int x = 0; x < tile.w; ++x) {

				// Map pixel coordinates to the complex plane (-2.0 to 2.0)
				float zx = (signed)(x + tile.x - bitmap.Width / 2) / (0.5f * zoom * bitmap.Width) + moveX;
				float zy = (signed)(y + tile.y - bitmap.Height / 2) / (0.5f * zoom * bitmap.Height) + moveY;

				int i = 0;
				// The core mathematical loop: Z = Z^2 + C
				while (zx * zx + zy * zy < 4.0f && i < maxIterations) {
					float temp = zx * zx - zy * zy + cRe;
					zy = 2.0f * zx * zy + cIm;
					zx = temp;
					i++;
				}

				// --- COLOR MAPPING ---
				// Convert the iteration count into an interesting color palette
				BYTE r = 0, g = 0, b = 0;
				if (i < maxIterations) {
					// Smooth psychedelic coloring based on sine waves
					r = static_cast<BYTE>(sine(0.1f * i + 0.0f) * 127.0f + 128.0f);
					g = static_cast<BYTE>(sine(0.1f * i + 2.0f) * 127.0f + 128.0f);
					b = static_cast<BYTE>(sine(0.1f * i + 4.0f) * 127.0f + 128.0f);
				}
				else {
					// Interior pixels that never "escaped" stay black
					r = g = b = 0;
				}

				bitmap.SetPixel(x+tile.x, y+tile.y, (r << 16) | (g << 8) | b);
			}
		}
	}

	static void DrawSomething(int index, void* data)
	{
		Bitmap* bitmap = (Bitmap*)data;

		Bitmap::Rect jobRect = JobList[index];
		RenderJuliaTile(jobRect, *bitmap);
	}

	void Draw() override
	{
		BackBuffer.Erase(0);
		// DefaultFont.DrawText(0,0,"Curves", 0xffffff, BackBuffer);
		//for (u32 i = 0; i < NumPoints; i++)
		//{
		//	BackBuffer.FillCircle((s32)Points[i].x, (s32)Points[i].y, 3, 0xff0000);
		//}
		//for (u32 i = 1; i < NumPoints; i++)
		//{
		//	BackBuffer.DrawLine((s32)Points[i - 1].x, (s32)Points[i - 1].y, (s32)Points[i].x, (s32)Points[i].y, 0xffffff);
		//}
		//if (Dragging)
		//{
		//	//float dx = Up.x - Down.x;
		//	//float dy = Up.y - Down.y;
		//	//float R = sqrtf(dx * dx + dy * dy);
		//	//BackBuffer.DrawCircle((s32)Down.x, (s32)Down.y, (s32)R, 0x00ff00);
		//	BackBuffer.FillCircle((s32)Down.x, (s32)Down.y, 3, 0xff0000);

		//	if (NumPoints)
		//	{
		//		BackBuffer.DrawLine((s32)Points[NumPoints - 1].x, (s32)Points[NumPoints - 1].y, (s32)Down.x, (s32)Down.y, 0xffffff);
		//	}
		//	// based on setting, draw curves between points
		//}
		//DrawParametricCurve();
		if (numJobs)
			jobs.Execute(DrawSomething, &BackBuffer, numJobs);
		Jogo::Show(BackBuffer.PixelBGRA, Width, Height);
	}
};

float JuliaSet::g_Time = 0;

const char* JuliaSet::Name = "JuliaSet";

int main(int argc, char* argv[])
{
	JuliaSet julia;
	Jogo::Run(julia, 1000);

	return 0;
}