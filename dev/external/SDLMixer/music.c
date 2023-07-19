/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>

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
#include "SDL_hints.h"
#include "SDL_log.h"
#include "SDL_timer.h"

#include "SDL_mixer.h"
#include "mixer.h"
#include "music.h"

#include "music_cmd.h"
#include "music_wav.h"
#include "music_modplug.h"
#include "music_xmp.h"
#include "music_nativemidi.h"
#include "music_fluidsynth.h"
#include "music_timidity.h"
#include "music_ogg.h"
#include "music_opus.h"
#include "music_drmp3.h"
#include "music_mpg123.h"
#include "music_drflac.h"
#include "music_flac.h"
#include "native_midi/native_midi.h"

#include "utils.h"

/* Check to make sure we are building with a new enough SDL */
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(2, 0, 7)
#error You need SDL 2.0.7 or newer from http://www.libsdl.org
#endif

/* Set this hint to true if you want verbose logging of music interfaces */
#define SDL_MIXER_HINT_DEBUG_MUSIC_INTERFACES \
    "SDL_MIXER_DEBUG_MUSIC_INTERFACES"

char *music_cmd = NULL;
static SDL_bool music_active = SDL_TRUE;
static int music_volume = MIX_MAX_VOLUME;
static Mix_Music * volatile music_playing = NULL;
SDL_AudioSpec music_spec;

struct _Mix_Music {
    Mix_MusicInterface *interface;
    void *context;

    SDL_bool playing;
    Mix_Fading fading;
    int fade_step;
    int fade_steps;

    char filename[1024];
};

/* Used to calculate fading steps */
static int ms_per_step;

/* rcg06042009 report available decoders at runtime. */
static const char **music_decoders = NULL;
static int num_decoders = 0;

/* Semicolon-separated SoundFont paths */
static char* soundfont_paths = NULL;

/* full path of timidity config file */
static char* timidity_cfg = NULL;

/* Meta-Tags utility */
void meta_tags_init(Mix_MusicMetaTags *tags)
{
    SDL_memset(tags, 0, sizeof(Mix_MusicMetaTags));
}

void meta_tags_clear(Mix_MusicMetaTags *tags)
{
    size_t i;

    for (i = 0; i < MIX_META_LAST; i++) {
        if (tags->tags[i]) {
            SDL_free(tags->tags[i]);
            tags->tags[i] = NULL;
        }
    }
}

void meta_tags_set(Mix_MusicMetaTags *tags, Mix_MusicMetaTag type, const char *value)
{
    char *out;
    size_t len;

    if (!value) {
        return;
    }
    if (type >= MIX_META_LAST) {
        return;
    }

    len = SDL_strlen(value);
    out = (char *)SDL_malloc(sizeof(char) * len + 1);
    SDL_strlcpy(out, value, len +1);

    if (tags->tags[type]) {
        SDL_free(tags->tags[type]);
    }

    tags->tags[type] = out;
}

const char *meta_tags_get(Mix_MusicMetaTags *tags, Mix_MusicMetaTag type)
{
    switch (type) {
    case MIX_META_TITLE:
    case MIX_META_ARTIST:
    case MIX_META_ALBUM:
    case MIX_META_COPYRIGHT:
        return tags->tags[type] ? tags->tags[type] : "";
    case MIX_META_LAST:
    default:
        break;
    }
    return "";
}

/* for music->filename */
#if defined(__WIN32__)||defined(__OS2__)
static SDL_INLINE const char *get_last_dirsep (const char *p) {
    const char *p1 = SDL_strrchr(p, '/');
    const char *p2 = SDL_strrchr(p, '\\');
    if (!p1) return p2;
    if (!p2) return p1;
    return (p1 > p2)? p1 : p2;
}
#else /* unix */
static SDL_INLINE const char *get_last_dirsep (const char *p) {
    return SDL_strrchr(p, '/');
}
#endif


/* Interfaces for the various music interfaces, ordered by priority */
static Mix_MusicInterface *s_music_interfaces[] =
{
#ifdef MUSIC_CMD
    &Mix_MusicInterface_CMD,
#endif
#ifdef MUSIC_WAV
    &Mix_MusicInterface_WAV,
#endif
#ifdef MUSIC_FLAC_DRFLAC
    &Mix_MusicInterface_DRFLAC,
#endif
#ifdef MUSIC_FLAC_LIBFLAC
    &Mix_MusicInterface_FLAC,
#endif
#ifdef MUSIC_OGG
    &Mix_MusicInterface_OGG,
#endif
#ifdef MUSIC_OPUS
    &Mix_MusicInterface_Opus,
#endif
#ifdef MUSIC_MP3_DRMP3
    &Mix_MusicInterface_DRMP3,
#endif
#ifdef MUSIC_MP3_MPG123
    &Mix_MusicInterface_MPG123,
#endif
#ifdef MUSIC_MOD_XMP
    &Mix_MusicInterface_XMP,
#endif
#ifdef MUSIC_MOD_MODPLUG
    &Mix_MusicInterface_MODPLUG,
#endif
#ifdef MUSIC_MID_FLUIDSYNTH
    &Mix_MusicInterface_FLUIDSYNTH,
#endif
#ifdef MUSIC_MID_TIMIDITY
    &Mix_MusicInterface_TIMIDITY,
#endif
#ifdef MUSIC_MID_NATIVE
    &Mix_MusicInterface_NATIVEMIDI,
#endif
};

