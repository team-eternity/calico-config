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

#include <stdlib.h>
#include <string.h>

#include "../calico/configvars.h"

#include "doomkeys.h"
#include "m_misc.h"

#include "txt_keyinput.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_label.h"
#include "txt_utf8.h"
#include "txt_window.h"

#define KEY_INPUT_WIDTH 20

typedef struct userdata_t 
{ 
    txt_window_t    *window; 
    txt_key_input_t *key_input; 
} userdata_t;

static void KeyboardCloseWindow(userdata_t *userdata)
{
    if(userdata->window)
    {
        TXT_SetPeriodicCallback(NULL, NULL, 0);
        TXT_SDL_SetEventCallback(NULL, NULL);
        TXT_CloseWindow(userdata->window);
        userdata->window = NULL;
    }
}

static int KeyboardEventCallback(SDL_Event *ev, void *user_data)
{
    userdata_t *const userdata = (userdata_t *)user_data;

    if(ev->type != SDL_KEYDOWN)
        return 0;

    Calico_SetKeyboardKey(userdata->key_input->key, ev->key.keysym.sym);
    KeyboardCloseWindow(userdata);

    return 1;
}

#define KBDWAIT 5000
static Uint32 kbdStartTime;

static void KeyboardIdleCallback(void *user_data)
{
    userdata_t *const userdata = (userdata_t *)user_data;

    if(SDL_GetTicks() - kbdStartTime >= KBDWAIT)
        KeyboardCloseWindow(userdata);
}

#if 0
static int KeyPressCallback(txt_window_t *window, int key, 
                            TXT_UNCAST_ARG(key_input))
{
    TXT_CAST_ARG(txt_key_input_t, key_input);

    if (key != KEY_ESCAPE)
    {
        // Got the key press. Save to the variable and close the window.

        *key_input->variable = key;

        if (key_input->check_conflicts)
        {
            TXT_EmitSignal(key_input, "set");
        }

        TXT_CloseWindow(window);

        // Return to normal input mode now that we have the key.
        TXT_SetInputMode(TXT_INPUT_NORMAL);

        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

static void OpenPromptWindow(txt_key_input_t *key_input)
{
    static userdata_t userdata;

    // Silently update when the shift button is held down.

    key_input->check_conflicts = !TXT_GetModifierState(TXT_MOD_SHIFT);

    userdata.window    = TXT_MessageBox(NULL, "Press the new key or wait 5 seconds...");
    userdata.key_input = key_input;

    TXT_SetWindowAction(userdata.window, TXT_HORIZ_CENTER, NULL);

    TXT_SDL_SetEventCallback(KeyboardEventCallback, &userdata);
    
    kbdStartTime = SDL_GetTicks();
    TXT_SetPeriodicCallback(KeyboardIdleCallback, &userdata, 100);
}

static void TXT_KeyInputSizeCalc(TXT_UNCAST_ARG(key_input))
{
    TXT_CAST_ARG(txt_key_input_t, key_input);

    // All keyinputs are the same size.

    key_input->widget.w = KEY_INPUT_WIDTH;
    key_input->widget.h = 1;
}

static void TXT_KeyInputDrawer(TXT_UNCAST_ARG(key_input))
{
    TXT_CAST_ARG(txt_key_input_t, key_input);
    int i;

    const char *const current_binding = Calico_GetCurrentKeyBinding(key_input->key);

    TXT_SetWidgetBG(key_input);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_DrawString(current_binding);

    for (i = TXT_UTF8_Strlen(current_binding); i < KEY_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_KeyInputDestructor(TXT_UNCAST_ARG(key_input))
{
}

static int TXT_KeyInputKeyPress(TXT_UNCAST_ARG(key_input), int key)
{
    TXT_CAST_ARG(txt_key_input_t, key_input);

    if (key == KEY_ENTER)
    {
        // Open a window to prompt for the new key press
        OpenPromptWindow(key_input);

        return 1;
    }

    // unbind the key
    if (key == KEY_BACKSPACE || key == KEY_DEL)
    {
        Calico_SetKeyboardKey(key_input->key, SDLK_UNKNOWN);
    }

    return 0;
}

static void TXT_KeyInputMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b)
{
    TXT_CAST_ARG(txt_key_input_t, widget);
            
    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_KeyInputKeyPress(widget, KEY_ENTER);
    }
}

txt_widget_class_t txt_key_input_class =
{
    TXT_AlwaysSelectable,
    TXT_KeyInputSizeCalc,
    TXT_KeyInputDrawer,
    TXT_KeyInputKeyPress,
    TXT_KeyInputDestructor,
    TXT_KeyInputMousePress,
    NULL,
};

txt_key_input_t *TXT_NewKeyInput(int key)
{
    txt_key_input_t *key_input;

    key_input = malloc(sizeof(txt_key_input_t));

    TXT_InitWidget(key_input, &txt_key_input_class);
    key_input->key = key;

    return key_input;
}

