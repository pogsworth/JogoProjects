#pragma once
#include "int_types.h"
#include <intrin.h>

namespace Jogo
{
	template<typename T>
	T abs(T x)
	{
		return x >= 0 ? x : -x;
	}

#ifdef min
#undef min
#endif

	template<typename T>
	T min(T x, T y)
	{
		return x <= y ? x : y;
	}

	template<typename T>
	T min3(T a, T b, T c)
	{
		return a <= b ? a <= c ? a : c : b <= c ? b : c;
	}

	template<typename T>
	T max3(T a, T b, T c)
	{
		return a >= b ? a >= c ? a : c : b >= c ? b : c;
	}

	template<typename T>
	T clamp(T x, T a, T b)
	{
		return (x < a ? a : x > b ? b : x);
	}

#ifdef max
#undef max
#endif

	template<typename T>
	T max(T x, T y)
	{
		return x >= y ? x : y;
	}

	inline s32 double2intround(double x)
	{
		__m128d m = _mm_set_sd(x);
		return _mm_cvtsd_si32(m);
	}

	inline s32 double2inttrunc(double x)
	{
		__m128d m = _mm_set_sd(x);
		return _mm_cvttsd_si32(m);
	}

	inline s32 float2intround(float x)
	{
		__m128 m = _mm_set_ss(x);
		return _mm_cvt_ss2si(m);
	}

	inline s32 float2inttrunc(float x)
	{
		__m128 m = _mm_set_ss(x);
		return _mm_cvtt_ss2si(m);
	}

	const float PI = 3.14159265358979323f;
	const float PIOVER2 = PI / 2.0f;
	const float PIOVER4 = PI / 4.0f;
	const float TWOOVERPI = 2.0f / PI;
	const float R2D = 180.0f / PI;
	const float D2R = PI / 180.0f;

	double intpow(float number, s32 power);
	float floor(float n);
	float ceil(float n);
	bool isinfinite(float f);
	bool isnan(float f);
	float sqrt(float x);
	void remainder(float num, float denom, float invdenom, int& quotient, float& remainder);
	float sine(float x);
	float cosine(float x);
	float tangent(float x);
	float arctangent(float x);
	float arctangent(float x, float y);
	float log2(float x);
	float log(float x);
	float exp(float x);

	struct Vector3
	{
		float x, y, z;

		static float Dot(const Vector3& a, const Vector3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		static Vector3 Cross(const Vector3& a, const Vector3& b)
		{
			return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
		}

		void operator+=(const Vector3& v)
		{
			x += v.x; y += v.y; z += v.z;
		}
	};

	inline Vector3 operator-(const Vector3& a, const Vector3& b)
	{
		return { a.x + b.x, a.y + b.y, a.z + b.z };
	}

	inline Vector3 operator+(const Vector3& a, const Vector3& b)
	{
		return{ a.x + b.x,a.y + b.y,a.z + b.z };
	}

	inline Vector3 operator-(const Vector3 v)
	{
		return { -v.x, -v.y, -v.z };
	}

	struct Vector4
	{
		float x, y, z, w;
	};

	struct Quat
	{
		float x, y, z, w;
	};

	struct Matrix3
	{
		Vector3 rows[3];

		static Matrix3 Identity()
		{
			return	{
						{
							{1.0f, 0.0f, 0.0f},
							{0.0f, 1.0f, 0.0f},
							{0.0f, 0.0f, 1.0f}
						}
					};
		}

		void RotateX(float radians)
		{
			float c = cosine(radians);
			float s = sine(radians);

			for (int i = 0; i < 3; i++)
			{
				float t = rows[i].y;
				rows[i].y = c * rows[i].y - s * rows[i].z;
				rows[i].z = s * t         + c * rows[i].z;
			}
		}

		void RotateY(float radians)
		{
			float c = cosine(radians);
			float s = sine(radians);

			for (int i = 0; i < 3; i++)
			{
				float t = rows[i].x;
				rows[i].x = c * rows[i].x + s * rows[i].z;
				rows[i].z = -s * t        + c * rows[i].z;
			}
		}

		void RotateZ(float radians)
		{
			float c = cosine(radians);
			float s = sine(radians);

			for (int i = 0; i < 3; i++)
			{
				float t = rows[i].x;
				rows[i].x = c * rows[i].x - s * rows[i].y;
				rows[i].y = s * t         + c * rows[i].y;
			}
		}

		void Inverse()
		{
			float t;

			// transpose the rotation: swap the off diagonal elements
			t = rows[0].y;
			rows[0].y = rows[1].x;
			rows[1].x = t;

			t = rows[0].z;
			rows[0].z = rows[2].x;
			rows[2].x = t;

			t = rows[1].z;
			rows[1].z = rows[2].y;
			rows[2].y = t;
		}
	};

	struct Matrix4 : public Matrix3
	{
		Vector3 translate;
		static Matrix4 Identity()
		{
			return	{{{
						{1.0f, 0.0f, 0.0f},
						{0.0f, 1.0f, 0.0f},
						{0.0f, 0.0f, 1.0f}}},
						{0.0f, 0.0f, 0.0f} 
					};
		}

		void Translate(Vector3 v)
		{
			translate += v;
		}

	};

	inline Vector3 operator*(Vector3 v, Matrix4& m)
	{
		return{ v.x * m.rows[0].x + v.y * m.rows[1].x + v.z * m.rows[2].x + m.translate.x,
				v.y * m.rows[0].y + v.y * m.rows[1].y + v.z * m.rows[2].y + m.translate.y,
				v.z * m.rows[0].z + v.z * m.rows[1].z + v.z * m.rows[2].z + m.translate.z };
	}

	struct Plane
	{
		Vector3 Normal;
		float Distance;

		u64 ClipPoly(u64 numVerts, Vector3* pIn, Vector3* pOut);
	};


	struct Frustum
	{
		Plane planes[6];	// front, back, left, right, top, bottom

		u32 clip_code(Vector3 pt) {
			u32 code = 0;

			for (u32 i = 0; i < 6; i++)
			{
				code <<= 1;
				code |= Vector3::Dot(pt, planes[i].Normal) < planes[i].Distance;
			}
			return code;
		}
		u64 ClipPoly(u32 numVerts, Vector3* pIn, Vector3* pOut);
	};

	struct Camera
	{
		Matrix4 View;

		float FOV;

		Frustum GetFrustum();
	};
};