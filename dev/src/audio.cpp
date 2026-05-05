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

nfr("play_wav", "filename,loops,prio", "SI?I?", "I",
    "plays a sound defined by a wav file (RAW or MS-ADPCM, any bitrate other than 22050hz 16bit"
    " will automatically be converted on first load). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value ins, Value loops, Value prio) {
        int ch = SDLPlaySound(ins.sval()->strv(), SOUND_WAV, 1.0, loops.intval(), prio.intval());
        return Value(ch);
    });

nfr("load_wav", "filename", "S", "B",
    "loads a sound the same way play_sound does, but ahead of playback, to avoid any"
    " delays later. returns false on error",
    [](StackPtr &, VM &, Value ins) {
        int ok = SDLLoadSound(ins.sval()->strv(), SOUND_WAV);
        return Value(ok);
    });

nfr("play_sfxr", "filename,loops,prio", "SI?I?", "I",
    "plays a synth sound defined by a .sfs file (use http://www.drpetter.se/project_sfxr.html"
    " to generate these). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value ins, Value loops, Value prio) {
        int ch = SDLPlaySound(ins.sval()->strv(), SOUND_SFXR, 1.0, loops.intval(), prio.intval());
        return Value(ch);
    });

nfr("load_sfxr", "filename", "S", "B",
    "loads a sound the same way play_sfxr does, but ahead of playback, to avoid any"
    " delays later. returns false on error",
    [](StackPtr &, VM &, Value ins) {
        int ok = SDLLoadSound(ins.sval()->strv(), SOUND_SFXR);
        return Value(ok);
    });

nfr("play_ogg", "filename,loops,prio", "SI?I?", "I",
    "plays an ogg file. the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value ins, Value loops, Value prio) {
        int ch = SDLPlaySound(ins.sval()->strv(), SOUND_OGG, 1.0, loops.intval(), prio.intval());
        return Value(ch);
    });

nfr("load_ogg", "filename", "S", "B",
    "loads a sound the same way play_ogg does, but ahead of playback, to avoid any"
    " delays later. returns false on error",
    [](StackPtr &, VM &, Value ins) {
        int ok = SDLLoadSound(ins.sval()->strv(), SOUND_OGG);
        return Value(ok);
    });

nfr("play_audio_stream", "freq,channels,prio", "III?", "I",
    "plays an audio stream which can be filled via put_audio_stream()."
    " the default volume is the max volume (1.0)"
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error",
    [](StackPtr &, VM &, Value freq, Value channels, Value prio) {
        auto ch = SDLPlayAudioStream(freq.intval(), channels.intval(), 1.0, prio.intval());
        return Value(ch);
    });

nfr("put_audio_stream", "channel,data", "IF]", "B",
    "puts audio samples in range [-1.0,1.0] into an audio stream created with play_audio_stream().",
    [](StackPtr &, VM &, Value channel, Value vdata) {
        vector<float> data;
        data.reserve(vdata.vval()->len);
        for (int i = 0; i < vdata.vval()->len; i++)
            data.push_back(vdata.vval()->AtSt(i)->fltval());
        auto ok = SDLPutAudioStream(channel.intval(), std::move(data));
        return Value(ok);
    });

nfr("has_audio_stream", "channel", "I", "B",
    "returns whether the given channel has an audio stream",
    [](StackPtr &, VM &, Value channel) {
        return Value(SDLHasAudioStream(channel.intval()));
    });

nfr("sound_status", "channel", "I", "I",
    "provides the status of the specified sound channel."
    " returns -1 on error or if the channel does not exist, 0 if the channel is free,"
    " 1 if it is playing, and 2 if the channel is active but paused.",
    [](StackPtr &, VM &, Value ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0) // we disallow 0 (which would then be -1; all channels in SDL_Mixer) because it is our error value!
            return Value(SDLSoundStatus(ch_idx));
        else
            return Value(-1);
    });

nfr("sound_halt", "channel", "I", "",
    "terminates a specific sound channel.",
    [](StackPtr &, VM &, Value ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLHaltSound(ch_idx);
        return NilVal();
    });

nfr("sound_pause", "channel", "I", "",
    "pauses the specified sound channel.",
    [](StackPtr &, VM &, Value ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLPauseSound(ch_idx);
        return NilVal();
    });

nfr("sound_resume", "channel", "I", "",
    "resumes a sound that was paused.",
    [](StackPtr &, VM &, Value ch) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLResumeSound(ch_idx);
        return NilVal();
    });

nfr("sound_volume", "channel,volume", "IF", "",
    "sets the channel volume in the range 0..1.",
    [](StackPtr &, VM &, Value ch, Value vol) {
        int ch_idx = ch.intval();
        if (ch_idx > 0)
            SDLSetVolume(ch_idx, vol.fltval());
        return NilVal();
    });

nfr("sound_position", "channel,vecfromlistener,listenerfwd,attnscale", "IF}:3F}:3F", "",
    "sets the channel volume and panning according to sound in a game world relative to"
    " the listener.",
    [](StackPtr &sp, VM &) {
        auto scale = Pop(sp).fltval();
        auto fwd = PopVec<float3>(sp);
        auto src = PopVec<float3>(sp);
        auto ch_idx = Pop(sp).intval();
        if (ch_idx > 0)
            SDLSetPosition(ch_idx, src, fwd, scale);
    });

nfr("sound_no_position", "channel", "I", "",
    "clears the channel position previously set by \"sound_position\"",
    [](StackPtr &sp, VM &) {
        auto ch_idx = Pop(sp).intval();
        if (ch_idx > 0)
            SDLUnsetPosition(ch_idx);
    });

