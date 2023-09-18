/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    mix.h
*/

#ifndef TIMIDITY_MIX_H
#define TIMIDITY_MIX_H

#define mix_voice TIMI_NAMESPACE(mix_voice)
#define recompute_envelope TIMI_NAMESPACE(recompute_envelope)
#define apply_envelope_to_amp TIMI_NAMESPACE(apply_envelope_to_amp)

extern void mix_voice(MidiSong *song, Sint32 *buf, int v, Sint32 c);
extern int recompute_envelope(MidiSong *song, int v);
extern void apply_envelope_to_amp(MidiSong *song, int v);

#endif /* TIMIDITY_MIX_H */
