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

BuiltinGroup sound_builtins;
#define BUILTIN_GROUP sound_builtins
#define BUILTIN_SYM(name) builtin_##name

BUILTIN(play_wav, "filename,loops,prio", "SI?I?", "I",
    "plays a sound defined by a wav file (RAW or MS-ADPCM, any bitrate other than 22050hz 16bit"
    " will automatically be converted on first load). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error")
(StackPtr &, VM &, LString *ins, iint loops, iint prio) {
    int ch = SDLPlaySound(ins->strv(), SOUND_WAV, 1.0, (int)loops, (int)prio);
    return ch;
}

BUILTIN(load_wav, "filename", "S", "B",
    "loads a sound the same way play_sound does, but ahead of playback, to avoid any"
    " delays later. returns false on error")
(StackPtr &, VM &, LString *ins) {
    int ok = SDLLoadSound(ins->strv(), SOUND_WAV);
    return ok;
}

BUILTIN(play_sfxr, "filename,loops,prio", "SI?I?", "I",
    "plays a synth sound defined by a .sfs file (use http://www.drpetter.se/project_sfxr.html"
    " to generate these). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error")
(StackPtr &, VM &, LString *ins, iint loops, iint prio) {
    int ch = SDLPlaySound(ins->strv(), SOUND_SFXR, 1.0, (int)loops, (int)prio);
    return ch;
}

BUILTIN(load_sfxr, "filename", "S", "B",
    "loads a sound the same way play_sfxr does, but ahead of playback, to avoid any"
    " delays later. returns false on error")
(StackPtr &, VM &, LString *ins) {
    int ok = SDLLoadSound(ins->strv(), SOUND_SFXR);
    return ok;
}

BUILTIN(play_ogg, "filename,loops,prio", "SI?I?", "I",
    "plays an ogg file. the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error")
(StackPtr &, VM &, LString *ins, iint loops, iint prio) {
    int ch = SDLPlaySound(ins->strv(), SOUND_OGG, 1.0, (int)loops, (int)prio);
    return ch;
}

BUILTIN(load_ogg, "filename", "S", "B",
    "loads a sound the same way play_ogg does, but ahead of playback, to avoid any"
    " delays later. returns false on error")
(StackPtr &, VM &, LString *ins) {
    int ok = SDLLoadSound(ins->strv(), SOUND_OGG);
    return ok;
}

BUILTIN(play_audio_stream, "freq,channels,prio", "III?", "I",
    "plays an audio stream which can be filled via put_audio_stream()."
    " the default volume is the max volume (1.0)"
    " prio is the priority of the sound which determines whether it can be deleted or not"
    " in case of too many play function calls (defaults to 0)"
    " returns the assigned channel number (1..8) or 0 on error")
(StackPtr &, VM &, iint freq, iint channels, iint prio) {
    auto ch = SDLPlayAudioStream((int)freq, (int)channels, 1.0, (int)prio);
    return ch;
}

BUILTIN(put_audio_stream, "channel,data", "IF]", "B",
    "puts audio samples in range [-1.0,1.0] into an audio stream created with play_audio_stream().")
(StackPtr &, VM &, iint channel, LVector *vdata) {
    vector<float> data;
    data.reserve(vdata->len);
    for (int i = 0; i < vdata->len; i++)
        data.push_back(vdata->AtSt(i)->fltval());
    auto ok = SDLPutAudioStream((int)channel, std::move(data));
    return ok;
}

BUILTIN(has_audio_stream, "channel", "I", "B",
    "returns whether the given channel has an audio stream")
(StackPtr &, VM &, iint channel) {
    return SDLHasAudioStream((int)channel);
}

BUILTIN(audio_stream_available, "channel", "I", "I",
    "returns the number of samples available in the audio stream")
(StackPtr &, VM &, iint channel) {
    auto ok = SDLAudioStreamAvailable((int)channel);
    return ok;
}

BUILTIN(sound_status, "channel", "I", "I",
    "provides the status of the specified sound channel."
    " returns -1 on error or if the channel does not exist, 0 if the channel is free,"
    " 1 if it is playing, and 2 if the channel is active but paused.")
(StackPtr &, VM &, iint ch) {
    int ch_idx = (int)ch;
    if (ch_idx > 0) // we disallow 0 (which would then be -1; all channels in SDL_Mixer) because it is our error value!
        return SDLSoundStatus(ch_idx);
    else
        return -1;
}

BUILTIN(sound_halt, "channel", "I", "",
    "terminates a specific sound channel.")
(StackPtr &, VM &, iint ch) {
    int ch_idx = (int)ch;
    if (ch_idx > 0)
        SDLHaltSound(ch_idx);
}

BUILTIN(sound_pause, "channel", "I", "",
    "pauses the specified sound channel.")
(StackPtr &, VM &, iint ch) {
    int ch_idx = (int)ch;
    if (ch_idx > 0)
        SDLPauseSound(ch_idx);
}

BUILTIN(sound_resume, "channel", "I", "",
    "resumes a sound that was paused.")
(StackPtr &, VM &, iint ch) {
    int ch_idx = (int)ch;
    if (ch_idx > 0)
        SDLResumeSound(ch_idx);
}

BUILTIN(sound_volume, "channel,volume", "IF", "",
    "sets the channel volume in the range 0..1.")
(StackPtr &, VM &, iint ch, double vol) {
    int ch_idx = (int)ch;
    if (ch_idx > 0)
        SDLSetVolume(ch_idx, (float)vol);
}

