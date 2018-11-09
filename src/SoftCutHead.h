//
// Created by ezra on 12/6/17.
//

#ifndef CUTFADEVOICE_CUTFADEVOICELOGIC_H
#define CUTFADEVOICE_CUTFADEVOICELOGIC_H

#include <cstdint>
#include "SubHead.h"

namespace  softcut{

    class SoftCutHead {

    public:
        SoftCutHead();
        void init();

        // per-sample update function
        void nextSample(float in, float *outPhase, float *outTrig, float *outAudio);

        void setSampleRate(float sr);
        void setBuffer(float *buf, uint32_t size);
        void setRate(float x);              // set the playback rate (as a ratio)
        void setLoopStartSeconds(float x);  // set the Voice endpoint in seconds
        void setLoopEndSeconds(float x);    // set the Voice start point in seconds
        void setFadeTime(float secs);
        void setLoopFlag(bool val);
        void setRec(float x);
        void setPre(float x);
        void setRecRun(bool val);

        /// FIXME: this method accepts samples and doesn't wrap.
        /// should add something like cutToPos(seconds)
        void cutToPos(float seconds);


        float getActivePhase();
        float getTrig();
        void resetTrig();

        float getRate();

    private:
        // fade in to new position (given in samples)
        // assumption: phase is in range!
        void cutToPhase(float newPhase);
        void takeAction(Action act, int id);
        float mixFade(float x, float y, float a, float b); // mix two inputs with phases

    public:
        typedef enum {
            FADE_LIN, FADE_EQ, FADE_EXP
        } fade_t;

    private:
        SubHead head[2];

        float *buf;     // audio buffer (allocated elsewhere)
        float sr;       // sample rate
        float start;    // start/end points
        float end;
        float fadeInc;  // linear fade increment per sample

        int active;     // current active play sch (0 or 1)
        bool loopFlag;  // set to loop, unset for 1-shot

        fade_t fadeMode; // type of fade to use
        float pre; // pre-record level
        float rec; // record level
        float fadePre; // pre-level modulated by xfade
        float fadeRec; // record level modulated by xfade
        bool recRun;

        float rate;
    };

}
#endif //CUTFADEVOICE_CUTFADEVOICELOGIC_H