int get_num_music_interfaces(void)
{
    return SDL_arraysize(s_music_interfaces);
}

Mix_MusicInterface *get_music_interface(int index)
{
    return s_music_interfaces[index];
}

int Mix_GetNumMusicDecoders(void)
{
    return(num_decoders);
}

const char *Mix_GetMusicDecoder(int index)
{
    if ((index < 0) || (index >= num_decoders)) {
        return NULL;
    }
    return(music_decoders[index]);
}

SDL_bool Mix_HasMusicDecoder(const char *name)
{
    int index;
    for (index = 0; index < num_decoders; ++index) {
        if (SDL_strcasecmp(name, music_decoders[index]) == 0) {
                return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static void add_music_decoder(const char *decoder)
{
    void *ptr;
    int i;

    /* Check to see if we already have this decoder */
    for (i = 0; i < num_decoders; ++i) {
        if (SDL_strcmp(music_decoders[i], decoder) == 0) {
            return;
        }
    }

    ptr = SDL_realloc((void *)music_decoders, ((size_t)num_decoders + 1) * sizeof (const char *));
    if (ptr == NULL) {
        return;  /* oh well, go on without it. */
    }
    music_decoders = (const char **) ptr;
    music_decoders[num_decoders++] = decoder;
}

/* Local low-level functions prototypes */
static void music_internal_initialize_volume(void);
static void music_internal_volume(int volume);
static int  music_internal_play(Mix_Music *music, int play_count, double position);
static int  music_internal_position(double position);
static SDL_bool music_internal_playing(void);
static void music_internal_halt(void);


/* Support for hooking when the music has finished */
static void (SDLCALL *music_finished_hook)(void) = NULL;

void Mix_HookMusicFinished(void (SDLCALL *music_finished)(void))
{
    Mix_LockAudio();
    music_finished_hook = music_finished;
    Mix_UnlockAudio();
}

/* Convenience function to fill audio and mix at the specified volume
   This is called from many music player's GetAudio callback.
 */
int music_pcm_getaudio(void *context, void *data, int bytes, int volume,
                       int (*GetSome)(void *context, void *data, int bytes, SDL_bool *done))
{
    Uint8 *snd = (Uint8 *)data;
    Uint8 *dst;
    int len = bytes;
    int zero_cycles = 0;
    const int MAX_ZERO_CYCLES = 10; /* just try to catch infinite loops */
    SDL_bool done = SDL_FALSE;

    if (volume == MIX_MAX_VOLUME) {
        dst = snd;
    } else {
        dst = SDL_stack_alloc(Uint8, (size_t)bytes);
    }
    while (len > 0 && !done) {
        int consumed = GetSome(context, dst, len, &done);
        if (consumed < 0) {
            break;
        }
        if (consumed == 0) {
            ++zero_cycles;
            if (zero_cycles > MAX_ZERO_CYCLES) {
                /* We went too many cycles with no data, we're done */
                done = SDL_TRUE;
            }
            continue;
        }
        zero_cycles = 0;

        if (volume == MIX_MAX_VOLUME) {
            dst += consumed;
        } else {
            SDL_MixAudioFormat(snd, dst, music_spec.format, (Uint32)consumed, volume);
            snd += consumed;
        }
        len -= consumed;
    }
    if (volume != MIX_MAX_VOLUME) {
        SDL_stack_free(dst);
    }
    return len;
}

/* Mixing function */
void SDLCALL music_mixer(void *udata, Uint8 *stream, int len)
{
    SDL_bool done = SDL_FALSE;

    (void)udata;

    while (music_playing && music_active && len > 0 && !done) {
        /* Handle fading */
        if (music_playing->fading != MIX_NO_FADING) {
            if (music_playing->fade_step++ < music_playing->fade_steps) {
                int volume;
                int fade_step = music_playing->fade_step;
                int fade_steps = music_playing->fade_steps;

                if (music_playing->fading == MIX_FADING_OUT) {
                    volume = (music_volume * (fade_steps-fade_step)) / fade_steps;
                } else { /* Fading in */
                    volume = (music_volume * fade_step) / fade_steps;
                }
                music_internal_volume(volume);
            } else {
                if (music_playing->fading == MIX_FADING_OUT) {
                    music_internal_halt();
                    if (music_finished_hook) {
                        music_finished_hook();
                    }
                    return;
                }
                music_playing->fading = MIX_NO_FADING;
            }
        }

        if (music_playing->interface->GetAudio) {
            int left = music_playing->interface->GetAudio(music_playing->context, stream, len);
            if (left != 0) {
                /* Either an error or finished playing with data left */
                music_playing->playing = SDL_FALSE;
                done = SDL_TRUE;
            }
            if (left > 0) {
                stream += (len - left);
                len = left;
            } else {
                len = 0;
            }
        } else {
            len = 0;
        }

        if (!music_internal_playing()) {
            music_internal_halt();
            if (music_finished_hook) {
                music_finished_hook();
            }
        }
    }
}

/* Load the music interface libraries for a given music type */
SDL_bool load_music_type(Mix_MusicType type)
{
    size_t i;
    int loaded = 0;
    for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
        Mix_MusicInterface *interface = s_music_interfaces[i];
        if (interface->type != type) {
            continue;
        }
        if (!interface->loaded) {
            char hint[64];
            SDL_snprintf(hint, sizeof(hint), "SDL_MIXER_DISABLE_%s", interface->tag);
            if (SDL_GetHintBoolean(hint, SDL_FALSE)) {
                continue;
            }

            if (interface->Load && interface->Load() < 0) {
                if (SDL_GetHintBoolean(SDL_MIXER_HINT_DEBUG_MUSIC_INTERFACES, SDL_FALSE)) {
                    SDL_Log("Couldn't load %s: %s\n", interface->tag, Mix_GetError());
                }
                continue;
            }
            interface->loaded = SDL_TRUE;
        }
        ++loaded;
    }
    return (loaded > 0) ? SDL_TRUE : SDL_FALSE;
}

/* Open the music interfaces for a given music type */
SDL_bool open_music_type(Mix_MusicType type)
{
    size_t i;
    int opened = 0;
    SDL_bool use_native_midi = SDL_FALSE;

    if (!music_spec.format) {
        /* Music isn't opened yet */
        return SDL_FALSE;
    }

#ifdef MUSIC_MID_NATIVE
    if (type == MUS_MID && SDL_GetHintBoolean("SDL_NATIVE_MUSIC", SDL_FALSE) && native_midi_detect()) {
        use_native_midi = SDL_TRUE;
    }
#endif

    for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
        Mix_MusicInterface *interface = s_music_interfaces[i];
        if (!interface->loaded) {
            continue;
        }
        if (type != MUS_NONE && interface->type != type) {
            continue;
        }

        if (interface->type == MUS_MID && use_native_midi && interface->api != MIX_MUSIC_NATIVEMIDI) {
            continue;
        }

        if (!interface->opened) {
            if (interface->Open && interface->Open(&music_spec) < 0) {
                if (SDL_GetHintBoolean(SDL_MIXER_HINT_DEBUG_MUSIC_INTERFACES, SDL_FALSE)) {
                    SDL_Log("Couldn't open %s: %s\n", interface->tag, Mix_GetError());
                }
                continue;
            }
            interface->opened = SDL_TRUE;
            add_music_decoder(interface->tag);
        }
        ++opened;
    }

    if (has_music(MUS_MOD)) {
        add_music_decoder("MOD");
        add_chunk_decoder("MOD");
    }
    if (has_music(MUS_MID)) {
        add_music_decoder("MIDI");
        add_chunk_decoder("MID");
    }
    if (has_music(MUS_OGG)) {
        add_music_decoder("OGG");
        add_chunk_decoder("OGG");
    }
    if (has_music(MUS_OPUS)) {
        add_music_decoder("OPUS");
        add_chunk_decoder("OPUS");
    }
    if (has_music(MUS_MP3)) {
        add_music_decoder("MP3");
        add_chunk_decoder("MP3");
    }
    if (has_music(MUS_FLAC)) {
        add_music_decoder("FLAC");
        add_chunk_decoder("FLAC");
    }

    return (opened > 0) ? SDL_TRUE : SDL_FALSE;
}

/* Initialize the music interfaces with a certain desired audio format */
void open_music(const SDL_AudioSpec *spec)
{
#ifdef MIX_INIT_SOUNDFONT_PATHS
    if (!soundfont_paths) {
        soundfont_paths = SDL_strdup(MIX_INIT_SOUNDFONT_PATHS);
    }
#endif

    /* Load the music interfaces that don't have explicit initialization */
    load_music_type(MUS_CMD);
    load_music_type(MUS_WAV);

    /* Open all the interfaces that are loaded */
    music_spec = *spec;
    open_music_type(MUS_NONE);

    Mix_VolumeMusic(MIX_MAX_VOLUME);

    /* Calculate the number of ms for each callback */
    ms_per_step = (int) (((float)spec->samples * 1000.0f) / spec->freq);
}

/* Return SDL_TRUE if the music type is available */
SDL_bool has_music(Mix_MusicType type)
{
    size_t i;
    for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
        Mix_MusicInterface *interface = s_music_interfaces[i];
        if (interface->type != type) {
            continue;
        }
        if (interface->opened) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

Mix_MusicType detect_music_type(SDL_RWops *src)
{
    Uint8 magic[12];

    if (SDL_RWread(src, magic, 1, 12) != 12) {
        Mix_SetError("Couldn't read first 12 bytes of audio data");
        return MUS_NONE;
    }
    SDL_RWseek(src, -12, RW_SEEK_CUR);

    /* WAVE files have the magic four bytes "RIFF"
       AIFF files have the magic 12 bytes "FORM" XXXX "AIFF" */
    if (((SDL_memcmp(magic, "RIFF", 4) == 0) && (SDL_memcmp((magic+8), "WAVE", 4) == 0)) ||
        (SDL_memcmp(magic, "FORM", 4) == 0)) {
        return MUS_WAV;
    }

    /* Ogg Vorbis files have the magic four bytes "OggS" */
    if (SDL_memcmp(magic, "OggS", 4) == 0) {
        SDL_RWseek(src, 28, RW_SEEK_CUR);
        SDL_RWread(src, magic, 1, 8);
        SDL_RWseek(src,-36, RW_SEEK_CUR);
        if (SDL_memcmp(magic, "OpusHead", 8) == 0) {
            return MUS_OPUS;
        }
        return MUS_OGG;
    }

    /* FLAC files have the magic four bytes "fLaC" */
    if (SDL_memcmp(magic, "fLaC", 4) == 0) {
        return MUS_FLAC;
    }

    /* MIDI files have the magic four bytes "MThd" */
    if (SDL_memcmp(magic, "MThd", 4) == 0) {
        return MUS_MID;
    }

    if (SDL_memcmp(magic, "ID3", 3) == 0 ||
    /* see: https://bugzilla.libsdl.org/show_bug.cgi?id=5322 */
        (magic[0] == 0xFF && (magic[1] & 0xE6) == 0xE2)) {
        return MUS_MP3;
    }

    /* Assume MOD format.
     *
     * Apparently there is no way to check if the file is really a MOD,
     * or there are too many formats supported by libmodplug or libxmp.
     * The mod library does this check by itself. */
    return MUS_MOD;
}

/* Load a music file */
Mix_Music *Mix_LoadMUS(const char *file)
{
    size_t i;
    void *context;
    char *ext;
    Mix_MusicType type;
    SDL_RWops *src;

    for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
        Mix_MusicInterface *interface = s_music_interfaces[i];
        if (!interface->opened || !interface->CreateFromFile) {
            continue;
        }

        context = interface->CreateFromFile(file);
        if (context) {
            const char *p;
            /* Allocate memory for the music structure */
            Mix_Music *music = (Mix_Music *)SDL_calloc(1, sizeof(Mix_Music));
            if (music == NULL) {
                Mix_OutOfMemory();
                return NULL;
            }
            music->interface = interface;
            music->context = context;
            p = get_last_dirsep(file);
            SDL_strlcpy(music->filename, (p != NULL)? p + 1 : file, 1024);
            return music;
        }
    }

    src = SDL_RWFromFile(file, "rb");
    if (src == NULL) {
        Mix_SetError("Couldn't open '%s'", file);
        return NULL;
    }

    /* Use the extension as a first guess on the file type */
    type = MUS_NONE;
    ext = SDL_strrchr(file, '.');
    if (ext) {
        ++ext; /* skip the dot in the extension */
        if (SDL_strcasecmp(ext, "WAV") == 0) {
            type = MUS_WAV;
        } else if (SDL_strcasecmp(ext, "MID") == 0 ||
                    SDL_strcasecmp(ext, "MIDI") == 0 ||
                    SDL_strcasecmp(ext, "KAR") == 0) {
            type = MUS_MID;
        } else if (SDL_strcasecmp(ext, "OGG") == 0) {
            type = MUS_OGG;
        } else if (SDL_strcasecmp(ext, "OPUS") == 0) {
            type = MUS_OPUS;
        } else if (SDL_strcasecmp(ext, "FLAC") == 0) {
            type = MUS_FLAC;
        } else  if (SDL_strcasecmp(ext, "MPG") == 0 ||
                     SDL_strcasecmp(ext, "MPEG") == 0 ||
                     SDL_strcasecmp(ext, "MP3") == 0 ||
                     SDL_strcasecmp(ext, "MAD") == 0) {
            type = MUS_MP3;
        } else if (SDL_strcasecmp(ext, "669") == 0 ||
                    SDL_strcasecmp(ext, "AMF") == 0 ||
                    SDL_strcasecmp(ext, "AMS") == 0 ||
                    SDL_strcasecmp(ext, "DBM") == 0 ||
                    SDL_strcasecmp(ext, "DSM") == 0 ||
                    SDL_strcasecmp(ext, "FAR") == 0 ||
                    SDL_strcasecmp(ext, "GDM") == 0 ||
                    SDL_strcasecmp(ext, "IT") == 0 ||
                    SDL_strcasecmp(ext, "MED") == 0 ||
                    SDL_strcasecmp(ext, "MDL") == 0 ||
                    SDL_strcasecmp(ext, "MOD") == 0 ||
                    SDL_strcasecmp(ext, "MOL") == 0 ||
                    SDL_strcasecmp(ext, "MTM") == 0 ||
                    SDL_strcasecmp(ext, "NST") == 0 ||
                    SDL_strcasecmp(ext, "OKT") == 0 ||
                    SDL_strcasecmp(ext, "PTM") == 0 ||
                    SDL_strcasecmp(ext, "S3M") == 0 ||
                    SDL_strcasecmp(ext, "STM") == 0 ||
                    SDL_strcasecmp(ext, "ULT") == 0 ||
                    SDL_strcasecmp(ext, "UMX") == 0 ||
                    SDL_strcasecmp(ext, "WOW") == 0 ||
                    SDL_strcasecmp(ext, "XM") == 0) {
            type = MUS_MOD;
        }
    }
    return Mix_LoadMUSType_RW(src, type, SDL_TRUE);
}

Mix_Music *Mix_LoadMUS_RW(SDL_RWops *src, int freesrc)
{
    return Mix_LoadMUSType_RW(src, MUS_NONE, freesrc);
}

Mix_Music *Mix_LoadMUSType_RW(SDL_RWops *src, Mix_MusicType type, int freesrc)
{
    size_t i;
    void *context;
    Sint64 start;

    if (!src) {
        Mix_SetError("RWops pointer is NULL");
        return NULL;
    }
    start = SDL_RWtell(src);

    /* If the caller wants auto-detection, figure out what kind of file
     * this is. */
    if (type == MUS_NONE) {
        if ((type = detect_music_type(src)) == MUS_NONE) {
            /* Don't call Mix_SetError() since detect_music_type() does that. */
            if (freesrc) {
                SDL_RWclose(src);
            }
            return NULL;
        }
    }

    Mix_ClearError();

    if (load_music_type(type) && open_music_type(type)) {
        for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
            Mix_MusicInterface *interface = s_music_interfaces[i];
            if (!interface->opened || type != interface->type || !interface->CreateFromRW) {
                continue;
            }

            context = interface->CreateFromRW(src, freesrc);
            if (context) {
                /* Allocate memory for the music structure */
                Mix_Music *music = (Mix_Music *)SDL_calloc(1, sizeof(Mix_Music));
                if (music == NULL) {
                    interface->Delete(context);
                    Mix_OutOfMemory();
                    return NULL;
                }
                music->interface = interface;
                music->context = context;

                if (SDL_GetHintBoolean(SDL_MIXER_HINT_DEBUG_MUSIC_INTERFACES, SDL_FALSE)) {
                    SDL_Log("Loaded music with %s\n", interface->tag);
                }
                return music;
            }

            /* Reset the stream for the next decoder */
            SDL_RWseek(src, start, RW_SEEK_SET);
        }
    }

    if (!*Mix_GetError()) {
        Mix_SetError("Unrecognized audio format");
    }
    if (freesrc) {
        SDL_RWclose(src);
    } else {
        SDL_RWseek(src, start, RW_SEEK_SET);
    }
    return NULL;
}

