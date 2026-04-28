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

// SDL2_mixer's FluidSynth code was written by James Le Cuirot; this was rewritten using that work as a reference.

#ifdef DECODER_MIDI_FLUIDSYNTH

#include "SDL_mixer_internal.h"

#include <fluidsynth.h>

#if defined(FLUIDSYNTH_DYNAMIC) && defined(SDL_ELF_NOTE_DLOPEN)
SDL_ELF_NOTE_DLOPEN(
    "midi-fluidsynth",
    "Support for MIDI audio using FluidSynth",
    SDL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
    FLUIDSYNTH_DYNAMIC
)
#endif

#ifdef FLUIDSYNTH_DYNAMIC
#define MIX_LOADER_DYNAMIC FLUIDSYNTH_DYNAMIC
#endif

#define MIX_LOADER_FUNCTIONS_fluidsynthbase \
    MIX_LOADER_FUNCTION(true,void,delete_fluid_settings,(fluid_settings_t*)) \
    MIX_LOADER_FUNCTION(true,int,fluid_audio_driver_register,(const char **)) \
    MIX_LOADER_FUNCTION(true,int,fluid_player_add_mem,(fluid_player_t*, const void*, size_t)) \
    MIX_LOADER_FUNCTION(true,int,fluid_player_get_status,(fluid_player_t*)) \
    MIX_LOADER_FUNCTION(true,int,fluid_player_play,(fluid_player_t*)) \
    MIX_LOADER_FUNCTION(true,int,fluid_player_stop,(fluid_player_t*)) \
    MIX_LOADER_FUNCTION(true,int,fluid_player_get_total_ticks,(fluid_player_t *)) \
    MIX_LOADER_FUNCTION(true,int,fluid_settings_setint,(fluid_settings_t*, const char*, int)) \
    MIX_LOADER_FUNCTION(true,int,fluid_settings_setnum,(fluid_settings_t*, const char*, double)) \
    MIX_LOADER_FUNCTION(true,int,fluid_settings_getnum,(fluid_settings_t*, const char*, double*)) \
    MIX_LOADER_FUNCTION(true,int,fluid_sfloader_set_callbacks,(fluid_sfloader_t *,fluid_sfloader_callback_open_t,fluid_sfloader_callback_read_t,fluid_sfloader_callback_seek_t,fluid_sfloader_callback_tell_t,fluid_sfloader_callback_close_t)) \
    MIX_LOADER_FUNCTION(true,void,fluid_synth_add_sfloader,(fluid_synth_t *, fluid_sfloader_t *)) \
    MIX_LOADER_FUNCTION(true,int,fluid_synth_sfload,(fluid_synth_t*, const char*, int)) \
    MIX_LOADER_FUNCTION(true,int,fluid_synth_write_float,(fluid_synth_t*, int, void*, int, int, void*, int, int)) \
    MIX_LOADER_FUNCTION(true,fluid_sfloader_t *,new_fluid_defsfloader,(fluid_settings_t *settings)) \
    MIX_LOADER_FUNCTION(true,fluid_player_t*,new_fluid_player,(fluid_synth_t*)) \
    MIX_LOADER_FUNCTION(true,fluid_settings_t*,new_fluid_settings,(void)) \
    MIX_LOADER_FUNCTION(true,fluid_synth_t*,new_fluid_synth,(fluid_settings_t*)) \

#if (FLUIDSYNTH_VERSION_MAJOR >= 2)
    #define MIX_LOADER_FUNCTIONS \
        MIX_LOADER_FUNCTIONS_fluidsynthbase \
        MIX_LOADER_FUNCTION(true,void,delete_fluid_player,(fluid_player_t*)) \
        MIX_LOADER_FUNCTION(true,void,delete_fluid_synth,(fluid_synth_t*)) \
        MIX_LOADER_FUNCTION(true,int,fluid_player_seek,(fluid_player_t *, int))
