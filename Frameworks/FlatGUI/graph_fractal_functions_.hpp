#ifndef fastroot
static inline float fastroot(float f)
{
	long *lp, l;

	lp = (long*)(&f);

	l = *lp;
	l -= 0x3F800000l;
	l >>= 1; l += 0x3F800000l;

	*lp = l;

	return f;
}
#endif

#pragma once

#include <math.h>

float dot(const float& x1, const float& y1, const float& x2, const float& y2)
{
	return fastroot(x1*x2 + y1*y2);
	//return x1*x2 + y1*y2;
}
float fract(const float& v)
{
	return v - (int)v;
}
float mix(float a, float b, float v)
{
	return a + v*(b - a);
}
float noise(const float& x, const float& y, const float& scale)
{
	float ix = scale * (float)x;
	float iy = scale * (float)y;
	float fx = fract(ix);
	float fy = fract(iy);
	ix -= fx; 
	iy -= fy; const float a = 12.9893f; const float b = 89.2337f; const float c = 14371.54531f;
	float ns_lb = fract(sin(dot(ix		, iy		, a, b))* c);
	float ns_rb = fract(sin(dot(ix + 1.f, iy		, a, b))* c);
	float ns_lt = fract(sin(dot(ix		, iy + 1.f	, a, b))* c);
	float ns_rt = fract(sin(dot(ix + 1.f, iy + 1.f	, a, b))* c);

	return mix(mix(ns_lb, ns_rb, fx), mix(ns_lt, ns_rt, fx), fy);
}
float fractal(const float& x, const float& y, const float& scale, const size_t& octaves, const float& lacunarity, const float& increment)
{
	float tmp = 0.0f; 
	float noisemap = 0.0f;
	float frq = 1.0f;
	float fx = x;
	float fy = y;
	for(size_t i = 0; i < octaves; ++i)
	{
		tmp = max(-1.0f, noise(fx, fy, scale) * -pow(frq,-increment)); //(sin(fx)/cos(fy))
		noisemap = noisemap + fabs(tmp);
		frq = frq*lacunarity;
		fx *= lacunarity;	
		fy *= lacunarity;	
		fx += cos(noisemap*4.0f)*53.0f;	//you can have fun with this! No need for the additions...
		fy += sin(noisemap*4.0f)*53.0f;	//you can have fun with this! No need for the additions...
	}
	return min(1.0f,noisemap*0.5f);
}

