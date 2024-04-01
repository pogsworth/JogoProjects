#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"
#include "Arena.h""

struct Point
{
	float x;
	float y;
};

class Curves : public Jogo::JogoApp
{
	static const char* Name;
	static const u32 MAX_POINTS = 100;
	Point Points[MAX_POINTS];
	bool Done = false;
	bool Dragging = false;
	Point Down;
	Point Up;
	u32 NumPoints;

public:

	const char* GetName() const override { return (char*)Name; }

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
			Down = { (float)x, (float)y };
			Dragging = true;
		}
	}

	virtual void MouseUp(s32 x, s32 y, u32 buttons) override
	{
		if (Dragging)
		{
			Dragging = false;
		}
	}

	virtual void MouseMove(s32 x, s32 y, u32 buttons) override
	{
		if (Dragging)
		{
			Up = { (float)x, (float)y };
		}
	}

	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		return Done;
	}

	void Draw() override
	{
		BackBuffer.Erase(0);
		DefaultFont.DrawText(0,0,"Curves", 0xffffff, BackBuffer);
		if (Dragging)
		{
			BackBuffer.DrawLine((s32)Down.x, (s32)Down.y, (s32)Up.x, (s32)Up.y, 0xffffff);
		}
		Jogo::Show(BackBuffer.PixelBGRA, Width, Height);
	}
};
const char* Curves::Name = "Curves";

int main(int argc, char* argv[])
{
	Curves curves;
	Jogo::Run(curves, 60);
	return 0;
}