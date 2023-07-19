/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    tables.h
*/

#ifndef TIMIDITY_TABLES_H
#define TIMIDITY_TABLES_H

#define timi_sine(x) (SDL_sin((2*PI/1024.0) * (x)))

#define SINE_CYCLE_LENGTH 1024

#define freq_table TIMI_NAMESPACE(freq_table)
#define vol_table TIMI_NAMESPACE(vol_table)
#define bend_fine TIMI_NAMESPACE(bend_fine)
#define bend_coarse TIMI_NAMESPACE(bend_coarse)

extern const Sint32 freq_table[];
extern const double vol_table[];
extern const double bend_fine[];
extern const double bend_coarse[];

#endif /* TIMIDITY_TABLES_H */
