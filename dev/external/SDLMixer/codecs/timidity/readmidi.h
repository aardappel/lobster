/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    readmidi.h
*/

#ifndef TIMIDITY_READMIDI_H
#define TIMIDITY_READMIDI_H

#define read_midi_file TIMI_NAMESPACE(read_midi_file)

extern MidiEvent *read_midi_file(MidiSong *song, Sint32 *count, Sint32 *sp);

#endif /* TIMIDITY_READMIDI_H */
