//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//      Miscellaneous.
//    


#ifndef __M_MISC__
#define __M_MISC__

#include <stdio.h>
#include <stdarg.h>

#include "../textscreen/textscreen.h"

#if defined(__cplusplus)
extern "C" {
#endif

char       *M_TempFile(const char *s);
boolean     M_FileExists(const char *file);
char       *M_FileCaseExists(const char *file);
boolean     M_StrToInt(const char *str, int *result);
char       *M_DirName(const char *path);
const char *M_BaseName(const char *path);
void        M_ExtractFileBase(const char *path, char *dest);
void        M_ForceUppercase(char *text);
void        M_ForceLowercase(char *text);
const char *M_StrCaseStr(const char *haystack, const char *needle);
char       *M_StringDuplicate(const char *orig);
boolean     M_StringCopy(char *dest, const char *src, size_t dest_size);
boolean     M_StringConcat(char *dest, const char *src, size_t dest_size);
char       *M_StringReplace(const char *haystack, const char *needle, const char *replacement);
char       *M_StringJoin(const char *s, ...);
boolean     M_StringStartsWith(const char *s, const char *prefix);
boolean     M_StringEndsWith(const char *s, const char *suffix);

#if defined(__cplusplus)
}
#endif

#endif