#else
    #define MIX_LOADER_FUNCTIONS \
        MIX_LOADER_FUNCTIONS_fluidsynthbase \
        MIX_LOADER_FUNCTION(true,int,delete_fluid_player,(fluid_player_t*)) \
        MIX_LOADER_FUNCTION(true,int,delete_fluid_synth,(fluid_synth_t*))
#endif

#define MIX_LOADER_MODULE fluidsynth
#include "SDL_mixer_loader.h"


typedef struct FLUIDSYNTH_AudioData
{
    const Uint8 *sfdata;
    size_t sfdatalen;
    SDL_PropertiesID fluidsynth_props;
} FLUIDSYNTH_AudioData;

typedef struct FLUIDSYNTH_TrackData
{
    const FLUIDSYNTH_AudioData *adata;
    fluid_synth_t *synth;
    fluid_settings_t *settings;
    fluid_player_t *player;
    Uint64 current_frame;
    int freq;
} FLUIDSYNTH_TrackData;


static bool SDLCALL FLUIDSYNTH_init(void)
{
    if (!LoadModule_fluidsynth()) {
        return false;
    }

    // don't let FluidSynth touch hardware directly under any circumstances.
    const char *no_drivers[] = { NULL };
    fluidsynth.fluid_audio_driver_register(no_drivers);
    return true;
}

static void SDLCALL FLUIDSYNTH_quit(void)
{
    UnloadModule_fluidsynth();
}

static bool SDLCALL FLUIDSYNTH_init_audio(SDL_IOStream *io, SDL_AudioSpec *spec, SDL_PropertiesID props, Sint64 *duration_frames, void **audio_userdata)
{
    // Try to load a soundfont file if we can.
    //  If the app provided an IOStream for it, use it. If not, see if they provided a path.
    bool closesfio = false;
    SDL_IOStream *sfio = SDL_GetPointerProperty(props, MIX_PROP_DECODER_FLUIDSYNTH_SOUNDFONT_IOSTREAM_POINTER, NULL);
    if (sfio) {
        closesfio = SDL_GetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, false);
    } else {
        // Try to load a correction file if available.
        closesfio = true;  // if we open it, we close it.
        const char *sfpath = SDL_GetStringProperty(props, MIX_PROP_DECODER_FLUIDSYNTH_SOUNDFONT_PATH_STRING, NULL);
        if (sfpath) {
            sfio = SDL_IOFromFile(sfpath, "rb");
        }
    }

    #ifdef SDL_PLATFORM_UNIX  // this happens to be where Ubuntu stores a usable soundfont, at least on my laptop. Try it if nothing else worked out.
    if (!sfio) {
        sfio = SDL_IOFromFile("/usr/share/sounds/sf2/default-GM.sf2", "rb");
    }
    #endif

    // !!! FIXME: should we fail if there's no soundfont? It's not going to generate sound without one, unless there's a system-provided one in some cases...?

    // just load the bare minimum from the IOStream to verify it's a MIDI file.
    char magic[4];
    if (SDL_ReadIO(io, magic, 4) != 4) {
        if (sfio && closesfio) { SDL_CloseIO(sfio); }
        return false;
    } else if (SDL_memcmp(magic, "MThd", 4) != 0) {
        if (sfio && closesfio) { SDL_CloseIO(sfio); }
        return SDL_SetError("Not a MIDI audio stream");
    }

    // Go back and do a proper load now to get metadata.
    if (SDL_SeekIO(io, SDL_IO_SEEK_SET, 0) < 0) {
        if (sfio && closesfio) { SDL_CloseIO(sfio); }
        return false;
    }

    size_t sfdatalen = 0;
    Uint8 *sfdata = NULL;
    if (sfio) {
        sfdata = (Uint8 *) SDL_LoadFile_IO(sfio, &sfdatalen, false);
        if (closesfio) {
            SDL_CloseIO(sfio);
        }
        if (!sfdata) {
            return false;
        }
    }

    FLUIDSYNTH_AudioData *adata = (FLUIDSYNTH_AudioData *) SDL_calloc(1, sizeof (*adata));
    if (!adata) {
        SDL_free(sfdata);
        return false;
    }

    adata->sfdata = sfdata;
    adata->sfdatalen = sfdatalen;

    spec->format = SDL_AUDIO_F32;
    spec->channels = 2;
    // Use the device's current sample rate, already set in spec->freq

    *duration_frames = -1;  // !!! FIXME: fluid_player_get_total_ticks can give us a time duration, but we don't have a player until we set up the track later.
    *audio_userdata = adata;

    const SDL_PropertiesID fluidsynth_props = (SDL_PropertiesID) SDL_GetNumberProperty(props, MIX_PROP_DECODER_FLUIDSYNTH_PROPS_NUMBER, 0);
    if (fluidsynth_props) {
        adata->fluidsynth_props = SDL_CreateProperties();
        if (!adata->fluidsynth_props || !SDL_CopyProperties(fluidsynth_props, adata->fluidsynth_props)) {
            if (adata->fluidsynth_props) {
                SDL_DestroyProperties(adata->fluidsynth_props);
            }
            SDL_free(sfdata);
            SDL_free(adata);
            return false;
        }
    }

    return true;
}

