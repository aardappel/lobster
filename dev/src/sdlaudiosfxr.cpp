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

#include "lobster/sdlincludes.h"
#include "lobster/sdlinterface.h"

#include "SDL3_mixer/SDL_mixer.h"

static const int sound_track_count = 16;

static MIX_Mixer *mixer;
static MIX_Track *sound_tracks[sound_track_count];
static int sound_pri[sound_track_count];
static uint64_t sound_age[sound_track_count];
static uint64_t sounds_played = 0;

struct Music {
    MIX_Track *track;
    float vol;
};

static vector<Music> musics;
static float music_volume = 1.f;

struct Recording {
    int device_id;
    unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)> stream;
    explicit Recording(int device_id, SDL_AudioStream *stream)
        : device_id(device_id), stream(stream, SDL_DestroyAudioStream) {}
};
static vector<Recording> recordings;

struct Audio {
    unique_ptr<MIX_Audio, decltype(&MIX_DestroyAudio)> audio;
    Audio() : audio(nullptr, MIX_DestroyAudio) {}
    explicit Audio(MIX_Audio *audio) : audio(audio, MIX_DestroyAudio) {}
};
static map<string, Audio, less<>> audio_files;

bool SDLSoundInit() {
    if (mixer) return true;
    if (!MIX_Init()) {
        LOG_ERROR("MIX_Init: ", SDL_GetError());
        return false;
    }
    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 44100 };
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (!mixer) {
        LOG_ERROR("MIX_CreateMixerDevice: ", SDL_GetError());
        return false;
    }
    for (int i = 0; i < sound_track_count; ++i) {
        sound_tracks[i] = MIX_CreateTrack(mixer);
    }
    return true;
}

void SDLSoundClose() {
    for (auto &r : recordings) {
        if (r.device_id >= 0) {
            SDL_CloseAudioDevice(r.device_id);
            r.stream.reset();
        }
    }
    MIX_Quit();
}

static MIX_Audio *AllocAudio(short *buf, size_t num_samples) {
    auto *io = SDL_IOFromMem(buf, num_samples * 2);
    SDL_AudioSpec spec = { SDL_AUDIO_S16, 2, 44100 };
    auto *audio = MIX_LoadRawAudio_IO(mixer, io, &spec, true);
    if (!audio) {
        LOG_ERROR("MIX_LoadRawAudio_IO: ", SDL_GetError());
    }
    return audio;
}

