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

#ifdef max
#undef max
#endif

	template<typename T>
	T max(T x, T y)
	{
		return x >= y ? x : y;
	}

	inline float isdigit(char n)
	{
		return n >= '0' && n <= '9';
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

	u32 itoa(s32 number, char* string, u32 maxstring);
	s32 atoi(const char* string);
	u32 itohex(u32 number, char* string, u32 maxstring, u32 maxdigits = 8);
	u32 hextoi(const char* input);
	double intpow(float number, s32 power);
	float floor(float n);
	float ceil(float n);
	bool isinfinite(float f);
	bool isnan(float f);
	u32 ftoa(f32 number, char* string, u32 maxstring, u32 precision = 6);
	float atof(const char* string);
	float sqrt(float x);
	void remainder(float num, float denom, float invdenom, int& quotient, float& remainder);
	float sine(float x);
	float cosine(float x);
};