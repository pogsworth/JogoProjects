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

	inline char* itoa(s32 number, char* string)
	{
		s32 n = abs(number);
		char* p = string;
		do
		{
			*p++ = n % 10 + '0';
			n /= 10;
		} while (n);
		if (number < 0)
			*p++ = '-';
		*p-- = 0;
		char* b = string;
		while (b < p)
		{
			char s = *b;
			*b++ = *p;
			*p-- = s;
		}
		return string;
	}

	inline s32 AsInteger(float f) { return *(s32*)&f; }
	inline float AsFloat(s32 i) { return *(float*)&i; }
	const s32 OneAsInteger = AsInteger(1.0f);
	inline float asqrt(float x)
	{
		float approx = AsFloat((AsInteger(x) >> 1) + (OneAsInteger >> 1));
		return (approx * approx + x) / (2.0f * approx);
	}

	inline float mmsqrt(float x)
	{
		__m128 m;
		m.m128_f32[0] = x;
		m = _mm_sqrt_ss(m);
		return m.m128_f32[0];
	}

	inline void remainder(float num, float denom, float invdenom, int& quotient, float& remainder)
	{
		float q = num * invdenom;
		__m128 m;
		m.m128_f32[0] = q;
		quotient = _mm_cvt_ss2si(m);
		remainder = num - quotient * denom;
	}

	const float PI = 3.14159265358979323f;
	const float PIOVER2 = PI / 2.0f;
	const float PIOVER4 = PI / 4.0f;
	const float TWOOVERPI = 2.0f / PI;
	const float R2D = 180.0f / PI;
	const float D2R = PI / 180.0f;

	// return sine of an angle between 0..pi/4
	inline float __sine(float x)
	{
		const float coef1 = 0.999994990f;
		const float coef3 = -0.166601570f;
		const float coef5 = 0.00812149339f;
		float xx = x * x;

		return x * (coef1 + xx * (coef3 + coef5 * xx));
	}

	// return sine of an angle between 0..pi/4
	inline float __cosine(float x)
	{
		const float coef2 = -0.499998566f;
		const float coef4 = 0.0416550209f;
		const float coef6 = -0.00135858439f;

		float xx = x * x;
		return 1 + xx * (coef2 + xx * (coef4 + xx * coef6));
	}

	inline float sine(float x)
	{
		// fold input into one octant and keep track of where we were
		int quadrant;
		float argument;
		// find out which quadrant we are in
		remainder(x, PIOVER2, TWOOVERPI, quadrant, argument);

		switch (quadrant & 3)
		{
		default:
		case 0: return __sine(argument);
		case 1: return __cosine(argument);
		case 2: return -__sine(argument);
		case 3: return -__cosine(argument);
		}
	}

	inline float cosine(float x)
	{
		// fold input 
		int quadrant;
		float argument;
		// find out which quadrant we are in
		remainder(x, PIOVER2, TWOOVERPI, quadrant, argument);

		switch (quadrant & 3)
		{
		default:
		case 0: return __cosine(argument);
		case 1: return -__sine(argument);
		case 2: return -__cosine(argument);
		case 3: return __sine(argument);
		}
	}

}