#include "Jogo.h"
#include "str8.h"
#include "gfx.h"
#include <winrt/Windows.Devices.Sensors.h>

using namespace Jogo;

// define the vertices in digit space
Vector2 digit_points[8] =
{
	{0.0f, -0.5f},
	{0.0f, 0.0f},
	{0.0f, 0.5f},
	{0.5f, 0.5f},
	{0.5f, 0.0f},
	{0.5f, -0.5f}
};
// define shapes for digits 0-9
char digits[][6] =
{
	{0x02, 0x23, 0x35, 0x50, 0x00},
	{0x35, 0x00},
	{0x23, 0x34, 0x41, 0x10, 0x05, 0x00},
	{0x23, 0x35, 0x41, 0x50, 0x00},
	{0x21, 0x14, 0x35, 0x00},
	{0x32, 0x21, 0x14, 0x45, 0x50, 0x00},
	{0x32, 0x20, 0x05, 0x54, 0x41, 0x00},
	{0x23, 0x35, 0x00},
	{0x02, 0x23, 0x35, 0x50, 0x14, 0x00},
	{0x05, 0x53, 0x32, 0x21, 0x14, 0x00},
};

void DrawNumber(s32 n, const Vector2& Pos, const Vector2& Up, Bitmap& b, u32 color)
{
	// just take the lower two digits of n
	u32 an = abs(n) % 100;
	// use the length of Up to determine scale factor
	float scale = Up.Length();

	Vector2 CurrentPos = Pos;
	Vector2 Right = { -Up.y, Up.x };
	f32 dist = 2.0f;
	if (n < 0)
		dist += 1.0f;
	CurrentPos += dist * Right;

	for (u32 d = 0; d < 2; d++)
	{
		u32 dd = an % 10;
		u32 i = 0;
		while (digits[dd][i])
		{
			char pp = digits[dd][i];
			Vector2 pt1 = digit_points[(pp >> 4)&7];
			Vector2 pt2 = digit_points[pp & 7];

			Vector2 v1 = CurrentPos + (Right * pt1.x + Up * pt1.y);
			Vector2 v2 = CurrentPos + (Right * pt2.x + Up * pt2.y);
			b.DrawLine(v1.x, v1.y, v2.x, v2.y, color);
			i++;
		}
		an /= 10;
		CurrentPos -= Right * 1.1f;
	}
	// draw a minus sign
	//if (n < 0)
	//{
	//	b.DrawLine(CurrentPos.x, CurrentPos.y, CurrentPos.x + Right.x * 0.5f, CurrentPos.y + Right.y, color);
	//}
}

class Horizon : public Jogo::App
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
	Bitmap Texture;
	Timer fps;
	Timer frametime;
	double framespersecond = 0;
	float frameDelta = 0;
	Mesh Solids[6];
	Matrix4 SolidTransforms[6];
	Camera MainCamera;
	s32 MouseX;
	s32 MouseY;

	// parameters to convert to and from Screen and Pitch space
	const s32 HorizonWidth = 500;
	const s32 HorizonHeight = 500;
	const float ScreenToPitchScale = 60.0f / HorizonHeight;
	const float PitchToScreenScale = HorizonHeight / 60.f;
	float PitchOriginScreenSpaceX = 0.f;
	float PitchOriginScreenSpaceY = 0.f;
	float triangleTheta = 20.0f * D2R;

	float fusedPitch = 0.0f;
	float fusedRoll = 0.0f;


