/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

// This file supports playing MIDI files with timidity

#ifdef DECODER_MIDI_TIMIDITY

#include "SDL_mixer_internal.h"

#include "timidity/timidity.h"

// Config file should contain any other directory that needs
//  to be added to the search path. The library adds the path
//  of the config file to its search path, too.
#if defined(SDL_PLATFORM_WIN32)
static const char *timidity_cfgs[] = { "C:\\TIMIDITY\\TIMIDITY.CFG" };
#else  // unix:
static const char *timidity_cfgs[] = { "/etc/timidity.cfg", "/etc/timidity/timidity.cfg", "/etc/timidity/freepats.cfg" };
#endif

typedef struct TIMIDITY_TrackData
{
    Sint32 samples[4096 * 2];   // !!! FIXME: there's a hardcoded thing about buffer_size in our copy of timidity that needs to be fixed; it's hardcoded to this at the moment.
    MidiSong *song;
    int freq;
} TIMIDITY_TrackData;


static bool SDLCALL TIMIDITY_init(void)
{
    const char *cfg = SDL_getenv("TIMIDITY_CFG");  // see if the user had one.
    if (cfg) {
        return (Timidity_Init(cfg) == 0); // env or user override: no other tries
    }

    for (int i = 0; i < (int)SDL_arraysize(timidity_cfgs); i++) {
        if (Timidity_Init(timidity_cfgs[i]) == 0) {
            return true;
        }
    }

    return (Timidity_Init(NULL) == 0); // library's default cfg.
}

static void SDLCALL TIMIDITY_quit(void)
{
    Timidity_Exit();
}

static bool SDLCALL TIMIDITY_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // just load the bare minimum from the IOStream to verify it's a MIDI file.
    char magic[4];
    if (SDL_ReadIO(io, magic, 4) != 4) {
        return false;
    } else if (SDL_memcmp(magic, "MThd", 4) != 0) {
        return SDL_SetError("Not a MIDI audio stream");
    }

    // Go back and do a proper load now to get metadata.
    if (SDL_SeekIO(io, 0, SDL_IO_SEEK_SET) < 0) {
        return false;
    }

    spec->format = SDL_AUDIO_S32;   // timidity wants to do Sint32, and converts to other formats internally from Sint32.
    spec->channels = 2;
    // Use the device's current sample rate, already set in spec->freq

    Sint64 song_length_in_frames = -1;
    MidiSong *song = Timidity_LoadSong(io, spec);
    if (!song) {
        return false;
    }

    song_length_in_frames = MIX_MSToFrames(spec->freq, Timidity_GetSongLength(song));
    if (song_length_in_frames < 0) {
        song_length_in_frames = 0;
    }
    Timidity_FreeSong(song);

    *duration_frames = song_length_in_frames;
    *audio_userdata = NULL;   // no state.

    return true;
}

static bool SDLCALL TIMIDITY_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.
    TIMIDITY_TrackData *tdata = (TIMIDITY_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    tdata->song = Timidity_LoadSong(io, spec);
    if (!tdata->song) {
        SDL_free(tdata);
        return SDL_SetError("Timidity_LoadSong failed");
    }

    Timidity_SetVolume(tdata->song, 800);  // !!! FIXME: maybe my test patches are really quiet?
    Timidity_Start(tdata->song);

    tdata->freq = spec->freq;

    *track_userdata = tdata;
    return true;
}

static bool SDLCALL TIMIDITY_decode(void *track_userdata, SDL_AudioStream *stream)
{
    TIMIDITY_TrackData *tdata = (TIMIDITY_TrackData *) track_userdata;
    //Sint32 samples[256];  // !!! FIXME: there's a hardcoded thing about buffer_size in our copy of timidity that needs to be fixed; it's hardcoded at the moment, so we use tdata->samples.
    const int amount = Timidity_PlaySome(tdata->song, tdata->samples, sizeof (tdata->samples));
    if (amount <= 0) {
        return false;  // EOF or error, we're done either way.
    }

    SDL_PutAudioStreamData(stream, tdata->samples, amount);
    return true;
}

static bool SDLCALL TIMIDITY_seek(void *track_userdata, Uint64 frame)
{
    TIMIDITY_TrackData *tdata = (TIMIDITY_TrackData *) track_userdata;
    Sint64 ticks = MIX_FramesToMS(tdata->freq, frame);
    if (ticks < 0) {
        ticks = 0;
    }
    Timidity_Seek(tdata->song, (Uint32)ticks);  // !!! FIXME: this returns void, what happens if we seek past EOF?
    return true;
}

static void SDLCALL TIMIDITY_quit_track(void *track_userdata)
{
    TIMIDITY_TrackData *tdata = (TIMIDITY_TrackData *) track_userdata;
    Timidity_Stop(tdata->song);
    Timidity_FreeSong(tdata->song);
    SDL_free(tdata);
}

static void SDLCALL TIMIDITY_quit_audio(void *audio_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.
}

const MIX_Decoder MIX_Decoder_TIMIDITY = {
    "TIMIDITY",
    TIMIDITY_init,
    TIMIDITY_init_audio,
    TIMIDITY_init_track,
    TIMIDITY_decode,
    TIMIDITY_seek,
    NULL,  // jump_to_order
    TIMIDITY_quit_track,
    TIMIDITY_quit_audio,
    TIMIDITY_quit
};

#endif

