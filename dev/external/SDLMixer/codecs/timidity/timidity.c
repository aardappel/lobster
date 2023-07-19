/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.
*/

#include "SDL.h"
#include "../../utils.h" /* for SDL_strtokr() */

#include "timidity.h"

#include "options.h"
#include "common.h"
#include "instrum.h"
#include "playmidi.h"
#include "readmidi.h"
#include "output.h"

#include "tables.h"

static ToneBank *master_tonebank[MAXBANK], *master_drumset[MAXBANK];

static char def_instr_name[256] = "";

#define MAXWORDS 10
#define MAX_RCFCOUNT 50

/* Quick-and-dirty fgets() replacement. */

static char *RWgets(SDL_RWops *rw, char *s, int size)
{
    int num_read = 0;
    char *p = s;

    --size;/* so that we nul terminate properly */

    for (; num_read < size; ++p)
    {
	if (SDL_RWread(rw, p, 1, 1) != 1)
	    break;

	num_read++;

	/* Unlike fgets(), don't store newline. Under Windows/DOS we'll
	 * probably get an extra blank line for every line that's being
	 * read, but that should be ok.
	 */
	if (*p == '\n' || *p == '\r')
	{
	    *p = '\0';
	    return s;
	}
    }

    *p = '\0';

    return (num_read != 0) ? s : NULL;
}