/* Free a music chunk previously loaded */
void Mix_FreeMusic(Mix_Music *music)
{
    if (music) {
        /* Stop the music if it's currently playing */
        Mix_LockAudio();
        if (music == music_playing) {
            /* Wait for any fade out to finish */
            while (music_active && music->fading == MIX_FADING_OUT) {
                Mix_UnlockAudio();
                SDL_Delay(100);
                Mix_LockAudio();
            }
            if (music == music_playing) {
                music_internal_halt();
            }
        }
        Mix_UnlockAudio();

        music->interface->Delete(music->context);
        SDL_free(music);
    }
}

/* Find out the music format of a mixer music, or the currently playing
   music, if 'music' is NULL.
*/
Mix_MusicType Mix_GetMusicType(const Mix_Music *music)
{
    Mix_MusicType type = MUS_NONE;

    if (music) {
        type = music->interface->type;
    } else {
        Mix_LockAudio();
        if (music_playing) {
            type = music_playing->interface->type;
        }
        Mix_UnlockAudio();
    }
    return(type);
}

static const char * get_music_tag_internal(const Mix_Music *music, Mix_MusicMetaTag tag_type)
{
    const char *tag = "";

    Mix_LockAudio();
    if (music && music->interface->GetMetaTag) {
        tag = music->interface->GetMetaTag(music->context, tag_type);
    } else if (music_playing && music_playing->interface->GetMetaTag) {
        tag = music_playing->interface->GetMetaTag(music_playing->context, tag_type);
    } else {
        Mix_SetError("Music isn't playing");
    }
    Mix_UnlockAudio();
    return tag;
}