BUILTIN(sound_position, "channel,vecfromlistener,listenerfwd,attnscale", "IF}:3F}:3F", "",
    "sets the channel volume and panning according to sound in a game world relative to"
    " the listener.")
(StackPtr &, VM &, iint channel, double3 vecfromlistener, double3 listenerfwd, double attnscale) {
    auto scale = (float)attnscale;
    auto fwd = ToVec<float3>(listenerfwd);
    auto src = ToVec<float3>(vecfromlistener);
    auto ch_idx = (int)channel;
    if (ch_idx > 0)
        SDLSetPosition(ch_idx, src, fwd, scale);
}

BUILTIN(sound_no_position, "channel", "I", "",
    "clears the channel position previously set by \"sound_position\"")
(StackPtr &, VM &, iint channel) {
    auto ch_idx = (int)channel;
    if (ch_idx > 0)
        SDLUnsetPosition(ch_idx);
}

BUILTIN(sound_time_length, "channel", "I", "F",
    "returns the length in seconds of the sound playing on this channel")
(StackPtr &, VM &, iint ch_idx) {
    float length = SDLGetTimeLength((int)ch_idx);
    return length;
}

BUILTIN(text_to_speech, "text", "S", "",
    "Queues up text for async text to speech output. Currently on: win32")
(StackPtr &, VM &, LString *text) {
    QueueTextToSpeech(text->strv());
}

BUILTIN(text_to_speech_stop, "", "", "",
    "Stops current text to speech output and clears queue")
(StackPtr &, VM &) {
    StopTextToSpeech();
}

BUILTIN(play_music, "filename,loops", "SI?", "I",
    "plays music in many common formats (WAV, MP3, OGG, etc.). the default volume is the max volume (1.0)"
    " loops is the number of repeats to play (-1 repeats endlessly, omit for no repeats)."
    " returns the music id or 0 on error")
(StackPtr &, VM &, LString *ins, iint loops) {
    int mus_id = SDLPlayMusic(ins->strv(), (int)loops);
    return mus_id;
}

BUILTIN(play_music_fade_in, "filename,ms,loops", "SII?", "I",
    "plays music while fading in over ms milliseconds. See play_music for more info.")
(StackPtr &, VM &, LString *ins, iint ms, iint loops) {
    int mus_id = SDLFadeInMusic(ins->strv(), (int)loops, (int)ms);
    return mus_id;
}

BUILTIN(play_music_cross_fade, "old_mus_id,new_filename,ms,loops", "ISII?", "I",
    "cross-fades new music with existing music over ms milliseconds. See play_music for more info.")
(StackPtr &, VM &, iint old_mus_id, LString *new_ins, iint ms, iint loops) {
    int new_mus_id = SDLCrossFadeMusic((int)old_mus_id, new_ins->strv(), (int)loops, (int)ms);
    return new_mus_id;
}

BUILTIN(music_fade_out, "mus_id,ms", "II", "",
    "fade out music over ms milliseconds.")
(StackPtr &, VM &, iint mus_id, iint ms) {
    SDLFadeOutMusic((int)mus_id, (int)ms);
}

BUILTIN(music_halt, "mus_id", "I", "",
    "stop music with the given id.")
(StackPtr &, VM &, iint mus_id) {
    SDLHaltMusic((int)mus_id);
}

BUILTIN(music_pause, "mus_id", "I", "",
    "pause music with the given id.")
(StackPtr &, VM &, iint mus_id) {
    SDLPauseMusic((int)mus_id);
}

BUILTIN(music_resume, "mus_id", "I", "",
    "resume music with the given id.")
(StackPtr &, VM &, iint mus_id) {
    SDLResumeMusic((int)mus_id);
}

BUILTIN(music_volume, "mus_id,vol", "IF", "",
    "set the music volume in the range 0..1.")
(StackPtr &, VM &, iint mus_id, double vol) {
    SDLSetMusicVolume((int)mus_id, (float)vol);
}

BUILTIN(music_is_playing, "mus_id", "I", "B",
    "returns whether the music with the given id has not yet finished. Paused music is still considered to be playing")
(StackPtr &, VM &, iint mus_id) {
    auto is_playing = SDLMusicIsPlaying((int)mus_id);
    return is_playing;
}

BUILTIN(music_set_general_volume, "vol", "F", "",
    "set the general music volume in the range 0..1.")
(StackPtr &, VM &, double vol) {
    SDLSetGeneralMusicVolume((float)vol);
}

BUILTIN_V(mic_devices, "", "", "I]S]",
    "returns the available microphone devices as two lists: their ids (used with mic_start) and their names")
(StackPtr &sp, VM &vm) {
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
}

BUILTIN(mic_start, "id,freq", "II", "I",
    "start recording audio from the given device id, at the given frequency. Returns a mic_id.")
(StackPtr &, VM &, iint id, iint freq) {
    auto ok = SDLRecordingStart((int)id, (int)freq);
    return ok;
}

BUILTIN(mic_stop, "id", "I", "B",
    "stop recording. Returns false if the mic_id is invalid")
(StackPtr &, VM &, iint id) {
    auto ok = SDLRecordingStop((int)id);
    return ok;
}

BUILTIN(mic_get, "id", "I", "F]",
    "gets audio sample data in float format from the given microphone device.")
(StackPtr &, VM &vm, iint id) {
    auto data = SDLRecordingGet((int)id);
    auto vdata = (LVector *)vm.NewVec(0, (int)data.size(), TYPE_ELEM_VECTOR_OF_FLOAT);
    for (auto val : data) vdata->Push(vm, Value(val));
    return vdata;
}

