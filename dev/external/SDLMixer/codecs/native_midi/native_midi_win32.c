/*
  native_midi:  Hardware Midi support for the SDL_mixer library
  Copyright (C) 2000,2001  Florian 'Proff' Schulze <florian.proff.schulze@gmx.net>

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

/* everything below is currently one very big bad hack ;) Proff */

#if __WIN32__
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>

#include "native_midi.h"
#include "native_midi_common.h"

struct _NativeMidiSong {
  int MusicLoaded;
  int MusicPlaying;
  int Loops;
  int CurrentHdr;
  MIDIHDR MidiStreamHdr[2];
  MIDIEVENT *NewEvents;
  Uint16 ppqn;
  int Size;
  int NewPos;
  SDL_mutex *mutex;
};

static UINT MidiDevice=MIDI_MAPPER;
static HMIDISTRM hMidiStream;
static NativeMidiSong *currentsong;

static int BlockOut(NativeMidiSong *song)
{
  MMRESULT err;
  int BlockSize;
  MIDIHDR *hdr;

  if ((song->MusicLoaded) && (song->NewEvents))
  {
    /* proff 12/8/98: Added for safety*/
    song->CurrentHdr = !song->CurrentHdr;
    hdr = &song->MidiStreamHdr[song->CurrentHdr];
    midiOutUnprepareHeader((HMIDIOUT)hMidiStream,hdr,sizeof(MIDIHDR));
    if (song->NewPos>=song->Size)
      return 0;
    BlockSize=(song->Size-song->NewPos);
    if (BlockSize<=0)
      return 0;
    if (BlockSize>36000)
      BlockSize=36000;
    hdr->lpData=(void *)((unsigned char *)song->NewEvents+song->NewPos);
    song->NewPos+=BlockSize;
    hdr->dwBufferLength=BlockSize;
    hdr->dwBytesRecorded=BlockSize;
    hdr->dwFlags=0;
    hdr->dwOffset=0;
    err=midiOutPrepareHeader((HMIDIOUT)hMidiStream,hdr,sizeof(MIDIHDR));
    if (err!=MMSYSERR_NOERROR)
      return 0;
    err=midiStreamOut(hMidiStream,hdr,sizeof(MIDIHDR));
      return 0;
  }
  return 1;
}

static void MIDItoStream(NativeMidiSong *song, MIDIEvent *evntlist)
{
  int eventcount;
  MIDIEvent *event;
  MIDIEVENT *newevent;

  eventcount=0;
  event=evntlist;
  while (event)
  {
    eventcount++;
    event=event->next;
  }
  song->NewEvents = SDL_malloc(eventcount*3*sizeof(DWORD));
  if (!song->NewEvents)
    return;
  SDL_memset(song->NewEvents,0,(eventcount*3*sizeof(DWORD)));

  eventcount=0;
  event=evntlist;
  newevent=song->NewEvents;
  while (event)
  {
        int status = (event->status&0xF0)>>4;
        switch (status)
        {
        case MIDI_STATUS_NOTE_OFF:
        case MIDI_STATUS_NOTE_ON:
        case MIDI_STATUS_AFTERTOUCH:
        case MIDI_STATUS_CONTROLLER:
        case MIDI_STATUS_PROG_CHANGE:
        case MIDI_STATUS_PRESSURE:
        case MIDI_STATUS_PITCH_WHEEL:
      newevent->dwDeltaTime=event->time;
          newevent->dwEvent=(event->status|0x80)|(event->data[0]<<8)|(event->data[1]<<16)|(MEVT_SHORTMSG<<24);
      newevent=(MIDIEVENT*)((char*)newevent+(3*sizeof(DWORD)));
      eventcount++;
            break;

        case MIDI_STATUS_SYSEX:
            if (event->status == 0xFF && event->data[0] == 0x51) /* Tempo change */
            {
                int tempo = (event->extraData[0] << 16) |
                              (event->extraData[1] << 8) |
                               event->extraData[2];
        newevent->dwDeltaTime=event->time;
                newevent->dwEvent=(MEVT_TEMPO<<24) | tempo;
        newevent=(MIDIEVENT*)((char*)newevent+(3*sizeof(DWORD)));
        eventcount++;
            }
            break;
    }

    event=event->next;
  }

  song->Size=eventcount*3*sizeof(DWORD);

  {
    int time;
    int temptime;

    song->NewPos=0;
    time=0;
    newevent=song->NewEvents;
    while (song->NewPos<song->Size)
    {
      temptime=newevent->dwDeltaTime;
      newevent->dwDeltaTime-=time;
      time=temptime;
      if ((song->NewPos+12)>=song->Size)
        newevent->dwEvent |= MEVT_F_CALLBACK;
      newevent=(MIDIEVENT*)((char*)newevent+(3*sizeof(DWORD)));
      song->NewPos+=12;
    }
  }
  song->NewPos=0;
  song->MusicLoaded=1;
}

