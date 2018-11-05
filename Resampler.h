//
// Created by ezra on 4/21/18.
//

#ifndef SOFTCUTHEAD_RESAMPLER_H
#define SOFTCUTHEAD_RESAMPLER_H

#include <iostream>
#include <cmath>

// ultra-simple linear-interpolated resampling class
// works on mono output buffer and processes one input sample at a time

namespace softcut {
    
    static int wrap(int val, int bound) {
        int x = val;
        while(x >= bound) { x -= bound; }
        while(x < 0) { return x += bound; }
        return x;
    }

    class Resampler {

    private:
        // output buffer
        float *buf_;
        // total frames in output buffer
        int bufFrames_;
        // last output frame
        int frame_;
        // last written phase
        double phase_;
        // last input value
        float x_;
        // output:input ratio
        double rate_;

    private:
        // write multiple output frames, interpolating between two values
        // FIXME: allow higher-order interpolation
        void writeInterp(float x, int n) {
            int i = frame_;                 // index into buffer
            double m = (x - x_) / rate_;    // slope
            double y;                       // interpolated value
            // interpolate up to first frame boundary; distance is 1-(old phase)
            y = x_ + m * (1.0 - phase_);
            buf_[i] = y;
            n--;
            // for the rest,
            while (n > 0) {
                y += m;

                i = (i + 1) % bufFrames_;
                buf_[i] = y;
                n--;
            }
        }

// write, upsampling
// return frames written (>= 1)
// assume rate_ > 1.0
        int writeUp(float x) {
            double phase = phase_ + rate_;
            int nframes = (int) phase;
            writeInterp(x, nframes);
            phase_ = phase - std::floor(phase);
            frame_ = (frame_ + nframes) % bufFrames_;
            x_ = x;
            return nframes;
        }

// write, downsampling
// return frames written (0 or 1)
// we assume rate_ < 1.0
        int writeDown(float x) {
            double phase = phase_ + rate_;
            int nframes = (int) phase;
            if (nframes > 0) {
                // use linear interpolation from last written value
                // FIXME: use higher order interpolation.
                // this would require enforcing a higher minimum latency...
                double m = (x - x_) / rate_;
                buf_[frame_] = x + (m * (1.0 - phase_));
                frame_ = (frame_ + 1) % bufFrames_;
            }
            phase_ = phase - std::floor(phase);
            x_ = x;
            return nframes;
        }

        void write(float x) {
            buf_[frame_] = x;
            frame_ = (frame_ + 1) % bufFrames_;
        }

    public:

        // constructor
        Resampler(float *buf, int frames) :
                buf_(buf), bufFrames_(frames),
                rate_(1.0), phase_(0.0), frame_(0) {}

        int processFrame(float x) {
            //frame_ = wrap(frame_, bufFrames_);
            frame_ %= bufFrames_;
            if (rate_ > 1.0) {
                return writeUp(x);
            } else if (rate_ < 1.0) {
                return writeDown(x);
            } else {
                write(x);
                return 1;
            }
        }

        void setRate(double r) {
            rate_ = r;
        }

        void setBuffer(float *buf, int frames) {
            buf_  = buf;
            bufFrames_ = frames;
        }
        void setPhase(double phase) { phase_ = phase; }

        float* buffer() { return buf_; }
        int bufferFrames() { return bufFrames_; }

        int frame() { return frame_; }

        void reset() {
            frame_ = 0;
            x_ = 0.f;
        }
    };

}

#endif //SOFTCUTHEAD_RESAMPLER_H

