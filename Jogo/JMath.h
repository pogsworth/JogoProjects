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

	template<typename T>
	void swap(T& a, T& b)
	{
		T t = a;
		a = b;
		b = t;
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

		float Length()
		{
			return sqrt(x * x + y * y + z * z);
		}

		float Normalize()
		{
			float L = Length();
			if (L > 0.00001f)
			{
				x /= L; y /= L; z /= L;
			}
			return L;
		}

		void operator+=(const Vector3& v)
		{
			x += v.x; y += v.y; z += v.z;
		}

		void operator-=(const Vector3& v)
		{
			x -= v.x; y -= v.y; z -= v.z;
		}
	};

	inline Vector3 operator-(const Vector3& a, const Vector3& b)
	{
		return { a.x - b.x, a.y - b.y, a.z - b.z };
	}

	inline Vector3 operator+(const Vector3& a, const Vector3& b)
	{
		return{ a.x + b.x,a.y + b.y,a.z + b.z };
	}

	inline Vector3 operator-(const Vector3 v)
	{
		return { -v.x, -v.y, -v.z };
	}

	inline Vector3 operator*(const float s, const Vector3 v)
	{
		return { s * v.x, s * v.y, s * v.z };
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

			// hacky version:
			//for (u32 i = 0; i < 3; i++)
			//{
			//	u32 j = (i + 1) % 3;
			//	t = (&rows[i].x)[j];
			//	(&rows[i].x)[j] = (&rows[j].x)[i];
			//	(&rows[j].x)[i] = t;
			//}

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

		void Normalize()
		{
			rows[0].Normalize();

			// subtract proj(Y,X) from Y, then normalize
			float DotXY = Vector3::Dot(rows[0], rows[1]);
			rows[1] -= DotXY * rows[0];
			rows[1].Normalize();

			// subtract proj(Z,X) from Z and proj(Z,Y) from Z, then normalize
			float DotXZ = Vector3::Dot(rows[0], rows[2]);
			rows[2] -= DotXZ * rows[0];
			float DotYZ = Vector3::Dot(rows[1], rows[2]);
			rows[2] -= DotYZ * rows[1];

			rows[2].Normalize();
		}
	};

	inline Vector3 operator*(Vector3 v, Matrix3& m)
	{
		return v.x * m.rows[0] + v.y * m.rows[1] + v.z * m.rows[2];
	}

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

		void Inverse()
		{
			Matrix3::Inverse();
			// now invert the translation
			translate = -translate.x * rows[0] - translate.y * rows[1] - translate.z * rows[2];
		}

		Matrix4 GetInverse()
		{
			Matrix4 Inverse = *this;
			Inverse.Inverse();
			return Inverse;
		}
	};

	inline Matrix4 operator*(Matrix4& m1, Matrix4& m2)
	{
		Matrix4 result;
		for (u32 i = 0; i < 3; i++)
		{
			result.rows[i] = m1.rows[i].x * m2.rows[0] + m1.rows[i].y * m2.rows[1] + m1.rows[i].z * m2.rows[2];
		}
		result.translate = m1.translate.x * m2.rows[0] + m1.translate.y * m2.rows[1] + m1.translate.z * m2.rows[2] + m2.translate;

		return result;
	}

	inline Vector3 operator*(Vector3 v, Matrix4& m)
	{
		return v.x * m.rows[0] + v.y * m.rows[1] + v.z * m.rows[2] + m.translate;
	}

	struct Plane
	{
		Vector3 Normal;
		float Distance;

		void Normalize()
		{
			float L = Normal.Normalize();
			if (L > 0.00001f)
				Distance /= L;
		}

		u64 ClipPoly(u64 numVerts, Vector3* pIn, Vector3* pOut);
	};
};