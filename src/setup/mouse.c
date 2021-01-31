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

#include <ctype.h>

#include "../elib/elib.h"
#include "textscreen.h"
#include "../calico/configvars.h"
#include "execute.h"
#include "mode.h"
#include "mouse.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-mouse"

static char *displayVals[KBJK_MAX+1];

static void InitDisplayVals(void)
{
    static boolean isInit = false;

    if(!isInit)
    {
        for(unsigned int i = 0; i < KBJK_MAX + 1; i++)
        {
            displayVals[i] = estrdup(kbJoyKeyJagNames[i]);
            *(displayVals[i]) = toupper(*(displayVals[i]));
        }
        isInit = true;
    }
}

static void MouseSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(mbutton))
{
    TXT_CAST_ARG(mbutton_t, mbutton);
    Calico_UpdateMouseButton(mbutton);
}

static void AddMouseControl(TXT_UNCAST_ARG(table), const char *label, mbutton_t *mb)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_dropdown_list_t *dropdown;

    InitDisplayVals();
    
    TXT_AddWidget(table, TXT_NewLabel(label));
    TXT_AddWidget(table, (dropdown = TXT_NewDropdownList(&mb->kbButton, displayVals, KBJK_MAX+1)));

    TXT_SignalConnect(dropdown, "changed", MouseSetCallback, &mb);
}

void ConfigMouse(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    // CALICO: init mouse button configuration
    Calico_InitMouseButtons();

    window = TXT_NewWindow("Mouse configuration");

    TXT_SetTableColumns(window, 2);

    TXT_SetColumnWidths(window, 23, 10);

    //TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(
        window,
        TXT_NewCheckBox("Enable mouse", &useMouse),
        TXT_TABLE_OVERFLOW_RIGHT,
        TXT_NewSeparator("Mouse motion"),
        TXT_NewLabel("Speed"),
        TXT_NewSpinControl(&mouseSensitivity, 1, 256),
        TXT_NewLabel("Acceleration"),
        TXT_NewDoubleSpinControl(&mouseAcceleration, 1.0, 5.0),
        TXT_NewLabel("Acceleration threshold"),
        TXT_NewSpinControl(&mouseThreshold, 0, 32),
        TXT_NewSeparator("Buttons"),
        NULL
    );

    AddMouseControl(window, "Left",   &sdlMouseToJagButton[CMB_LEFT]);
    AddMouseControl(window, "Middle", &sdlMouseToJagButton[CMB_MIDDLE]);
    AddMouseControl(window, "Right",  &sdlMouseToJagButton[CMB_RIGHT]);
    AddMouseControl(window, "X1",     &sdlMouseToJagButton[CMB_X1]);
    AddMouseControl(window, "X2",     &sdlMouseToJagButton[CMB_X2]);
}
