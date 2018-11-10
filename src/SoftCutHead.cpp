//
// Created by ezra on 12/6/17.
//
#include <cmath>
#include <limits>


#include "Interpolate.h"
#include "Resampler.h"

#include "SoftCutHead.h"

using namespace softcut;
using namespace std;

SoftCutHead::SoftCutHead() {
    this->init();
}

void SoftCutHead::init() {
    sr = 44100.f;
    start = 0.f;
    end = 0.f;
    active = 0;
    rate = 1.f;
    setFadeTime(0.1f);
    fadeMode = FADE_EQ;
    recRun = false;
}

void SoftCutHead::processSample(sample_t in, phase_t *outPhase, float *outTrig, sample_t *outAudio) {
    *outAudio = mixFade(head[0].peek(), head[1].peek(), head[0].fade(), head[1].fade());
    *outTrig = head[0].trig() + head[1].trig();

    if(outPhase != nullptr) {
        *outPhase = head[active].phase();
    }

    if(recRun) {
        head[0].poke(in, pre, rec);
        head[1].poke(in, pre, rec);
    }

    Action act0 = head[0].updatePhase(start, end, loopFlag);
    takeAction(act0);
    Action act1 = head[1].updatePhase(start, end, loopFlag);
    takeAction(act1);

    head[0].updateFade(fadeInc);
    head[1].updateFade(fadeInc);
}


void SoftCutHead::setRate(rate_t x)
{
    rate = x;
    calcFadeInc();
    head[0].setRate(x);
    head[1].setRate(x);
}

void SoftCutHead::setLoopStartSeconds(float x)
{
    start = x * sr;
}

void SoftCutHead::setLoopEndSeconds(float x)
{
    end = x * sr;
}

void SoftCutHead::takeAction(Action act)
{
    switch (act) {
        case Action::LOOP_POS:
            cutToPhase(start);
            break;
        case Action::LOOP_NEG:
            cutToPhase(end);
            break;
        case Action::STOP:
        case Action::NONE:
        default: ;;
    }
}

void SoftCutHead::cutToPhase(float pos) {
    State s = head[active].state();

    // ignore if we are already in a crossfade
    if(s == State::FADEIN || s == State::FADEOUT) {
        // std::cout << "skipping phase change due to ongoing xfade" << std::endl;
        return;
    }

    // activate the inactive head
    int newActive = active ^ 1;
    if(s != State::INACTIVE) {
        head[active].setState(State::FADEOUT);
    }

    head[newActive].setState(State::FADEIN);
    head[newActive].setPhase(pos);

    head[active].active_ = false;
    head[newActive].active_ = true;
    active = newActive;
}

void SoftCutHead::setFadeTime(float secs) {
    fadeTime = secs;
    calcFadeInc();
}
void SoftCutHead::calcFadeInc() {
    fadeInc = (float) fabs(rate) / std::max(1.f, (fadeTime * sr));
    fadeInc = std::min(fadeInc, 1.f);
    // printf("fade time = %f; rate = %f; inc = %f\n", fadeTime, rate, fadeInc);
}

void SoftCutHead::setBuffer(float *b, uint32_t bf) {
    buf = b;
    head[0].setBuffer(b, bf);
    head[1].setBuffer(b, bf);
}

void SoftCutHead::setLoopFlag(bool val) {
    loopFlag = val;
}

void SoftCutHead::setSampleRate(float sr_) {
    sr = sr_;
    head[0].setSampleRate(sr);
    head[1].setSampleRate(sr);
}

sample_t SoftCutHead::mixFade(sample_t x, sample_t y, float a, float b) {
#if 1
        return x * sinf(a * (float)M_PI_2) + y * sinf(b * (float) M_PI_2);
#else
        return (x * a) + (y * b);
#endif
}

void SoftCutHead::setRec(float x) {
    rec = x;
}

void SoftCutHead::setPre(float x) {
    pre= x;
}

void SoftCutHead::setRecRun(bool val) {
    recRun = val;
}

phase_t SoftCutHead::getActivePhase() {
  return head[active].phase();
}

float SoftCutHead::getTrig() {
  return head[0].trig()+ head[1].trig();
}

void SoftCutHead::resetTrig() {
  head[0].setTrig(0);
  head[1].setTrig(0);
}

void SoftCutHead::cutToPos(float seconds) {
    cutToPhase(seconds * sr);
}

rate_t SoftCutHead::getRate() {
    return rate;
}