static MIX_Audio *RenderSFXR(string_view buf) {
    int wave_type = 0;

    float p_base_freq = 0.3f;
    float p_freq_limit = 0.0f;
    float p_freq_ramp = 0.0f;
    float p_freq_dramp = 0.0f;
    float p_duty = 0.0f;
    float p_duty_ramp = 0.0f;

    float p_vib_strength = 0.0f;
    float p_vib_speed = 0.0f;
    float p_vib_delay = 0.0f;

    float p_env_attack = 0.0f;
    float p_env_sustain = 0.3f;
    float p_env_decay = 0.4f;
    float p_env_punch = 0.0f;

    bool filter_on = false;
    float p_lpf_resonance = 0.0f;
    float p_lpf_freq = 1.0f;
    float p_lpf_ramp = 0.0f;
    float p_hpf_freq = 0.0f;
    float p_hpf_ramp = 0.0f;

    float p_pha_offset = 0.0f;
    float p_pha_ramp = 0.0f;

    float p_repeat_speed = 0.0f;

    float p_arp_speed = 0.0f;
    float p_arp_mod = 0.0f;

    float master_vol = 0.05f;

    float sound_vol = 0.5f;

    int phase;
    double fperiod;
    double fmaxperiod;
    double fslide;
    double fdslide;
    int period;
    float square_duty;
    float square_slide;
    int env_stage;
    int env_time;
    int env_length[3];
    float env_vol;
    float fphase;
    float fdphase;
    int iphase;
    float phaser_buffer[1024];
    int ipp;
    float noise_buffer[32];
    float fltp;
    float fltdp;
    float fltw;
    float fltw_d;
    float fltdmp;
    float fltphp;
    float flthp;
    float flthp_d;
    float vib_phase;
    float vib_speed;
    float vib_amp;
    int rep_time;
    int rep_limit;
    int arp_time;
    int arp_limit;
    double arp_mod;

    auto file = buf.data();
    auto fread_mem = [&](auto &dest) {
        assert(file < buf.data() + buf.size());
        memcpy(&dest, file, sizeof(dest));
        file += sizeof(dest);
    };
    int version = 0;
    fread_mem(version);
    if (version != 102) return nullptr;
    fread_mem(wave_type);
    fread_mem(sound_vol);
    fread_mem(p_base_freq);
    fread_mem(p_freq_limit);
    fread_mem(p_freq_ramp);
    fread_mem(p_freq_dramp);
    fread_mem(p_duty);
    fread_mem(p_duty_ramp);
    fread_mem(p_vib_strength);
    fread_mem(p_vib_speed);
    fread_mem(p_vib_delay);
    fread_mem(p_env_attack);
    fread_mem(p_env_sustain);
    fread_mem(p_env_decay);
    fread_mem(p_env_punch);
    fread_mem(filter_on);
    fread_mem(p_lpf_resonance);
    fread_mem(p_lpf_freq);
    fread_mem(p_lpf_ramp);
    fread_mem(p_hpf_freq);
    fread_mem(p_hpf_ramp);
    fread_mem(p_pha_offset);
    fread_mem(p_pha_ramp);
    fread_mem(p_repeat_speed);
    fread_mem(p_arp_speed);
    fread_mem(p_arp_mod);

    auto frnd = [](float range) -> float { return (float)(rand() % (10000 + 1)) / 10000 * range; };

    auto ResetSample = [&](bool restart) {
        if (!restart) phase = 0;
        fperiod = 100.0 / (p_base_freq * p_base_freq + 0.001);
        period = (int)fperiod;
        fmaxperiod = 100.0 / (p_freq_limit * p_freq_limit + 0.001);
        fslide = 1.0 - pow((double)p_freq_ramp, 3.0) * 0.01;
        fdslide = -pow((double)p_freq_dramp, 3.0) * 0.000001;
        square_duty = 0.5f - p_duty * 0.5f;
        square_slide = -p_duty_ramp * 0.00005f;
        if (p_arp_mod >= 0.0f)
            arp_mod = 1.0 - pow((double)p_arp_mod, 2.0) * 0.9;
        else
            arp_mod = 1.0 + pow((double)p_arp_mod, 2.0) * 10.0;
        arp_time = 0;
        arp_limit = (int)(pow(1.0f - p_arp_speed, 2.0f) * 20000 + 32);
        if (p_arp_speed == 1.0f) arp_limit = 0;
        if (!restart) {
            // reset filter
            fltp = 0.0f;
            fltdp = 0.0f;
            fltw = pow(p_lpf_freq, 3.0f) * 0.1f;
            fltw_d = 1.0f + p_lpf_ramp * 0.0001f;
            fltdmp = 5.0f / (1.0f + pow(p_lpf_resonance, 2.0f) * 20.0f) * (0.01f + fltw);
            if (fltdmp > 0.8f) fltdmp = 0.8f;
            fltphp = 0.0f;
            flthp = pow(p_hpf_freq, 2.0f) * 0.1f;
            flthp_d = 1.0f + p_hpf_ramp * 0.0003f;
            // reset vibrato
            vib_phase = 0.0f;
            vib_speed = pow(p_vib_speed, 2.0f) * 0.01f;
            vib_amp = p_vib_strength * 0.5f;
            // reset envelope
            env_vol = 0.0f;
            env_stage = 0;
            env_time = 0;
            env_length[0] = (int)(p_env_attack * p_env_attack * 100000.0f);
            env_length[1] = (int)(p_env_sustain * p_env_sustain * 100000.0f);
            env_length[2] = (int)(p_env_decay * p_env_decay * 100000.0f);

            fphase = pow(p_pha_offset, 2.0f) * 1020.0f;
            if (p_pha_offset < 0.0f) fphase = -fphase;
            fdphase = pow(p_pha_ramp, 2.0f) * 1.0f;
            if (p_pha_ramp < 0.0f) fdphase = -fdphase;
            iphase = abs((int)fphase);
            ipp = 0;
            for (int i = 0; i < 1024; i++)
                phaser_buffer[i] = 0.0f;

            for (int i = 0; i < 32; i++)
                noise_buffer[i] = frnd(2.0f) - 1.0f;

            rep_time = 0;
            rep_limit = (int)(pow(1.0f - p_repeat_speed, 2.0f) * 20000 + 32);
            if (p_repeat_speed == 0.0f) rep_limit = 0;
        }
    };

    auto SynthSample = [&](int length, float *buffer) -> int {
        for (int i = 0; i < length; i++) {
            rep_time++;
            if (rep_limit != 0 && rep_time >= rep_limit) {
                rep_time = 0;
                ResetSample(true);
            }

            // frequency envelopes/arpeggios
            arp_time++;
            if (arp_limit != 0 && arp_time >= arp_limit) {
                arp_limit = 0;
                fperiod *= arp_mod;
            }
            fslide += fdslide;
            fperiod *= fslide;
            if (fperiod > fmaxperiod) {
                fperiod = fmaxperiod;
                if (p_freq_limit > 0.0f) {
                    return i;
                }
            }
            float rfperiod = (float)fperiod;
            if (vib_amp > 0.0f) {
                vib_phase += vib_speed;
                rfperiod = float(fperiod * (1.0 + sin(vib_phase) * vib_amp));
            }
            period = (int)rfperiod;
            if (period < 8) period = 8;
            square_duty += square_slide;
            if (square_duty < 0.0f) square_duty = 0.0f;
            if (square_duty > 0.5f) square_duty = 0.5f;
            // volume envelope
            env_time++;
            if (env_time > env_length[env_stage]) {
                env_time = 0;
                env_stage++;
                if (env_stage == 3) {
                    return i;
                }
            }
            if (env_stage == 0) env_vol = (float)env_time / env_length[0];
            if (env_stage == 1)
                env_vol =
                    1.0f + pow(1.0f - (float)env_time / env_length[1], 1.0f) * 2.0f * p_env_punch;
            if (env_stage == 2) env_vol = 1.0f - (float)env_time / env_length[2];

            // phaser step
            fphase += fdphase;
            iphase = abs((int)fphase);
            if (iphase > 1023) iphase = 1023;

            if (flthp_d != 0.0f) {
                flthp *= flthp_d;
                if (flthp < 0.00001f) flthp = 0.00001f;
                if (flthp > 0.1f) flthp = 0.1f;
            }

            float ssample = 0.0f;
            for (int si = 0; si < 8; si++) {  // 8x supersampling
                float sample = 0.0f;
                phase++;
                if (phase >= period) {
                    //				phase=0;
                    phase %= period;
                    if (wave_type == 3)
                        for (int i = 0; i < 32; i++)
                            noise_buffer[i] = frnd(2.0f) - 1.0f;
                }
                // base waveform
                float fp = (float)phase / period;
                switch (wave_type) {
                    case 0:  // square
                        if (fp < square_duty)
                            sample = 0.5f;
                        else
                            sample = -0.5f;
                        break;
                    case 1:  // sawtooth
                        sample = 1.0f - fp * 2;
                        break;
                    case 2:  // sine
                        sample = (float)sin(fp * 2 * PI);
                        break;
                    case 3:  // noise
                        sample = noise_buffer[phase * 32 / period];
                        break;
                }
                // lp filter
                float pp = fltp;
                fltw *= fltw_d;
                if (fltw < 0.0f) fltw = 0.0f;
                if (fltw > 0.1f) fltw = 0.1f;
                if (p_lpf_freq != 1.0f) {
                    fltdp += (sample - fltp) * fltw;
                    fltdp -= fltdp * fltdmp;
                } else {
                    fltp = sample;
                    fltdp = 0.0f;
                }
                fltp += fltdp;
                // hp filter
                fltphp += fltp - pp;
                fltphp -= fltphp * flthp;
                sample = fltphp;
                // phaser
                phaser_buffer[ipp & 1023] = sample;
                sample += phaser_buffer[(ipp - iphase + 1024) & 1023];
                ipp = (ipp + 1) & 1023;
                // final accumulation and envelope application
                ssample += sample * env_vol;
            }
            ssample = ssample / 8 * master_vol;

            ssample *= 2.0f * sound_vol;

            if (buffer != nullptr) {
                if (ssample > 1.0f) ssample = 1.0f;
                if (ssample < -1.0f) ssample = -1.0f;
                *buffer++ = ssample;
            }
        }
        return length;
    };

    ResetSample(false);
    vector<Sint16> synth;
    for (;;) {
        float sample;
        auto gen = SynthSample(1, &sample);
        if (!gen) break;
        auto ss = (Sint16)(sample * 0x7FFF);
        // FIXME: backwards way to make it 44100.
        // Instead, make it actually synth at that rate.
        if (!synth.empty()) synth.back() = (synth[synth.size() - 1] + ss) / 2;
        synth.push_back(ss);
        synth.push_back(0);
    }
    synth.pop_back();
    return AllocAudio(synth.data(), synth.size());
}