static int read_config_file(const char *name, int rcf_count)
{
  SDL_RWops *rw;
  char tmp[1024];
  char *w[MAXWORDS], *cp;
  char *endp;
  ToneBank *bank;
  int i, j, k, line, r, words;

  if (rcf_count >= MAX_RCFCOUNT) {
    SNDDBG(("Probable source loop in configuration files\n"));
    return -1;
  }

  if (!(rw=timi_openfile(name)))
   return -1;

  bank = NULL;
  line = 0;
  r = -1; /* start by assuming failure, */

  while (RWgets(rw, tmp, sizeof(tmp)))
  {
    line++;
    words=0;
    w[0]=SDL_strtokr(tmp, " \t\240", &endp);
    if (!w[0]) continue;

        /* Originally the TiMidity++ extensions were prefixed like this */
    if (SDL_strcmp(w[0], "#extension") == 0)
    {
        w[0]=SDL_strtokr(0, " \t\240", &endp);
        if (!w[0]) continue;
    }

    if (*w[0] == '#')
        continue;

    while (words < MAXWORDS - 1) /* -1 : next arg */
    {
      while (*endp == ' ' || *endp == '\t' || *endp == '\240')
        endp++;
      if (*endp == '\0' || *endp == '#')
        break;
      if (*endp == '"' || *endp == '\'') /* quoted string */
      {
        char *terminator;
        if ((terminator = SDL_strchr(endp + 1, *endp)) != NULL) /* terminated */
        {
          if (terminator[1] == ' ' || terminator[1] == '\t' || terminator[1] == '\240' || terminator[1] == '\0')
          {
            char *extraQuote;
            if ((extraQuote = SDL_strchr(endp + 1, *endp == '"' ? '\'' : '"')) != NULL && extraQuote < terminator)
            {
                SNDDBG(("%s: line %d: Quote characters are not allowed inside a quoted string", name, line));
                goto fail;
            }

            w[++words] = endp + 1;
            endp = terminator + 1;
            *terminator = '\0';
          }
          else /* no space after quoted string */
          {
            SNDDBG(("%s: line %d: There must be at least one whitespace between string terminator (%c) and the next parameter", name, line, *endp));
            goto fail;
          }
        }
        else /* not terminated */
        {
          SNDDBG(("%s: line %d: The quoted string is not terminated", name, line));
          goto fail;
        }
      }
      else /* not quoted string */
      {
        w[++words] = endp;
        while (!(*endp == ' ' || *endp == '\t' || *endp == '\240' || *endp == '\0'))
        {
          if (*endp == '"' || *endp == '\'') /* no space before quoted string */
          {
              SNDDBG(("%s: line %d: There must be at least one whitespace between previous parameter and a beginning of the quoted string (%c)", name, line, *endp));
              goto fail;
          }
          endp++;
        }
        if (*endp != '\0') /* unless at the end-of-string (i.e. EOF) */
        {
          *endp = '\0';    /* terminate the token */
          endp++;
        }
      }
    }
    w[++words] = NULL;

    /* TiMidity++ adds a number of extensions to the config file format.
     * Many of them are completely irrelevant to SDL_sound, but at least
     * we shouldn't choke on them.
     *
     * Unfortunately the documentation for these extensions is often quite
     * vague, gramatically strange or completely absent.
     */
    if (!SDL_strcmp(w[0], "comm")      /* "comm" program second        */ ||
        !SDL_strcmp(w[0], "HTTPproxy") /* "HTTPproxy" hostname:port    */ ||
        !SDL_strcmp(w[0], "FTPproxy")  /* "FTPproxy" hostname:port     */ ||
        !SDL_strcmp(w[0], "mailaddr")  /* "mailaddr" your-mail-address */ ||
        !SDL_strcmp(w[0], "opt")       /* "opt" timidity-options       */  )
    {
      /* + "comm" sets some kind of comment -- the documentation is too
       *   vague for me to understand at this time.
       * + "HTTPproxy", "FTPproxy" and "mailaddr" are for reading data
       *   over a network, rather than from the file system.
       * + "opt" specifies default options for TiMidity++.
       *
       * Quite useless for us, so they can safely remain no-ops.
       */
    }
    else if (!SDL_strcmp(w[0], "timeout")) /* "timeout" program second */
    {
      /* Specifies a timeout value of the program. A number of seconds
       * before TiMidity kills the note. No urgent need for it.
       */
      SNDDBG(("FIXME: Implement \"timeout\" in TiMidity config.\n"));
    }
    else if (!SDL_strcmp(w[0], "copydrumset")  /* "copydrumset" drumset */ ||
             !SDL_strcmp(w[0], "copybank")) /* "copybank" bank       */
    {
      /* Copies all the settings of the specified drumset or bank to
       * the current drumset or bank. May be useful later, but not a
       * high priority.
       */
      SNDDBG(("FIXME: Implement \"%s\" in TiMidity config.\n", w[0]));
    }
    else if (!SDL_strcmp(w[0], "undef")) /* "undef" progno */
    {
      /* Undefines the tone "progno" of the current tone bank (or
       * drum set?). Not a high priority.
       */
      SNDDBG(("FIXME: Implement \"undef\" in TiMidity config.\n"));
    }
    else if (!SDL_strcmp(w[0], "altassign")) /* "altassign" prog1 prog2 ... */
    {
      /* Sets the alternate assign for drum set. Whatever that's
       * supposed to mean.
       */
      SNDDBG(("FIXME: Implement \"altassign\" in TiMidity config.\n"));
    }
    else if (!SDL_strcmp(w[0], "soundfont") ||
             !SDL_strcmp(w[0], "font"))
    {
      /* "soundfont" sf_file "remove"
       * "soundfont" sf_file ["order=" order] ["cutoff=" cutoff]
       *                     ["reso=" reso] ["amp=" amp]
       * "font" "exclude" bank preset keynote
       * "font" "order" order bank preset keynote
       */
      SNDDBG(("FIXME: Implmement \"%s\" in TiMidity config.\n", w[0]));
    }
    else if (!SDL_strcmp(w[0], "progbase"))
    {
      /* The documentation for this makes absolutely no sense to me, but
       * apparently it sets some sort of base offset for tone numbers.
       */
      SNDDBG(("FIXME: Implement \"progbase\" in TiMidity config.\n"));
    }
    else if (!SDL_strcmp(w[0], "map")) /* "map" name set1 elem1 set2 elem2 */
    {
      /* This one is used by the "eawpats". Looks like it's used
       * for remapping one instrument to another somehow.
       */
      SNDDBG(("FIXME: Implement \"map\" in TiMidity config.\n"));
    }

    /* Standard TiMidity config */
    else if (!SDL_strcmp(w[0], "dir"))
    {
      if (words < 2)
      {
	SNDDBG(("%s: line %d: No directory given\n", name, line));
	goto fail;
      }
      for (i=1; i<words; i++) {
	if (timi_add_pathlist(w[i], SDL_strlen(w[i])) < 0)
	  goto fail;
      }
    }
    else if (!SDL_strcmp(w[0], "source"))
    {
      if (words < 2)
      {
	SNDDBG(("%s: line %d: No file name given\n", name, line));
	goto fail;
      }
      for (i=1; i<words; i++)
      {
	r = read_config_file(w[i], rcf_count + 1);
	if (r != 0)
	  goto fail;
      }
      r = -1; /* not finished yet, */
    }
    else if (!SDL_strcmp(w[0], "default"))
    {
      if (words != 2)
      {
	SNDDBG(("%s: line %d: Must specify exactly one patch name\n",
		name, line));
	goto fail;
      }
      SDL_strlcpy(def_instr_name, w[1], 256);
    }
    else if (!SDL_strcmp(w[0], "drumset"))
    {
      if (words < 2)
      {
	SNDDBG(("%s: line %d: No drum set number given\n", name, line));
	goto fail;
      }
      i=SDL_atoi(w[1]);
      if (i<0 || i>(MAXBANK-1))
      {
	SNDDBG(("%s: line %d: Drum set must be between 0 and %d\n",
		name, line, MAXBANK-1));
	goto fail;
      }
      if (!master_drumset[i])
      {
	master_drumset[i] = SDL_calloc(1, sizeof(ToneBank));
	if (!master_drumset[i]) goto fail;
	master_drumset[i]->tone = SDL_calloc(128, sizeof(ToneBankElement));
	if (!master_drumset[i]->tone) goto fail;
      }
      bank=master_drumset[i];
    }
    else if (!SDL_strcmp(w[0], "bank"))
    {
      if (words < 2)
      {
	SNDDBG(("%s: line %d: No bank number given\n", name, line));
	goto fail;
      }
      i=SDL_atoi(w[1]);
      if (i<0 || i>(MAXBANK-1))
      {
	SNDDBG(("%s: line %d: Tone bank must be between 0 and %d\n",
		name, line, MAXBANK-1));
	goto fail;
      }
      if (!master_tonebank[i])
      {
	master_tonebank[i] = SDL_calloc(1, sizeof(ToneBank));
	if (!master_tonebank[i]) goto fail;
	master_tonebank[i]->tone = SDL_calloc(128, sizeof(ToneBankElement));
	if (!master_tonebank[i]->tone) goto fail;
      }
      bank=master_tonebank[i];
    }
    else
    {
      size_t sz;
      if ((words < 2) || (*w[0] < '0' || *w[0] > '9'))
      {
	SNDDBG(("%s: line %d: syntax error\n", name, line));
	goto fail;
      }
      i=SDL_atoi(w[0]);
      if (i<0 || i>127)
      {
	SNDDBG(("%s: line %d: Program must be between 0 and 127\n",
		name, line));
	goto fail;
      }
      if (!bank)
      {
	SNDDBG(("%s: line %d: Must specify tone bank or drum set before assignment\n",
		name, line));
	goto fail;
      }
      SDL_free(bank->tone[i].name);
      sz = SDL_strlen(w[1])+1;
      bank->tone[i].name = SDL_malloc(sz);
      if (!bank->tone[i].name) goto fail;
      SDL_memcpy(bank->tone[i].name,w[1],sz);
      bank->tone[i].note=bank->tone[i].amp=bank->tone[i].pan=
      bank->tone[i].strip_loop=bank->tone[i].strip_envelope=
      bank->tone[i].strip_tail=-1;

      for (j=2; j<words; j++)
      {
	if (!(cp=SDL_strchr(w[j], '=')))
	{
	  SNDDBG(("%s: line %d: bad patch option %s\n", name, line, w[j]));
	  goto fail;
	}
	*cp++=0;
	if (!SDL_strcmp(w[j], "amp"))
	{
	  k=SDL_atoi(cp);
	  if ((k<0 || k>MAX_AMPLIFICATION) || (*cp < '0' || *cp > '9'))
	  {
	    SNDDBG(("%s: line %d: amplification must be between 0 and %d\n",
		    name, line, MAX_AMPLIFICATION));
	    goto fail;
	  }
	  bank->tone[i].amp=k;
	}
	else if (!SDL_strcmp(w[j], "note"))
	{
	  k=SDL_atoi(cp);
	  if ((k<0 || k>127) || (*cp < '0' || *cp > '9'))
	  {
	    SNDDBG(("%s: line %d: note must be between 0 and 127\n",
		    name, line));
	    goto fail;
	  }
	  bank->tone[i].note=k;
	}
	else if (!SDL_strcmp(w[j], "pan"))
	{
	  if (!SDL_strcmp(cp, "center"))
	    k=64;
	  else if (!SDL_strcmp(cp, "left"))
	    k=0;
	  else if (!SDL_strcmp(cp, "right"))
	    k=127;
	  else
	    k=((SDL_atoi(cp)+100) * 100) / 157;
	  if ((k<0 || k>127) || (k==0 && *cp!='-' && (*cp < '0' || *cp > '9')))
	  {
	    SNDDBG(("%s: line %d: panning must be left, right, center, or between -100 and 100\n",
		    name, line));
	    goto fail;
	  }
	  bank->tone[i].pan=k;
	}
	else if (!SDL_strcmp(w[j], "keep"))
	{
	  if (!SDL_strcmp(cp, "env"))
	    bank->tone[i].strip_envelope=0;
	  else if (!SDL_strcmp(cp, "loop"))
	    bank->tone[i].strip_loop=0;
	  else
	  {
	    SNDDBG(("%s: line %d: keep must be env or loop\n", name, line));
	    goto fail;
	  }
	}
	else if (!SDL_strcmp(w[j], "strip"))
	{
	  if (!SDL_strcmp(cp, "env"))
	    bank->tone[i].strip_envelope=1;
	  else if (!SDL_strcmp(cp, "loop"))
	    bank->tone[i].strip_loop=1;
	  else if (!SDL_strcmp(cp, "tail"))
	    bank->tone[i].strip_tail=1;
	  else
	  {
	    SNDDBG(("%s: line %d: strip must be env, loop, or tail\n",
		    name, line));
	    goto fail;
	  }
	}
	else
	{
	  SNDDBG(("%s: line %d: bad patch option %s\n", name, line, w[j]));
	  goto fail;
	}
      }
    }
  }

  r = 0; /* we're good. */
fail:
  SDL_RWclose(rw);
  return r;
}

