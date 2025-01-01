#include "Jogo.h"

class Horizon : public Jogo::JogoApp
{
	static const char* Name;
	bool Done = false;
	float pitch = 0.0f;
	float roll = 0.0f;
	bool dragging = false;
	s32 dragx = 0;
	s32 dragy = 0;
	s32 deltax = 0;
	s32 deltay = 0;
	float originx = 0;
	float originy = 0;
	s32 scroll = 0;
	float scale = 1.0;
	u32 SkyColor = 0x0080A0;
	u32 GroundColor = 0x806000;
	Font AtariFont;
	Arena HorizonArena;
	Bitmap F;

	// parameters to convert to and from Screen and Pitch space
	const s32 HorizonWidth = 500;
	const s32 HorizonHeight = 500;
	const float ScreenToPitchScale = 60.0f / HorizonHeight;
	const float PitchToScreenScale = HorizonHeight / 60.f;
	float PitchOriginScreenSpaceX = 0.f;
	float PitchOriginScreenSpaceY = 0.f;
	float triangleTheta = 0.f;

public:
	Horizon() 
	{
		HorizonArena = Arena::Create(DefaultArenaSize);
		AtariFont = Font::Load("../Jogo/Atari8.fnt", HorizonArena);
		F = Bitmap::Create(8, 8, 1, HorizonArena);
		F.Erase(0xffffff);
		F.PasteBitmapSelection(0, 0, AtariFont.FontBitmap, { 48, 8, 8, 8 }, 0);
	}