static MIX_Audio *LoadAudioFromBuffer(string_view buf, SoundType st) {
    MIX_Audio *audio = nullptr;
    switch (st) {
        case SOUND_WAV:
        case SOUND_OGG: {
            auto *io = SDL_IOFromMem((void *)buf.data(), (int)buf.length());
            if (!io) return nullptr;
            audio = MIX_LoadAudio_IO(mixer, io, true, true);
            if (!audio) {
                LOG_ERROR("MIX_LoadAudio_IO: ", SDL_GetError());
            }
            break;
        }
        case SOUND_SFXR: {
            audio = RenderSFXR(buf);
            break;
        }
    }
    if (!audio) return nullptr;
    return audio;
}

static MIX_Audio *LoadAudio(string_view filename, SoundType st) {
    auto it = audio_files.find(filename);
    if (it != audio_files.end()) {
        return it->second.audio.get();
    }
    string buf;
    if (LoadFile(filename, &buf) < 0) return {};
    auto *audio = LoadAudioFromBuffer(buf, st);
    if (!audio) return nullptr;
    audio_files.insert({ string(filename), Audio(audio) });
    return audio;
}


int SDLLoadSound(string_view filename, SoundType st) {
    if (!SDLSoundInit()) return 0;
    return LoadAudio(filename, st) != nullptr;
}