#if defined(_WIN32)||defined(__CYGWIN__)||defined(__OS2__)
/* FIXME: What about C:FOO ? */
static SDL_INLINE char *get_last_dirsep (const char *p) {
  char *p1 = SDL_strrchr(p, '/');
  char *p2 = SDL_strrchr(p, '\\');
  if (!p1) return p2;
  if (!p2) return p1;
  return (p1 > p2)? p1 : p2;
}
#else /* assumed UNIX-ish : */
static SDL_INLINE char *get_last_dirsep (const char *p) {
  return SDL_strrchr(p, '/');
}
#endif

static int init_alloc_banks(void)
{
  /* Allocate memory for the standard tonebank and drumset */
  master_tonebank[0] = SDL_calloc(1, sizeof(ToneBank));
  if (!master_tonebank[0]) goto _nomem;
  master_tonebank[0]->tone = SDL_calloc(128, sizeof(ToneBankElement));
  if (!master_tonebank[0]->tone) goto _nomem;

  master_drumset[0] = SDL_calloc(1, sizeof(ToneBank));
  if (!master_drumset[0]) goto _nomem;
  master_drumset[0]->tone = SDL_calloc(128, sizeof(ToneBankElement));
  if (!master_drumset[0]->tone) goto _nomem;

  return 0;
_nomem:
  SNDDBG(("Out of memory\n"));
  Timidity_Exit ();
  return -2;
}

