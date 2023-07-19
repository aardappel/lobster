/*
  native_midi_macosx:  Native Midi support on Mac OS X for the SDL_mixer library
  Copyright (C) 2009  Ryan C. Gordon <icculus@icculus.org>

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

#include "SDL_config.h"

#if __MACOSX__

/* Mac OS X 10.6+, using Core MIDI. */

#include "SDL_stdinc.h"

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AvailabilityMacros.h>

#include "SDL_endian.h"
#include "SDL_mixer.h"
#include "../../mixer.h"
#include "native_midi.h"

/* Native Midi song */
struct _NativeMidiSong
{
    MusicPlayer player;
    MusicSequence sequence;
    MusicTimeStamp endTime;
    AudioUnit audiounit;
    int loops;
};

static NativeMidiSong *currentsong = NULL;
static int latched_volume = MIX_MAX_VOLUME;

static OSStatus
GetSequenceLength(MusicSequence sequence, MusicTimeStamp *_sequenceLength)
{
    /* http://lists.apple.com/archives/Coreaudio-api/2003/Jul/msg00370.html
     * figure out sequence length */
    UInt32 ntracks, i;
    MusicTimeStamp sequenceLength = 0;
    OSStatus err;

    err = MusicSequenceGetTrackCount(sequence, &ntracks);
    if (err != noErr)
        return err;

    for (i = 0; i < ntracks; ++i)
    {
        MusicTrack track;
        MusicTimeStamp tracklen = 0;
        UInt32 tracklenlen = sizeof (tracklen);

        err = MusicSequenceGetIndTrack(sequence, i, &track);
        if (err != noErr)
            return err;

        err = MusicTrackGetProperty(track, kSequenceTrackProperty_TrackLength,
                                    &tracklen, &tracklenlen);
        if (err != noErr)
            return err;

        if (sequenceLength < tracklen)
            sequenceLength = tracklen;
    }

    *_sequenceLength = sequenceLength;

    return noErr;
}

static OSStatus
GetSequenceAudioUnitMatching(MusicSequence sequence, AudioUnit *aunit,
                             OSType type, OSType subtype)
{
    AUGraph graph;
    UInt32 nodecount, i;
    OSStatus err;

    err = MusicSequenceGetAUGraph(sequence, &graph);
    if (err != noErr)
        return err;

    err = AUGraphGetNodeCount(graph, &nodecount);
    if (err != noErr)
        return err;

    for (i = 0; i < nodecount; i++) {
        AUNode node;
        AudioComponentDescription desc;

        if (AUGraphGetIndNode(graph, i, &node) != noErr)
            continue;  /* better luck next time. */

        if (AUGraphNodeInfo(graph, node, &desc, aunit) != noErr)
            continue;
        else if (desc.componentType != type)
            continue;
        else if (desc.componentSubType != subtype)
            continue;

        return noErr;  /* found it! */
    }

    *aunit = NULL;
    return kAUGraphErr_NodeNotFound;
}

typedef struct {
    AudioUnit aunit;
    SDL_bool soundfont_set;
    CFURLRef default_url;
} macosx_load_soundfont_ctx;

static int SDLCALL
macosx_load_soundfont(const char *path, void *data)
{
    CFURLRef url;
    OSStatus err;
    macosx_load_soundfont_ctx *ctx = data;
    if (ctx->soundfont_set)
        return SDL_FALSE;

    url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                  (const UInt8*)path,
                                                  strlen(path), false);
    if (!url)
        return SDL_FALSE;

    err = AudioUnitSetProperty(ctx->aunit, kMusicDeviceProperty_SoundBankURL,
                               kAudioUnitScope_Global, 0, &url, sizeof(url));
    CFRelease(url);
    if (err != noErr) {
        if (ctx->default_url)
            err = AudioUnitSetProperty(ctx->aunit,
                                       kMusicDeviceProperty_SoundBankURL,
                                       kAudioUnitScope_Global, 0,
                                       &ctx->default_url, sizeof(CFURLRef));
        if (err != noErr) {
            /* uh-oh, this might leave the audio unit in an unusable state
               (e.g. if the soundfont was an incompatible file type) */
        }
        return SDL_FALSE;
    }

    ctx->soundfont_set = SDL_TRUE;
    return SDL_TRUE;
}

static void
SetSequenceSoundFont(MusicSequence sequence)
{
    OSStatus err;
    macosx_load_soundfont_ctx ctx;
    ctx.soundfont_set = SDL_FALSE;
    ctx.default_url = NULL;

    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(
                                   CFSTR("com.apple.audio.units.Components"));
    if (bundle)
        ctx.default_url = CFBundleCopyResourceURL(bundle,
                                                  CFSTR("gs_instruments"),
                                                  CFSTR("dls"), NULL);

    err = GetSequenceAudioUnitMatching(sequence, &ctx.aunit,
                                 kAudioUnitType_MusicDevice,
                                 kAudioUnitSubType_DLSSynth);
    if (err != noErr)
        return;

    Mix_EachSoundFont(macosx_load_soundfont, &ctx);
    if (ctx.default_url)
        CFRelease(ctx.default_url);
    return;
}