// this is obnoxious, but you have to implement a whole loader to get a soundfont from memory.
static void *SoundFontOpen(const char *filename)
{
    void *ptr = NULL;
    if (SDL_sscanf(filename, "&FAKESFNAME&%p", &ptr) != 1) {
        return NULL;
    }
    return ptr;  // (this is actually an SDL_IOStream pointer.)
}

static int SoundFontRead(void *buf, fluid_long_long_t count, void *handle)
{
    #if (SDL_SIZE_MAX < SDL_MAX_SINT64)
    if (count > (fluid_long_long_t)(SDL_MAX_UINT32)) {
        return FLUID_FAILED;
    }
    #endif
    if (count < 0) {
        return FLUID_FAILED;
    }
    return (SDL_ReadIO((SDL_IOStream *) handle, buf, count) == (size_t)count) ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontSeek(void *handle, fluid_long_long_t offset, int origin)
{
    SDL_IOWhence whence;
    switch (origin) {
        case SEEK_SET: whence = SDL_IO_SEEK_SET; break;
        case SEEK_CUR: whence = SDL_IO_SEEK_CUR; break;
        case SEEK_END: whence = SDL_IO_SEEK_END; break;
        default: return FLUID_FAILED;
    }
    return (SDL_SeekIO((SDL_IOStream *) handle, offset, whence) >= 0) ? FLUID_OK : FLUID_FAILED;
}

static int SoundFontClose(void *handle)
{
    SDL_CloseIO((SDL_IOStream *) handle);
    return FLUID_OK;
}

static fluid_long_long_t SoundFontTell(void *handle)
{
    return SDL_TellIO((SDL_IOStream *) handle);
}

static void SDLCALL SetCustomFluidsynthProperties(void *userdata, SDL_PropertiesID props, const char *name)
{
    FLUIDSYNTH_TrackData *tdata = (FLUIDSYNTH_TrackData *) userdata;
    switch (SDL_GetPropertyType(props, name)) {
        case SDL_PROPERTY_TYPE_NUMBER:
            fluidsynth.fluid_settings_setint(tdata->settings, name, (int) SDL_GetNumberProperty(props, name, 0));
            break;
        case SDL_PROPERTY_TYPE_FLOAT:
            fluidsynth.fluid_settings_setnum(tdata->settings, name, (double) SDL_GetFloatProperty(props, name, 0.0f));
            break;
        default: break;  // oh well.
    }
}

static bool SDLCALL FLUIDSYNTH_init_track(void *audio_userdata, SDL_IOStream *io, const SDL_AudioSpec *spec, SDL_PropertiesID props, void **track_userdata)
{
    FLUIDSYNTH_TrackData *tdata = (FLUIDSYNTH_TrackData *) SDL_calloc(1, sizeof (*tdata));
    if (!tdata) {
        return false;
    }

    const FLUIDSYNTH_AudioData *adata = (const FLUIDSYNTH_AudioData *) audio_userdata;
    double samplerate = 0.0;
    void *buffer = NULL;
    bool copied = false;
    size_t datalen = 0;

    tdata->settings = fluidsynth.new_fluid_settings();
    if (!tdata->settings) {
        SDL_SetError("Failed to create FluidSynth settings");
        goto failed;
    }

    //fluidsynth.fluid_settings_setint(tdata->settings, "synth.cpu-cores", 2);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.gain", 1.0);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.sample-rate", (double) spec->freq);
    fluidsynth.fluid_settings_getnum(tdata->settings, "synth.sample-rate", &samplerate);

    // these settings were recommended as good defaults in https://github.com/libsdl-org/SDL_mixer/issues/284
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.chorus.depth", 5.0);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.chorus.level", 0.35);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.reverb.damp", 0.4);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.reverb.level", 0.15);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.reverb.width", 4);
    fluidsynth.fluid_settings_setnum(tdata->settings, "synth.reverb.room-size", 0.6);

    // let custom properties override anything we already set internally. You break it, you buy it!
    SDL_EnumerateProperties(adata->fluidsynth_props, SetCustomFluidsynthProperties, tdata);

    tdata->synth = fluidsynth.new_fluid_synth(tdata->settings);
    if (!tdata->synth) {
        SDL_SetError("Failed to create FluidSynth synthesizer");
        goto failed;
    }

    if (adata->sfdata) {
        fluid_sfloader_t *sfloader = fluidsynth.new_fluid_defsfloader(tdata->settings);
        if (!sfloader) {
            SDL_SetError("Failed to create FluidSynth sfloader");
            goto failed;
        }

        char fakefname[64];
        SDL_snprintf(fakefname, sizeof (fakefname), "&FAKESFNAME&%p", SDL_IOFromConstMem(adata->sfdata, adata->sfdatalen));
        fluidsynth.fluid_sfloader_set_callbacks(sfloader, SoundFontOpen, SoundFontRead, SoundFontSeek, SoundFontTell, SoundFontClose);
        fluidsynth.fluid_synth_add_sfloader(tdata->synth, sfloader);
        if (fluidsynth.fluid_synth_sfload(tdata->synth, fakefname, 1) == FLUID_FAILED) {
            SDL_SetError("Failed to load FluidSynth soundfont");
            goto failed;
        }
    }

    // libfluidsynth doesn't have an abstract loader mechanism, you have to either give it a file path or the whole thing
    //  in one memory buffer upfront. Fortunately MIDI files tend to be extremely small, so just slurp it all in.
    // (and, if `io` appears to be a memory-based SDL_IOStream, just hand it the memory buffer directly.)
    buffer = MIX_SlurpConstIO(io, &datalen, &copied);
    if (!buffer) {
        goto failed;
    }

    tdata->player = fluidsynth.new_fluid_player(tdata->synth);
    if (!tdata->player) {
        SDL_SetError("Failed to create FluidSynth player");
        goto failed;
    }

    const bool okay = (fluidsynth.fluid_player_add_mem(tdata->player, buffer, datalen) == FLUID_OK);
    if (copied) {
        SDL_free(buffer);
        buffer = NULL;
    }

    if (!okay) {
        SDL_SetError("FluidSynth failed to load in-memory song");
        goto failed;
    }

    if (fluidsynth.fluid_player_play(tdata->player) != FLUID_OK) {
        SDL_SetError("Failed to start FluidSynth player");
        goto failed;
    }

    tdata->adata = adata;
    tdata->freq = (int) samplerate;

    *track_userdata = tdata;
    return true;