static int init_begin_config(const char *cf)
{
  const char *p = get_last_dirsep(cf);
  if (p != NULL)
      return timi_add_pathlist(cf, p - cf + 1); /* including DIRSEP */
  return 0;
}

static int init_with_config(const char *cf)
{
  int rc = init_begin_config(cf);
  if (rc != 0) {
      Timidity_Exit ();
      return rc;
  }
  rc = read_config_file(cf, 0);
  if (rc != 0) {
      Timidity_Exit ();
  }
  return rc;
}

int Timidity_Init_NoConfig(void)
{
  master_tonebank[0] = NULL;
  master_drumset[0] = NULL;
  return init_alloc_banks();
}

int Timidity_Init(const char *config_file)
{
  int rc = Timidity_Init_NoConfig();
  if (rc != 0) {
      return rc;
  }
  if (config_file == NULL || *config_file == '\0') {
      return init_with_config(TIMIDITY_CFG);
  }
  return init_with_config(config_file);
}

static void do_song_load(SDL_RWops *rw, SDL_AudioSpec *audio, MidiSong **out)
{
  MidiSong *song;
  int i;

  *out = NULL;
  if (rw == NULL)
      return;

  /* Allocate memory for the song */
  song = (MidiSong *)SDL_calloc(1, sizeof(*song));
  if (song == NULL)
      return;

  for (i = 0; i < MAXBANK; i++)
  {
    if (master_tonebank[i]) {
      song->tonebank[i] = SDL_calloc(1, sizeof(ToneBank));
      if (!song->tonebank[i]) goto fail;
      song->tonebank[i]->tone = master_tonebank[i]->tone;
    }
    if (master_drumset[i]) {
      song->drumset[i] = SDL_calloc(1, sizeof(ToneBank));
      if (!song->drumset[i]) goto fail;
      song->drumset[i]->tone = master_drumset[i]->tone;
    }
  }

  song->amplification = DEFAULT_AMPLIFICATION;
  song->voices = DEFAULT_VOICES;
  song->drumchannels = DEFAULT_DRUMCHANNELS;

  song->rw = rw;

  song->rate = audio->freq;
  song->encoding = 0;
  if ((audio->format & 0xFF) == 16)
      song->encoding |= PE_16BIT;
  else if ((audio->format & 0xFF) == 32)
      song->encoding |= PE_32BIT;
  if (audio->format & 0x8000)
      song->encoding |= PE_SIGNED;
  if (audio->channels == 1)
      song->encoding |= PE_MONO;
  else if (audio->channels > 2) {
      SDL_SetError("Surround sound not supported");
      goto fail;
  }
  switch (audio->format) {
  case AUDIO_S8:
    song->write = timi_s32tos8;
    break;
  case AUDIO_U8:
    song->write = timi_s32tou8;
    break;
  case AUDIO_S16LSB:
    song->write = timi_s32tos16l;
    break;
  case AUDIO_S16MSB:
    song->write = timi_s32tos16b;
    break;
  case AUDIO_U16LSB:
    song->write = timi_s32tou16l;
    break;
  case AUDIO_U16MSB:
    song->write = timi_s32tou16b;
    break;
  case AUDIO_S32LSB:
    song->write = timi_s32tos32l;
    break;
  case AUDIO_S32MSB:
    song->write = timi_s32tos32b;
    break;
  case AUDIO_F32SYS:
    song->write = timi_s32tof32;
    break;
  default:
    SDL_SetError("Unsupported audio format");
    goto fail;
  }

  song->buffer_size = audio->samples;
  song->resample_buffer = SDL_malloc(audio->samples * sizeof(sample_t));
  if (!song->resample_buffer) goto fail;
  song->common_buffer = SDL_malloc(audio->samples * 2 * sizeof(Sint32));
  if (!song->common_buffer) goto fail;

  song->control_ratio = audio->freq / CONTROLS_PER_SECOND;
  if (song->control_ratio < 1)
      song->control_ratio = 1;
  else if (song->control_ratio > MAX_CONTROL_RATIO)
      song->control_ratio = MAX_CONTROL_RATIO;

  song->lost_notes = 0;
  song->cut_notes = 0;

  song->events = read_midi_file(song, &(song->groomed_event_count),
      &song->samples);

  /* Make sure everything is okay */
  if (!song->events)
    goto fail;

  song->default_instrument = NULL;
  song->default_program = DEFAULT_PROGRAM;

  if (*def_instr_name)
    set_default_instrument(song, def_instr_name);

  load_missing_instruments(song);

  if (! song->oom)
      *out = song;
  else {
fail: Timidity_FreeSong(song);
  }
}

