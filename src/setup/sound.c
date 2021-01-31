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

// Sound control menu

#include "../elib/elib.h"
#include "../calico/configvars.h"
#include "../calico/j_eeprom.h"
#include "textscreen.h"
#include "m_misc.h"

#include "execute.h"
#include "mode.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-sound"

static const char *cfg_extension[] = { "cfg", NULL };

void ConfigSound(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    // Build the window

    window = TXT_NewWindow("Sound configuration");
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 16, 10);
    //TXT_SetWindowPosition(window, TXT_HORIZ_CENTER, TXT_VERT_TOP, TXT_SCREEN_W / 2, 3);

    TXT_AddWidgets(
        window,
        TXT_NewSeparator("Sound effects"),
        TXT_NewLabel("Volume"),
        TXT_NewSpinControl(&sfxvolume, 0, 255),
        TXT_NewLabel("Low band Hz"),
        TXT_NewDoubleInputBox(&s_lowfreq, 12),
        TXT_NewLabel("High band Hz"),
        TXT_NewDoubleInputBox(&s_highfreq, 12),
        TXT_NewLabel("3-band pre-amp"),
        TXT_NewDoubleInputBox(&s_preampmul, 12),
        TXT_NewLabel("Low band gain"),
        TXT_NewDoubleInputBox(&s_lowgain, 12),
        TXT_NewLabel("Mid band gain"),
        TXT_NewDoubleInputBox(&s_midgain, 12),
        TXT_NewLabel("High band gain"),
        TXT_NewDoubleInputBox(&s_highgain, 12),
            
        TXT_NewSeparator("Music"),
        TXT_NewLabel("Volume"),
        TXT_NewSpinControl(&musicvolume, 0, 255),
        NULL
    );
}

