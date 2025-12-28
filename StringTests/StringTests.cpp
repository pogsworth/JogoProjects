#include "Jogo.h"
#include "Bitmap.h"
#include "Font.h"
#include "Arena.h"
#include <stdio.h>
#include "str8.h"

using namespace Jogo;


#if 1
#include <iostream>
#include <string>
#include <vector>

void TestFloatFormat(Arena& arena)
{
	struct TestCase {
		float value;
		std::string format; // e.g., "%.2f", "%g", "%.5e"
		str8 fmt;
		std::string description;
	};

	std::vector<TestCase> tests;

	// helper to add tests easily
	auto add = [&](float v, const char* fmt, const str8& fmt8, const char* desc) {
		tests.push_back({ v, fmt, fmt8, desc });
		};

	// 1. BASICS
	add(0.0f, "%.6f", "{}", "Positive Zero");
	add(-0.0f, "%.6f", "{}", "Negative Zero");
	add(123.456f, "%.6f", "{}", "Basic Float");
	add(-123.456f, "%.6f", "{}", "Basic Negative");

	// 2. ROUNDING CLIFFS (Carry Propagation)
	// These test if your function correctly ripples the carry to the left.
	add(0.999999f, "%.3f", "{:.3}", "Carry Propagate near 1.0");
	add(19.999f, "%.2f", "{:.2}", "Carry Propagate to Integer");
	add(9.999f, "%.2f", "{:.2}", "Rounding 9.999 to 2 places (should be 10.00)");

	// 3. MIDPOINT ROUNDING (Ties)
	// Behavior here depends on C++ std lib implementation (usually ties to even).
	add(1.5f, "%.0f", "{:.0}", "Round 1.5 to int"); // Usually 2
	add(2.5f, "%.0f", "{:.0}", "Round 2.5 to int"); // Usually 2 (if ties-to-even) or 3
	add(0.125f, "%.2f", "{:.2}", "Round 0.125 to 2 places");

	// 4. PRECISION EXTREMES
	add(3.14159f, "%.0f", "{:.0}", "Zero Precision");
	add(3.14159f, "%.20f", "{:.20}", "High Precision(Garbage digits check)");

	// 5. EXTREME MAGNITUDES
	add(FLT_MAX, "%.2f", "{:.2}", "Max Float");
	add(-FLT_MAX, "%.2f", "{:.2}", "Min Float (Negative Max)");
	add(FLT_MIN, "%.50f", "{:.50}", "Normalized Min (Tiny)");

	// 6. SUBNORMALS (Denormals)
	// These are values smaller than FLT_MIN. 
	// They are tricky because float representation changes here.
	float subnormal = std::numeric_limits<float>::denorm_min();
	add(subnormal, "%.50f", "{:.50}", "Denormal Min");
	add(subnormal * 10.0f, "%.50e", "{:.50e}", "Denormal Scientific");

	// 7. FORMAT SPECIFIERS
	// %e tests (Scientific)
	add(1000.0f, "%.2e", "{:.2e}", "Scientific 1000");
	add(0.00123f, "%.2e", "{:.2e}", "Scientific small");

	// %g tests (Compact)
	// %g uses %f or %e depending on value size
	add(0.00001f, "%g", "{:.6g}", "%g Switch to Scientific (Small)");
	add(1000000.0f, "%g", "{:.6g}", "%g Switch to Scientific (Large)");
	add(0.1f, "%g", "{:.6g}", "%g Keep as Float");

	// 8. SPECIAL VALUES
	add(std::numeric_limits<float>::infinity(), "%f", "{}", "Positive Infinity");
	add(-std::numeric_limits<float>::infinity(), "%f", "{}", "Negative Infinity");
	add(std::numeric_limits<float>::quiet_NaN(), "%f", "{}", "NaN");

	// EXECUTION LOOP
	int passed = 0;
	int failed = 0;
	//char my_buf[256];
	char std_buf[256];

	std::cout << "Starting Test Suite...\n" << std::endl;
	printf("%-30s | %-10s | %-20s | %-20s | %s\n", "Description", "Input", "My ftoa", "Standard", "Result");
	printf("------------------------------------------------------------------------------------------------------\n");

	for (const auto& t : tests) {
		// 1. Run Standard Sprintf
		// We construct the full format string (e.g., "%.2f") inside the harnesss
		u32 len = sprintf_s(std_buf, sizeof(std_buf), t.format.c_str(), t.value);

		// 2. Run Your ftoa
		// NOTE: Adjust arguments to match your signature
		str8 my_str = str8::format(arena, t.fmt, t.value);

		// 3. Compare
		str8 std_str(std_buf, len);
		//std::string my_str = my_buf;
		//std::string std_str = std_buf;

		bool ok = (my_str == std_str);
		if (ok) passed++; else failed++;

		// Print only failures or special interest
		if (!ok) {
			Print(str8::format(arena, "{:<30} | {:10.4e} | {:<20} | {:<20} | [FAIL]\n",
				t.description.c_str(), t.value, my_str, std_str));
		}
	}

	std::cout << "\n------------------------------------------------------------------------------------------------------\n";
	std::cout << "Tests Completed: " << passed << " Passed, " << failed << " Failed." << std::endl;
}
#endif

