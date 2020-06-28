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
        int ch = SDLPlaySound(ins.sval()->strv(), false, 1.0, loops.True() ? loops.intval() : 0);
        return Value(ch);
    });

nfr("play_sfxr", "filename,loops", "SI?", "I",
    "plays a synth sound defined by a .sfs file (use http://www.drpetter.se/project_sfxr.html"
    " to generate these). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value &ins, Value &loops) {
        int ch = SDLPlaySound(ins.sval()->strv(), true, 1.0, loops.True() ? loops.intval() : 0);
        return Value(ch);
    });

nfr("sound_halt", "channel", "I", "B",
    "terminates sound channel. returns false on error",
    [](StackPtr &, VM &, Value &ch) {
        bool ok = SDLHaltSound(ch.True() ? ch.intval() : -1);
        return Value(ok);
    });

nfr("sound_pause", "channel", "I", "B",
    "pauses sound channel. returns false on error",
    [](StackPtr &, VM &, Value &ch) {
        int ch_idx = ch.intval();
        bool ok = false;
        if (ch_idx != -1)
            ok = SDLPauseSound(ch_idx);
        return Value(ok);
    });

/* not needed since user pauses and can track this by themselves 
nfr("sound_ispaused", "channel", "I", "B",
    "returns true if sound channel is paused, otherwise (or on error) returns false",
    [](StackPtr &, VM &, Value &ch) {
        int ch_idx = ch.intval();
        bool ok = false;
        if (ch_idx != -1)
            ok = SDLIsPausedSound(ch_idx);
        return Value(ok);
    });
*/

nfr("sound_resume", "channel", "I", "B",
    "resumes any sounds that were paused. returns false on error",
    [](StackPtr &, VM &, Value &ch) {
        int ch_idx = ch.intval();
        bool ok = false;
        if (ch_idx != -1)
            ok = SDLResumeSound(ch_idx);
        return Value(ok);
    });

nfr("sound_volume", "channel,volume", "IF", "B",
    "sets the channel volume in the range 0.0..1.0. returns false on error",
    [](StackPtr &, VM &, Value &ch, Value &vol) {
        int ch_idx = ch.intval();
        bool ok = false;
        if (ch_idx != -1)        
            ok = SDLSetVolume(ch_idx, vol.fval());
        return Value(ok);
    });

}