failed:
    SDL_assert(tdata != NULL);
    if (tdata->player) {
        fluidsynth.fluid_player_stop(tdata->player);
        fluidsynth.delete_fluid_player(tdata->player);
    }
    if (tdata->synth) {
        fluidsynth.delete_fluid_synth(tdata->synth);
    }
    if (tdata->settings) {
        fluidsynth.delete_fluid_settings(tdata->settings);
    }
    if (buffer && copied) {
        SDL_free(buffer);
    }
    SDL_free(tdata);

    return false;
}

static bool SDLCALL FLUIDSYNTH_decode(void *track_userdata, SDL_AudioStream *stream)
{
    FLUIDSYNTH_TrackData *tdata = (FLUIDSYNTH_TrackData *) track_userdata;

    if (fluidsynth.fluid_player_get_status(tdata->player) != FLUID_PLAYER_PLAYING) {
        return false;
    }

    float samples[256];
    if (fluidsynth.fluid_synth_write_float(tdata->synth, SDL_arraysize(samples) / 2, samples, 0, 2, samples, 1, 2) != FLUID_OK) {
        return false;  // maybe EOF...?
    }

    SDL_PutAudioStreamData(stream, samples, sizeof (samples));
    tdata->current_frame += SDL_arraysize(samples) / 2;
    return true;
}

