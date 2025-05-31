#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"
#include "Arena.h"
#include <stdio.h>
#include "str8.h"

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

class Curves : public Jogo::App
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
			BackBuffer.DrawLine((s32)Points[i - 1].x, (s32)Points[i - 1].y, (s32)Points[i].x, (s32)Points[i].y, 0xffffff);
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

void TestFloatFormat()
{

}

int main(int argc, char* argv[])
{
	Curves curves;
	//	Jogo::Run(curves, 60);
	//	float i = 0.31415926f;
	float t = .999999f;
	u32 p = 7;
	Jogo::Random rand = { 12345 };

	char ftoa_result[32];
	char printfg[36];

	bool bTestRange = false;
	if (bTestRange)
	{
		float numbers[400];
		float numbers2[300];
		float numbers10[100];
		float f;
		s32 two230 = 1 << 30;
		float begin = 128.0f;
		begin *= (float)two230;
		begin *= (float)two230;
		begin *= (float)two230;
		begin *= (float)two230;

		s32 i = 0;
		for (f = begin; f > 1e-45; f /= 2)
		{
			numbers2[i++] = f;
		}
		s32 size2 = i;
		begin = 1.00000002e38f;
		i = 0;
		s32 j = 38;
		for (f = begin; f > 1e-45; f /= 10)
		{
			numbers10[i] = f;
			numbers10[i] = (float)Jogo::intpow(10.0, j);
			i++;
			j--;
		}
		s32 size10 = i;

		s32 i2 = 0;
		s32 i10 = 0;
		s32 allsize = size2 + size10;
		for (s32 i = 0; i < allsize; i++)
		{
			if (numbers2[i2] > numbers10[i10])
			{
				numbers[i] = numbers2[i2];
				i2++;
			}
			else
			{
				numbers[i] = numbers10[i10];
				i10++;
			}
		}

		u32 d = 1048576;
		u32 p = 9;
		//		numbers[0] = 10;
		//		allsize = 1;
		for (i = 0; i < allsize; i++)
		{
			u32 len = str8::ftoa(numbers[i], ftoa_result, 32, p);
			str8 numberstr(ftoa_result, len);
			float result = numberstr.atof();
//			sprintf(printfg, "%.*g", p, numbers[i]);
//			if (strcmp(ftoa_result, printfg))
//				printf("%s - %.*g - %g %.*g\n", ftoa_result, p, result, numbers[i], p, numbers[i]);
		}
	}

	union IntFloat
	{
		f32 f;
		s32 i;
	};

	bool bTestExhaustive = false;
	if (bTestExhaustive)
	{
		//exhaustively check round trip of all floats that are not nan or inf
		u32 count = 0;
		for (s32 f = 0; f < 0x3f800000; f++)
		{
			IntFloat intf;
			intf.i = f;

			u32 len = str8::ftoa(intf.f, ftoa_result, 32, p);
			str8 resultstr(ftoa_result, len);
			float result = resultstr.atof();
			if (result != intf.f)
			{
				printf("%g:%g, %g\n", intf.f, result, result - intf.f);
				count++;
			}
		}
		printf("Total errors: %d\n", count);
	}

	bool bTestIntegers = false;
	if (bTestIntegers)
	{
		for (s32 i = -2147483647; i < 2147480000; i += 2047)
		{
			float number = (float)i;
			u32 len = str8::ftoa(number, ftoa_result, 32, p);
			str8 resultstr(ftoa_result, len);
			float result = resultstr.atof();
			sprintf_s(printfg, sizeof(printfg), "%.*g", p, number);

//			if (strcmp(ftoa_result, printfg))
			{
				char dtoa_result[80] = { 32 };
				//				Jogo::dtoa(number, dtoa_result, (p - 1) | 0x80000000);

				printf("%s - %.*g - %g %.*g \n", ftoa_result, p, result, number, p, number);
			}
		}
	}

	bool bTestPowers = false;
	if (bTestPowers)
	{
		double pof10[] = {
			1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30,
			1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20,
			1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,
			1e-09, 1e-08, 1e-07, 1e-06, 1e-05, 1e-04, 1e-03, 1e-02, 1e-01,
			1e0, 1e1, 1e2,1e3,1e4,1e5, 1e6,1e7,1e8, 1e9,
			1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
			1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
			1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38
		};

		for (double powten : pof10)
		{
			float f = 1234.567f;
			//double powten = Jogo::intpow(10.0, i);
			f = (float)(f * powten);
			u32 len = str8::ftoa(f, ftoa_result, 32, p);
			str8 resultstr(ftoa_result, len);
			float result = resultstr.atof();

			sprintf_s(printfg, sizeof(printfg), "%.*g", p, f);
//			if (strcmp(ftoa_result, printfg))
//			{
//				printf("%s - %.*g - %g %.*g\n", ftoa_result, p, result, f, p, f);
//			}
		}
	}
	bool bTestExhaustiveHex = false;
	if (bTestExhaustiveHex)
	{
		for (u32 i = 0; i < 0xffffffff; i++)
		{
			char hexbuf[9];
			u32 len = str8::itohex(i, hexbuf, sizeof(hexbuf));
			str8 hexstr(hexbuf, len);
			u32 hex = hexstr.hextoi();
			if (i != hex)
			{
				printf("%s %d %d\n", hexbuf, i, hex);
			}
			if (i % 16777216 == 0)
			{
				printf(".");
			}
		}
	}

	bool bTestRandomFloats = false;
	if (bTestRandomFloats)
	{
		u32 count = 0;
		IntFloat intf;
		for (u32 i = 0; i < 100000000; i++)
		{
			intf.i = rand.GetNext();
			intf.i &= 0x7effffff;
			char jogoout[32];
			str8::ftoa(intf.f, jogoout, 32);
			char sprintfout[32];
			sprintf_s(sprintfout, sizeof(sprintfout), "%g", intf.f);

			//if (strcmp(jogoout, sprintfout))
			//{
			//	printf("%08x %s %s\n", intf.i, jogoout, sprintfout);
			//	count++;
			//}
		}
	}
	str8 s = str8::format("{    } {{}}", curves.FrameArena, 65, 100);

	s = str8::format("This is a test.  I am {    } years old and my name is {{}}.  22/7 = {}\n", curves.FrameArena, 57, 22.0f / 7.0f);
	printf("%*s\n", (int)s.len, s.chars);

	s = str8::format("number: {0:1X}", curves.FrameArena, -55);
	printf("%*s\n", (int)s.len, s.chars);
	s = str8::format("number: {0:2x}", curves.FrameArena, 15);
	printf("%*s\n", (int)s.len, s.chars);

	bool bTestLog = false;
	if (bTestLog)
	{
		for (s32 i = 0; i < 32; i++)
		{
			float l2 = Jogo::log2((float)(1 << i));
			s = str8::format("log of {} is {}", curves.FrameArena, 1 << i, l2);
			printf("%*s\n", (int)s.len, s.chars);
		}
	}
	bool bTestHundredRandom = false;
	if (bTestHundredRandom)
	{
		Jogo::Random R;
		for (u32 i = 0; i < 100; i++)
		{
			float x = R.GetNext() / 65536.0f;
			s = str8::format("{:5.5}", curves.FrameArena, x);
			printf("%*s\n", (int)s.len, s.chars);
			s = str8::format("{:10.4}", curves.FrameArena, x);
			printf("%*s\n", (int)s.len, s.chars);
			printf("%4.g\n", x);
		}
	}
	bool bTestPrecision = true;
	if (bTestPrecision)
	{
		float a = 99998765.0f;
		s = str8::format("{:.1}", curves.FrameArena, a);
		printf("%*s\n", (int)s.len, s.chars);
		printf("%.1f\n", a);
		s = str8::format("{:.2}", curves.FrameArena, a);
		printf("%*s\n", (int)s.len, s.chars);
		printf("%.2f\n", a);
		s = str8::format("{:.3}", curves.FrameArena, a);
		printf("%*s\n", (int)s.len, s.chars);
		printf("%.3f\n", a);
		s = str8::format("{:.4}", curves.FrameArena, a);
		printf("%*s\n", (int)s.len, s.chars);
		printf("%.4f\n", a);

		float n = 0.0003f;
		for (int i = 0; i < 20; i++)
		{
			//str8 t = str8::format("{{:.{}}}", curves.FrameArena, i);
 			s = str8::format("{:.2}", curves.FrameArena, n);
			printf("%*s\n", (int)s.len, s.chars);
			printf("%.2f\n", n);
			n *= 10;
		}
	}
	return 0;
}