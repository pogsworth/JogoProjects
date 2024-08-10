#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"
#include "Arena.h"
#include <stdio.h>
#include <string.h>

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

class Curves : public Jogo::JogoApp
{
	static const char* Name;
	static const u32 MAX_POINTS = 100;
	PointF Points[MAX_POINTS];
	bool Done = false;
	bool Dragging = false;
	PointF Down = { 0,0 };
	PointF Up = { 0,0 };
	u32 NumPoints = 0;

public:

	Curves() {}

	const char* GetName() const override { return Name; }

	virtual void KeyDown(u32 key) override
	{
		if (key == KEY_ESC)
		{
			Done = true;
		}
	}

	virtual void MouseDown(s32 x, s32 y, u32 buttons) override
	{
		if (buttons & 1)
		{
			Down = PointF(x, y);
			Dragging = true;
		}
	}

	virtual void MouseUp(s32 x, s32 y, u32 buttons) override
	{
		if (Dragging)
		{
			Points[NumPoints++] = Down;
			Dragging = false;
		}
	}

	virtual void MouseMove(s32 x, s32 y, u32 buttons) override
	{
		if (Dragging)
		{
			Down = PointF(x, y);
		}
	}

	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		char output[32];
		float num = 623.456f;
		for (s32 i = 1; i < 32; i++)
		{
			Jogo::ftoa(num, output, -1);
			printf("%s\n", output);
			num += 1.0f;
			if (i % 10 == 0)
			{
				num *= 10;
			}
		}
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

	void Draw() override
	{
		BackBuffer.Erase(0);
		// DefaultFont.DrawText(0,0,"Curves", 0xffffff, BackBuffer);
		for (u32 i = 0; i < NumPoints; i++)
		{
			BackBuffer.FillCircle((s32)Points[i].x, (s32)Points[i].y, 3, 0xff0000);
		}
		for (u32 i = 1; i < NumPoints; i++)
		{
			BackBuffer.DrawLine((s32)Points[i-1].x, (s32)Points[i-1].y, (s32)Points[i].x, (s32)Points[i].y, 0xffffff);
		}
		if (Dragging)
		{
			//float dx = Up.x - Down.x;
			//float dy = Up.y - Down.y;
			//float R = sqrtf(dx * dx + dy * dy);
			//BackBuffer.DrawCircle((s32)Down.x, (s32)Down.y, (s32)R, 0x00ff00);
			BackBuffer.FillCircle((s32)Down.x, (s32)Down.y, 3, 0xff0000);

			if (NumPoints)
			{
				BackBuffer.DrawLine((s32)Points[NumPoints - 1].x, (s32)Points[NumPoints - 1].y, (s32)Down.x, (s32)Down.y, 0xffffff);
			}
			// based on setting, draw curves between points
		}
		DrawParametricCurve();
		Jogo::Show(BackBuffer.PixelBGRA, Width, Height);
	}
};
const char* Curves::Name = "Curves";

int main(int argc, char* argv[])
{
	Curves curves;
//	Jogo::Run(curves, 60);
//	float i = 0.31415926f;
	float t = .999999f;
	u32 p = 2;
	// ftoa and ithex testing code
/* Test ftoa
	Jogo::Random rand = { 12345 };
	for (s32 j = 0; j < 100; j++)
	{
		u32 d = 0;
		for (s32 i = 0; i < 3; i++)
		{
			d *= 10;
			d += rand.GetNext() % 10;
		}
		s32 e = rand.GetNext() % 60 - 30;

		float f = (float)d / 100.f * Jogo::intpow(10.0f, e);	// *(float)i;
		char ftoa_result[32];
		Jogo::ftoa(f, ftoa_result, sizeof(ftoa_result), p);
		//Jogo::itohex(j, ftoa_result, sizeof(ftoa_result), 2);
		float output = (float)atof(ftoa_result);
		//if (f != output)
		{
			printf("%s: %.*g\n", ftoa_result,p,f);
			//printf("%s: %g - %g\n", ftoa_result, i, output);
		}
	}
*/

/* Exhaustive test of itohex and hextoi
	for (u32 i = 0; i < 0xffffffff; i++)
	{
		char hexbuf[9];
		Jogo::itohex(i, hexbuf, sizeof(hexbuf));
		u32 hex = Jogo::hextoi(hexbuf);
		if (i != hex)
		{
			printf("%s %d %d\n", hexbuf, i, hex);
		}
		if (i % 16777216 == 0)
		{
			printf(".");
		}
	}
*/

	return 0;
}