/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    instrum.h
*/

#ifndef TIMIDITY_INSTRUM_H
#define TIMIDITY_INSTRUM_H

/* Bits in modes: */
#define MODES_16BIT	(1<<0)
#define MODES_UNSIGNED	(1<<1)
#define MODES_LOOPING	(1<<2)
#define MODES_PINGPONG	(1<<3)
#define MODES_REVERSE	(1<<4)
#define MODES_SUSTAIN	(1<<5)
#define MODES_ENVELOPE	(1<<6)

/* A hack to delay instrument loading until after reading the
   entire MIDI file. */
#define MAGIC_LOAD_INSTRUMENT ((Instrument *) (-1))

#define SPECIAL_PROGRAM -1

#define load_missing_instruments TIMI_NAMESPACE(load_missing_instruments)
#define free_instruments TIMI_NAMESPACE(free_instruments)
#define set_default_instrument TIMI_NAMESPACE(set_default_instrument)

extern int load_missing_instruments(MidiSong *song);
extern void free_instruments(MidiSong *song);
extern int set_default_instrument(MidiSong *song, const char *name);

#endif /* TIMIDITY_INSTRUM_H */
