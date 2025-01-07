#include "JMath.h"

namespace Jogo
{
	s32 itoa(s32 number, char* string, u32 maxstring)
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
		s32 digits = Jogo::double2intround(intf.f * Jogo::intpow(10.0, 8 - exp10));
		if (digits >= 1e9)
			exp10++;

		// round at requested precision
		if (precision >= 0 && precision <= 8)
		{
			precision = precision ? precision : 1;
			u32 numdigits = digits >= 1e9 ? 10 : 9;
			if (precision < numdigits)
			{
				s32 rounder = ((s32)intpow(10.0, numdigits - precision));
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
		s32 ilen = itoa(digits, decimaldigits, 20);
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
		while (*src && e <= (s32)precision)
		{
			if (e == exp10)
				*dst++ = '.';
			*dst++ = *src++;
			e++;
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

			itoa(e, decimaldigits, 20);
			src = decimaldigits;
			if (e < 10)
				*dst++ = '0';
			while (*src)
				*dst++ = *src++;
		}
		*dst++ = 0;

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

		double pow10 = intpow(10.0, exp - fraclen);
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