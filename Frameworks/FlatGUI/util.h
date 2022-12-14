/* -*- mode: c++ -*- */
#pragma once

//#include <sys/time.h>
//#include <cstdint>
#include <string>
#include <time.h>
#include <cassert>


#define UNCOPYABLE(T) \
  private: \
  T(const T&) = delete; \
  T& operator=(const T&) = delete;

typedef unsigned char TUnsigned7; // Value 0..127
typedef char TSigned7; // Value -64..63
typedef float TFraction; // Value 0..1
typedef float TFrequency; // in Hz
typedef float TTime; // in ms

static inline float octaves(float x)
{
    return x;
}
static inline float semitones(float x)
{
    return x / 12.0f;
}
static inline float cents(float x)
{
    return x / 1200.0f;
}
//static inline int ms_to_samples(int x)
//{
//    return (SampleRate * x) / 1000;
//}
static inline TUnsigned7 hi7(int v)
{
    return (v >> 7) & 0x7f;
}
static inline TUnsigned7 lo7(int v)
{
    return v & 0x7f;
}


static inline float fpow2(const float y)
{
    // musicdsp.org, Johannes M.-R.
    union
    {
        float f;
        int i;
    } c;

    int integer = (int) y;
    if (y < 0) integer = integer - 1;

    float frac = y - (float) integer;

    c.i = (integer + 127) << 23;
    c.f *= 0.33977f * frac * frac + (1.0f - 0.33977f) * frac + 1.0f;

    return c.f;
}

// Map 0..127 to 1..17696 Hz
//#define VAL2HZ_HI(n) exp2f(n/9.0f)
// Map 0..127 to 12..17955 Hz, same scale as Blofeld
#ifdef __MACH__
#define VAL2HZ_HI(n) (12*__exp10f(n/40.0f))
#else
#define VAL2HZ_HI(n) (12*exp10f(n/40.0f))
#endif
// Map 0..127 to 0.19..1261 Hz
#define VAL2HZ_LO(n) exp2f((n-24)/12.0f)
// Map 0..127 to 0.35..21247 ms
#define VAL2MS(n) exp2f((n-12)/8.0f)

#define NOTE2HZ(note) (exp2f((float(note) - TGlobal::MidiNoteA4) / 12.0) * TGlobal::HzA4)


template<typename T>
class THysteresis
{
//UNCOPYABLE(THysteresis);

public:
    THysteresis(T range, T v) : Range(range), Value(v) {}
    THysteresis& operator=(T v) { if (v > Value + Range || v < Value - Range) Value = v; return *this; }
    const T Get() const { return Value; }
    operator const T() const { return Value; }
private:
    T Range;
    T Value;
};



template<typename T>
T clamp(T v, T min, T max)
{
    return std::max(std::min(v, max), min);
}

template<typename T>
T linterpolate(const T& s0, const T& s1, float pos)
{
    float frac = pos - int(pos);
    return s0 * (1 - frac) + s1 * frac;
}

template<typename T>
T linterpolate(const T* table, int tablelen, double pos)
{
    const unsigned intpos = unsigned(pos);
    const T s0 = table[intpos % tablelen];
    const T s1 = table[(intpos + 1) % tablelen];
    const float frac = pos - intpos;
    return s0 * (1.0f - frac) + s1 * frac;
}

template<typename T>
T mix(T a, T b, TFraction mix)
{
    return a * (1.0f - mix) + b * mix;
}
