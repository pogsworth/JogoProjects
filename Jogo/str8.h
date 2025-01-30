
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

		static size_t Length(const char* string)
		{
			const char* s = string;
			size_t size = 0;
			while (*s++)
				size++;
			return size;
		}

		static u32 copystring(char* dest, const char* src, size_t len)
		{
			__movsb((unsigned char*)dest, (unsigned char*)src, len);
			return (u32)len;
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

		static double tenpow(s32 power);
		static u32 itoa(s32 number, char* string, u32 maxstring);
		s32 atoi();
		static u32 itohex(u32 number, char* string, u32 maxstring, bool leadingzeros = true, bool upper = true);
		u32 hextoi();
		static u32 ftoa(f32 number, char* string, u32 maxstring, u32 precision = 6);
		float atof();

		static u32 toString(s32 number, const str8& spec, char* stringspace, u32 maxlen);

		static u32 toString(f32 fnumber, const str8& spec, char* stringspace, u32 maxlen)
		{
			return ftoa(fnumber, stringspace, maxlen);
		}

		static u32 toString(const char* string, const str8& spec, char* stringspace, u32 maxlen)
		{
			size_t len = Length(string);
			len = Jogo::min((u32)len, maxlen);
			copystring(stringspace, string, len);
			return (u32)len;
		}

		struct formatter
		{
			u32 format(const str8& fmt, char* dest, auto arg, auto... rest)
			{
				// find all escaped braces
				u32 pos = 0;
				u32 len = 0;
				str8 fmtsub = fmt;
				char* d = dest;
				bool escaped = false;
				do {
					escaped = false;
					fmtsub = fmtsub.substr(pos);
					pos = fmtsub.find('{');
					if (pos == (u32)-1 || (pos == (fmtsub.len - 1)))
					{
						// TODO: remove escaped right braces } up to pos
						return len + copystring(d, fmtsub.chars, fmtsub.len);
					}
					len += copystring(d, fmtsub.chars, pos);
					d += pos;

					// put escaped { in output 
					if (fmtsub[pos + 1] == '{')
					{
						*d++ = '{';
						len++;
						pos += 2;
						escaped = true;
					}
				} while (escaped);
				fmtsub = fmtsub.substr(pos);

				// find matching } and pass format specifiers to toString
				str8 spec(nullptr, (size_t)0);
				u32 end = fmtsub.find('}');
				if (end == (u32)-1)
				{
					return len + copystring(d, fmtsub.chars + 1, fmtsub.len - 1);
				}
				else
				{
					spec = fmtsub.substr(1, end-1);
					u32 l = toString(arg, spec, d, (u32)-1);
					d += l;
					len += l;
				}
				return len + format(fmtsub.substr((u32)spec.len + 2), d, rest...);
			}

			u32 format(const str8& fmt, char* dest)
			{
				return copystring(dest, fmt.chars, fmt.len);
			}

		};

		template <typename... Args>
		static str8 format(const str8& fmt, Arena& arena, const Args&... args)
		{
			str8 newstr;
			char* newchars = (char*)arena.Allocate(0);

			formatter format_arg;

			newstr.len = format_arg.format(fmt, newchars, args...);

			newstr.chars = newchars;
			arena.Allocate(newstr.len);	// bump arena by size of string
			return newstr;
		}
	};
};