int SDLLoadSoundFromBuffer(string_view buffer, SoundType st) {
    if (!SDLSoundInit()) return 0;
    return LoadAudioFromBuffer(buffer, st) != nullptr;
}

// Tracks are used internally and are 0-based. Channels are used by the lobster
// API and are 1-based.
static int sound_channel_from_track(int tr) {
    return tr >= 0 && tr < sound_track_count ? tr + 1 : -1;
}

static MIX_Track *get_sound_track(int ch) {
    return (ch >= 1 && ch <= sound_track_count) ? sound_tracks[ch - 1] : nullptr;
}

static int GetAvailableSoundTrack(int pri) {
    int tr = -1;
    // First look for an available track.
    for (int i = 0; i < sound_track_count; ++i) {
        if (MIX_TrackPlaying(sound_tracks[i])) continue;
        // If a track has an audio stream, consider it playing.
        if (MIX_GetTrackAudioStream(sound_tracks[i])) continue;
        tr = i;
        break;
    }
    if (tr == -1) {
        // No available track, find a different track to stop.
        int p = pri;
        uint64_t sa = UINT64_MAX;
        for (int i = 0; i < sound_track_count; ++i) {
            if (sound_pri[i] < p) {
                tr = i;
                p = sound_pri[i];
                // save sound age too in case multiple tracks equal this new priority!
                sa = sound_age[i];
            } else if (sound_pri[i] == p && sound_age[i] < sa) {
                tr = i;
                sa = sound_age[i];
            }
        }
    }
    return tr;
}