public:
	Horizon()
	{
		HorizonArena = Arena::Create(DefaultArenaSize);

		// throw this string away immediately after use...
		Arena scratch = DefaultArena.GetScratchArena(4096);
		str8 cwd = Jogo::CWD(scratch);
		Jogo::Print(cwd);

		AtariFont = Font::Load("../Jogo/Atari8.fnt", HorizonArena);
		F = Bitmap::Create(8, 8, 1, HorizonArena);
		F.Erase(0);
		F.PasteBitmapSelection(0, 0, AtariFont.FontBitmap, { 48, 8, 8, 8 }, 0);
		Texture = Bitmap::Load("checker.bmp", HorizonArena);
		Solids[0] = CreateCube();
		Solids[1] = CreateTetra();
		Solids[2] = CreateOcta();
		Solids[3] = CreateIcosa();
		Solids[4] = CreateDodeca();
		Solids[5] = CreateSphere(16, 16, DefaultArena);
		for (u32 i = 0; i < 6; i++)
		{
			SolidTransforms[i] = Matrix4::Identity();
			SolidTransforms[i].Translate({ (i % 3) * 4.0f - 4.0f, (i / 3) * -4.0f + 4.0f, 6.0f });
		}
		*(Matrix4*)&MainCamera = Matrix4::Identity();
		MainCamera.Translate({ 0.0f, 0.0f, -8.0f });
		MainCamera.SetProjection(53.0f, Width, Height, 1.0f, 50.f);
		fps.Start();
	}

	const char* GetName() const override { return Name; }

	void UpdateSensors(float dt) {
		using namespace winrt::Windows::Devices::Sensors;
		//OrientationSensor sensor = OrientationSensor::GetDefault();
		//if (sensor)
		//{
		//	auto reading = sensor.GetCurrentReading();
		//	auto q = reading.Quaternion(); // Get the 4D rotation
		//	// Convert Quaternion to Euler Pitch/Roll

		//	fusedPitch = atan2(2 * (q.W() * q.X() + q.Y() * q.Z()), 1 - 2 * (q.X() * q.X() + q.Y() * q.Y())) * (180.0f / PI);
		//	fusedRoll = asin(2 * (q.W() * q.Y() - q.Z() * q.X())) * (180.0f / PI);
		//}


		static Gyrometer gyro = Gyrometer::GetDefault();
		static Accelerometer accel = Accelerometer::GetDefault();

		if (gyro && accel) {
			auto gReading = gyro.GetCurrentReading();
			auto aReading = accel.GetCurrentReading();

			float accelX = aReading.AccelerationX();
			float accelY = aReading.AccelerationY();
			float accelZ = aReading.AccelerationZ();
			float gyroX = gReading.AngularVelocityX();
			float gyroY = gReading.AngularVelocityY();
			float gyroZ = gReading.AngularVelocityZ();

			float force = Jogo::sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
			if (force < 0.1)
				return;
			accelX /= force;
			accelY /= force;
			accelZ /= force;

			// 1. Calculate Pitch/Roll from Accelerometer (Static Gravity)
			// atan2 helps keep these values stable
			float accRoll = atan2(accelX, accelY) * 180.0f / PI;
			float accPitch = atan2(accelY, accelZ) * 180.0f / PI;

			// 2. Integrate Gyroscope Data (Angular Velocity)
			// Gyro measures degrees per second, so multiply by delta time

			gyroZ = gyroZ < 0.05 ? 0.0 : gyroZ;
			gyroX = gyroX < 0.05 ? 0.0 : gyroX;
			// 3. The Complementary Filter
			float alpha = 0.95f;
			fusedRoll = alpha * (fusedRoll + gyroZ * dt) + (1.0f - alpha) * accRoll;
			fusedPitch = alpha * (fusedPitch + gyroX * dt) + (1.0f - alpha) * accPitch;
		}
	}

	bool Tick(float DT /* do we need anything else passed in here?*/) override
	{
		Input::GetMousePos(MouseX, MouseY);

		frameDelta = DT;
		double s = fps.GetSecondsSinceLast();
		framespersecond = 1.0f / s;

//		UpdateSensors(DT);

		if (Input::IsKeyPressed(Input::KEY_RIGHT))
		{
			roll += 30.0f * DT;
		}
		if (Input::IsKeyPressed(Input::KEY_LEFT))
		{
			roll -= 30.0f * DT;
		}
//		roll = fusedRoll;

		float localupx = sine(roll * D2R);
		float localupy = cosine(roll * D2R);
		static float timer = 0;
		timer += DT;

		if (Input::IsKeyPressed(Input::KEY_UP))
		{
			pitch += 20.0f * localupy * DT;
		}
		if (Input::IsKeyPressed(Input::KEY_DOWN))
		{
			pitch -= 20.0f * localupy * DT;
		}
//		pitch = fusedPitch;
		return Done;
	}

	bool KeyDown(Input::Keys key) override
	{
		if (key == Input::KEY_ESC)
		{
			Done = true;
		}

		return true;
	}

	bool MouseDown(s32 x, s32 y, Input::Keys buttons) override
	{
		dragx = x;
		dragy = y;
		dragging = true;

		return true;
	}

	bool MouseUp(s32 x, s32 y, Input::Keys buttons) override
	{
		originx += (float)deltax;
		originy += (float)deltay;
		deltax = 0;
		deltay = 0;
		dragging = false;

		return true;
	}

	bool MouseMove(s32 x, s32 y) override
	{
		if (dragging)
		{
			deltax = x - dragx;
			deltay = y - dragy;
		}

		return true;
	}

	bool MouseWheel(s32 scroll) override
	{
		this->scroll = scroll;
		scale *= 1.0f + (float)scroll / 20;

		return true;
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
		
		s32 q;
		float p;
		remainder(pitch + 180.0f, 360.0f, 1.0f / 360.0f, q, p);
		if (p < 0)
		{
			p += 360.0f;
		}
		p -= 180.0f;

		float drawPitch = p;
		float drawRoll = roll;

		if (p > 90.0f)
		{
			drawPitch = 180.0f - p;
			drawRoll += 180.0f;
		}
		else if (p < -90.0f)
		{
			drawPitch = -180.0 - p;
			drawRoll += 180.0f;
		}

		float c = cosine(-drawRoll * D2R);
		float s = sine(-drawRoll * D2R);
		float upx = s;
		float upy = -c;
		PitchOriginScreenSpaceX = cx + drawPitch * PitchToScreenScale * upx;
		PitchOriginScreenSpaceY = cy + drawPitch * PitchToScreenScale * upy;

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
			// TODO: compute where sky turns to ground etc.
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
		}

		AtariFont.DrawText(cx, cy - frame.h / 2 - 10, str8::format(FrameArena, "{:0.2} {:0.2}", pitch, roll), 0, 0xffffffff, BackBuffer);

		// draw degree hashes
		// TODO: draw these from pitch,roll coord space and transform instead of this
		float pp;
		remainder(drawPitch, 10.0f, 0.1f, q, pp);
		pp -= 40.f;
		for (float h=0.f; h <= 80.0f; h += 10.0f)
		{
			if (abs(pp + h - drawPitch) < 1.0f)
				continue;
			float t = (pp + h) * PitchToScreenScale;
			float ppx = cx + t * upx;
			float ppy = cy + t * upy;

			// find the endpoints of the horizon line in screen space
			s32 leftx = (s32)(ppx - 0.25f * r * c);
			s32 lefty = (s32)(ppy - 0.25f * r * s);
			s32 rightx = (s32)(ppx + 0.25f * r * c);
			s32 righty = (s32)(ppy + 0.25f * r * s);


			BackBuffer.DrawLine(leftx, lefty, rightx, righty, 0xffffff);
			
			Vector2 ScaledUp = { 10 * upx, 10 * upy };
			// now draw the corresponding Pitch at this hash mark
			DrawNumber(pp + h - drawPitch, { (f32)rightx, (f32)righty }, ScaledUp, BackBuffer, 0xffffffff);
		}

		BackBuffer.DrawCircle((s32)cx, (s32)cy, 10, 0xffffff);

		MeshVertex worldTriangle[] =
		{
			{{-2,2,-2},{0,0,-1},0,0},
			{{0,2,3}, {0,0,-1},1,0},
			{ { -2,0,0 },{0,0,-1},0,1 },
		};

		Bitmap::VertexTexLit triangle[3];
		//{
		//	{100.0f, 100.0f, 0xff0000},
		//	{400.0f, 100.0f, 0xff00},
		//	{100.0f, 600, 0xff}
		//};
		Matrix4 View = MainCamera.GetInverse();
		for (u32 i = 0; i < 3; i++)
		{
			Vector3 pos = worldTriangle[i].Pos * View;
			//Vector3 norm = worldTriangle[i].Normal * View;
			*(Vector4*)&triangle[i] = MainCamera.Project(pos);
			triangle[i].c = 0xffffff;
			triangle[i].u = worldTriangle[i].u * triangle[i].w;
			triangle[i].v = worldTriangle[i].v * triangle[i].w;
		}
		triangleTheta += D2R * 0.5f;
		//if (triangleTheta > 20.0f * D2R)
		//	triangleTheta = -20.0f * D2R;
		static float xbump = 0.f;
		xbump += 0.1f;
		cx = 180.0f;
		cy = 240.0f;
		//float co = cosine(triangleTheta);
		//float si = sine(triangleTheta);
		//for (int i = 0; i < 3; i++)
		//{
		//	float x1 = (triangle[i].x - cx);
		//	float y1 = (triangle[i].y - cy);
		//	float x = co * x1 - si * y1;
		//	float y = si * x1 + co * y1;
		//	triangle[i].x = cx + x;
		//	triangle[i].y = cy + y;
		//}