const char *Mix_GetMusicTitleTag(const Mix_Music *music)
{
    return get_music_tag_internal(music, MIX_META_TITLE);
}

/* Get music title from meta-tag if possible */
const char *Mix_GetMusicTitle(const Mix_Music *music)
{
    const char *tag = Mix_GetMusicTitleTag(music);
    if (SDL_strlen(tag) > 0) {
        return tag;
    }
    if (music) {
        return music->filename;
    }
    if (music_playing) {
        return music_playing->filename;
    }
    return "";
}

const char *Mix_GetMusicArtistTag(const Mix_Music *music)
{
    return get_music_tag_internal(music, MIX_META_ARTIST);
}

const char *Mix_GetMusicAlbumTag(const Mix_Music *music)
{
    return get_music_tag_internal(music, MIX_META_ALBUM);
}

const char *Mix_GetMusicCopyrightTag(const Mix_Music *music)
{
    return get_music_tag_internal(music, MIX_META_COPYRIGHT);
}

/* Play a music chunk.  Returns 0, or -1 if there was an error.
 */
static int music_internal_play(Mix_Music *music, int play_count, double position)
{
    int retval = 0;

    /* Note the music we're playing */
    if (music_playing) {
        music_internal_halt();
    }
    music_playing = music;
    music_playing->playing = SDL_TRUE;

    /* Set the initial volume */
    music_internal_initialize_volume();

    /* Set up for playback */
    retval = music->interface->Play(music->context, play_count);

    /* Set the playback position, note any errors if an offset is used */
    if (retval == 0) {
        if (position > 0.0) {
            if (music_internal_position(position) < 0) {
                Mix_SetError("Position not implemented for music type");
                retval = -1;
            }
        } else {
            music_internal_position(0.0);
        }
    }

    /* If the setup failed, we're not playing any music anymore */
    if (retval < 0) {
        music->playing = SDL_FALSE;
        music_playing = NULL;
    }
    return(retval);
}

