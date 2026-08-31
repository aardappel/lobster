/* This file is a part of MIR project.
   Copyright (C) 2020-2024 Vladimir Makarov <vmakarov.gcc@gmail.com>.
*/

/* See C11 7.19 */
static char stddef_str[]
  = "#ifndef __STDDEF_H\n"
    "#define __STDDEF_H\n"
    "\n"
    "typedef long ptrdiff_t;\n"
    "typedef unsigned long size_t;\n"
    "typedef long double max_align_t;\n"
#if defined(__APPLE__)
    "typedef int wchar_t;\n"
#else
    /* unsigned in AAPCS64; matters because musl's alltypes.h redeclares it */
    "typedef unsigned int wchar_t;\n"
#endif
    "\n"
    "#define NULL ((void *) 0)\n"
    "\n"
    "#define offsetof(type, member_designator) ((size_t) & ((type *) 0)->member_designator)\n"
    "\n"
    "#endif /* #ifndef __STDDEF_H */\n";
