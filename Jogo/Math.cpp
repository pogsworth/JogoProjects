#include "JMath.h"

namespace Jogo
{
	u32 itoa(s32 number, char* string, u32 maxstring)
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

	s32 atoi(const char* string)
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

	u32 itohex(u32 number, char* string, u32 maxstring, u32 maxdigits)
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

	u32 hextoi(const char* input)
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

	double intpow(float number, s32 power)
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

	float floor(float n)
	{
		__m128 m = _mm_set_ss(n);
		s32 i = _mm_cvtt_ss2si(m);
		m = _mm_cvt_si2ss(m, i);
		float tn = m.m128_f32[0];
		if (n < 0 && tn != n)
		{
			tn -= 1.0f;
		}
		return tn;
	}

	float ceil(float n)
	{
		__m128 m = _mm_set_ss(n);
		s32 i = _mm_cvtt_ss2si(m);
		m = _mm_cvt_si2ss(m, i);
		float tn = m.m128_f32[0];
		if (n > 0 && tn != n)
		{
			tn += 1.0f;
		}
		return tn;
	}

	union IntFloat
	{
		f32 f;
		s32 i;
	};

	bool isinfinite(float f)
	{
		IntFloat x;
		x.f = f;
		x.i &= 0x7fffffff;
		s32 exp = x.i >> 23;
		if (exp == 0xff && x.i & 0x7fffff)
			return true;
		return false;
	}

	bool isnan(float f)
	{
		IntFloat x;
		x.f = f;
		x.i &= 0x7fffffff;
		s32 exp = x.i >> 23;
		if (exp == 0xff && x.i & 0x7fffff)
			return true;
		return false;
	}

	double tenpowers[] = {
		1e-46, 1e-45, 1e-44, 1e-43, 1e-42, 1e-41, 1e-40,
		1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30,
		1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20,
		1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,
		1e-09, 1e-08, 1e-07, 1e-06, 1e-05, 1e-04, 1e-03, 1e-02, 1e-01,
		1e0, 1e1, 1e2,1e3,1e4,1e5, 1e6,1e7,1e8, 1e9,
		1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
		1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
		1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39,
		1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49,
		1e50, 1e51, 1e52, 1e53, 1e54
	};

	double tenpow(s32 power)
	{
		if (power > -47 && power < 55)
			return tenpowers[power + 46];
		return intpow(10.0f, power);
	}

	// inspired by stbsp__real_to_str in stb_sprintf at https://github.com/nothings/stb
	u32 ftoa(f32 number, char* string, u32 maxstring, u32 precision)
	{
		const float log10of2 = 0.30103f;
		Jogo::IntFloat intf{ number };

		s32 neg = intf.i >> 31;
		intf.i &= 0x7fffffff;

		s32 exp = ((intf.i >> 23) & 0xff);
		if (exp == 0xff)	// infinity
		{
			if (intf.i & 0x7fffff)
			{	// nan
				char* s = string;
				*s++ = 'n'; *s++ = 'a'; *s++ = 'n'; *s = 0;
				return 3;
			}
			char* s = string;
			*s++ = 'i'; *s++ = 'n'; *s++ = 'f'; *s = 0;
			return 3;
		}

		if (exp == 0)
		{
			// check for zero/denormal
			if ((intf.i & 0x7fffffff) == 0)
			{
				string[0] = '0';
				string[1] = 0;
				return 1;
			}
			u32 highbit = 1 << 22;
			while (!(intf.i & highbit) && highbit)
			{
				highbit >>= 1;
				exp--;
			}
		}

		// get power of ten estimate of the float
		s32 exp10 = Jogo::float2inttrunc(Jogo::floor((exp - 127) * log10of2));

		// save at most 8 significant digits from the float
		s32 digits = Jogo::double2intround(intf.f * tenpow(8 - exp10));
		if (digits >= 1e9)
			exp10++;

		// round at requested precision
		if (precision >= 0 && precision <= 9)
		{
			precision = precision ? precision : 1;
			u32 numdigits = digits >= 1e9 ? 10 : 9;
			if (precision < numdigits)
			{
				s32 rounder = ((s32)tenpow(numdigits - precision));
				s32 even = 1 - ((digits / rounder) & 1);
				s32 rounded = digits + (rounder / 2 - even);
				if (digits < 1e9 && rounded >= 1e9)
					exp10++;
				if (digits >= 1e10 && rounded >= 1e11)
					exp10++;
				digits = rounded / rounder;
			}
		}

		// remove trailing zeroes
		while (digits && digits % 10 == 0)
			digits /= 10;

		// output the string of digits
		char decimaldigits[21];
		u32 ilen = itoa(digits, decimaldigits, 20);
		char* src = decimaldigits;
		char* dst = string;
		if (neg)
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

		s32 e = 0;
		while (ilen && e <= (s32)precision)
		{
			if (e == exp10)
				*dst++ = '.';
			*dst++ = *src++;
			e++;
			ilen--;
		}

		// add back necessary trailing zeroes
		if (exp10 > e && exp10 < (s32)precision)
		{
			s32 p = exp10 - e;
			while (p--)
				*dst++ = '0';
			e = exp10;
		}

		// print signed exponent
		if (exp10 >= (s32)precision || exp10 < -4)
		{
			*dst++ = 'e';
			*dst++ = exp10 < 0 ? '-' : '+';;
			s32 e = exp10 > 0 ? exp10 : -exp10;

			u32 ilen = itoa(e, decimaldigits, 20);
			src = decimaldigits;
			if (e < 10)
				*dst++ = '0';
			while (ilen--)
				*dst++ = *src++;
		}
		*dst = 0;

		return (u32)(dst - string);
	}

	float atof(const char* string)
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

		double pow10 = tenpow(exp - fraclen);
		return (float)(integer * pow10);
	}

	float sqrt(float x)
	{
		__m128 m = _mm_set_ss(x);
		m = _mm_sqrt_ss(m);
		return m.m128_f32[0];
	}

	void remainder(float num, float denom, float invdenom, int& quotient, float& remainder)
	{
		__m128 m = _mm_set_ss(num * invdenom);
		quotient = _mm_cvt_ss2si(m);
		remainder = num - quotient * denom;
	}

	// return sine of an angle between 0..pi/4
	static float __sine(float x)
	{
		const float coef1 = 0.999994990f;
		const float coef3 = -0.166601570f;
		const float coef5 = 0.00812149339f;
		float xx = x * x;

		return x * (coef1 + xx * (coef3 + coef5 * xx));
	}

	// return sine of an angle between 0..pi/4
	static float __cosine(float x)
	{
		const float coef2 = -0.499998566f;
		const float coef4 = 0.0416550209f;
		const float coef6 = -0.00135858439f;

		float xx = x * x;
		return 1 + xx * (coef2 + xx * (coef4 + xx * coef6));
	}

	float sine(float x)
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

	float cosine(float x)
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