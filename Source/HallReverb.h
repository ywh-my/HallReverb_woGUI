/**
 *  ElephantDSP.com Hall Reverb
 *
 *  Copyright (C) 2022 Christian Voigt
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

#include "freeverb/earlyref.hpp"
#include "freeverb/zrev2.hpp"

class HallReverb
{
public:
    HallReverb();

    void setSampleRate(float newSampleRate);
    void process(const float* leftChannelIn, const float* rightChannelIn, float* leftChannelOut, float* rightChannelOut, int numSamples);
    void mute();

    // output
    void setDryLevel(float newDryLevel);
    void setEarlyLevel(float newEarlyLevel);
    void setEarlySendLevel(float newEarlySend);
    void setLateLevel(float newLateLevel);

    // early reflections
    void setEarlyOutputHPF(float newEarlyOutputHPF);
    void setEarlyOutputLPF(float newEarlyOutputLPF);
    void setEarlyRoomSize(float newEarlyRoomSize);
    void setEarlyStereoWidth(float newEarlyStereoWidth);

    // late reverb
    void setLateApFeedback(float newLateApFeedback);
    void setLateCrossOverFreqHigh(float newLateCrossOverFreqHigh);
    void setLateCrossOverFreqLow(float newLateCrossOverFreqLow);
    void setLateDecay(float newLateDecay);
    void setLateDecayFactorHigh(float newLateDecayFactorHigh);
    void setLateDecayFactorLow(float newLateDecayFactorLow);
    void setLateDiffusion(float newLateDiffusion);
    void setLateLFO1Freq(float newLateLFO1Freq);
    void setLateLFO2Freq(float newLateLFO2Freq);
    void setLateLFOFactor(float newLateLFOFactor);
    void setLateOutputHPF(float newLateOutputHPF);
    void setLateOutputLPF(float newLateOutputLPF);
    void setLatePredelay(float newLatePredelay);
    void setLateRoomSize(float newLateRoomSize);
    void setLateSpin(float newLateSpin);
    void setLateSpinFactor(float newLateSpinFactor);
    void setLateStereoWidth(float newLateStereoWidth);
    void setLateWander(float newLateWander);

private:
    void ensureBufferCapacity(int numSamples);

    float dryLevel;
    float earlyLevel;
    float earlySendLevel;
    float lateLevel;

    bool earlyRoomSizeNeedsUpdate = false;
    float earlyRoomSize;
    bool lateRoomSizeNeedsUpdate = false;
    float lateRoomSize;
    bool latePredelayNeedsUpdate = false;
    float latePredelay;

    std::vector<float> leftBufferIn;
    std::vector<float> rightBufferIn;
    std::vector<float> leftEarlyOut;
    std::vector<float> rightEarlyOut;
    std::vector<float> leftLateIn;
    std::vector<float> rightLateIn;
    std::vector<float> leftLateOut;
    std::vector<float> rightLateOut;

    fv3::earlyref_f early;
    fv3::zrev2_f late;
};
