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

#ifndef SETUP_MODE_H
#define SETUP_MODE_H

typedef void (*GameSelectCallback)(void);

#if defined(__cplusplus)
extern "C" {
#endif

void SetupMission(GameSelectCallback callback);
void InitBindings(void);
const char *GetExecutableName(void);
const char *GetGameTitle(void);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef SETUP_MODE_H */