int Mix_FadeInMusicPos(Mix_Music *music, int loops, int ms, double position)
{
    int retval;

    if (ms_per_step == 0) {
        Mix_SetError("Audio device hasn't been opened");
        return(-1);
    }

    /* Don't play null pointers :-) */
    if (music == NULL) {
        Mix_SetError("music parameter was NULL");
        return(-1);
    }

    /* Setup the data */
    if (ms) {
        music->fading = MIX_FADING_IN;
    } else {
        music->fading = MIX_NO_FADING;
    }
    music->fade_step = 0;
    music->fade_steps = (ms + ms_per_step - 1) / ms_per_step;

    /* Play the puppy */
    Mix_LockAudio();
    /* If the current music is fading out, wait for the fade to complete */
    while (music_playing && (music_playing->fading == MIX_FADING_OUT)) {
        Mix_UnlockAudio();
        SDL_Delay(100);
        Mix_LockAudio();
    }
    if (loops == 0) {
        /* Loop is the number of times to play the audio */
        loops = 1;
    }
    retval = music_internal_play(music, loops, position);
    /* Set music as active */
    music_active = (retval == 0);
    Mix_UnlockAudio();

    return(retval);
}
int Mix_FadeInMusic(Mix_Music *music, int loops, int ms)
{
    return Mix_FadeInMusicPos(music, loops, ms, 0.0);
}
int Mix_PlayMusic(Mix_Music *music, int loops)
{
    return Mix_FadeInMusicPos(music, loops, 0, 0.0);
}

