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
		while ((s - chars < (ptrdiff_t)len) && isdigit(*s))
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
		while (p - chars < (ptrdiff_t)len && places < 8)
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

	u64 tenpowersint[] = {
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 10000000000, 100000000000
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

	void str8::ftoi(f32 n, u32& mantissa, s32& exponent, u32& digits)
	{
		const float log10of2 = 0.30103f;
		IntFloat intf{ n };

		intf.i &= 0x7fffffff;

		s32 exp = ((intf.i >> 23) & 0xff);
		if (exp == 0xff)	// infinity
		{
			mantissa = 0;
			digits = 0;
			// return special values for:	nan			inf
			exponent = intf.i & 0x7fffff ? 0xffff0000 : 0xff000000;
			return;
		}

		if (exp == 0)
		{
			// check for zero/denormal
			if ((intf.i & 0x7fffffff) == 0)
			{
				mantissa = 0;
				exponent = 0;
				digits = 1;
				return;
			}
			u32 highbit = 1 << 22;
			while (!(intf.i & highbit) && highbit)
			{
				highbit >>= 1;
				exp--;
			}
		}

		// get power of ten estimate of the float
		exponent = Jogo::float2inttrunc(Jogo::floor((exp - 127) * log10of2));

		// save at most 9 significant digits from the float
		mantissa = Jogo::double2intround(intf.f * tenpow(8 - exponent));
		digits = 9;
		if (mantissa >= tenpowersint[9])
		{
			exponent++;
			digits++;
		}
	}

	// inspired by stbsp__real_to_str in stb_sprintf at https://github.com/nothings/stb
	u32 str8::ftoa(f32 number, char* string, u32 maxstring, u32 precision, u32 format)
	{
		char result[64];

		u32 mantissa;
		s32 exponent;
		u32 digits;
		ftoi(number, mantissa, exponent, digits);
		char* d = string;

		if (format != 'f' && format != 'e' && format != 'g')
			format = 'f';

		if (exponent == 0xffff0000)
		{
			*(u32*)d = 'nan';
			return 3;
		}
		if (*(s32*)&number & 0x80000000)
			*d++ = '-';
		if (exponent == 0xff000000)
		{
			*(u32*)d = 'fni';
			d += 3;
			return (u32)(d - string);
		}
		if (mantissa == 0 && format == 'g')
		{
			*d++ = '0';
			return (u32)(d - string);
		}

		s32 roundplace = precision;

		if (format != 'g')
			roundplace++;

		if (format == 'f')
		{
			roundplace += exponent;
		}

		u32 outdigits = roundplace < 0 ? 0 : roundplace;

		u32 exponent_bumped = 0;
		// round at requested precision
		if (0 <= roundplace && roundplace < 10 && mantissa != 0)
		{
			u64 rounder = tenpowersint[digits - roundplace];
			// test for halfway, then tie goes to even
			u64 halfrounder = rounder / 2;
			if (mantissa % rounder != halfrounder || (mantissa / rounder & 1))
			{
				mantissa += (u32)halfrounder;
				if (mantissa > tenpowersint[digits])
				{
					exponent++;
					outdigits++;
					exponent_bumped = 1;
				}
			}
			mantissa = (u32)(mantissa / rounder);
			digits = outdigits;
		}

		// output the string of digits
		char decimaldigits[64];
		u32 ilen = itoa(mantissa, decimaldigits, 20);
		// special extend zeroes when value is zero
		if (mantissa == 0)
		{
			for (u32 d = 1; d < outdigits; d++)
				decimaldigits[d] = '0';
		}
		char* src = decimaldigits;
		char* dst = result;
		if (*(s32*)&number & 0x80000000)
			*dst++ = '-';

		bool eat_trailing_zeroes = format == 'g';
		if (format == 'g')
		{
			if ((exponent >= (s32)outdigits || exponent < -4) || (exponent && (exponent == precision)))
			{
				// if the exonent is big enough or small enough, use sci notation
				// pretend the format is 'e' to prcoess the exponent the same
				format = 'e';
			}
			else
			{
				// process the 'g' format like 'f' here
				// bump the number of prefixed zeroes by -exponent - 1
				precision += exponent_bumped - exponent - 1;
			}
		}


		if (format == 'e')
		{
			// print out the pattern f.ffffff0000e+dd
			*dst++ = *src++;
			*dst++ = '.';
			u32 fdigits = min(digits - 1, outdigits - 1);
			fdigits = min(fdigits, precision);
			for (u32 f = 0; f < fdigits; f++)
				*dst++ = *src++;
			for (s32 f = 0; f < (s32)(precision - fdigits); f++)
				*dst++ = '0';
		}
		else
		{
			if (exponent >= 0)
			{
				// print out the pattern ffff.ffff0000
				u32 idigits = exponent + 1;
				u32 dig = min(idigits, digits);
				u32 d = 0;
				for (; d < dig; d++)
					*dst++ = *src++;
				for (; d < idigits; d++)
					*dst++ = '0';
				*dst++ = '.';
				u32 f = 0;
				if (idigits < digits)
				{
					u32 fdigits = min(digits - idigits, outdigits);
					for (; f < fdigits; f++)
						*dst++ = *src++;
				}
				if (!eat_trailing_zeroes)
					for (; f < outdigits - idigits; f++)
						*dst++ = '0';
			}
			else
			{
				// print out the pattern 0.000ffff000 where f are digits of the fraction
				*dst++ = '0';
				*dst++ = '.';

				u32 zeroes = max((s32)(precision - outdigits), (s32)0);

				for (u32 f = 0; f < zeroes; f++)
					*dst++ = '0';
				s32 fdigits = min(digits,outdigits);
				for (s32 f = 0; f < fdigits; f++)
					*dst++ = *src++;
				if (!eat_trailing_zeroes)
				{
					for (s32 f = 0; f < (s32)(precision - zeroes - fdigits); f++)
						*dst++ = '0';
				}
			}
		}

		if (eat_trailing_zeroes)
		{
			while (dst[-1] == '0')
				dst--;
		}
		if (dst[-1] == '.')
			dst--;

		// print exponent
		if (format == 'e')
		{
			*dst++ = 'e';
			*dst++ = exponent < 0 ? '-' : '+';;
			s32 e = exponent > 0 ? exponent : -exponent;

			ilen = itoa(e, decimaldigits, 20);
			src = decimaldigits;
			if (e < 10)
				*dst++ = '0';
			while (ilen--)
				*dst++ = *src++;
		}

		return (u32)copystring(result, string, dst - result, maxstring);
	}

	u32 str8::f2a(f32 number, char* string)
	{
		u32 len = 0;
		s32 numint = (s32)number;
		if (numint == 0x80000000)
		{
			*(u32*)string = 'fni';
			return 3;
		}

		len = itoa(numint, string, 10);
		string[len] = 0;
		
		s32 fraclen = 10 - len;
		fraclen += numint < 0 ? 1 : 0;
		s32 frac = ((s32)((abs(number) - abs(numint)) * tenpowersint[fraclen])+500)/1000;
		if (frac == 0)
			return len;

		string[len++] = '.';

		len += itoa(frac, string + len, 10);
		string[len] = 0;
		return len;
	}

	float str8::atof()
	{
		const char* b = chars;
		const char* c = b;
		ptrdiff_t l = len;
		s32 neg = 1;
		if (*c == '-') neg = -1, c++;

		IntFloat intf;
		// catch 'inf' and 'nan'
		u32 threechars = (*(u32*)c)&0xffffff;
		if (threechars == 'fni')
		{
			intf.i = 0x7f800000;
			return intf.f;
		}
		if (threechars == 'nan')
		{
			intf.i = 0x7fffffff;
			return intf.f;
		}

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
		return (float)(neg * integer * pow10);
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
			else if (spec[pos] == 'e')
			{
				bits |= SPEC_EXP_SCI_NOTATION << SPEC_EXP_SHIFT;
			}
			else if (spec[pos] == 'g')
			{
				bits |= SPEC_EXP_SHORTEST << SPEC_EXP_SHIFT;
			}
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

	u32 str8::toString(f32 fnumber, const str8& spec, char* stringspace, u32 maxlen)
	{
		u32 bits = parseSpec(spec);
		u32 precision = DEFAULT_PREC;
		if (bits & SPEC_PREC)
		{
			precision = (bits >> SPEC_PREC_SHIFT) & SPEC_PREC_MASK;
		}
		u32 floatformat = 'f';
		u32 expformat = (bits >> SPEC_EXP_SHIFT) & SPEC_EXP_MASK;
		if (expformat == SPEC_EXP_SCI_NOTATION)
			floatformat = 'e';
		if (expformat == SPEC_EXP_SHORTEST)
			floatformat = 'g';

		char number[256];
		u32 flen = ftoa(fnumber, number, 256, precision, floatformat);


		u32 width = 0;
		s32 left = 0;
		s32 right = 0;
		if (bits & SPEC_WIDTH)
		{
			width = (bits >> SPEC_WIDTH_SHIFT) & SPEC_WIDTH_MASK;
		}
		if (bits & SPEC_LEFT)
		{
			left = 0;
			right = width - flen;
		}
		else if (bits & SPEC_CTR)
		{
			width -= flen;
			left = width / 2;
			right = width - left;
		}
		else
		{
			left = width - flen;
			right = 0;
		}
		if (left < 0)
			left = 0;
		if (right < 0)
			right = 0;

		for (s32 i = 0; i < left; i++)
		{
			stringspace[i] = ' ';
		}

		copystring(number, stringspace + left, flen, maxlen - left);

		for (s32 i = 0; i < right; i++)
		{
			stringspace[flen+left+i] = ' ';
		}

		return flen + left + right;
	}

	str8 str8::lower(const str8& in, Arena& arena)
	{
		str8 low;
		if (in.len)
		{
			char* newchars = (char*)arena.Allocate(in.len);
			for (u32 i = 0; i < in.len; i++)
			{
				char c = in.chars[i];
				if (isupper(c))
					c += 'a' - 'A';
				newchars[i] = c;
			}
			low.chars = newchars;
		}
		return low;
	}

	str8 str8::upper(const str8& in, Arena& arena)
	{
		str8 up;
		if (in.len)
		{
			char* newchars = (char*)arena.Allocate(in.len);
			for (u32 i = 0; i < in.len; i++)
			{
				char c = in.chars[i];
				if (islower(c))
					c -= 'a' - 'A';
				newchars[i] = c;
			}
			up.chars = newchars;
		}
		return up;
	}

};