static void PlayTrack(int tr, float vol, int loops, int pri) {
    auto *track = sound_tracks[tr];
    if (!MIX_SetTrack3DPosition(track, nullptr)) {
        LOG_ERROR("MIX_SetTrack3DPosition: ", SDL_GetError());
    }
    auto props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    if (!MIX_PlayTrack(track, props)) {
        LOG_ERROR("MIX_PlayTrack: ", SDL_GetError());
    }
    SDL_DestroyProperties(props);
    if (!MIX_SetTrackGain(track, vol)) {
        LOG_ERROR("MIX_SetTrackGain: ", SDL_GetError());
    }
    sound_pri[tr] = pri;
    sound_age[tr] = sounds_played++;
}

static int PlaySoundAudio(MIX_Audio *audio, float vol, int loops, int pri) {
    int tr = GetAvailableSoundTrack(pri);
    if (tr < 0) return 0;
    auto *track = sound_tracks[tr];
    if (!MIX_SetTrackAudio(track, audio)) {
        LOG_ERROR("MIX_SetTrackAudio: ", SDL_GetError());
    }
    PlayTrack(tr, vol, loops, pri);
    return sound_channel_from_track(tr);
}

int SDLPlaySound(string_view filename, SoundType st, float vol, int loops, int pri) {
    if (!SDLSoundInit()) return 0;
    auto *audio = LoadAudio(filename, st);
    if (!audio) return 0;
    return PlaySoundAudio(audio, vol, loops, pri);
}

int SDLPlaySoundFromBuffer(string_view buffer, SoundType st, float vol, int loops, int pri) {
    if (!SDLSoundInit()) return 0;
    auto *audio = LoadAudioFromBuffer(buffer, st);
    if (!audio) return 0;
    return PlaySoundAudio(audio, vol, loops, pri);
}

int SDLPlayAudioStream(int freq, int channels, float vol, int pri) {
    if (!SDLSoundInit()) return 0;
    int tr = GetAvailableSoundTrack(pri);
    if (tr < 0) return 0;
    auto *track = sound_tracks[tr];
    SDL_AudioSpec spec = { SDL_AUDIO_F32, channels, freq };
    auto *stream = SDL_CreateAudioStream(&spec, &spec);
    if (!stream) {
        LOG_ERROR("SDL_CreateAudioStream: ", SDL_GetError());
        return 0;
    }
    if (!MIX_SetTrackAudioStream(track, stream)) {
        LOG_ERROR("MIX_SetTrackAudioStream: ", SDL_GetError());
        return 0;
    }
    printf("Playing audio stream on track %d\n", tr);
    PlayTrack(tr, vol, 0, pri);
    return sound_channel_from_track(tr);
}

bool SDLPutAudioStream(int ch, vector<float> data) {
    if (!SDLSoundInit()) return 0;
    auto *track = get_sound_track(ch);
    if (!track) return 0;
    auto *stream = MIX_GetTrackAudioStream(track);
    if (!stream) return 0;
    if (!SDL_PutAudioStreamData(stream, data.data(), (int)(data.size() * sizeof(float)))) {
        LOG_ERROR("SDL_PutAudioStreamData: ", SDL_GetError());
        return false;
    }
    // Restart the track, in case it stopped.
    if (!MIX_TrackPlaying(track)) {
        auto props = SDL_CreateProperties();
        if (!MIX_PlayTrack(track, props)) {
            LOG_ERROR("MIX_PlayTrack: ", SDL_GetError());
        }
        SDL_DestroyProperties(props);
    }
    return true;
}

bool SDLHasAudioStream(int ch) {
    if (!SDLSoundInit()) return false;
    auto *track = get_sound_track(ch);
    if (!track) return false;
    auto *stream = MIX_GetTrackAudioStream(track);
    return stream != nullptr;
}