/* Jump to a given order in mod music. */
int Mix_ModMusicJumpToOrder(int order)
{
    int retval = -1;

    Mix_LockAudio();
    if (music_playing) {
        if (music_playing->interface->Jump) {
            retval = music_playing->interface->Jump(music_playing->context, order);
        } else {
            Mix_SetError("Jump not implemented for music type");
        }
    } else {
        Mix_SetError("Music isn't playing");
    }
    Mix_UnlockAudio();

    return retval;
}

/* Set the playing music position */
int music_internal_position(double position)
{
    if (music_playing->interface->Seek) {
        return music_playing->interface->Seek(music_playing->context, position);
    }
    return -1;
}
int Mix_SetMusicPosition(double position)
{
    int retval;

    Mix_LockAudio();
    if (music_playing) {
        retval = music_internal_position(position);
        if (retval < 0) {
            Mix_SetError("Position not implemented for music type");
        }
    } else {
        Mix_SetError("Music isn't playing");
        retval = -1;
    }
    Mix_UnlockAudio();

    return(retval);
}

/* Set the playing music position */
static double music_internal_position_get(Mix_Music *music)
{
    if (music->interface->Tell) {
        return music->interface->Tell(music->context);
    }
    return -1;
}
double Mix_GetMusicPosition(Mix_Music *music)
{
    double retval;

    Mix_LockAudio();
    if (music) {
        retval = music_internal_position_get(music);
    } else if (music_playing) {
        retval = music_internal_position_get(music_playing);
    } else {
        Mix_SetError("Music isn't playing");
        retval = -1.0;
    }
    Mix_UnlockAudio();

    return(retval);
}

static double music_internal_duration(Mix_Music *music)
{
    if (music->interface->Duration) {
        return music->interface->Duration(music->context);
    } else {
        Mix_SetError("Duration not implemented for music type");
        return -1;
    }
}
double Mix_MusicDuration(Mix_Music *music)
{
    double retval;

    Mix_LockAudio();
    if (music) {
        retval = music_internal_duration(music);
    } else if (music_playing) {
        retval = music_internal_duration(music_playing);
    } else {
        Mix_SetError("music is NULL and no playing music");
        retval = -1.0;
    }
    Mix_UnlockAudio();

    return(retval);
}

/* Get Loop start position */
static double music_internal_loop_start(Mix_Music *music)
{
    if (music->interface->LoopStart) {
        return music->interface->LoopStart(music->context);
    }
    return -1;
}
double Mix_GetMusicLoopStartTime(Mix_Music *music)
{
    double retval;

    Mix_LockAudio();
    if (music) {
        retval = music_internal_loop_start(music);
    } else if (music_playing) {
        retval = music_internal_loop_start(music_playing);
    } else {
        Mix_SetError("Music isn't playing");
        retval = -1.0;
    }
    Mix_UnlockAudio();

    return(retval);
}

