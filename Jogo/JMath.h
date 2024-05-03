#pragma once
#include "int_types.h"
#include <intrin.h>

s32 AsInteger(float f) { return *(s32*)&f; }
float AsFloat(s32 i) { return *(float*)&i; }
const s32 OneAsInteger = AsInteger(1.0f);
float asqrt(float x)
{
	float approx = AsFloat((AsInteger(x) >> 1) + (OneAsInteger >> 1));
	return (approx * approx + x) / (2.0f * approx);
}

float mmsqrt(float x)
{
	__m128 m;
	m.m128_f32[0] = x;
	m = _mm_sqrt_ss(m);
	return m.m128_f32[0];
}

const float PI = 3.14159265358979323f;
const float PIOVER2 = PI / 2.0f;
const float PIOVER4 = PI / 4.0f;
float sine(float x)
{
	// fold input into one octant and keep track of where we were

}

float cosine(float x)
{
	// fold input 
}