void SDLHaltSound(int ch) {
    if (!SDLSoundInit()) return;
    auto *track = get_sound_track(ch);
    if (!track) return;
    // If a track has an audio stream, remove it.
    auto *stream = MIX_GetTrackAudioStream(track);
    if (stream) {
        if (!MIX_SetTrackAudioStream(track, nullptr)) {
            LOG_ERROR("MIX_SetTrackAudioStream: ", SDL_GetError());
        }
    }
    if (!MIX_StopTrack(track, 0)) {
        LOG_ERROR("MIX_StopTrack: ", SDL_GetError());
    }
}

void SDLPauseSound(int ch) {
    if (!SDLSoundInit()) return;
    auto *track = get_sound_track(ch);
    if (!track) return;
    if (!MIX_PauseTrack(track)) {
        LOG_ERROR("MIX_PauseTrack: ", SDL_GetError());
    }
}

int SDLSoundStatus(int ch) {
    if (!SDLSoundInit()) return -1;
    auto *track = get_sound_track(ch);
    if (!track) return -1;
    if (MIX_TrackPlaying(track)) return 1;
    if (MIX_TrackPaused(track)) return 2;
    return 0;
}

float SDLGetTimeLength(int ch) {
    if (!SDLSoundInit()) return 0.f;
    auto *track = get_sound_track(ch);
    if (!track) return 0.f;
    auto *audio = MIX_GetTrackAudio(track);
    if (!audio) return 0.f;
    auto frames = MIX_GetAudioDuration(audio);
    auto ms = MIX_AudioMSToFrames(audio, frames);
    return ms / 1000.f;
}

int SDLRegisterEffect(int /*ch*/, SDLEffectFunc /*func*/, SDLEffectDone /*done*/,
                      void * /*userdata*/) {
    // TODO(SDL3)
    return 0;
}

void SDLResumeSound(int ch) {
    if (!SDLSoundInit()) return;
    auto *track = get_sound_track(ch);
    if (!track) return;
    if (!MIX_ResumeTrack(track)) {
        LOG_ERROR("MIX_ResumeTrack: ", SDL_GetError());
    }
}

void SDLSetVolume(int ch, float vol) {
    if (!SDLSoundInit()) return;
    auto *track = get_sound_track(ch);
    if (!track) return;
    if (!MIX_SetTrackGain(track, vol)) {
        LOG_ERROR("MIX_SetTrackGain: ", SDL_GetError());
    }
}

void SDLSetPosition(int ch, float3 vecfromlistener, float3 listenerfwd, float attnscale) {
    if (!SDLSoundInit()) return;
    auto *track = get_sound_track(ch);
    if (!track) return;
    vecfromlistener /= attnscale;
    auto world_up = float3_z;
    auto F = normalize(listenerfwd);
    auto R = normalize(cross(F, world_up));
    auto U = cross(R, F);
    auto P = float3(dot(R, vecfromlistener), dot(F, vecfromlistener), dot(U, vecfromlistener));
    MIX_Point3D pos = { P.x, P.z, -P.y };  // SDL uses +X: right, +Y: up, +Z: backward
    if (!MIX_SetTrack3DPosition(track, &pos)) {
        LOG_ERROR("MIX_SetTrack3DPosition: ", SDL_GetError());
    }
}

void SDLUnsetPosition(int ch) {
    if (!SDLSoundInit()) return;
    auto *track = get_sound_track(ch);
    if (!track) return;
    if (!MIX_SetTrack3DPosition(track, nullptr)) {
        LOG_ERROR("MIX_SetTrack3DPosition: ", SDL_GetError());
    }
}

// Tracks are used internally and are 0-based. Channels are used by the lobster
// API and are 1-based.
static int music_channel_from_track(int tr) {
    return tr >= 0 && tr < (int)musics.size() ? tr + 1 : -1;
}

static Music *get_music(int mus_id) {
    return (mus_id >= 1 && mus_id <= (int)musics.size()) ? &musics[mus_id - 1] : nullptr;
}

