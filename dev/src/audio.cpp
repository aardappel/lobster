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

#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "sdlinterface.h"

using namespace lobster;

void AddSoundOps()
{
    STARTDECL(play_wav) (Value &ins)
    {
        bool ok = SDLPlaySound(ins.sval->str(), false);
        ins.DECRT();
        return Value(ok);
    }
    ENDDECL1(play_wav, "filename", "S", "I",
        "plays a sound defined by a wav file (RAW or MS-ADPCM, any bitrate other than 44.1khz 16bit will automatically"
        " be converted on first load). returns false on error");

    STARTDECL(play_sfxr) (Value &ins)
    {
        bool ok = SDLPlaySound(ins.sval->str(), true);
        ins.DECRT();
        return Value(ok);
    }
    ENDDECL1(play_sfxr, "filename", "S", "I",
        "plays a synth sound defined by a .sfs file (use http://www.drpetter.se/project_sfxr.html to generate these)."
        " returns false on error");
}

AutoRegister __aso("sound", AddSoundOps);
