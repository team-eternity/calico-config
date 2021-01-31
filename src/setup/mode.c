//
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

#include "mode.h"

// Miscellaneous variables that aren't used in setup.

static const char *const executable = "calico-doom";
static const char *const game_title = "Doom";

const char *GetExecutableName(void)
{
    return executable;
}

const char *GetGameTitle(void)
{
    return game_title;
}

