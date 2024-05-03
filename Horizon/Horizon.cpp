#include "Jogo.h"

class Horizon : public Jogo::JogoApp
{
	static const char* Name;
	bool Done = false;
public:
	Horizon() {}

	const char* GetName() const override { return Name; }

	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		return Done;
	}

	void DrawHorizon(Bitmap::Rect& frame, float pitch, float roll)
	{
		float x = frame.x + frame.w / 2;
		float y = frame.y + frame.h / 2;
		BackBuffer.DrawLine()
	}

	void Draw() override
	{
		BackBuffer.Erase(0);

		Bitmap::Rect horizonBox = { 250,250,500,500 };
		DrawHorizon(horizonBox, 0);

		Jogo::Show(BackBuffer.PixelBGRA, Width, Height);
	}
};

const char* Horizon::Name = "Horizon";

int main(int argc, char* argv[])
{
	Horizon horizon;
	Jogo::Run(horizon, 60);
	return 0;
}
