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

/* misc helper routines */

#include "utils.h"
#include <stddef.h>

#if !defined(HAVE_SDL_STRTOKR)
/*
 * Adapted from _PDCLIB_strtok() of PDClib library at
 * https://github.com/DevSolar/pdclib.git
 *
 * The code was under CC0 license:
 * https://creativecommons.org/publicdomain/zero/1.0/legalcode :
 *
 *                        No Copyright
 *
 * The person who associated a work with this deed has dedicated the
 * work to the public domain by waiving all of his or her rights to
 * the work worldwide under copyright law, including all related and
 * neighboring rights, to the extent allowed by law.
 *
 * You can copy, modify, distribute and perform the work, even for
 * commercial purposes, all without asking permission. See Other
 * Information below.
 */
char *SDL_strtokr(char *s1, const char *s2, char **ptr)
{
    const char *p = s2;

    if (!s2 || !ptr || (!s1 && !*ptr)) return NULL;

    if (s1 != NULL) {  /* new string */
        *ptr = s1;
    } else { /* old string continued */
        if (*ptr == NULL) {
        /* No old string, no new string, nothing to do */
            return NULL;
        }
        s1 = *ptr;
    }

    /* skip leading s2 characters */
    while (*p && *s1) {
        if (*s1 == *p) {
        /* found separator; skip and start over */
            ++s1;
            p = s2;
            continue;
        }
        ++p;
    }

    if (! *s1) { /* no more to parse */
        *ptr = s1;
        return NULL;
    }

    /* skipping non-s2 characters */
    *ptr = s1;
    while (**ptr) {
        p = s2;
        while (*p) {
            if (**ptr == *p++) {
            /* found separator; overwrite with '\0', position *ptr, return */
                *((*ptr)++) = '\0';
                return s1;
            }
        }
        ++(*ptr);
    }

    /* parsed to end of string */
    return s1;
}
#endif /* HAVE_SDL_STRTOKR */

/* Is given tag a loop tag? */
SDL_bool _Mix_IsLoopTag(const char *tag)
{
    char buf[5];
    SDL_strlcpy(buf, tag, 5);
    return SDL_strcasecmp(buf, "LOOP") == 0;
}

/* Parse time string of the form HH:MM:SS.mmm and return equivalent sample
 * position */
Sint64 _Mix_ParseTime(char *time, long samplerate_hz)
{
    char *num_start, *p;
    Sint64 result;
    char c;
    int val;

    /* Time is directly expressed as a sample position */
    if (SDL_strchr(time, ':') == NULL) {
        return SDL_strtoll(time, NULL, 10);
    }

    result = 0;
    num_start = time;

    for (p = time; *p != '\0'; ++p) {
        if (*p == '.' || *p == ':') {
            c = *p; *p = '\0';
            if ((val = SDL_atoi(num_start)) < 0)
                return -1;
            result = result * 60 + val;
            num_start = p + 1;
            *p = c;
        }

        if (*p == '.') {
            double val_f = SDL_atof(p);
            if (val_f < 0) return -1;
            return result * samplerate_hz + (Sint64) (val_f * samplerate_hz);
        }
    }

    if ((val = SDL_atoi(num_start)) < 0) return -1;
    return (result * 60 + val) * samplerate_hz;
}
