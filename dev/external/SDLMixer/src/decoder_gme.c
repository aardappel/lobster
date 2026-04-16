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

#ifdef DECODER_GME

#include "SDL_mixer_internal.h"

#include <gme/gme.h>

#if defined(FLUIDSYNTH_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "gme",
    "Support for audio of classic video game consoles using game-music-emu",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    GME_DYNAMIC
)
#endif

#ifdef GME_DYNAMIC
#define MIX_LOADER_DYNAMIC GME_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS \
    MIX_LOADER_FUNCTION(true,gme_err_t,gme_open_data,(void const*, long, Music_Emu**, int)) \
    MIX_LOADER_FUNCTION(true,const char*,gme_identify_header,(void const*)) \
    MIX_LOADER_FUNCTION(true,gme_err_t,gme_start_track,(Music_Emu*, int)) \
    MIX_LOADER_FUNCTION(true,int,gme_track_ended,(Music_Emu const*)) \
    MIX_LOADER_FUNCTION(true,int,gme_voice_count,(Music_Emu const*)) \
    MIX_LOADER_FUNCTION(true,gme_err_t,gme_track_info,(Music_Emu const*, gme_info_t**, int)) \
    MIX_LOADER_FUNCTION(true,void,gme_free_info,(gme_info_t*)) \
    MIX_LOADER_FUNCTION(true,gme_err_t,gme_seek_samples,(Music_Emu*, int)) \
    MIX_LOADER_FUNCTION(true,gme_err_t,gme_play,(Music_Emu*, int count, short out[])) \
    MIX_LOADER_FUNCTION(true,void,gme_delete,(Music_Emu*)) \
    MIX_LOADER_FUNCTION(false,void,gme_set_autoload_playback_limit,(Music_Emu*, int)) \

#define MIX_LOADER_MODULE gme
#include "SDL_mixer_loader.h"


static bool SDLCALL GME_init(void)
{
    return LoadModule_gme();
}

static void SDLCALL GME_quit(void)
{
    UnloadModule_gme();
}

static bool SDLCALL GME_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // just load the bare minimum from the IOStream to verify it's a supported file.
    char magic[4];
    if (SDL_ReadIO(io, magic, 4) != 4) {
        return false;
    }

    const char *ext = gme.gme_identify_header(magic);
    if (!ext || !*ext) {
        return SDL_SetError("Not a libgme-supported audio stream");
    }

    // Go back and do a proper load now to get metadata.
    bool copied = false;
    size_t datalen = 0;
    Uint8 *data = (Uint8 *) MIX_SlurpConstIO(io, &datalen, &copied);
    if (!data) {
        return false;
    }

    Music_Emu *emu = NULL;
    gme_err_t err = gme.gme_open_data(data, (long)datalen, &emu, gme_info_only);

    if (copied) {
        SDL_free(data);
    }

    if (err) {
        return SDL_SetError("gme_open_data failed: %s", err);
    }

    *duration_frames = -1;

    gme_info_t *info = NULL;
    err = gme.gme_track_info(emu, &info, 0);
    if (!err) {  // if this fails, oh well.
        #define SET_GME_METADATA(gmestr, mixerprop) { \
            if (info->gmestr && *info->gmestr) { \
                SDL_SetStringProperty(props, "SDL_mixer.metadata.gme." #gmestr, info->gmestr); \
                if (mixerprop) { SDL_SetStringProperty(props, mixerprop, info->gmestr); } \
            } \
        }
        SET_GME_METADATA(song, MIX_PROP_METADATA_TITLE_STRING);
        SET_GME_METADATA(author, MIX_PROP_METADATA_ARTIST_STRING);
        SET_GME_METADATA(game, MIX_PROP_METADATA_ALBUM_STRING);
        SET_GME_METADATA(copyright, MIX_PROP_METADATA_COPYRIGHT_STRING);
        SET_GME_METADATA(system, NULL);
        SET_GME_METADATA(comment, NULL);
        SET_GME_METADATA(dumper, NULL);
        #undef SET_GME_METADATA

        if ((info->intro_length >= 0) && (info->loop_length > 0)) {
            *duration_frames = MIX_DURATION_INFINITE;
        } else if (info->length >= 0) {
            *duration_frames = MIX_MSToFrames(spec->freq, info->length);
            if (*duration_frames < 0) {
                *duration_frames = 0;
            }
        }

        gme.gme_free_info(info);
    }

    gme.gme_delete(emu);

    // libgme only outputs Sint16 stereo data.
    spec->format = SDL_AUDIO_S16;
    spec->channels = 2;
    // libgme generates in whatever sample rate, so use the current device spec->freq.

    *audio_userdata = NULL;  // no state.

    return true;
}

static bool SDLCALL GME_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.

    bool copied = false;
    size_t datalen = 0;
    Uint8 *data = (Uint8 *) MIX_SlurpConstIO(io, &datalen, &copied);
    if (!data) {
        return false;
    }

    Music_Emu *emu = NULL;
    gme_err_t err = gme.gme_open_data(data, (long) datalen, &emu, spec->freq);

    if (copied) {
        SDL_free(data);
    }

    if (err) {
        return SDL_SetError("gme_open_data failed: %s", err);
    }

    // Set this flag BEFORE calling the gme_start_track() to fix an inability to loop forever
    if (gme.gme_set_autoload_playback_limit) {
        gme.gme_set_autoload_playback_limit(emu, 0);
    }

    err = gme.gme_start_track(emu, 0);
    if (err) {
        gme.gme_delete(emu);
        return SDL_SetError("gme_start_track failed: %s", err);
    }

    *track_userdata = emu;

    return true;
}

static bool SDLCALL GME_decode(void *track_userdata, SDL_AudioStream *stream)
{
    Music_Emu *emu = (Music_Emu *) track_userdata;
    if (gme.gme_track_ended(emu)) {
        return false;  // all done.
    }

    Sint16 samples[256];
    gme_err_t err = gme.gme_play(emu, SDL_arraysize(samples), (short*) samples);
    if (err != NULL) {
        return SDL_SetError("GME: %s", err);  // i guess we're done.
    }

    SDL_PutAudioStreamData(stream, samples, sizeof (samples));

    return true;  // had more data to decode.
}

static bool SDLCALL GME_seek(void *track_userdata, Uint64 frame)
{
    Music_Emu *emu = (Music_Emu *) track_userdata;
    gme_err_t err = gme.gme_seek_samples(emu, (int) frame);
    return err ? SDL_SetError("gme_seek_samples failed: %s", err) : true;
}

static void SDLCALL GME_quit_track(void *track_userdata)
{
    Music_Emu *emu = (Music_Emu *) track_userdata;
    gme.gme_delete(emu);
}

static void SDLCALL GME_quit_audio(void *audio_userdata)
{
    SDL_assert(audio_userdata == NULL);  // no state.
}

const MIX_Decoder MIX_Decoder_GME = {
    "GME",
    GME_init,
    GME_init_audio,
    GME_init_track,
    GME_decode,
    GME_seek,
    NULL,  // jump_to_order
    GME_quit_track,
    GME_quit_audio,
    GME_quit
};

#endif
