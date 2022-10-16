// nusofting.com 2019

#pragma once

#include <math.h>


#ifndef FLT_LIM
#define FLT_LIM         1.175494351e-29F        /* min positive value */
#endif
#ifndef SNAPPING_ZERO
#define SNAPPING_ZERO(value) value = !(value > FLT_LIM || value < -FLT_LIM)? 0.0f : value
#define AVOID_CLOSE_TO_ZERO(value) !(value > 1.0e-24f || value < -1.0e-24f)? 0.0f : value
#endif

#ifndef undenormalise
#define undenormalise(sample) SNAPPING_ZERO(sample)
#endif

#ifndef denormal
#define denormal(sample) SNAPPING_ZERO(sample)
#endif



//#define TTAntiDenormal(value) dsp_sin(!(value > FLT_LIM || value < -FLT_LIM)? 0.0f : 1.4f*value)