static bool SDLCALL FLUIDSYNTH_seek(void *track_userdata, Uint64 frame)
{
    FLUIDSYNTH_TrackData *tdata = (FLUIDSYNTH_TrackData *) track_userdata;

    // This is expensive, but it's not trivial to seek to a specific frame in fluidsynth for various reasons.
    //  (see some explanations in https://github.com/libsdl-org/SDL_mixer/issues/519)
    if (tdata->current_frame > frame) {
        if (fluidsynth.fluid_player_seek(tdata->player, 0) != FLUID_OK) {
            return SDL_SetError("Couldn't rewind MIDI track");
        }
        tdata->current_frame = 0;

        if (fluidsynth.fluid_player_get_status(tdata->player) != FLUID_PLAYER_PLAYING) {
            if (fluidsynth.fluid_player_play(tdata->player) != FLUID_OK) {
                return SDL_SetError("Failed to restart FluidSynth player");
            }
        }
    }

    Uint64 remaining_frames = frame - tdata->current_frame;
    while (remaining_frames > 0) {
        if (fluidsynth.fluid_player_get_status(tdata->player) != FLUID_PLAYER_PLAYING) {
            return SDL_SetError("Seek past end of MIDI file");
        }

        float samples[512];
        const Uint64 write_frames = SDL_min((Uint64) (SDL_arraysize(samples) / 2), remaining_frames);
        if (fluidsynth.fluid_synth_write_float(tdata->synth, (int) write_frames, samples, 0, 2, samples, 1, 2) != FLUID_OK) {
            return SDL_SetError("Seek past end of MIDI file");  // maybe EOF...?
        }
        tdata->current_frame += write_frames;
        remaining_frames -= write_frames;
    }

    return true;
}

static void SDLCALL FLUIDSYNTH_quit_track(void *track_userdata)
{
    FLUIDSYNTH_TrackData *tdata = (FLUIDSYNTH_TrackData *) track_userdata;
    fluidsynth.fluid_player_stop(tdata->player);
    fluidsynth.delete_fluid_player(tdata->player);
    fluidsynth.delete_fluid_synth(tdata->synth);
    fluidsynth.delete_fluid_settings(tdata->settings);
    SDL_free(tdata);
}

static void SDLCALL FLUIDSYNTH_quit_audio(void *audio_userdata)
{
    FLUIDSYNTH_AudioData *adata = (FLUIDSYNTH_AudioData *) audio_userdata;
    SDL_DestroyProperties(adata->fluidsynth_props);
    SDL_free((void *) adata->sfdata);
    SDL_free(adata);
}

const MIX_Decoder MIX_Decoder_FLUIDSYNTH = {
    "FLUIDSYNTH",
    FLUIDSYNTH_init,
    FLUIDSYNTH_init_audio,
    FLUIDSYNTH_init_track,
    FLUIDSYNTH_decode,
    FLUIDSYNTH_seek,
    NULL,  // jump_to_order
    FLUIDSYNTH_quit_track,
    FLUIDSYNTH_quit_audio,
    FLUIDSYNTH_quit
};

#endif