/* Get Loop end position */
static double music_internal_loop_end(Mix_Music *music)
{
    if (music->interface->LoopEnd) {
        return music->interface->LoopEnd(music->context);
    }
    return -1;
}
double Mix_GetMusicLoopEndTime(Mix_Music *music)
{
    double retval;

    Mix_LockAudio();
    if (music) {
        retval = music_internal_loop_end(music);
    } else if (music_playing) {
        retval = music_internal_loop_end(music_playing);
    } else {
        Mix_SetError("Music isn't playing");
        retval = -1.0;
    }
    Mix_UnlockAudio();

    return(retval);
}

/* Get Loop end position */
static double music_internal_loop_length(Mix_Music *music)
{
    if (music->interface->LoopLength) {
        return music->interface->LoopLength(music->context);
    }
    return -1;
}
double Mix_GetMusicLoopLengthTime(Mix_Music *music)
{
    double retval;

    Mix_LockAudio();
    if (music) {
        retval = music_internal_loop_length(music);
    } else if (music_playing) {
        retval = music_internal_loop_length(music_playing);
    } else {
        Mix_SetError("Music isn't playing");
        retval = -1.0;
    }
    Mix_UnlockAudio();

    return(retval);
}

/* Set the music's initial volume */
static void music_internal_initialize_volume(void)
{
    if (music_playing->fading == MIX_FADING_IN) {
        music_internal_volume(0);
    } else {
        music_internal_volume(music_volume);
    }
}

/* Set the music volume */
static void music_internal_volume(int volume)
{
    if (music_playing->interface->SetVolume) {
        music_playing->interface->SetVolume(music_playing->context, volume);
    }
}
int Mix_VolumeMusic(int volume)
{
    int prev_volume;

    prev_volume = music_volume;
    if (volume < 0) {
        return prev_volume;
    }
    if (volume > SDL_MIX_MAXVOLUME) {
        volume = SDL_MIX_MAXVOLUME;
    }
    music_volume = volume;
    Mix_LockAudio();
    if (music_playing) {
        music_internal_volume(music_volume);
    }
    Mix_UnlockAudio();
    return(prev_volume);
}

int Mix_GetMusicVolume(Mix_Music *music)
{
    int prev_volume;

    if (music && music->interface->GetVolume)
        prev_volume = music->interface->GetVolume(music->context);
    else if (music_playing && music_playing->interface->GetVolume) {
        prev_volume = music_playing->interface->GetVolume(music_playing->context);
    } else {
        prev_volume = music_volume;
    }

    return prev_volume;
}

/* Halt playing of music */
static void music_internal_halt(void)
{
    if (music_playing->interface->Stop) {
        music_playing->interface->Stop(music_playing->context);
    }

    music_playing->playing = SDL_FALSE;
    music_playing->fading = MIX_NO_FADING;
    music_playing = NULL;
}
int Mix_HaltMusic(void)
{
    Mix_LockAudio();
    if (music_playing) {
        music_internal_halt();
        if (music_finished_hook) {
            music_finished_hook();
        }
    }
    Mix_UnlockAudio();

    return(0);
}

/* Progressively stop the music */
int Mix_FadeOutMusic(int ms)
{
    int retval = 0;

    if (ms_per_step == 0) {
        Mix_SetError("Audio device hasn't been opened");
        return 0;
    }

    if (ms <= 0) {  /* just halt immediately. */
        Mix_HaltMusic();
        return 1;
    }

    Mix_LockAudio();
    if (music_playing) {
        int fade_steps = (ms + ms_per_step - 1) / ms_per_step;
        if (music_playing->fading == MIX_NO_FADING) {
            music_playing->fade_step = 0;
        } else {
            int step;
            int old_fade_steps = music_playing->fade_steps;
            if (music_playing->fading == MIX_FADING_OUT) {
                step = music_playing->fade_step;
            } else {
                step = old_fade_steps - music_playing->fade_step + 1;
            }
            music_playing->fade_step = (step * fade_steps) / old_fade_steps;
        }
        music_playing->fading = MIX_FADING_OUT;
        music_playing->fade_steps = fade_steps;
        retval = 1;
    }
    Mix_UnlockAudio();

    return(retval);
}

Mix_Fading Mix_FadingMusic(void)
{
    Mix_Fading fading = MIX_NO_FADING;

    Mix_LockAudio();
    if (music_playing) {
        fading = music_playing->fading;
    }
    Mix_UnlockAudio();

    return(fading);
}

/* Pause/Resume the music stream */
void Mix_PauseMusic(void)
{
    Mix_LockAudio();
    if (music_playing) {
        if (music_playing->interface->Pause) {
            music_playing->interface->Pause(music_playing->context);
        }
    }
    music_active = SDL_FALSE;
    Mix_UnlockAudio();
}

void Mix_ResumeMusic(void)
{
    Mix_LockAudio();
    if (music_playing) {
        if (music_playing->interface->Resume) {
            music_playing->interface->Resume(music_playing->context);
        }
    }
    music_active = SDL_TRUE;
    Mix_UnlockAudio();
}

void Mix_RewindMusic(void)
{
    Mix_SetMusicPosition(0.0);
}

int Mix_PausedMusic(void)
{
    return (music_active == SDL_FALSE);
}

