#include "JMath.h"

namespace Jogo
{
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

	float tangent(float x)
	{
		return sine(x) / cosine(x);
	}

	float arctangent(float x)
	{
		const float PIOVER6 = PI / 6.0f;
		const float PIOVER12 = PI / 12.0f;
		const float ROOT3OVER3 = 0.577350269f;
		const float TWOMINUSROOT3 = 0.267949192f;
		const float c0 = 0.999999020228907f;
		const float c1 = 0.257977658811405f;
		const float c2 = 0.59120450521312f;

		float a = x;
		bool neg = x < 0;
		if (neg)
			a = -x;

		bool inv = a > 1.0f;
		if (inv)
			a = 1 / a;

		bool high = a > TWOMINUSROOT3;
		if (high)
			a = (a - ROOT3OVER3) / (1 + ROOT3OVER3 * a);

		float aa = a * a;
		a = a * (c0 + c1 * aa) / (1 + c2 * aa);

		if (high)
			a += PIOVER6;

		if (inv)
			a = PIOVER2 - a;

		if (neg)
			a = -a;

		return a;
	}

	float arctangent(float x, float y)
	{
		if (x == 0)
		{
			if (y < 0)
				return -PIOVER2;
			return PIOVER2;
		}

		float a = arctangent(y / x);
		if (x < 0)
			a += PI;

		return a;
	}

	// exp2/log2 functions adapted from here:
	// http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html
	// https://github.com/etheory/fastapprox/tree/master/fastapprox/src

	float log2(float x)
	{
		if (x < 0)
		{
			IntFloat nan;
			nan.i = 0xffffffff;
			return nan.f;
		}

		IntFloat f = { x };
		IntFloat mx;
		mx.i = (f.i & 0x7fffff) | 0x3f000000;

		float y = f.i * (1.0f / (1<<23));
		return y - 124.22551499f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
	}

	float log(float x)
	{
		return 0.69314718f * log2(x);
	}

	float exp2(float x)
	{
		float offset = (x < 0) ? 1.0f : 0.0f;
		float clipp = (x < -126) ? -126.0f : x;
		s32 w = (s32)clipp;
		float z = clipp - w + offset;
		IntFloat v;
		v.i = (u32)((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z));

		return v.f;
	}

	float exp(float x)
	{
		return exp2(1.442695040f * x);
	}

	float pow(float x, float y)
	{
		return exp2(y * log2(x));
	}

	u64 Plane::ClipPoly(u64 numVerts, Vector3* pIn, Vector3* pOut)
	{

		int nextVert;
		bool curIn, nextIn;
		float curDot, nextDot, scale;
		Vector3* pInVert;
		Vector3* pOutVert;

		pInVert = pIn;
		pOutVert = pOut;

		curDot = Vector3::Dot(*pInVert, Normal);
		curIn = (curDot >= Distance);

		for (u32 i = 0; i < numVerts; i++)
		{
			nextVert = (i + 1) % numVerts;

			// if current vert is in, keep it
			if (curIn)
				*pOutVert++ = *pInVert;

			nextDot = Vector3::Dot(pIn[nextVert], Normal);
			nextIn = (nextDot >= Distance);

			// add a clipped vert if one end is inside and the other outside
			if (curIn != nextIn)
			{
				scale = (Distance - curDot) / (nextDot - curDot);
				pOutVert->x = pInVert->x + (pIn[nextVert].x - pInVert->x) * scale;
				pOutVert->y = pInVert->y + (pIn[nextVert].y - pInVert->y) * scale;
				pOutVert->z = pInVert->z + (pIn[nextVert].z - pInVert->z) * scale;
				pOutVert++;
			}

			curDot = nextDot;
			curIn = nextIn;
			pInVert++;
		}

		u64 verts = pOutVert - pOut;
		if (verts < 3)
			return 0;
		return verts;
	}

	// TODO: pass in an outcode for this 
	u64 Frustum::ClipPoly(u32 numVerts, Vector3* pIn, Vector3* pOut)
	{
		u64 verts = numVerts;

		// TODO: use scratch allocation from an arena instead of alloca
		Vector3* a = (Vector3*)alloca(sizeof(Vector3) * (numVerts + 6));

		verts = planes[0].ClipPoly(verts, pIn, a);
		if (!verts) return 0;
		verts = planes[1].ClipPoly(verts, a, pOut);
		if (!verts) return 0;
		verts = planes[2].ClipPoly(verts, pOut, a);
		if (!verts) return 0;
		verts = planes[3].ClipPoly(verts, a, pOut);
		if (!verts) return 0;
		verts = planes[4].ClipPoly(verts, pOut, a);
		if (!verts) return 0;
		verts = planes[5].ClipPoly(verts, a, pOut);

		return verts;
	}

	Frustum Camera::GetFrustum()
	{
		return Frustum{};
	}

};