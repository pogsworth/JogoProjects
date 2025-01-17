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

		//str8(const char* string)
		//{
		//	chars = string;
		//	len = Length(string);
		//}

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

		bool operator==(str8& b)
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

		bool operator<(str8& b)
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

		static u32 toString(s32 number, char* stringspace, u32 maxlen)
		{
			return Jogo::itoa(number, stringspace, maxlen);
		}

		static u32 toString(f32 fnumber, char* stringspace, u32 maxlen)
		{
			return Jogo::ftoa(fnumber, stringspace, maxlen);
		}

		static u32 toString(const char* string, char* stringspace, u32 maxlen)
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
				u32 pos = fmt.find("{}");
				if (pos == (u32)-1)
				{
					return copystring(dest, fmt.chars, fmt.len);
				}
				else
				{
					u32 len = copystring(dest, fmt.chars, pos);
					len += toString(arg, dest + pos, (u32)-1);
					return len + format(fmt.substr(pos + 2), dest + len, rest...);
				}
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

	// what kinds of functions for strings?
	//
	// DONE Make a string from a null-terminated char*
	// DONE make a string from part of another s8
	// DONE make a string by formatting multiple strings
	// DONE make a string from part of a null-terminated char*, especially from a file
	// When do we want to copy parts out of a char[] into somewhere else and who owns it?
	// if we're just putting into an arena, then then arena owns it and frees it
	// but what if we want the s8 to live longer than the arena lifetime?
	//		then I think we need to copy the char[] data into a longer living arena...


	// one useage pattern:
	// read in a file into a large char[]
	// parse it into tokens and pass them around as s8s that are just pieces of the file char[]
	// if any of these s8s need to live longer than the file data, then the piecse of char[]
	// must be copied out to an 'older' arena that will live longer
};