int main(int argc, char* argv[])
{
	size_t ScratchArenaSize = 64 * 1024 * 1024;
	Arena scratch = Arena::Create(ScratchArenaSize);
	float t = .999999f;
	u32 p = 7;
	Jogo::Random rand = { 12345 };

	char ftoa_result[32];
	char printfg[36];

	Arena& fa = scratch;

	bool bTestRange = false;
	if (bTestRange)
	{
		float numbers[400];
		float numbers2[300];
		float numbers10[100];
		float f;
		s32 two230 = 1 << 30;
		float begin = 128.0f;
		begin *= (float)two230;
		begin *= (float)two230;
		begin *= (float)two230;
		begin *= (float)two230;

		s32 i = 0;
		for (f = begin; f > 1e-45; f /= 2)
		{
			numbers2[i++] = f;
		}
		s32 size2 = i;
		begin = 1.00000002e38f;
		i = 0;
		s32 j = 38;
		for (f = begin; f > 1e-45; f /= 10)
		{
			numbers10[i] = f;
			numbers10[i] = (float)Jogo::intpow(10.0, j);
			i++;
			j--;
		}
		s32 size10 = i;

		s32 i2 = 0;
		s32 i10 = 0;
		s32 allsize = size2 + size10;
		for (s32 i = 0; i < allsize; i++)
		{
			if (numbers2[i2] > numbers10[i10])
			{
				numbers[i] = numbers2[i2];
				i2++;
			}
			else
			{
				numbers[i] = numbers10[i10];
				i10++;
			}
		}

		u32 d = 1048576;
		u32 p = 9;
		//		numbers[0] = 10;
		//		allsize = 1;
		for (i = 0; i < allsize; i++)
		{
			u32 len = str8::ftoa(numbers[i], ftoa_result, 32, p);
			str8 numberstr(ftoa_result, len);
			float result = numberstr.atof();
			//			sprintf(printfg, "%.*g", p, numbers[i]);
			//			if (strcmp(ftoa_result, printfg))
			//				printf("%s - %.*g - %g %.*g\n", ftoa_result, p, result, numbers[i], p, numbers[i]);
		}
	}

	union IntFloat
	{
		f32 f;
		s32 i;
	};

	bool bTestExhaustive = false;
	if (bTestExhaustive)
	{
		//exhaustively check round trip of all floats that are not nan or inf
		u32 count = 0;
		for (s32 f = 0; f < 0x3f800000; f++)
		{
			IntFloat intf;
			intf.i = f;

			str8 resultstr = str8::format(scratch, "{:.7g}", intf.f);
			float result = resultstr.atof();
			if (result != intf.f)
			{
				//				printf("%g:%g, %g\n", intf.f, result, result - intf.f);
				count++;
			}
			if (!(f % 1000000))
				printf(".");
		}
		printf("Total errors: %d\n", count);
		scratch.Clear();
	}

	bool bTestIntegers = false;
	if (bTestIntegers)
	{
		u32 count = 0;
		for (s32 i = -2147483647; i < 2147480000; i += 2047)
		{
			float number = (float)i;
			str8 resultstr = str8::format(scratch, "{:.7g}", number);
			float result = resultstr.atof();
			u32 len = sprintf_s(printfg, sizeof(printfg), "%.*g", p, number);

			if (resultstr != str8(printfg, len))
			{
				printf("%s - %.*g - %g %.*g \n", ftoa_result, p, result, number, p, number);
				count++;
			}
			scratch.Clear();
		}
		printf("TestIntegers: %d out of %d\n", count, 2147480000 / 2047 * 2);
	}

	bool bTestPowers = false;
	if (bTestPowers)
	{
		double pof10[] = {
			1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30,
			1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20,
			1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,
			1e-09, 1e-08, 1e-07, 1e-06, 1e-05, 1e-04, 1e-03, 1e-02, 1e-01,
			1e0, 1e1, 1e2,1e3,1e4,1e5, 1e6,1e7,1e8, 1e9,
			1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
			1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
			1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38
		};

		for (double powten : pof10)
		{
			float f = 1234.567f;
			//double powten = Jogo::intpow(10.0, i);
			f = (float)(f * powten);
			str8 resultstr = str8::format(scratch, "{:.7g}", f);
			float result = resultstr.atof();

			u32 len = sprintf_s(printfg, sizeof(printfg), "%.*g", p, f);
			if (resultstr != str8(printfg, len))
			{
				printf("%s - %.*g - %g %.*g\n", ftoa_result, p, result, f, p, f);
			}
		}
	}
	bool bTestExhaustiveHex = false;
	if (bTestExhaustiveHex)
	{
		for (u32 i = 0; i < 0xffffffff; i++)
		{
			char hexbuf[9];
			u32 len = str8::itohex(i, hexbuf, sizeof(hexbuf));
			str8 hexstr(hexbuf, len);
			u32 hex = hexstr.hextoi();
			if (i != hex)
			{
				printf("%s %d %d\n", hexbuf, i, hex);
			}
			if (i % 16777216 == 0)
			{
				printf(".");
			}
		}
	}

	bool bTestRandomFloats = false;
	if (bTestRandomFloats)
	{
		u32 count = 0;
		IntFloat intf;
		u32 failcount = 0;
		u32 limit = 10000000;
		for (u32 i = 0; i < limit; i++)
		{
			intf.i = rand.GetNext();
			intf.i &= 0x7effffff;
			str8 fta = str8::format(scratch, "{:.6g}", intf.f);
			char sprintfout[32];
			str8 spf(sprintfout, sprintf_s(sprintfout, sizeof(sprintfout), "%g", intf.f));

			if (fta != spf)
			{
				//				Printf(scratch, "{} != {}", fta, spf);
				failcount++;
				//				Printf(scratch, " -- {:.8g}\n", intf.f);
			}
			scratch.Clear();
		}
		Printf(scratch, "Random Float Fails: {} out of {}\n", failcount, limit);
	}
	str8 s = str8::format(fa, "{    } {{}}", 65, 100);

	Print(str8::format(fa, "This is a test.  I am {    } years old and my name is {{}}.  22/7 = {}\n", 57, 22.0f / 7.0f));
	Print(str8::format(fa, "number: {0:1X}", -55));
	Print(str8::format(fa, "number: {0:2x}", 15));

	bool bTestLog = false;
	if (bTestLog)
	{
		for (s32 i = 0; i < 32; i++)
		{
			float l2 = Jogo::log2((float)(1 << i));
			Print(str8::format(fa, "log of {} is {}", 1 << i, l2));
		}
	}
	bool bTestHundredRandom = false;
	if (bTestHundredRandom)
	{
		Jogo::Random R;
		for (u32 i = 0; i < 100; i++)
		{
			float x = R.GetNext() / 65536.0f;
			Print(str8::format(fa, "{:5.5}", x));
			Print(str8::format(fa, "{:10.4}", x));
			printf("%4.g\n", x);
		}
	}
	IntFloat intf;
	intf.i = 0x7f800000;
	str8::ftoa(intf.f, ftoa_result, 20);
	bool bTestPrecision = false;
	if (bTestPrecision)
	{
		float a = 99998765.0f;
		Printf(fa, "{:.1}", a);
		printf("%.1f\n", a);
		Printf(fa, "{:.2}", a);
		printf("%.2f\n", a);
		Printf(fa, "{:.3}", a);
		printf("%.3f\n", a);
		Printf(fa, "{:.4}", a);
		printf("%.4f\n", a);

		float n = 0.0003f;
		for (int i = 0; i < 20; i++)
		{
			printf("printf: %.2f\n", n);
			Printf(fa, "format: {:.2}\n\n", n);
			n *= 10;
		}

		IntFloat x;
		x.i = 0x5f800000;
		printf("%f\n", x.f);
		char result[64];
		char result2[64];
		str8::f2a(x.f, result);
		printf("%s\n", result);
		u32 l = str8::ftoa(x.f, result, 64);
		result[l] = 0;
		printf("%s\n", result);

		str8::f2a(3.14159265358f, result);
		str8::f2a(-3.14159265358f, result);
		str8::f2a(16777216.0f, result);
		str8::f2a(-16777216.0f, result);
		str8::f2a(-3.14f, result);
		str8::f2a(12345.67890f, result);
		str8::f2a(-12345.67890f, result);
		str8::f2a(268435457.0f, result);
		str8::f2a(2147483520.99999f, result);
		str8::f2a(9999.999f, result);
		str8::f2a(32.45678f, result);
		for (int i = 1; i < 100; i += 5)
		{
			float j = (float)i;
			u32 l = str8::f2a(j, result);
			u32 ll = sprintf_s(result2, 64, "%.0f", j);
			str8 r1 = str8(result).substr(0, l);
			str8 r2 = str8(result2).substr(0, ll);
			if (r1 != r2)
			{
				printf("%s != %s\n", result, result2);
			}
		}
	}
	for (float f = 1.23456e-7f; f < 1234560.0f; f *= 10.0f)
	{
		char result[64];
		char result2[64];
		u32 l = str8::ftoa(f, result, 64);
		u32 ll = sprintf_s(result2, 64, "%f", f);
		str8 r1 = str8(result).substr(0, l);
		str8 r2 = str8(result2).substr(0, ll);
		if (r1 != r2)
		{
			Print(str8::format(fa, "{} != {}\n", r1, r2));
		}
	}
	{
		float f = 5.6e-7f;
		char result[64];
		char result2[64];
		u32 l = str8::ftoa(f, result, 64);
		u32 ll = sprintf_s(result2, 64, "%f", f);
		str8 r1 = str8(result).substr(0, l);
		str8 r2 = str8(result2).substr(0, ll);
		if (r1 != r2)
		{
			Printf(fa, "{} != {}\n", r1, r2);
		}
	}
	Printf(fa, "{:.4}\n", 3.1415926f);
	Printf(fa, "{:8.6e}\n", 0.0f);
	printf("%8.6e\n", 0.0f);

	float x = 9.59595959e-10f;
	for (int i = -10; i < 10; i++)
	{
		printf("%20.2f, %20.2e, %20.2g\n", x, x, x);

		Printf(fa, "{:20.2f}, {:20.2e}, {:20.2g}\n", x, x, x);
		x *= 10;
	}

	TestFloatFormat(fa);

	printf("%.50f\n", FLT_MIN);
	Printf(fa, "{:.50}\n\n", FLT_MIN);

	float subnormal = std::numeric_limits<float>::denorm_min();
	printf("%.50e\n", subnormal);
	Printf(fa, "{:.50e}\n\n", subnormal);

	subnormal *= 10.0f;
	printf("%.50e\n", subnormal);
	Printf(fa, "{:.50e}\n\n", subnormal);

	return 0;
}