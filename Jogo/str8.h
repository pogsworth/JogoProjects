
#pragma once
#include <intrin.h>
#include "int_types.h"
#include "Arena.h"
#include "JMath.h"

namespace Jogo
{
	struct str8
	{
		const char* chars;
		size_t	len;

		static size_t cstringlength(const char* src)
		{
			const char* s = src;
			while (*s) s++;
			return (size_t)(s - src);
		}

		static size_t copystring(const char* src, char* dest, size_t len, size_t destmax)
		{
			size_t num = min(len, destmax);
			__movsb((unsigned char*)dest, (unsigned char*)src, num);
			return num;
		}

		str8() {}

		str8(const char* begin, char* end)
		{
			chars = begin;
			len = (size_t)(end - begin);
		}

		str8(const char* begin, size_t length)
		{
			chars = begin;
			len = length;
		}

		template<size_t N>
		str8(const char(&str)[N])
		{
			chars = str;
			len = N-1;
		}

		str8 substr(u32 start, u32 length) const
		{
			return str8(chars + start, length);
		}

		str8 substr(u32 start) const
		{
			return str8(chars + start, len - start);
		}

		u32 find(const char c) const
		{
			const char* d = chars;
			for (u32 i = 0; i < len; i++)
			{
				if (*d == c)
				{
					return i;
				}
				d++;
			}
			return (u32)-1;
		}

		u32 find(const str8& lookfor) const
		{
			const char* c = lookfor.chars;
			const char* d = chars;
			for (u32 i = 0; i < len; i++)
			{
				if (*d == *c)
				{
					if (len - i < lookfor.len)
						break;

					const char* e = d;
					bool match = true;
					for (u32 j = 0; j < lookfor.len; j++)
					{
						if (*e++ != *c++)
						{
							match = false;
							break;
						}
					}
					if (match)
						return i;
				}
				c = lookfor.chars;
				d++;
			}
			return (u32)-1;
		}

		bool operator==(const str8& b)
		{
			if (len != b.len)
				return false;
			if (chars == b.chars)
				return true;
			for (s32 i = 0; i < len; i++)
			{
				if (chars[i] != b.chars[i])
				{
					return false;
				}
			}
			return true;
		}

		bool operator!=(const str8& b)
		{
			return (!operator==(b));
		}

		bool operator<(const str8& b)
		{
			if (chars == b.chars)
			{
				if (len < b.len)
					return true;
			}
			u32 strlen = (u32)min(this->len, b.len);
			for (u32 i = 0; i < strlen; i++)
			{
				if (chars[i] != b.chars[i])
					return chars[i] < b.chars[i];
			}
			return len < b.len;
		}

		s32 operator[](u32 pos) const
		{
			if (pos < len)
			{
				return chars[pos];
			}
			return -1;
		}

		static bool isdigit(char n)
		{
			return n >= '0' && n <= '9';
		}