//		for (s32 t = 0; t < 100; t++)
		//{
		//	//			BackBuffer.FillTriangle(triangle);
		//	if (!Input::IsKeyPressed(' '))
		//		BackBuffer.FillTriangleTexLit(triangle[0], triangle[1], triangle[2], Texture);
		//	else
		//	{
		//		BackBuffer.FillTriangle(triangle[0], triangle[1], triangle[2], Texture);
		//	}
		//}
		//BackBuffer.DrawCircle((s32)triangle[0].x, (s32)triangle[0].y, 5, 0);
		//BackBuffer.DrawCircle((s32)triangle[1].x, (s32)triangle[1].y, 5, 0);
		//BackBuffer.DrawCircle((s32)triangle[2].x, (s32)triangle[2].y, 5, 0);

//#define TEST_BARYCENTRIC
#ifdef TEST_BARYCENTRIC
		float x0 = triangle[0].x;
		float x1 = triangle[1].x;
		float x2 = triangle[2].x;
		float y0 = triangle[0].y;
		float y1 = triangle[1].y;
		float y2 = triangle[2].y;

		float w01 = 1.0f / (triangle[0].w * triangle[1].w);
		float w02 = 1.9f / (triangle[0].w * triangle[2].w);
		float w12 = 1.0f / (triangle[1].w * triangle[2].w);

		float minx = Jogo::max(min3(x0, x1, x2), 0.0f);
		float miny = Jogo::max(min3(y0, y1, y2), 0.0f);
		float maxx = min(max3(x0, x1, x2), (float)Width);
		float maxy = min(max3(y0, y1, y2), (float)Height);
		if (minx < maxx && miny < maxy)
		{
			// edge vectors
			float dx10 = x1 - x0; float dy10 = y1 - y0;
			float dx21 = x2 - x1; float dy21 = y2 - y1;
			float dx02 = x0 - x2; float dy02 = y0 - y2;

			dx10 *= w01;
			dx21 *= w12;
			dx02 *= w02;
			dy10 *= w01;
			dy21 *= w12;
			dy02 *= w02;

			// edge functions
			float e0 = (dx21 * (miny - y1) - (minx - x1) * dy21);
			float e1 = (dx02 * (miny - y2) - (minx - x2) * dy02);
			float e2 = (dx10 * (miny - y0) - (minx - x0) * dy10);

			float px = MouseX - triangle[0].x;
			float py = MouseY - triangle[0].y;

			float w = (e1 - px * dy02 + py * dx02);
			float u = (e2 - px * dy10 + py * dx10);
			float v = (e0 - px * dy21 + py * dx21);

			float uvw = 1.0f / ( u + v + w );
			float uw = v * uvw;
			float vw = u * uvw;

			AtariFont.DrawText(MouseX - 60, MouseY - 20, str8::format(FrameArena, "{:0.3} {:0.3} {:0.3}", uw, vw, 1 - uw - vw), 0, 0xffffffff, BackBuffer);
		}
