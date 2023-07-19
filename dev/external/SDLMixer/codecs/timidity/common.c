/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    common.c
*/

#include "SDL.h"

#include "options.h"
#include "common.h"

#if defined(__WIN32__) || defined(__OS2__)
#define CHAR_DIRSEP '\\'
#define is_dirsep(c) ((c) == '/' || (c) == '\\')
#define is_abspath(p) ((p)[0] == '/' || (p)[0] == '\\' || ((p)[0] && (p)[1] == ':'))
#else /* unix: */
#define CHAR_DIRSEP '/'
#define is_dirsep(c) ((c) == '/')
#define is_abspath(p) ((p)[0] == '/')
#endif

/* The paths in this list will be tried whenever we're reading a file */
typedef struct _PathList {
  char *path;
  struct _PathList *next;
} PathList;

static PathList *pathlist = NULL;

/* This is meant to find and open files for reading */
SDL_RWops *timi_openfile(const char *name)
{
  SDL_RWops *rw;

  if (!name || !(*name)) {
      SNDDBG(("Attempted to open nameless file.\n"));
      return NULL;
  }

  /* First try the given name */

  SNDDBG(("Trying to open %s\n", name));
  if ((rw = SDL_RWFromFile(name, "rb")) != NULL)
    return rw;

  if (!is_abspath(name))
  {
    char current_filename[1024];
    PathList *plp = pathlist;
    char *p;
    size_t l;

    while (plp) { /* Try along the path then */
	*current_filename = 0;
	p = current_filename;
	l = SDL_strlen(plp->path);
	if(l >= sizeof(current_filename) - 3) l = 0;
	if(l) {
	    SDL_memcpy(current_filename, plp->path, l);
	    p += l;
	    if(!is_dirsep(p[-1])) {
	      *p++ = CHAR_DIRSEP;
	       l++;
	    }
	  }
	SDL_strlcpy(p, name, sizeof(current_filename) - l);
	SNDDBG(("Trying to open %s\n", current_filename));
	if ((rw = SDL_RWFromFile(current_filename, "rb")))
	  return rw;
	plp = plp->next;
      }
  }

  /* Nothing could be opened. */
  SNDDBG(("Could not open %s\n", name));
  return NULL;
}

/* This adds a directory to the path list */
int timi_add_pathlist(const char *s, size_t l)
{
  PathList *plp = SDL_malloc(sizeof(PathList));
  if (plp == NULL) return -2;
  plp->path = SDL_malloc(l + 1);
  if (plp->path == NULL) {
      SDL_free (plp);
      return -2;
  }
  SDL_memcpy(plp->path, s, l);
  plp->path[l] = 0;
  plp->next = pathlist;
  pathlist = plp;
  return 0;
}

void timi_free_pathlist(void)
{
    PathList *plp = pathlist;
    PathList *next;

    while (plp) {
	next = plp->next;
	SDL_free(plp->path);
	SDL_free(plp);
	plp = next;
    }
    pathlist = NULL;
}