		static bool isalpha(char a)
		{
			return (a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z');
		}

		static bool isupper(char a)
		{
			return a >= 'A' && a <= 'Z';
		}

		static bool islower(char a)
		{
			return a >= 'a' && a <= 'z';
		}

		static const u32 DEFAULT_PREC = 6;
		static double tenpow(s32 power);
		static u32 itoa(u32 number, char* string, u32 maxstring, bool isSigned = true);
		s32 atoi() const;
		static u32 itohex(u32 number, char* string, u32 maxstring, bool leadingzeros = true, bool upper = true);
		u32 hextoi();
		static void ftoi(f32 n, u32& mantissa, s32& exponent, u32& digits);
		static u32 ftoa(f32 number, char* string, u32 maxstring, u32 precision = DEFAULT_PREC, u32 format = 'f');
		static u32 ftoa_old(f32 number, char* string, u32 maxstring, u32 precision = DEFAULT_PREC, u32 format = 'f');
		static u32 f2a(f32 number, char* string);
		float atof();
		static u32 parseSpec(const str8& spec);

		static const u32 SPEC_WIDTH = 1;
		static const u32 SPEC_PREC = 2;
		static const u32 SPEC_HEX = 4;
		static const u32 SPEC_HEX_UPPER = 8;
		static const u32 SPEC_LEFT = 16;
		static const u32 SPEC_CTR = 32;
		static const u32 SPEC_RIGHT = 64;
		static const u32 SPEC_ZERO = 128;
		static const u32 SPEC_WIDTH_SHIFT = 8; // starts at bit 8 = 255
		static const u32 SPEC_WIDTH_MASK = 255;	// allow width up to 255
		static const u32 SPEC_PREC_SHIFT = 16; // start at bit 16 = 65536
		static const u32 SPEC_PREC_MASK = 255; // allow precision up to 255 digits
		static const u32 SPEC_EXP_SHIFT = 24;
		static const u32 SPEC_EXP_MASK = 3;
		static const u32 SPEC_EXP_DEFAULT = 0x0;		// float %f format
		static const u32 SPEC_EXP_SCI_NOTATION = 0x1;	// float %e format
		static const u32 SPEC_EXP_SHORTEST = 0x3;		// float %g format

		static u32 toString(u32 number, const str8& spec, char* stringspace, u32 maxlen, bool isSigned = false);
		static u32 toString(s32 number, const str8& spec, char* stringspace, u32 maxlen)
		{
			return toString((u32)number, spec, stringspace, maxlen, true);
		}
		static u32 toString(f32 fnumber, const str8& spec, char* stringspace, u32 maxlen);
		static u32 toString(const char* string, const str8& spec, char* stringspace, size_t maxlen)
		{
			u32 bits = parseSpec(spec);
			size_t len = cstringlength(string);
			len = Jogo::min(len, maxlen);
			if (bits & SPEC_WIDTH)
			{
				u32 width = (bits >> SPEC_WIDTH_SHIFT) & SPEC_WIDTH_MASK;
				if (width > len)
				{
					u32 diff = width - (u32)len;
					len += diff;
					for (u32 i = 0; i < diff; i++)
						*stringspace++ = ' ';
				}
			}
			copystring(string, stringspace, len, maxlen);
			return (u32)len;
		}

		static u32 toString(const str8& string, const str8& spec, char* stringspace, size_t maxlen)
		{
			u32 bits = parseSpec(spec);
			size_t len = Jogo::min(string.len, maxlen);
			if (bits & SPEC_WIDTH)
			{
				u32 width = (bits >> SPEC_WIDTH_SHIFT) & SPEC_WIDTH_MASK;
				if (width > len)
				{
					u32 diff = width - (u32)len;
					len += diff;
					for (u32 i = 0; i < diff; i++)
						*stringspace++ = ' ';
				}
			}
			copystring(string.chars, stringspace, len, maxlen);
			return (u32)len;
		}

		struct formatter
		{
			u32 format(const str8& fmt, char* dest, auto arg, auto... rest)
			{
				// find all escaped braces
				u32 pos = 0;
				str8 fmtsub = fmt;
				char* d = dest;
				s32 opened = -1;
				s32 closed = -1;

				while (pos < fmtsub.len && closed == -1)
				{
					if (fmtsub[pos] == '{')
					{
						// ignore any open braces inside a spec
						if (opened != -1)
						{
							pos++;
						}
						else if (pos + 1 < fmtsub.len && fmtsub[pos + 1] == '{')
						{
							// put escaped { in output
							*d++ = '{';
							pos += 2;
						}
						else
						{
							opened = pos;
							pos++;
						}
						continue;
					}
					if (fmtsub[pos] == '}')
					{
						if (opened != -1)
						{
							pos++;
							closed = pos;
						}
						else if (pos + 1 < fmtsub.len && fmtsub[pos + 1] == '}')
						{
							*d++ = '}';
							pos += 2;
						}
						else
						{
							// ignore single } outside spec
							pos++;
						}
						continue;
					}
					if (opened == -1)
					{
						*d++ = fmtsub[pos];
						pos++;
					}
					else
					{
						pos++;
					}
				}

				if (closed == -1)
				{
					return (u32)(d-dest);
				}

				// pass format specifiers to toString
				str8 spec = fmtsub.substr(opened, closed - opened);
				u32 l = toString(arg, spec, d, (u32)-1);
				d += l;
				u32 len = (u32)(d - dest);

				if (closed < fmtsub.len)
					return len + format(fmtsub.substr(closed), d, rest...);

				return len;
			}

			// ran out of parameters, output the remainder of the format string
			u32 format(const str8& fmt, char* dest)
			{
				u32 pos = 0;
				char* d = dest;

				// replace double braces in the remainder of the format string...
				while (pos < fmt.len)
				{
					if (fmt[pos] == '{')
					{
						if (pos + 1 < fmt.len && fmt[pos + 1] == '{')
						{
							*d++ = '{';
							pos += 2;
						}
						else
						{
							pos++;	// skip single {
						}
					}
					else if (fmt[pos] == '}')
					{
						if (pos + 1 < fmt.len && fmt[pos + 1] == '}')
						{
							*d++ = '}';
							pos += 2;
						}
						else
						{
							pos++;	// else skip single }
						}
					}
					else
					{
						*d++ = fmt[pos++];
					}
				}

				return (u32)(d - dest);
			}

		};

		template <typename... Args>
		static str8 format(Arena& arena, const str8& fmt, const Args&... args)
		{
			str8 newstr;
			char* newchars = (char*)arena.Allocate(0);

			formatter format_arg;

			newstr.len = (size_t)format_arg.format(fmt, newchars, args...);

			newstr.chars = newchars;
			arena.Allocate(newstr.len);	// bump arena by size of string
			return newstr;
		}

		str8 lower(const str8& in, Arena& arena);
		str8 upper(const str8& in, Arena& arena);
	};
};