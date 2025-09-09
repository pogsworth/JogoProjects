#pragma once
#include "str8.h"

namespace Jogo
{
	u32 str8::itoa(s32 number, char* string, u32 maxstring)
	{
		char result[32];
		u32 n = abs(number);
		char* p = result;
		do
		{
			*p++ = n % 10 + '0';
			n /= 10;
		} while (n);

		if (number < 0)
		{
			*p++ = '-';
		}
		u32 len = (u32)(p - result);
		p--;
		char* b = result;
		while (b < p)
		{
			char s = *b;
			*b++ = *p;
			*p-- = s;
		}
		return (u32)copystring(result, string, len, maxstring);
	}

	s32 str8::atoi() const
	{
		const char* s = chars;
		s32 neg = 1;
		if (*s == '-') neg = -1, s++;
		s32 value = 0;
		while ((s-chars < (ptrdiff_t)len) && isdigit(*s))
		{
			// TODO: check for overflow and return large num to represent infinity
			value = value * 10 + *s - '0';
			s++;
		}
		return neg * value;
	}

	u32 str8::itohex(u32 number, char* string, u32 maxstring, bool leadingzeros, bool upper)
	{
		char result[32];
		u32 numdigits = 8;
		if (!leadingzeros)
		{
			if (_BitScanReverse((unsigned long*)&numdigits, number))
				numdigits = (numdigits + 4) >> 2;
			else
				numdigits = 1;
		}

		u32 n = number;
		char* p = result + numdigits - 1;

		char a = 'a';
		if (upper)
			a = 'A';

		do
		{
			char hex = (n & 0xf) + '0';
			if (hex > '9')
				hex += a - '9' - 1;
			*p-- = hex;
			n >>= 4;
		} while (n);

		return (u32)copystring(result, string, numdigits, maxstring);
	}

	u32 str8::hextoi()
	{
		u32 output = 0;
		const char* p = chars;
		u32 places = 0;
		while (p-chars < (ptrdiff_t)len && places < 8)
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

	double str8::tenpow(s32 power)
	{
		if (power > -47 && power < 55)
			return tenpowers[power + 46];
		return Jogo::intpow(10.0f, power);
	}

	union IntFloat
	{
		f32 f;
		s32 i;
	};

	// inspired by stbsp__real_to_str in stb_sprintf at https://github.com/nothings/stb
	u32 str8::ftoa(f32 number, char* string, u32 maxstring, u32 precision)
	{
		char result[32];
		const float log10of2 = 0.30103f;
		IntFloat intf{ number };

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
		precision += exp10;
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

		// remove trailing zeroes up to requested precision
//		while (digits && digits % 10 == 0)
//			digits /= 10;

		// output the string of digits
		char decimaldigits[21];
		u32 ilen = itoa(digits, decimaldigits, 20);
		char* src = decimaldigits;
		char* dst = result;
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
		ilen--;
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
		}
		else if (e - exp10 < (s32)precision)
		{
			s32 p = precision - e;
			while (p--)
				*dst++ = '0';
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
		return (u32)copystring(result, string, dst - result, maxstring);
	}

	float str8::atof()
	{
		const char* b = chars;
		const char* c = b;
		ptrdiff_t l = len;
		s32 neg = 1;
		if (*c == '-') neg = -1, c++;

		s32 integer = 0;
		s32 intlen = 0;

		while (c-b < l && isdigit(*c))
		{
			char d = *c++ - '0';
			integer = integer * 10 + d;
			intlen++;
		}

		s32 fraclen = 0;
		if (*c == '.')
		{
			c++;
			while (c-b < l && isdigit(*c))
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
			if (c - b < l && *c == '-') expsign = -1, c++;
			if (c - b < l && *c == '+') c++;
			while (c - b < l && isdigit(*c))
			{
				exp = exp * 10 + (*c++ - '0');
			}
			exp *= expsign;
		}

		double pow10 = tenpow(exp - fraclen);
		return (float)(integer * pow10);
	}

	// return bits indicating what fields are in the spec
	u32 str8::parseSpec(const str8& spec)
	{
		u32 bits = 0;
		u32 pos = 0;

		// look for width and precision
		u32 colon = spec.find(':');
		if (colon != (u32)-1 && colon < spec.len - 1)
		{
			// look for digits
			pos = colon + 1;
			// look for alignment
			if (spec[pos] == '<')
			{
				pos++;
				bits |= SPEC_LEFT;
			}
			if (spec[pos] == '>')
			{
				pos++;
				bits &= ~SPEC_LEFT;
				bits |= SPEC_RIGHT;
			}
			if (spec[pos] == '^')
			{
				pos++;
				bits &= ~(SPEC_LEFT | SPEC_RIGHT);
				bits |= SPEC_CTR;
			}
			// look for leading '0'
			if (spec[pos] == '0')
			{
				pos++;
				bits |= SPEC_ZERO;
			}

			while (!isdigit(spec[pos]) && spec[pos] != '.' && pos < spec.len)
				pos++;
			if (pos < spec.len)
			{
				if (isdigit(spec[pos]))
				{
					char widthchars[16] = {};
					u32 w = 0;
					while (isdigit(spec[pos]) && pos < spec.len)
						widthchars[w++] = spec[pos++];
					str8 widthstr(widthchars, w);
					u32 widthvalue = widthstr.atoi();
					widthvalue = Jogo::min(widthvalue, (u32)SPEC_WIDTH_MASK);
					bits |= widthvalue << SPEC_WIDTH_SHIFT;
					bits |= SPEC_WIDTH;
				}
				if (spec[pos] == '.')
				{
					char precchars[16] = {};
					pos++;
					u32 p = 0;
					while (isdigit(spec[pos]) && pos < spec.len)
						precchars[p++] = spec[pos++];
					str8 precstr(precchars, p);
					u32 precvalue = precstr.atoi();
					precvalue = Jogo::min(precvalue, (u32)SPEC_PREC_MASK);
					bits |= precvalue << SPEC_PREC_SHIFT;
					bits |= SPEC_PREC;
				}
			}
		}

		// look for types
		if (pos < spec.len)
		{
			if (spec.substr(pos).find('x') != (u32)-1)
				bits |= SPEC_HEX;
			else if (spec.substr(pos).find('X') != (u32)-1)
				bits |= SPEC_HEX | SPEC_HEX_UPPER;
		}

		return bits;
	}

	u32 str8::toString(s32 number, const str8& spec, char* stringspace, u32 maxlen)
	{
		u32 bits = parseSpec(spec);
		bool neg = number < 0 && !(bits & SPEC_HEX) && number != 1 << 31;
		char intchars[64];
		u32 len = 0;
		if (bits & SPEC_HEX)
		{
			len = itohex(number, intchars, 63, false, (bits & SPEC_HEX_UPPER) != 0);
		}
		else
			len = itoa(Jogo::abs(number), intchars, 63);

		u32 result = len;
		u32 width = 0;
		if (bits & SPEC_WIDTH)
			width = (bits >> SPEC_WIDTH_SHIFT) & SPEC_WIDTH_MASK;
		char* p = stringspace;
		if (width > len)
		{
			char fill = ' ';
			if (bits & SPEC_ZERO)
			{
				fill = '0';
				if (neg)
				{
					*p++ = '-';
					width--;
				}
			}
			u32 diff = width - len;
			while (diff--)
			{
				*p++ = fill;
			}
			result = width + neg;
		}
		char* q = intchars;
		u32 l = len;
		if (neg && !(bits & SPEC_ZERO))
			*p++ = '-';
		while (l--)
		{
			*p++ = *q++;
		}

		return result;
	}

};
