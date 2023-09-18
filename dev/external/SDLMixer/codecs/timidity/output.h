/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    output.h
*/

#ifndef TIMIDITY_OUTPUT_H
#define TIMIDITY_OUTPUT_H

/* Data format encoding bits */

#define PE_MONO 	0x01  /* versus stereo */
#define PE_SIGNED	0x02  /* versus unsigned */
#define PE_16BIT 	0x04  /* versus 8-bit */
#define PE_32BIT 	0x08  /* versus 8-bit or 16-bit */

/* Conversion functions -- These overwrite the Sint32 data in *lp with
   data in another format */

/* 8-bit signed and unsigned*/
extern void timi_s32tos8(void *dp, Sint32 *lp, Sint32 c);
extern void timi_s32tou8(void *dp, Sint32 *lp, Sint32 c);

/* 16-bit */
extern void timi_s32tos16(void *dp, Sint32 *lp, Sint32 c);
extern void timi_s32tou16(void *dp, Sint32 *lp, Sint32 c);

/* byte-exchanged 16-bit */
extern void timi_s32tos16x(void *dp, Sint32 *lp, Sint32 c);
extern void timi_s32tou16x(void *dp, Sint32 *lp, Sint32 c);

/* 32-bit */
extern void timi_s32tof32(void *dp, Sint32 *lp, Sint32 c);
extern void timi_s32tos32(void *dp, Sint32 *lp, Sint32 c);

/* byte-exchanged 32-bit */
extern void timi_s32tos32x(void *dp, Sint32 *lp, Sint32 c);

/* little-endian and big-endian specific */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define timi_s32tou16l timi_s32tou16
#define timi_s32tou16b timi_s32tou16x
#define timi_s32tos16l timi_s32tos16
#define timi_s32tos16b timi_s32tos16x
#define timi_s32tos32l timi_s32tos32
#define timi_s32tos32b timi_s32tos32x
#else
#define timi_s32tou16l timi_s32tou16x
#define timi_s32tou16b timi_s32tou16
#define timi_s32tos16l timi_s32tos16x
#define timi_s32tos16b timi_s32tos16
#define timi_s32tos32l timi_s32tos32x
#define timi_s32tos32b timi_s32tos32
#endif

#endif /* TIMIDITY_OUTPUT_H */
