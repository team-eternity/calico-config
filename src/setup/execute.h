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

#pragma once

#include "textscreen.h"

typedef struct execute_context_s execute_context_t;

#if defined(__cplusplus)
extern "C" {
#endif

execute_context_t *NewExecuteContext(void);

void    AddCmdLineParameter(execute_context_t *context, const char *s, ...) PRINTF_ATTR(2, 3);
void    PassThroughArguments(execute_context_t *context);
int     ExecuteDoom(execute_context_t *context);
int     FindInstalledIWADs(void);
boolean OpenFolder(const char *path);

txt_window_action_t *TestConfigAction(void);

#if defined(__cplusplus)
}
#endif

