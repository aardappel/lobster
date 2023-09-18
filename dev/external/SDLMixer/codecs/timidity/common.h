/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    common.h
*/

#ifndef TIMIDITY_COMMON_H
#define TIMIDITY_COMMON_H

extern SDL_RWops *timi_openfile(const char *name);

/* pathlist funcs only to be used during Timidity_Init/Timidity_Exit */
extern int timi_add_pathlist(const char *s, size_t len);
extern void timi_free_pathlist(void);

/* hide private symbols by prefixing with "_timi_" */
#undef  TIMI_NAMESPACE
#define TIMI_NAMESPACE(x) _timi_ ## x

/* debug output */
#ifdef DEBUG_CHATTER
#define SNDDBG(X) SDL_Log X
#else
#define SNDDBG(X)
#endif

#endif /* TIMIDITY_COMMON_H */
