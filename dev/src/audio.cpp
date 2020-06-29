// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#include "lobster/sdlinterface.h"

using namespace lobster;

void AddSound(NativeRegistry &nfr) {

nfr("play_wav", "filename,loops", "SI?", "I",
    "plays a sound defined by a wav file (RAW or MS-ADPCM, any bitrate other than 22050hz 16bit"
    " will automatically be converted on first load). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value &ins, Value &loops) {
        int ch = SDLPlaySound(ins.sval()->strv(), false, 1.0, loops.intval());
        return Value(ch);
    });

nfr("play_sfxr", "filename,loops", "SI?", "I",
    "plays a synth sound defined by a .sfs file (use http://www.drpetter.se/project_sfxr.html"
    " to generate these). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value &ins, Value &loops) {
        int ch = SDLPlaySound(ins.sval()->strv(), true, 1.0, loops.intval());
        return Value(ch);
    });

nfr("sound_halt", "channel", "I", "",
    "terminates a specific sound channel.",
    [](StackPtr &, VM &, Value &ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0) // we disallow 0 (which would then be -1; all channels in SDL_Mixer) because it is our error value!
            SDLHaltSound(ch_idx);
        return Value();
    });

nfr("sound_pause", "channel", "I", "",
    "pauses the specified sound channel.",
    [](StackPtr &, VM &, Value &ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLPauseSound(ch_idx);
        return Value();
    });

nfr("sound_resume", "channel", "I", "",
    "resumes a sound that was paused.",
    [](StackPtr &, VM &, Value &ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLResumeSound(ch_idx);
        return Value();
    });

nfr("sound_volume", "channel,volume", "IF", "",
    "sets the channel volume in the range 0..1.",
    [](StackPtr &, VM &, Value &ch, Value &vol) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLSetVolume(ch_idx, vol.fltval());
        return Value();
    });

}