	const char* GetName() const override { return Name; }


	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		if (Jogo::IsKeyPressed(KEY_RIGHT))
		{
			roll += 30.0f * DT;
		}
		if (Jogo::IsKeyPressed(KEY_LEFT))
		{
			roll -= 30.0f * DT;
		}
		float localupx = Jogo::sine(roll);
		float localupy = Jogo::cosine(roll);
		static float timer = 0;
		timer += DT;
		char timerString[32];
		Jogo::ftoa(timer, timerString, sizeof(timerString));
		char* p = timerString;
		for (s32 i = 0; i < 32; i++, p++)
		{
			if (!*p)
			{
				*p++ = '\n';
				*p = 0;
				break;
			}
		}
		Jogo::DebugOut(timerString);
		if (Jogo::IsKeyPressed(KEY_UP))
		{
			pitch += 20.0f * DT;
		}
		if (Jogo::IsKeyPressed(KEY_DOWN))
		{
			pitch -= 20.0f * DT;
		}
		return Done;
	}

	void KeyDown(u32 key)
	{
		if (key == KEY_ESC)
		{
			Done = true;
		}
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

	template<class T>
	T Abs(T input)
	{
		return input >= 0 ? input : -input;
	}

	void VectorToPitchOriginScreenSpace(float& x, float& y)
	{
		x = x - PitchOriginScreenSpaceX;
		y = y - PitchOriginScreenSpaceY;
	}

	void DrawHorizon(Bitmap::Rect& frame, float pitch, float roll)
	{
		float cx = frame.x + frame.w / 2.0f;
		float cy = frame.y + frame.h / 2.0f;
		float r = 1.5f*frame.w / 2;
		float c = Jogo::cosine(-roll * Jogo::D2R);
		float s = Jogo::sine(-roll * Jogo::D2R);
		
		s32 q;
		float p;
		Jogo::remainder(-pitch, 180.0f, 1.0f / 180.0f, q, p);

		float upx = s;
		float upy = -c;
		// are we upside-down?
		//if (q & 1)
		//{
		//	upx = -s;
		//	upy = c;
		//}

		// find x,y where pitch is 0
		//if (q & 1)
		//{
		//	PitchOriginScreenSpaceX = cx + p * PitchToScreenScale * upx;
		//	PitchOriginScreenSpaceY = cy - p * PitchToScreenScale * upy;
		//}
		//else
		{
			PitchOriginScreenSpaceX = cx + p * PitchToScreenScale * upx;
			PitchOriginScreenSpaceY = cy + p * PitchToScreenScale * upy;
		}

		// are we upside-down?
		if (q & 1)
		{
			upx = -s;
			upy = c;
		}

		// find the endpoints of the horizon line in screen space
		s32 x1 = (s32)(PitchOriginScreenSpaceX - r * c);
		s32 y1 = (s32)(PitchOriginScreenSpaceY - r * s);
		s32 x2 = (s32)(PitchOriginScreenSpaceX + r * c);
		s32 y2 = (s32)(PitchOriginScreenSpaceY + r * s);

		// for each horizontal line of the display
		// determine where the line segment intersects the frame
		// then go from left side to the right with one color, then the other

		// determine the first line of sky, which would be 
		u32 color = SkyColor;
		for (s32 screeny = frame.y; screeny < frame.y + frame.h; screeny++)
		{
			// is this line sky or ground?
			float testx = (float)frame.x;
			float testy = (float)screeny;
			VectorToPitchOriginScreenSpace(testx, testy);

			float LeftDot = testx * upx + testy * upy;
			color = LeftDot > 0 ? SkyColor : GroundColor;
			// TODO: coompute where sky turns to ground etc.
			testx = (float)(frame.x + frame.w);
			testy = (float)screeny;
			VectorToPitchOriginScreenSpace(testx, testy);
			float RightDot = testx * upx + testy * upy;
			s32 RightEdge = frame.x + frame.w;
			if ((RightDot >= 0 && LeftDot < 0) || (RightDot < 0 && LeftDot >= 0))
			{
				// compute the intersection of the horizon line with current scanline
				RightEdge = frame.x + (s32)(LeftDot * frame.w / (LeftDot - RightDot));
			}
			BackBuffer.DrawHLine(screeny, frame.x, RightEdge, color);
			color = color == SkyColor ? GroundColor: SkyColor;
			BackBuffer.DrawHLine(screeny, RightEdge, frame.x + frame.w, color);
		}
		// draw the horizon line

		if (BackBuffer.ClipLine(x1, y1, x2, y2, frame))
		{
			BackBuffer.DrawLine(x1, y1, x2, y2, 0xffffff);

			//s32 pox = PitchOriginScreenSpaceX;
			//s32 poy = PitchOriginScreenSpaceY;
		}

		BackBuffer.DrawCircle((s32)cx, (s32)cy, 10, 0xffffff);


		Bitmap::Vertex triangle[] =
		{
			{180.0f, 180.0f, 0xff0000},
			{240.0f, 200.0f, 0x00ff00},
			{180.0f, 240, 0x0000ff}
		};
		
		triangleTheta += Jogo::D2R * 15.5f;
		static float xbump = 0.f;
		xbump += 0.1f;
		cx = 200.0f;
		cy = 200.0f;
		float co = Jogo::cosine(triangleTheta);
		float si = Jogo::sine(triangleTheta);
		for (int i = 0; i < 3; i++)
		{
			float x1 = triangle[i].x - cx;
			float y1 = triangle[i].y - cy;
			float x = co * x1 - si * y1;
			float y = si * x1 + co * y1;
			triangle[i].x = cx + x;
			triangle[i].y = cy + y;
		}
		
		for (s32 t = 0; t < 100; t++)
		{
			BackBuffer.FillTriangle(triangle);
		}
		x1 = (s32)cx;	// PitchOriginScreenSpaceX;
		y1 = (s32)cy;	// PitchOriginScreenSpaceY;
		x2 = x1 + (s32)(upx * 30);
		y2 = y1 + (s32)(upy * 30);
		BackBuffer.DrawLine(x1, y1, x2, y2, 0xffff);
		//BackBuffer.DrawLine(triangle[0].x, triangle[0].y, triangle[1].x, triangle[1].y, 0);
		//BackBuffer.DrawLine(triangle[1].x, triangle[1].y, triangle[2].x, triangle[2].y, 0);
		//BackBuffer.DrawLine(triangle[2].x, triangle[2].y, triangle[0].x, triangle[0].y, 0);
		char pitchString[32];
		Jogo::itoa((int)pitch%360, pitchString, 32);
//		DefaultFont.DrawText(0, 0, pitchString, 0, BackBuffer);
	}

	void DrawSineWave()
	{
		s32 ox = BackBuffer.Width / 2;
		s32 oy = BackBuffer.Height / 2;
		int x1=0, y1=oy;
		for (s32 x = 0; x < (s32)BackBuffer.Width; x++)
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
		BackBuffer.Erase(0xffffff);

		Bitmap::Rect horizonBox = { 250,250,500,500 };
		DrawHorizon(horizonBox, pitch, roll);
		//DrawSineWave();
		//char scrollString[32];
		//Jogo::itoa(scroll, scrollString);
		//DefaultFont.DrawText(0,0,scrollString, 0xffffff, BackBuffer);
		AtariFont.DrawText(0, 0, "Hello", 0, BackBuffer);

		static float counter = 1234567;
		counter++;
		char counterText[32];
		Jogo::ftoa(counter, counterText, sizeof(counterText));
		static s32 offset = 0;
		static s32 dx = -1;
		offset += dx;
		if (offset < -100 || offset >= 100)
			dx = -dx;
//		AtariFont.DrawText(offset, 20, counterText, 0, BackBuffer);

		static float theta = 0.0f;
		static float radius = 400.0f;
		static float dtheta = 1.0f;
		static float dradius = 0.1f;
		if (radius < 10.f || radius > 150.f)
		{
			dradius = -dradius;
		}
		float x = radius * Jogo::cosine(theta * Jogo::D2R);
		float y = radius * Jogo::sine(theta * Jogo::D2R);
		float cx = 350.f;
		float cy = 300.f;
		BackBuffer.PasteBitmapSelectionScaled({ (s32)cx, (s32)cy, (s32)x, (s32)y }, F, { 0, 0, 64, 64 }, 0);
		radius += dradius;
		theta += dtheta;

		Jogo::Show(BackBuffer.PixelBGRA, BackBuffer.Width, BackBuffer.Height);
	}
};

const char* Horizon::Name = "Horizon";

int main(int argc, char *argv[])
{
	Horizon horizon;
	Jogo::Run(horizon, 60);
	return 0;
}
