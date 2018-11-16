//
// Created by emb on 11/10/18.
//

#ifndef SOFTCUT_SOFTCUT_H
#define SOFTCUT_SOFTCUT_H

#include "SoftCutVoice.h"

namespace softcut {

    class SoftCut {


    public:
        enum { numVoices = 2 };
        enum { bufFrames = 16777216 };
        enum { maxBlockSize = 8192 };

        int getNumVoices() { return numVoices; }

        SoftCut();

        void init();

        void processBlock(const float *in0, const float* in1, float *out0, float* out1, int numFrames);

        void setSampleRate(unsigned int i);

        void setRate(int voice, float rate);
        void setLoopStart(int voice, float sec);
        void setLoopEnd(int voice, float sec);
        void setLoopFlag(int voice, bool val);
        void setFadeTime(int voice, float sec);
        void setRecLevel(int voice, float amp);
        void setPreLevel(int voice, float amp);
        void setRecFlag(int voice, bool val);
		void cutToPos(int voice, float sec);

        void setFilterFc(int voice, float);
        void setFilterRq(int voice, float);
        void setFilterLp(int voice, float);
        void setFilterHp(int voice, float);
        void setFilterBp(int voice, float);
        void setFilterBr(int voice, float);
        void setFilterDry(int voice, float);
        void setFilterFcMod(int voice, float x);

        // FIXME: yes this is an ugly API, but easiest r/n to keep consistent count of args for all setters
		void setAmpLeft(int voice, float x);
		void setAmpRight(int voice, float x);

		void setPreFadeWindow(float x);
        void setRecFadeDelay(float x);
        void setPreFadeShape(float x);
        void setRecFadeShape(float x);

    private:
        SoftCutVoice scv[numVoices];
        float outAmp[numVoices][2];
        float outBus[maxBlockSize];
		float buf[bufFrames];
    };
}


#endif //SOFTCUT_SOFTCUT_H