void CALLBACK MidiProc( HMIDIIN hMidi, UINT uMsg, DWORD_PTR dwInstance,
                        DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
{
    NativeMidiSong *song = (NativeMidiSong *)dwInstance;
    (void)hMidi;
    (void)dwParam2;

    if (!song) {
        return;
    }

    SDL_LockMutex(song->mutex);
    switch( uMsg )
    {
    case MOM_DONE:
      if (song->MusicPlaying && song->MusicLoaded && (dwParam1 == (DWORD_PTR)&song->MidiStreamHdr[song->CurrentHdr]))
        BlockOut(song);
      break;
    case MOM_POSITIONCB:
      if (song->MusicPlaying && song->MusicLoaded && (dwParam1 == (DWORD_PTR)&song->MidiStreamHdr[song->CurrentHdr])) {
        if (song->Loops) {
          if (song->Loops > 0)
            --song->Loops;
          song->NewPos=0;
          BlockOut(song);
        } else {
          song->MusicPlaying=0;
        }
      }
      break;
    case MOM_CLOSE:
      song->MusicPlaying=0;
      break;
    default:
      break;
    }
    SDL_UnlockMutex(song->mutex);
}

int native_midi_detect(void)
{
  MMRESULT merr;
  HMIDISTRM MidiStream;

  merr=midiStreamOpen(&MidiStream,&MidiDevice,(DWORD)1,(DWORD_PTR)MidiProc,(DWORD_PTR)0,CALLBACK_FUNCTION);
  if (merr!=MMSYSERR_NOERROR)
    return 0;
  midiStreamClose(MidiStream);
  return 1;
}

NativeMidiSong *native_midi_loadsong_RW(SDL_RWops *src, int freesrc)
{
    NativeMidiSong *newsong;
    MIDIEvent       *evntlist = NULL;

    newsong = SDL_malloc(sizeof(NativeMidiSong));
    if (!newsong) {
        return NULL;
    }
    SDL_memset(newsong,0,sizeof(NativeMidiSong));

    /* Attempt to load the midi file */
    evntlist = CreateMIDIEventList(src, &newsong->ppqn);
    if (!evntlist)
    {
        SDL_free(newsong);
        return NULL;
    }

    MIDItoStream(newsong, evntlist);

    FreeMIDIEventList(evntlist);

    newsong->mutex = SDL_CreateMutex();

    if (freesrc) {
        SDL_RWclose(src);
    }
    return newsong;
}

void native_midi_freesong(NativeMidiSong *song)
{
  if (song)
  {
    if (song->NewEvents)
      SDL_free(song->NewEvents);
    SDL_DestroyMutex(song->mutex);
    SDL_free(song);
  }
}

void native_midi_start(NativeMidiSong *song, int loops)
{
  MMRESULT merr;
  MIDIPROPTIMEDIV mptd;

  native_midi_stop();
  if (!hMidiStream)
  {
    merr=midiStreamOpen(&hMidiStream,&MidiDevice,(DWORD)1,(DWORD_PTR)MidiProc,(DWORD_PTR)song,CALLBACK_FUNCTION);
    if (merr!=MMSYSERR_NOERROR)
    {
      hMidiStream = NULL; /* should I do midiStreamClose(hMidiStream) before? */
      return;
    }
    /* midiStreamStop(hMidiStream); */
    currentsong=song;
    currentsong->NewPos=0;
    currentsong->MusicPlaying=1;
    currentsong->Loops=loops;
    mptd.cbStruct=sizeof(MIDIPROPTIMEDIV);
    mptd.dwTimeDiv=currentsong->ppqn;
    merr=midiStreamProperty(hMidiStream,(LPBYTE)&mptd,MIDIPROP_SET | MIDIPROP_TIMEDIV);
    BlockOut(song);
    merr=midiStreamRestart(hMidiStream);
  }
}

void native_midi_pause(void)
{
  if (!hMidiStream)
    return;
  midiStreamPause(hMidiStream);
}

void native_midi_resume(void)
{
  if (!hMidiStream)
    return;
  midiStreamRestart(hMidiStream);
}

void native_midi_stop(void)
{
  NativeMidiSong *song = currentsong;

  if (!hMidiStream)
    return;

  SDL_LockMutex(song->mutex);
  midiStreamStop(hMidiStream);
  midiStreamClose(hMidiStream);
  currentsong=NULL;
  hMidiStream = NULL;
  SDL_UnlockMutex(song->mutex);
}

int native_midi_active(void)
{
  if (!hMidiStream)
    return 0;
  if (!currentsong)
    return 0;
  return currentsong->MusicPlaying;
}

void native_midi_setvolume(int volume)
{
  int calcVolume;
  if (volume > 128)
    volume = 128;
  if (volume < 0)
    volume = 0;
  calcVolume = (65535 * volume / 128);

  midiOutSetVolume((HMIDIOUT)hMidiStream, MAKELONG(calcVolume , calcVolume));
}

const char *native_midi_error(void)
{
  return "";
}

#endif /* Windows native MIDI support */
