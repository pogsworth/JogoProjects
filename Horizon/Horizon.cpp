#include "Jogo.h"
//#include "JMath.h"

class Horizon : public Jogo::JogoApp
{
	static const char* Name;
	bool Done = false;
	float pitch = 0.0f;
	float roll = 0.0f;
	bool dragging = false;
	s32 dragx, dragy;
	s32 deltax = 0;
	s32 deltay = 0;
	float originx = 0;
	float originy = 0;
	s32 scroll = 0;
	float scale = 1.0;

public:
	Horizon() {}

	const char* GetName() const override { return Name; }


	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		roll += DT;

		return Done;
	}

	void MouseDown(s32 x, s32 y, u32 buttons)
	{
		dragx = x;
		dragy = y;
		dragging = true;
	}

	void MouseUp(s32 x, s32 y, u32 buttons)
	{
		originx += (float)deltax;
		originy += (float)deltay;
		deltax = 0;
		deltay = 0;
		dragging = false;
	}

	void MouseMove(s32 x, s32 y, u32 buttons)
	{
		if (dragging)
		{
			deltax = x - dragx;
			deltay = y - dragy;
		}
	}

	void MouseWheel(s32 scroll)
	{
		this->scroll = scroll;
		scale *= 1.0f + (float)scroll / 20;
	}

	void DrawHorizon(Bitmap::Rect& frame, float pitch, float roll)
	{
		float x = frame.x + frame.w / 2;
		float y = frame.y + frame.h / 2;
		float r = frame.w / 2;
		float x1 = r * Jogo::cosine(roll);
		float y1 = r * Jogo::sine(roll);
		BackBuffer.DrawLine((int)(x + x1), (int)(y - y1), (int)(x - x1), (int)(y + y1), 0xffffff);
	}

	void DrawSineWave()
	{
		s32 ox = BackBuffer.Width / 2;
		s32 oy = BackBuffer.Height / 2;
		int x1=0, y1=oy;
		for (int x = 0; x < BackBuffer.Width; x++)
		{
			
			float a = (x/scale - ox) * Jogo::D2R - (originx + deltax)/100.f;
			float y = 100 * scale * (Jogo::cosine(a)) - originy - deltay;
			int y2 = oy - (int)y;
			int x2 = x;
			BackBuffer.DrawLine(x1, y1, x2, y2, 0xffffff);
			x1 = x2;
			y1 = y2;
		}
	}

	void Draw() override
	{
		BackBuffer.Erase(0);

		Bitmap::Rect horizonBox = { 250,250,500,500 };
		DrawHorizon(horizonBox, pitch, roll);
		DrawSineWave();
		char scrollString[32];
		Jogo::itoa(scroll, scrollString);
		DefaultFont.DrawText(0,0,scrollString, 0xffffff, BackBuffer);

		Jogo::Show(BackBuffer.PixelBGRA, BackBuffer.Width, BackBuffer.Height);
	}
};

const char* Horizon::Name = "Horizon";

int main(int argc, char* argv[])
{
	Horizon horizon;
	Jogo::Run(horizon, 60);
	return 0;
}