static int PlayMusicAudio(MIX_Audio *audio, int loops, int fade_ms) {
    int tr = -1;
    // First look for an available track.
    for (int i = 0; i < (int)musics.size(); ++i) {
        auto &music = musics[i];
        if (!MIX_TrackPlaying(music.track)) {
            tr = i;
            break;
        }
    }
    if (tr == -1) {
        tr = (int)musics.size();
        musics.push_back(Music { MIX_CreateTrack(mixer), 1.0f });
    }
    if (tr >= 0) {
        auto &music = musics[tr];
        if (!MIX_SetTrackAudio(music.track, audio)) {
            LOG_ERROR("MIX_SetTrackAudio: ", SDL_GetError());
        }
        auto props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        if (fade_ms) {
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_ms);
        }
        if (!MIX_PlayTrack(music.track, props)) {
            LOG_ERROR("MIX_PlayTrack: ", SDL_GetError());
        }
        SDL_DestroyProperties(props);
        if (!MIX_SetTrackGain(music.track, music_volume)) {
            LOG_ERROR("MIX_SetTrackGain: ", SDL_GetError());
        }
    }
    return music_channel_from_track(tr);
}

int SDLPlayMusic(string_view filename, int loops) {
    if (!SDLSoundInit()) return 0;
    auto *audio = LoadAudio(filename, SOUND_OGG);
    if (!audio) return 0;
    return PlayMusicAudio(audio, loops, 0);
}

int SDLFadeInMusic(string_view filename, int loops, int ms) {
    if (!SDLSoundInit()) return 0;
    auto *audio = LoadAudio(filename, SOUND_OGG);
    if (!audio) return 0;
    return PlayMusicAudio(audio, loops, ms);
}

int SDLCrossFadeMusic(int old_mus_id, string_view new_filename, int loops, int ms) {
    if (!SDLSoundInit()) return 0;
    SDLFadeOutMusic(old_mus_id, ms);
    return SDLFadeInMusic(new_filename, loops, ms);
}

void SDLFadeOutMusic(int mus_id, int ms) {
    if (!SDLSoundInit()) return;
    auto *music = get_music(mus_id);
    if (!music) return;
    if (!MIX_StopTrack(music->track, MIX_TrackMSToFrames(music->track, ms))) {
        LOG_ERROR("MIX_StopTrack: ", SDL_GetError());
    }
}

void SDLHaltMusic(int mus_id) {
    SDLFadeOutMusic(mus_id, 0);
}

void SDLPauseMusic(int mus_id) {
    if (!SDLSoundInit()) return;
    auto *music = get_music(mus_id);
    if (!music) return;
    if (!MIX_PauseTrack(music->track)) {
        LOG_ERROR("MIX_PauseTrack: ", SDL_GetError());
    }
}

void SDLResumeMusic(int mus_id) {
    if (!SDLSoundInit()) return;
    auto *music = get_music(mus_id);
    if (!music) return;
    if (!MIX_ResumeTrack(music->track)) {
        LOG_ERROR("MIX_ResumeTrack: ", SDL_GetError());
    }
}

void SDLSetMusicVolume(int mus_id, float vol) {
    if (!SDLSoundInit()) return;
    auto *music = get_music(mus_id);
    if (!music) return;
    music->vol = geom::clamp(vol, 0.f, 1.f);
    if (!MIX_SetTrackGain(music->track, music_volume * music->vol)) {
        LOG_ERROR("MIX_SetTrackGain: ", SDL_GetError());
    }
}

bool SDLMusicIsPlaying(int mus_id) {
    if (!SDLSoundInit()) return false;
    auto *music = get_music(mus_id);
    if (!music) return false;
    return MIX_TrackPlaying(music->track);
}

void SDLSetGeneralMusicVolume(float vol) {
    music_volume = geom::clamp(vol, 0.f, 1.f);
    for (auto &music : musics) {
        if (!MIX_SetTrackGain(music.track, music_volume * music.vol)) {
            LOG_ERROR("MIX_SetTrackGain: ", SDL_GetError());
        }
    }
}

