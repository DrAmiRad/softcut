//
// Created by ezra on 11/10/18.
//

#ifndef SOFTCUT_SLEW_H
#define SOFTCUT_SLEW_H

#pragma once

#include <cassert>
#include <array>
#include <math.h>

/////////////////////////////////
#pragma mark constants, conversions, basic utilities

namespace softcut {

#ifndef BUILD_SC_UGEN // supercollider headers define these themselves
    static const float log001 = -6.9078;

    inline float zapgremlins(float x) {
        float absx = fabs(x);
        return (absx > (float) 1e-15 && absx < (float) 1e15) ? x : 0.f;
    }
#endif

    static float sign(float x) {
        return x > 0.f ? 1.f : -1.f;
    }

// convert a time-to-convergence to a pole coefficient
// "ref" argument defines the amount of convergence
// target ratio is e^ref.
// -6.9 corresponds to standard -60db convergence
    static float tau2pole(float t, float sr, float ref = -6.9) {
        return exp(ref / (t * sr));
    }

// one-pole lowpass smoother
// define this here for a consistent interpretation of pole coefficient
// we use faust's definition where b=0 is instant, b in [0, 1)
    static float smooth1pole(float x, float x0, float b) {
        // return x * (1.f - b) + b * x0;f
        // refactored
        return x + (x0 - x) * b;
    }

    static float dbamp(float db) {
        return isinf(db) ? 0.f : powf(10.f, db * 0.05);
    }

    static float ampdb(float amp) {
        return log10(amp) * 20.f;
    }

    template<typename T, size_t size>
    class RunningAverage {
    private:
        // history
        std::array<T, size> values;
        // index of oldest value
        size_t oldIdx;
        // index of most recent value
        size_t newIdx;
        // runnning sum
        T sum;

        void advanceIdx(size_t &idx) {
            if (++idx >= size) {idx = 0;}
        }

    public:
        RunningAverage() {
            assert(size >= 2);
            values.fill(0);
            oldIdx = 1;
            newIdx = 0;
            sum = (T) 0;
        }

        T update(T value) {
            values[newIdx] = value;
            sum += value;
            sum -= values[oldIdx];
            advanceIdx(oldIdx);
            advanceIdx(newIdx);
            return sum / size;
        }
    };


    class LinearRamp {
    private:

        float sampleRate;
        float target;
        float inc;
        float time;
        float val;
        bool rising;

    private:
        void calcInc() {
            float d = target - val;
            rising = d > 0.f;
            inc = d / (time * sampleRate);
        }

    public:
        void setSampleRate(float sr) {
            sampleRate = sr;
            setTime(time);
        }

        void setTime(float t) {
            time = t;
            calcInc();
        }

        void setTarget(float t) {
            target = t;
            calcInc();
        }

        LinearRamp(float sr, float t = 0.0001) :
                sampleRate(sr), target(0.f), val(0.f), rising(false) {
            setTime(t);
        }

        float process(float target) {
            setTarget(target);
            val += inc;
            if (rising) {
                if (val > target) {
                    val = target;
                }
            } else {
                if (val < target) {
                    val = target;
                }
            }
            return val;
        }
    };

// logarithmic interpolator (aka 1-pole LPF)
    class LogRamp {
        float sampleRate;
        float time;
        float b;
        float x0;
        float y0;
    public:
        LogRamp(float sr, float t = 0.001) : sampleRate(sr), b(1.f), x0(0.f), y0(0.f) {
            sampleRate = sr;
            setTime(t);
        }

        ~LogRamp() {
        }

        void setSampleRate(float sr) {
            sampleRate = sr;
            setTime(time);
        }

        // in this case, _t_ is the time for the filter to converge within -60dB of target
        void setTime(float t) {
            time = t;
            b = tau2pole(t, sampleRate);
        }

        // update input only
        void setTarget(float x) {
            x0 = x;
        }

        // update output only
        float update() {
            float y = smooth1pole(x0, y0, b);
            y0 = y;
            return y;
        }

        // update input and output
        float process(float x) {
            setTarget(x);
            return this->update();
        }

        float getTarget() {
            return x0;
        }

    };

// a smoother with separate rise and fall times
// "ye old AFG"
    class Slew {
    private:
        float sampleRate;
        float x0;
        float bR;
        float bF;
        float tR;
        float tF;
    public:
        Slew(float sr, float rise = 0.2, float fall = 0.2) :
            sampleRate(sr), x0(0.f) {
            setRiseTime(rise);
            setFallTime(fall);
        }


        void setSampleRate(float sr) {
            sampleRate = sr;
            setRiseTime(tR);
            setFallTime(tF);
        }

        float process(float x) {
            float b = x > x0 ? bR : bF;
            float y = smooth1pole(x, x0, b);
            // FIXME: zapgremlins is weirdly slow (at least on x86)
            //x0 = zapgremlins(y);
            x0 = y;
            return y;
        }

        void setRiseTime(float t) {
            tR = t;
            bR = tau2pole(tR, sampleRate);
        }

        void setFallTime(float t) {
            tF = t;
            bF = tau2pole(tF, sampleRate);
        }
    };


}

#endif //SOFTCUT_SLEW_H