MidiSong *Timidity_LoadSong(SDL_RWops *rw, SDL_AudioSpec *audio)
{
  MidiSong *song;
  do_song_load(rw, audio, &song);
  return song;
}

void Timidity_FreeSong(MidiSong *song)
{
  int i;

  if (!song) return;

  free_instruments(song);

  for (i = 0; i < 128; i++) {
    SDL_free(song->tonebank[i]);
    SDL_free(song->drumset[i]);
  }

  SDL_free(song->common_buffer);
  SDL_free(song->resample_buffer);
  SDL_free(song->events);

  SDL_free(song);
}

void Timidity_Exit(void)
{
  int i, j;

  for (i = 0; i < MAXBANK; i++) {
    if (master_tonebank[i]) {
      ToneBankElement *e = master_tonebank[i]->tone;
      if (e != NULL) {
        for (j = 0; j < 128; j++) {
          SDL_free(e[j].name);
        }
        SDL_free(e);
      }
      SDL_free(master_tonebank[i]);
      master_tonebank[i] = NULL;
    }
    if (master_drumset[i]) {
      ToneBankElement *e = master_drumset[i]->tone;
      if (e != NULL) {
        for (j = 0; j < 128; j++) {
          SDL_free(e[j].name);
        }
        SDL_free(e);
      }
      SDL_free(master_drumset[i]);
      master_drumset[i] = NULL;
    }
  }

  timi_free_pathlist();
}
