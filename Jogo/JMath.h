#pragma once
#include "int_types.h"
#include <intrin.h>
//#include <math.h>

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

	inline s32 atoi(const char* string)
	{
		s32 neg = 1;
		if (*string == '-') neg = -1, string++;
		s32 value = 0;
		while (*string && isdigit(*string))
		{
			// TODO: check for overflow and return large num to represent infinity
			value = value * 10 + *string - '0';
			string++;
		}
		return neg * value;
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

	inline double intpow(float number, s32 power)
	{
		s32 p = abs(power);
		double result = 1.0f;
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
		f32 f;
		s32 i;
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

	inline u32  ftoa(f32 number, char* string, u32 maxstring, u32 precision = 8)
	{
		// get power of ten estimate of the float
		const float log10of2 = 0.30103f;
		Jogo::IntFloat intf{number};
		s32 exp = ((intf.i >> 23) & 0xff);
		if (exp == 0xff)	// infinity
		{
			if (intf.i & 0x7fffff)
			{	// nan
				copystring("nan", string, 4, 4);
				return 3;
			}
			copystring("inf", string, 4, 4);
			return 3;
		}

		if (exp == 0)
		{
			// check for zero/denormal
			if ((intf.i & 0x7fffffff) == 0)
			{
				copystring("0", string, 2, 2);
				return 1;
			}
			u32 highbit = 1 << 22;
			while (!(intf.i & highbit) && highbit)
			{
				highbit >>= 1;
				exp--;
			}
		}

		s32 exp10 = Jogo::float2inttrunc(Jogo::floor((exp - 127) * log10of2));
		// save at most 8 significant digits from the float
		s32 digits = Jogo::float2intround((float)(intf.f * Jogo::intpow(10.0, 8 - exp10)));
		if (digits >= 1e9)	exp10++;

		// round at requested precision
		// TODO: allow precision to be larger
		if (precision >= 0 && precision < 9)
		{
			precision = precision ? precision : 1;
			s32 numdigits = digits >= 1e9 ? 10 : 9;
			s32 rounder = ((s32)intpow(10.0, numdigits - precision));
			s32 rounded = digits + rounder/2;
			if (digits < 1e9 && rounded >= 1e9) exp10++;
			if (digits >= 1e9 && rounded >= 1e10) exp10++;
			digits = rounded / rounder;
		}

		// remove trailing zeroes
		while (digits % 10 == 0)
			digits /= 10;

		// output the string of digits
		char decimaldigits[21];
		s32 ilen = itoa(digits, decimaldigits, 20);
		char* src = decimaldigits;
		char* dst = string;
		if (intf.i & 0x80000000)
			*dst++ = '-';
		if (exp10 < 0 && exp10 > -5)
		{
			*dst++ = '0';
			*dst++ = '.';
			s32 e = -exp10;
			while (--e)
				*dst++ = '0';
		}
		*dst++ = *src++;
		if ((exp10 >= (s32)precision || exp10 < -4) && digits >= 10)
			*dst++ = '.';

		u32 d = 1;
		while (*src && d < precision)
		{
			*dst++ = *src++;
			d++;
		}

		// fix exact powers of ten less than 10^precision
		if (digits == 1 && exp10 > 0 && exp10 < (s32)precision)
		{
			s32 p = exp10;
			while (p--)
				*dst++ = '0';
			d += exp10;
		}

		// print signed exponent
		if (exp10 >= (s32)precision || exp10 < -4)
		{
			*dst++ = 'e';
			*dst++ = exp10 < 0 ? '-' : '+';;
			s32 e = exp10 > 0 ? exp10: -exp10;

			itoa(e, decimaldigits, 20);
			src = decimaldigits;
			if (e < 10) *dst++ = '0';
			while (*src)
				*dst++ = *src++;
		}
		*dst++ = 0;

		return (u32)(dst - string);
	}

	inline float atof(const char* string)
	{
		const char* c = string;
		s32 neg = 1;
		if (*c == '-') neg = -1, c++;

		s32 integer = 0;
		s32 intlen = 0;

		while (*c && isdigit(*c))
		{
			char d = *c++ - '0';
			integer = integer * 10 + d;
			intlen++;
		}

		s32 fraclen = 0;
		if (*c == '.')
		{
			c++;
			while (*c && isdigit(*c))
			{
				char d = *c++ - '0';
				integer = integer * 10 + d;
				fraclen++;
			}
		}

		s32 expsign = 1;
		s32 exp = 0;
		if (*c == 'e' || *c == 'E')
		{
			c++;
			if (*c && *c == '-')	expsign = -1, c++;
			if (*c && *c == '+') c++;
			while (*c && isdigit(*c))
			{
				exp = exp * 10 + (*c++ - '0');
			}
			exp *= expsign;
		}

		double pow10 = intpow(10.0, exp - fraclen);
		return (float)(integer * pow10);
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