int native_midi_detect(void)
{
    return 1;  /* always available. */
}

NativeMidiSong *native_midi_loadsong_RW(SDL_RWops *src, int freesrc)
{
    NativeMidiSong *retval = NULL;
    void *buf = NULL;
    Sint64 len = 0;
    CFDataRef data = NULL;

    if (SDL_RWseek(src, 0, RW_SEEK_END) < 0)
        goto fail;
    len = SDL_RWtell(src);
    if (len < 0)
        goto fail;
    if (SDL_RWseek(src, 0, RW_SEEK_SET) < 0)
        goto fail;

    buf = SDL_malloc(len);
    if (buf == NULL)
        goto fail;

    if (SDL_RWread(src, buf, len, 1) != 1)
        goto fail;

    retval = SDL_malloc(sizeof(NativeMidiSong));
    if (retval == NULL)
        goto fail;

    SDL_memset(retval, '\0', sizeof (*retval));

    if (NewMusicPlayer(&retval->player) != noErr)
        goto fail;
    if (NewMusicSequence(&retval->sequence) != noErr)
        goto fail;

    data = CFDataCreate(NULL, (const UInt8 *) buf, len);
    if (data == NULL)
        goto fail;

    SDL_free(buf);
    buf = NULL;

    if (MusicSequenceFileLoadData(retval->sequence, data, 0, 0) != noErr)
        goto fail;

    CFRelease(data);
    data = NULL;

    if (GetSequenceLength(retval->sequence, &retval->endTime) != noErr)
        goto fail;

    if (MusicPlayerSetSequence(retval->player, retval->sequence) != noErr)
        goto fail;

    if (freesrc)
        SDL_RWclose(src);

    return retval;

fail:
    if (retval) {
        if (retval->sequence)
            DisposeMusicSequence(retval->sequence);
        if (retval->player)
            DisposeMusicPlayer(retval->player);
        SDL_free(retval);
    }

    if (data)
        CFRelease(data);

    if (buf)
        SDL_free(buf);

    return NULL;
}

void native_midi_freesong(NativeMidiSong *song)
{
    if (song != NULL) {
        if (currentsong == song)
            currentsong = NULL;
        MusicPlayerStop(song->player);

        /* needed to prevent error and memory leak when disposing sequence */
        MusicPlayerSetSequence(song->player, NULL);

        DisposeMusicSequence(song->sequence);
        DisposeMusicPlayer(song->player);
        SDL_free(song);
    }
}

void native_midi_start(NativeMidiSong *song, int loops)
{
    int vol;

    if (song == NULL)
        return;

    if (currentsong)
        MusicPlayerStop(currentsong->player);

    currentsong = song;
    currentsong->loops = loops;

    MusicPlayerPreroll(song->player);
    GetSequenceAudioUnitMatching(song->sequence, &song->audiounit,
                                 kAudioUnitType_Output,
                                 kAudioUnitSubType_DefaultOutput);
    SetSequenceSoundFont(song->sequence);

    vol = latched_volume;
    latched_volume++;  /* just make this not match. */
    native_midi_setvolume(vol);

    MusicPlayerSetTime(song->player, 0);
    MusicPlayerStart(song->player);
}

void native_midi_pause(void)
{
}

void native_midi_resume(void)
{
}

void native_midi_stop(void)
{
    if (currentsong) {
        MusicPlayerStop(currentsong->player);
        currentsong = NULL;
    }
}

int native_midi_active(void)
{
    MusicTimeStamp currentTime = 0;
    if (currentsong == NULL)
        return 0;

    MusicPlayerGetTime(currentsong->player, &currentTime);
    if ((currentTime < currentsong->endTime) ||
        (currentTime >= kMusicTimeStamp_EndOfTrack)) {
        return 1;
    } else if (currentsong->loops) {
        --currentsong->loops;
        MusicPlayerSetTime(currentsong->player, 0);
        return 1;
    }
    return 0;
}

void native_midi_setvolume(int volume)
{
    if (latched_volume == volume)
        return;

    latched_volume = volume;
    if ((currentsong) && (currentsong->audiounit)) {
        const float floatvol = ((float) volume) / ((float) MIX_MAX_VOLUME);
        AudioUnitSetParameter(currentsong->audiounit, kHALOutputParam_Volume,
                              kAudioUnitScope_Global, 0, floatvol, 0);
    }
}

const char *native_midi_error(void)
{
    return "";  /* !!! FIXME */
}

#endif /* Mac OS X native MIDI support */