static Recording *get_recording(int id) {
    return (id >= 1 && id <= (int)recordings.size()) ? &recordings[id - 1] : nullptr;
}

bool SDLGetRecordingDeviceNames(vector<int> &out_ids, vector<string> &out_names) {
    if (!SDLSoundInit()) return false;
    int count;
    auto *devices = SDL_GetAudioRecordingDevices(&count);
    if (!devices) {
        LOG_ERROR("SDL_GetAudioRecordingDevices: ", SDL_GetError());
    }
    for (int i = 0; i < count; ++i) {
        auto id = devices[i];
        out_ids.push_back((int)id);
        out_names.push_back(SDL_GetAudioDeviceName(id));
    }
    return true;
}

int SDLRecordingStart(int phys_device_id, int freq) {
    if (!SDLSoundInit()) return 0;
    SDL_AudioSpec spec = { SDL_AUDIO_F32, 1, freq };
    int device_id = SDL_OpenAudioDevice(phys_device_id, &spec);
    if (!device_id) {
        LOG_ERROR("SDL_OpenAudioDevice: ", SDL_GetError());
        return 0;
    }
    const char *format;
    switch (spec.format) {
      default:              format = "SDL_AUDIO_UNKNOWN"; break;
      case SDL_AUDIO_U8:    format = "SDL_AUDIO_U8"; break;
      case SDL_AUDIO_S8:    format = "SDL_AUDIO_S8"; break;
      case SDL_AUDIO_S16LE: format = "SDL_AUDIO_S16LE"; break;
      case SDL_AUDIO_S16BE: format = "SDL_AUDIO_S16BE"; break;
      case SDL_AUDIO_S32LE: format = "SDL_AUDIO_S32LE"; break;
      case SDL_AUDIO_S32BE: format = "SDL_AUDIO_S32BE"; break;
      case SDL_AUDIO_F32LE: format = "SDL_AUDIO_F32LE"; break;
      case SDL_AUDIO_F32BE: format = "SDL_AUDIO_F32BE"; break;
    }
    LOG_INFO("SDL opened recording device \"", SDL_GetAudioDeviceName(phys_device_id),
             "\": format:", format, " channels:", spec.channels, " freq:", spec.freq, "\n");
    auto *stream = SDL_CreateAudioStream(&spec, &spec);
    if (!SDL_BindAudioStream(device_id, stream)) {
        LOG_ERROR("SDL_BindAudioStream: ", SDL_GetError());
        return 0;
    }
    int index = -1;
    for (int i = 0; i < (int)recordings.size(); ++i) {
        if (recordings[i].device_id < 0) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        index = (int)recordings.size();
        recordings.push_back(Recording{ device_id, stream });
    } else {
        recordings[index] = Recording{ device_id, stream };
    }
    return index + 1;
}

bool SDLRecordingStop(int id) {
    if (!SDLSoundInit()) return false;
    auto *recording = get_recording(id);
    if (!recording) return false;
    SDL_UnbindAudioStream(recording->stream.get());
    SDL_CloseAudioDevice(recording->device_id);
    recording->device_id = -1;
    recording->stream.reset();
    return true;
}

vector<float> SDLRecordingGet(int id) {
    vector<float> data;
    if (!SDLSoundInit()) return data;
    auto *recording = get_recording(id);
    if (!recording) return data;
    auto avail = SDL_GetAudioStreamAvailable(recording->stream.get());
    if (avail < 0) {
        LOG_ERROR("SDL_GetAudioStreamAvailable: ", SDL_GetError());
        return data;
    }
    if (avail == 0) return data;
    data.resize(avail);
    auto read_bytes = SDL_GetAudioStreamData(recording->stream.get(), data.data(),
                                             (int)(data.size() * sizeof(float)));
    if (read_bytes < 0) {
        LOG_ERROR("SDL_GetAudioStreamData: ", SDL_GetError());
        data.clear();
        return data;
    }
    data.resize(read_bytes / sizeof(float));
    return data;
}

#ifdef _MSC_VER
    #pragma warning(disable : 4244 4457 4245 4701)
#endif