nfr("sound_time_length", "channel", "I", "F",
    "returns the length in seconds of the sound playing on this channel",
    [](StackPtr &, VM &, Value ch_idx) {
        float length = SDLGetTimeLength(ch_idx.intval());
        return Value(length);
    });

nfr("text_to_speech", "text", "S", "",
    "Queues up text for async text to speech output. Currently on: win32",
    [](StackPtr &, VM &, Value text) {
        QueueTextToSpeech(text.sval()->strv());
        return NilVal();
    });

nfr("text_to_speech_stop", "", "", "",
    "Stops current text to speech output and clears queue",
    [](StackPtr &, VM &) {
        StopTextToSpeech();
        return NilVal();
    });

nfr("play_music", "filename,loops", "SI?", "I",
    "plays music in many common formats (WAV, MP3, OGG, etc.). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " returns the music id or 0 on error",
    [](StackPtr &, VM &, Value ins, Value loops) {
        int mus_id = SDLPlayMusic(ins.sval()->strv(), loops.intval());
        return Value(mus_id);
    });

nfr("play_music_fade_in", "filename,ms,loops", "SII?", "I",
    "plays music while fading in over ms milliseconds. See play_music for more info.",
    [](StackPtr &, VM &, Value ins, Value ms, Value loops) {
        int mus_id = SDLFadeInMusic(ins.sval()->strv(), loops.intval(), ms.intval());
        return Value(mus_id);
    });

nfr("play_music_cross_fade", "old_mus_id,new_filename,ms,loops", "ISII?", "I",
    "cross-fades new music with existing music over ms milliseconds. See play_music for more info.",
    [](StackPtr &, VM &, Value old_mus_id, Value new_ins, Value ms, Value loops) {
        int new_mus_id = SDLCrossFadeMusic(old_mus_id.intval(), new_ins.sval()->strv(), loops.intval(), ms.intval());
        return Value(new_mus_id);
    });

nfr("music_fade_out", "mus_id,ms", "II", "",
    "fade out music over ms milliseconds.",
    [](StackPtr &, VM &, Value mus_id, Value ms) {
        SDLFadeOutMusic(mus_id.intval(), ms.intval());
        return NilVal();
    });

nfr("music_halt", "mus_id", "I", "",
    "stop music with the given id.",
    [](StackPtr &, VM &, Value mus_id) {
        SDLHaltMusic(mus_id.intval());
        return NilVal();
    });

nfr("music_pause", "mus_id", "I", "",
    "pause music with the given id.",
    [](StackPtr &, VM &, Value mus_id) {
        SDLPauseMusic(mus_id.intval());
        return NilVal();
    });

nfr("music_resume", "mus_id", "I", "",
    "resume music with the given id.",
    [](StackPtr &, VM &, Value mus_id) {
        SDLResumeMusic(mus_id.intval());
        return NilVal();
    });

nfr("music_volume", "mus_id,vol", "IF", "",
    "set the music volume in the range 0..1.",
    [](StackPtr &, VM &, Value mus_id, Value vol) {
        SDLSetMusicVolume(mus_id.intval(), vol.fltval());
        return NilVal();
    });

nfr("music_is_playing", "mus_id", "I", "B",
    "returns whether the music with the given id has not yet finished. Paused music is still considered to be playing",
    [](StackPtr &, VM &, Value mus_id) {
        auto is_playing = SDLMusicIsPlaying(mus_id.intval());
        return Value(is_playing);
    });

nfr("music_set_general_volume", "vol", "F", "",
    "set the general music volume in the range 0..1.",
    [](StackPtr &, VM &, Value vol) {
        SDLSetGeneralMusicVolume(vol.fltval());
        return NilVal();
    });

nfr("mic_devices", "", "", "I]S]",
    "returns the available microphone devices as two lists: their ids (used with mic_start) and their names",
    [](StackPtr &sp, VM &vm) {
        vector<int> ids;
        vector<string> names;
        SDLGetRecordingDeviceNames(ids, names);
        assert(ids.size() == names.size());
        auto vids = (LVector *)vm.NewVec(0, (int)ids.size(), TYPE_ELEM_VECTOR_OF_INT);
        auto vnames = (LVector *)vm.NewVec(0, (int)names.size(), TYPE_ELEM_VECTOR_OF_STRING);
        for (int i = 0; i < (int)ids.size(); ++i) {
            vids->Push(vm, Value(ids[i]));
            vnames->Push(vm, Value(vm.NewString(names[i])));
        }
        Push(sp, Value(vids));
        Push(sp, Value(vnames));
    });

nfr("mic_start", "id,freq", "II", "I",
    "start recording audio from the given device id, at the given frequency. Returns a mic_id.",
    [](StackPtr &, VM &, Value id, Value freq) {
        auto ok = SDLRecordingStart(id.intval(), freq.intval());
        return Value(ok);
    });

nfr("mic_stop", "id", "I", "B",
    "stop recording. Returns false if the mic_id is invalid",
    [](StackPtr &, VM &, Value id) {
        auto ok = SDLRecordingStop(id.intval());
        return Value(ok);
    });

nfr("mic_get", "id", "I", "F]",
    "gets audio sample data in float format from the given microphone device.",
    [](StackPtr &, VM &vm, Value id) {
        auto data = SDLRecordingGet(id.intval());
        auto vdata = (LVector *)vm.NewVec(0, (int)data.size(), TYPE_ELEM_VECTOR_OF_FLOAT);
        for (auto val : data) vdata->Push(vm, Value(val));
        return Value(vdata);
    });

}

