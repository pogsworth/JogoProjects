#pragma once
#include "int_types.h"
#include <intrin.h>
#include <math.h>

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

	inline s32 itoa(s32 number, char* string, u32 maxstring)
	{
		u32 n = abs(number);
		u32 numchars = 0;
		char* p = string;
		do
		{
			if (numchars < maxstring)
			{
				*p++ = n % 10 + '0';
				n /= 10;
				numchars++;
			}
			else
			{
				return numchars;
			}
		} while (n);

		if (number < 0 && numchars < maxstring)
		{
			*p++ = '-';
			numchars++;
		}
		if (numchars < maxstring)
		{
			*p-- = 0;
			numchars++;
		}
		char* b = string;
		while (b < p)
		{
			char s = *b;
			*b++ = *p;
			*p-- = s;
		}
		return numchars;
	}

	inline u32 itohex(u32 number, char* string, u32 maxstring, u32 maxdigits = 8)
	{
		if (maxstring < maxdigits + 1)
		{
			return 0;
		}
		u32 n = number;
		char* p = string + maxdigits;
		*p-- = 0;

		for (u32 hexdigits = 0; hexdigits < maxdigits; hexdigits++)
		{
			char hex = (n & 0xf) + '0';
			if (hex > '9')
				hex += 'A' - '9' - 1;
			*p-- = hex;
			n >>= 4;
		}
		return maxdigits + 1;
	}

	inline u32 hextoi(const char* input)
	{
		u32 output = 0;
		const char* p = input;
		u32 places = 0;
		while (*p && places < 8)
		{
			output *= 16;
			u32 hexdigit = *p - '0';
			if (hexdigit > 9)
				hexdigit -= 'A' - '9' - 1;
			output += hexdigit;
			p++;
			places++;
		}
		return output;
	}

	inline float intpow(float number, s32 power)
	{
		s32 p = abs(power);
		float result = 1.0f;
		float n = number;
		s32 pindex = 0;
		while (p)
		{
			if (p & 1)
			{
				result *= n;
			}
			n *= n;
			p >>= 1;
		}
		if (power < 0)
			result = 1 / result;
		return result;
	}

	inline u32 copystring(const char* src, char* dst, u32 len, u32 destmax)
	{
		u32 num = min(len, destmax);
		for (u32 i = 0; i < num; i++)
		{
			*dst++ = *src++;
		}
		*dst = 0;
		return num;
	}

	inline float floor(float n)
	{
		__m128 m;
		m.m128_f32[0] = n;
		s32 i = _mm_cvtt_ss2si(m);
		m = _mm_cvt_si2ss(m, i);
		float tn = m.m128_f32[0];
		if (n < 0 && tn != n)
		{
			tn -= 1.0f;
		}
		return tn;
	}

	inline float ceil(float n)
	{
		__m128 m;
		m.m128_f32[0] = n;
		s32 i = _mm_cvtt_ss2si(m);
		m = _mm_cvt_si2ss(m, i);
		float tn = m.m128_f32[0];
		if (n > 0 && tn != n)
		{
			tn += 1.0f;
		}
		return tn;
	}

	inline s32 float2intround(float x)
	{
		__m128 m;
		m.m128_f32[0] = x;
		return _mm_cvt_ss2si(m);
	}

	inline s32 float2inttrunc(float x)
	{
		__m128 m;
		m.m128_f32[0] = x;
		return _mm_cvtt_ss2si(m);
	}

	union IntFloat
	{
		s32 i;
		f32 f;
	};

	inline bool isinfinite(float f)
	{
		IntFloat x;
		x.f = f;
		x.i &= 0x7fffffff;
		s32 exp = x.i >> 23;
		if (exp == 0xff && x.i & 0x7fffff)
			return true;
		return false;
	}

	inline bool isnan(float f)
	{
		IntFloat x;
		x.f = f;
		x.i &= 0x7fffffff;
		s32 exp = x.i >> 23;
		if (exp == 0xff && x.i & 0x7fffff)
			return true;
		return false;
	}

	inline u32 ftoa(f32 number, char* string, u32 maxstring, s32 precision = -1)
	{
		static const float log10base2 = 3.3219281f;
		static const float log2base10 = 0.30103f;
		static const char inf[] = "inf";
		static const char nan[] = "nan";
		static const char zero[] = "0";
		static const s32 defaultprecision = 3;
		u32 charsprinted = 0;
		char localint[32];
		bool isdefaultprecision = false;

		if (maxstring < sizeof(zero))
		{
			return charsprinted;
		}

		if (precision <= 0)
		{
			isdefaultprecision = true;
			precision = defaultprecision;
		}

		// extract exponent and sign and take abs of number
		IntFloat x;
		x.f = number;
		u32 exp = (x.i >> 23) & 0xff;
		x.i &= 0x7fffffff;
		char* dest = string;
		if (number < 0)
		{
			*dest++ = '-';
			charsprinted++;
		}

		// return "nan" or "inf"
		if (exp == 0xff)
		{
			if (x.i & 0x7fffff)
			{
				if (sizeof(nan) <= maxstring)
				{
					copystring(nan, string, sizeof(nan), sizeof(nan));
					charsprinted = sizeof(nan);
				}
				return charsprinted;
			}
			else
			{
				if (charsprinted + sizeof(inf) <= maxstring)
				{
					copystring(inf, dest, sizeof(inf), sizeof(inf));
					charsprinted += sizeof(inf);
				}
				return charsprinted;
			}
		}

		// return zero
		if (x.i == 0)
		{
			// TODO: handle denormals here, this may require checking for 0 exponent field
			if (charsprinted + sizeof(zero) <= maxstring)
			{
				copystring(zero, dest, sizeof(zero), sizeof(zero));
				charsprinted += sizeof(zero);
			}
			return charsprinted;
		}

		s32 intlog2 = exp - 127;
		if (intlog2 > 0)
			intlog2++;
		s32 intlog10 = float2intround(intlog2 * log2base10);
		s32 intdigits = intlog10 + 1;

		float ten2power = intpow(10.0f, intlog10);
		if (ten2power - x.f > 0.0000001f)
		{
			intlog10--;
			intdigits--;
		}
		if (intdigits < 8 && intdigits > -6)
		{
			precision = max(precision, intdigits);
		}
		s32 scale10 = precision - intlog10;
		if (scale10 > 38)
		{
			x.f *= 1e38f;
			scale10 -= 38;
		}
		float scalepower10 = intpow(10.0f, scale10);

		float xtimes10 = x.f * scalepower10;
		s32 intnumber = float2intround(xtimes10);
		// TODO: did we round up to the next power of 10? If so, we need to inc # of digits

		s32 totaldigits = itoa(intnumber, localint, 32) - 1;
		s32 exponent = 0;
		char* z = localint + totaldigits - 1;

		// remove trailing zeroes
		while (*z == '0' && (totaldigits > intdigits || intdigits > precision))
		{
			*z-- = 0;
			totaldigits--;
		}

		// point is index in integer where to place decimal point
		// if it is negative it represents number of leading 0s - after 0.
		// if it is positive it is is in the middle of the integer somewhere
		// if it's greater than totaldigits, then it becomes 1 and we use sci notation
		s32 point = 0;
		if (intdigits <= 0)
		{
			if (intdigits > -precision)
			{
				if (charsprinted - intdigits + 2 <= maxstring)
				{
					*dest++ = '0';
					*dest++ = '.';
					for (s32 i = 0; i < -intdigits; i++)
					{
						*dest++ = '0';
					}
					charsprinted += 2 - intdigits;
				}
				else
				{
					return charsprinted;
				}
			}
			else
			{
				point = 1;
				exponent = intdigits;
			}
		}
		if (intdigits > 0)
		{
			if (intdigits > totaldigits)
			{
				exponent = intdigits - 1;
				point = 1;
			}
			else
			{
				if (intdigits > precision)
				{
					point = 1;
					exponent = intdigits;
				}
				else
				{
					point = intdigits;
				}
			}
		}

		u32 charsneeded = totaldigits + 1; // count the null
		if (totaldigits > 1 && point)
		{
			charsneeded++;
		}
		if (charsprinted + charsneeded <= maxstring)
		{
			char* p = localint;
			for (s32 i = 0; i < totaldigits; i++)
			{
				if (point && (i == point))
				{
					*dest++ = '.';
				}
				*dest++ = *p++;
			}
			*dest = 0;
			charsprinted += charsneeded;

			// add exponent if needed
			if (exponent && charsprinted + 5 <= maxstring)
			{
				s32 abslog10 = abs(intlog10);
				*dest++ = 'e';
				*dest++ = intlog10 >= 0 ? '+' : '-';
				if (abslog10 < 10)
					*dest++ = '0';
				itoa(abslog10, dest, 3);
				charsprinted += 4;
			}
		}

		return charsprinted;
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

};