/* Check the status of the music */
static SDL_bool music_internal_playing(void)
{
    if (!music_playing) {
        return SDL_FALSE;
    }

    if (music_playing->interface->IsPlaying) {
        music_playing->playing = music_playing->interface->IsPlaying(music_playing->context);
    }
    return music_playing->playing;
}
int Mix_PlayingMusic(void)
{
    SDL_bool playing;

    Mix_LockAudio();
    playing = music_internal_playing();
    Mix_UnlockAudio();

    return playing ? 1 : 0;
}

/* Set the external music playback command */
int Mix_SetMusicCMD(const char *command)
{
    Mix_HaltMusic();
    if (music_cmd) {
        SDL_free(music_cmd);
        music_cmd = NULL;
    }
    if (command) {
        size_t length = SDL_strlen(command) + 1;
        music_cmd = (char *)SDL_malloc(length);
        if (music_cmd == NULL) {
            return SDL_OutOfMemory();
        }
        SDL_memcpy(music_cmd, command, length);
    }
    return 0;
}

int Mix_SetSynchroValue(int i)
{
    /* Not supported by any players at this time */
    (void) i;
    return -1;
}

int Mix_GetSynchroValue(void)
{
    /* Not supported by any players at this time */
    return -1;
}


/* Uninitialize the music interfaces */
void close_music(void)
{
    size_t i;

    Mix_HaltMusic();

    for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
        Mix_MusicInterface *interface = s_music_interfaces[i];
        if (!interface || !interface->opened) {
            continue;
        }

        if (interface->Close) {
            interface->Close();
        }
        interface->opened = SDL_FALSE;
    }

    if (soundfont_paths) {
        SDL_free(soundfont_paths);
        soundfont_paths = NULL;
    }

    /* rcg06042009 report available decoders at runtime. */
    if (music_decoders) {
        SDL_free((void *)music_decoders);
        music_decoders = NULL;
    }
    num_decoders = 0;

    ms_per_step = 0;
}

/* Unload the music interface libraries */
void unload_music(void)
{
    size_t i;
    for (i = 0; i < SDL_arraysize(s_music_interfaces); ++i) {
        Mix_MusicInterface *interface = s_music_interfaces[i];
        if (!interface || !interface->loaded) {
            continue;
        }

        if (interface->Unload) {
            interface->Unload();
        }
        interface->loaded = SDL_FALSE;
    }
}

int Mix_SetTimidityCfg(const char *path)
{
    if (timidity_cfg) {
        SDL_free(timidity_cfg);
        timidity_cfg = NULL;
    }

    if (path && *path) {
        if (!(timidity_cfg = SDL_strdup(path))) {
            Mix_SetError("Insufficient memory to set Timidity cfg file");
            return 0;
        }
    }

    return 1;
}

const char* Mix_GetTimidityCfg(void)
{
    return timidity_cfg;
}

int Mix_SetSoundFonts(const char *paths)
{
    if (soundfont_paths) {
        SDL_free(soundfont_paths);
        soundfont_paths = NULL;
    }

    if (paths) {
        if (!(soundfont_paths = SDL_strdup(paths))) {
            Mix_SetError("Insufficient memory to set SoundFonts");
            return 0;
        }
    }
    return 1;
}

const char* Mix_GetSoundFonts(void)
{
    const char *env_paths = SDL_getenv("SDL_SOUNDFONTS");
    SDL_bool force_env_paths = SDL_GetHintBoolean("SDL_FORCE_SOUNDFONTS", SDL_FALSE);
    if (force_env_paths && (!env_paths || !*env_paths)) {
        force_env_paths = SDL_FALSE;
    }
    if (soundfont_paths && *soundfont_paths && !force_env_paths) {
        return soundfont_paths;
    }
    if (env_paths) {
        return env_paths;
    }

    /* We don't have any sound fonts set programmatically or in the environment
       Time to start guessing where they might be...
     */
    {
        static char *s_soundfont_paths[] = {
            "/usr/share/sounds/sf2/FluidR3_GM.sf2"  /* Remember to add ',' here */
        };
        unsigned i;

        for (i = 0; i < SDL_arraysize(s_soundfont_paths); ++i) {
            SDL_RWops *rwops = SDL_RWFromFile(s_soundfont_paths[i], "rb");
            if (rwops) {
                SDL_RWclose(rwops);
                return s_soundfont_paths[i];
            }
        }
    }
    return NULL;
}

int Mix_EachSoundFont(int (SDLCALL *function)(const char*, void*), void *data)
{
    char *context, *path, *paths;
    const char* cpaths = Mix_GetSoundFonts();
    int soundfonts_found = 0;

    if (!cpaths) {
        Mix_SetError("No SoundFonts have been requested");
        return 0;
    }

    if (!(paths = SDL_strdup(cpaths))) {
        Mix_SetError("Insufficient memory to iterate over SoundFonts");
        return 0;
    }

#if defined(_WIN32) || defined(__OS2__)
#define PATHSEP ";"
#else
#define PATHSEP ":;"
#endif
    for (path = SDL_strtokr(paths, PATHSEP, &context); path;
         path = SDL_strtokr(NULL,  PATHSEP, &context)) {
        if (!function(path, data)) {
            continue;
        }
        soundfonts_found++;
    }

    SDL_free(paths);
    return (soundfonts_found > 0);
}

/* vi: set ts=4 sw=4 expandtab: */