#endif TEST_BARYCENTRIC

//		Bitmap::VertexLit triangle[] =
//		{
//			{180.0f, 180.0f, 0xff0000},
//			{340.0f, 180.0f, 0xff00},
//			{180.0f, 240, 0xff}
//		};
//		
//		triangleTheta += D2R * 0.05f;
//		//if (triangleTheta > 20.0f * D2R)
//		//	triangleTheta = -20.0f * D2R;
//		static float xbump = 0.f;
//		xbump += 0.1f;
//		cx = 180.0f;
//		cy = 240.0f;
//		float co = cosine(triangleTheta);
//		float si = sine(triangleTheta);
//		for (int i = 0; i < 3; i++)
//		{
//			float x1 = (triangle[i].x - cx);
//			float y1 = (triangle[i].y - cy);
//			float x = co * x1 - si * y1;
//			float y = si * x1 + co * y1;
//			triangle[i].x = cx + x;
//			triangle[i].y = cy + y;
//		}
//		
////		for (s32 t = 0; t < 100; t++)
//		{
////			BackBuffer.FillTriangle(triangle);
//			BackBuffer.FillTriangle(triangle);
//		}

		//for (u32 i = 0; i < 6; i++)
		//{
		//	RenderMesh(Solids[i], SolidTransforms[i], MainCamera, BackBuffer, Texture, FrameArena, !Input::IsKeyPressed(' '));
		//	SolidTransforms[i].RotateY(frameDelta);
		//}
	}

	void DrawSineWave()
	{
		s32 ox = BackBuffer.Width / 2;
		s32 oy = BackBuffer.Height / 2;
		int x1=0, y1=oy;
		for (s32 x = 0; x < (s32)BackBuffer.Width; x++)
		{
			
			float a = (x/scale - ox) * D2R - (originx + deltax)/100.f;
			float y = 100 * scale * (cosine(a)) - originy - deltay;
			int y2 = oy - (int)y;
			int x2 = x;
			BackBuffer.DrawLine(x1, y1, x2, y2, 0xffffff);
			x1 = x2;
			y1 = y2;
		}
	}

	void Draw() override
	{
		frametime.Start();

		BackBuffer.Erase(0xffffffff);

		Bitmap::Rect horizonBox = { 250,250,500,500 };
		DrawHorizon(horizonBox, pitch, roll);

		AtariFont.DrawText(0, 20, str8::format(FrameArena, "{:}", frameDelta), 0, 0, BackBuffer);
		AtariFont.DrawText(0, 0, str8::format(FrameArena, "{:}", (float)framespersecond), 0, 0, BackBuffer);
		//		AtariFont.DrawText(0, 0, "Hello", 0, 0, BackBuffer);

		double frametimeseconds = frametime.GetSecondsSinceLast();
		AtariFont.DrawText(0, 40, str8::format(FrameArena, "{:}", (float)frametimeseconds), 0, 0, BackBuffer);

		Show(BackBuffer.PixelBGRA, BackBuffer.Width, BackBuffer.Height);
		FrameArena.Clear();
	}
};

const char* Horizon::Name = "Horizon";

int main(int argc, char *argv[])
{
	Horizon horizon;
	Run(horizon, 1000);
	return 0;
}
