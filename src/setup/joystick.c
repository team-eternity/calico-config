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
#include "../elib/misc.h"
#include "../calico/configvars.h"
#include "m_misc.h"
#include "textscreen.h"

#include "execute.h"
#include "joystick.h"
#include "mode.h"
#include "txt_joyaxis.h"
#include "txt_joybinput.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-gamepad"

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

// SDL joystick successfully initialized?

static int joystick_initted = 0;

// Calibration button. This is the button the user pressed at the
// start of the calibration sequence. They *must* press this button
// for each subsequent sequence.

static int calibrate_button = -1;

// Which joystick axis to use for horizontal movement, and whether to
// invert the direction:

static int joystick_x_axis = 0;
static int joystick_x_invert = 0;

// Which joystick axis to use for vertical movement, and whether to
// invert the direction:

static int joystick_y_axis = 1;
static int joystick_y_invert = 0;

// Strafe axis.

static int joystick_strafe_axis = -1;
static int joystick_strafe_invert = 0;

// Look axis.

static int joystick_look_axis = -1;
static int joystick_look_invert = 0;

static txt_button_t *joystick_button;
static txt_joystick_axis_t *x_axis_widget;
static txt_joystick_axis_t *y_axis_widget;

//
// Calibration
//

static txt_window_t *calibration_window;
static SDL_GameController **all_joysticks = NULL;
static int all_joysticks_len = 0;

static void InitJoystick(void)
{
    if (!joystick_initted)
    {
        joystick_initted = SDL_Init(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER) >= 0;
    }
}

static void UnInitJoystick(void)
{
    if (joystick_initted)
    {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK|SDL_INIT_GAMECONTROLLER);
        joystick_initted = 0;
    }
}

// Set the label showing the name of the currently selected joystick
static void SetJoystickButtonLabel(void)
{
    const char *name = "None set";

    if(useGamepad)
    {
        // try to find the device named in the configuration file
        if(!estrempty(gamepadDevice))
        {
            const SDL_JoystickGUID currentGUID = SDL_JoystickGetGUIDFromString(gamepadDevice);
            name = "Not found (device disconnected?)";

            for(int i = 0; i < all_joysticks_len; i++)
            {
                if(all_joysticks[i])
                {
                    const SDL_JoystickGUID thisGUID = SDL_JoystickGetDeviceGUID(i);
                    if(!memcmp(&thisGUID, &currentGUID, sizeof(SDL_JoystickGUID)))
                    {
                        const char *const sdlName = SDL_GameControllerName(all_joysticks[i]);
                        if(sdlName)
                        {
                            name = sdlName;
                            break;
                        }
                    }
                }
            }
        }
    }

    TXT_SetButtonLabel(joystick_button, name);
}

// Try to open all joysticks visible to SDL.

static int OpenAllJoysticks(void)
{
    int i;
    int result;

    InitJoystick();

    all_joysticks_len = SDL_NumJoysticks();
    all_joysticks = calloc(all_joysticks_len, sizeof(SDL_Joystick *));

    result = 0;

    for (i = 0; i < all_joysticks_len; ++i)
    {
        if(SDL_IsGameController(i))
            all_joysticks[i] = SDL_GameControllerOpen(i);

        // If any joystick is successfully opened, return true.

        if (all_joysticks[i] != NULL)
        {
            result = 1;
        }
    }

    // Success? Turn on joystick events.

    if (result)
    {
        SDL_GameControllerEventState(SDL_ENABLE);
    }
    else
    {
        free(all_joysticks);
        all_joysticks = NULL;
    }

    return result;
}

// Close all the joysticks opened with OpenAllJoysticks()

static void CloseAllJoysticks(void)
{
    int i;

    for (i = 0; i < all_joysticks_len; ++i)
    {
        if (all_joysticks[i] != NULL)
        {
            SDL_GameControllerClose(all_joysticks[i]);
        }
    }

    SDL_GameControllerEventState(SDL_DISABLE);

    free(all_joysticks);
    all_joysticks = NULL;

    UnInitJoystick();
}

static void CalibrateXAxis(void)
{
    TXT_ConfigureJoystickAxis(x_axis_widget, calibrate_button, NULL);
}

// Given the SDL_JoystickID instance ID from a button event, set the
// joystick_guid and joystick_index config variables.
static boolean SetJoystickGUID(SDL_JoystickID joy_id)
{
    SDL_GameController *const pController = SDL_GameControllerFromInstanceID(joy_id);
    if(pController)
    {
        const SDL_JoystickGUID newGUID = SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(pController));
        char newGUIDStr[33];
        
        SDL_JoystickGetGUIDString(newGUID, newGUIDStr, earrlen(newGUIDStr));

        if(gamepadDevice)
            efree(gamepadDevice);
        gamepadDevice = estrdup(newGUIDStr);

        return true;
    }

    return false;
}

static int CalibrationEventCallback(SDL_Event *event, void *user_data)
{
    if (event->type != SDL_CONTROLLERBUTTONDOWN)
    {
        return 0;
    }

    if (!SetJoystickGUID(event->jbutton.which))
    {
        return 0;
    }

    // At this point, we have a button press.
    // In the first "center" stage, we're just trying to work out which
    // joystick is being configured and which button the user is pressing.
    useGamepad = 1;
    calibrate_button = event->cbutton.button;

    // proceed with calibration.
    TXT_CloseWindow(calibration_window);

    // Calibrate joystick axes: Y axis first, then X axis once
    // completed.
    //TXT_ConfigureJoystickAxis(y_axis_widget, calibrate_button, CalibrateXAxis);

    return 1;
}

static void NoJoystick(void)
{
    TXT_MessageBox(NULL, "No gamepads or joysticks could be found.\n\n"
                         "Try configuring your controller from within\n"
                         "your OS first. Maybe you need to install\n"
                         "some drivers or otherwise configure it.");

    useGamepad = 0;
    SetJoystickButtonLabel();
}

static void CalibrateWindowClosed(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    TXT_SDL_SetEventCallback(NULL, NULL);
    SetJoystickButtonLabel();
    CloseAllJoysticks();
}

static void CalibrateJoystick(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    // Try to open all available joysticks.  If none are opened successfully,
    // bomb out with an error.

    if (!OpenAllJoysticks())
    {
        NoJoystick();
        return;
    }

    calibration_window = TXT_NewWindow("Gamepad calibration");

    TXT_AddWidgets(calibration_window,
                   TXT_NewStrut(0, 1),
                   TXT_NewLabel("Center the D-pad or joystick,\n"
                                "and press a button."),
                   TXT_NewStrut(0, 1),
                   NULL);

    TXT_SetWindowAction(calibration_window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(calibration_window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(calibration_window));
    TXT_SetWindowAction(calibration_window, TXT_HORIZ_RIGHT, NULL);

    TXT_SDL_SetEventCallback(CalibrationEventCallback, NULL);

    TXT_SignalConnect(calibration_window, "closed", CalibrateWindowClosed, NULL);

    // Start calibration
    useGamepad = 0;
}

//
// GUI
//

static void GamepadButtonSetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(gpbutton))
{
    TXT_CAST_ARG(gpbutton_t, gpbutton);
    Calico_UpdateGamepadButton(gpbutton);
}

static void AddJoystickControl(TXT_UNCAST_ARG(table), const char *label, gpbutton_t *gpb)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_dropdown_list_t *dropdown;

    InitDisplayVals();
    
    TXT_AddWidgets(
        table, 
        TXT_NewLabel(label),
        (dropdown = TXT_NewDropdownList(&gpb->kbButton, displayVals, KBJK_MAX+1)),
        TXT_TABLE_EMPTY,
        NULL
    );

    TXT_SignalConnect(dropdown, "changed", GamepadButtonSetCallback, &gpb);
}

static const char *triggerActionNames[GPT_MAX] =
{
    "None",
    "Strafe Left",
    "Strafe Right",
    "Prev Weapon",
    "Next Weapon",
    "Attack",
    "Use"
};

static void AddTriggerControl(TXT_UNCAST_ARG(table), const char *label, int *triggerVar)
{
    TXT_CAST_ARG(txt_table_t, table);

    TXT_AddWidgets(
        table,
        TXT_NewLabel(label),
        TXT_NewDropdownList(triggerVar, triggerActionNames, earrlen(triggerActionNames)),
        TXT_TABLE_EMPTY,
        NULL
    );
}

static void AdvancedCallback(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    window = TXT_NewWindow("Advanced options");
    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 18, 12);

    TXT_AddWidgets(
        window,
        TXT_NewCheckBox("Invert Y axis", &gamepadInvertY),
        TXT_TABLE_OVERFLOW_RIGHT,
        TXT_NewLabel("Axis threshold"),
        TXT_NewIntInputBox(&gamepadThreshold, 6),
        TXT_NewLabel("Trigger threshold"),
        TXT_NewIntInputBox(&gamepadTriggerThreshold, 6),
        NULL
    );
}

static txt_window_action_t *AdvancedAction(void)
{
    txt_window_action_t *adv_action;
    
    adv_action = TXT_NewWindowAction('a', "Advanced");
    TXT_SignalConnect(adv_action, "pressed", AdvancedCallback, NULL);

    return adv_action;
}

void ConfigJoystick(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    // CALICO: init gamepad button configuration
    Calico_InitGamepadButtons();

    window = TXT_NewWindow("Gamepad configuration");
    TXT_SetTableColumns(window, 6);
    TXT_SetColumnWidths(window, 18, 10, 1, 15, 10, 0);
    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewCheckBox("Enable gamepad", &useGamepad),
                   TXT_TABLE_EOL,

                   TXT_NewLabel("Gamepad device"),
                   joystick_button = TXT_NewButton("zzzz"),
                   TXT_TABLE_EOL,
                   NULL);

    TXT_AddWidget(window, TXT_NewSeparator("Buttons"));

    AddJoystickControl(window, "A",              &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_A]);
    AddJoystickControl(window, "B",              &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_B]);
    AddJoystickControl(window, "X",              &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_X]);
    AddJoystickControl(window, "Y",              &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_Y]);
    AddJoystickControl(window, "Back",           &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_BACK]);
    AddJoystickControl(window, "Start",          &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_START]);
    AddJoystickControl(window, "Left Stick",     &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSTICK]);
    AddJoystickControl(window, "Right Stick",    &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK]);
    AddJoystickControl(window, "Left Shoulder",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]);
    AddJoystickControl(window, "Right Shoulder", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER]);
    AddJoystickControl(window, "DPad Up",        &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_UP]);
    AddJoystickControl(window, "DPad Down",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN]);
    AddJoystickControl(window, "DPad Left",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT]);
    AddJoystickControl(window, "DPad Right",     &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]);

    TXT_AddWidget(window, TXT_NewSeparator("Trigger Actions"));

    AddTriggerControl(window, "L. Trigger", &gamepadLTriggerAction);
    AddTriggerControl(window, "R. Trigger", &gamepadRTriggerAction);

    TXT_SignalConnect(joystick_button, "pressed", CalibrateJoystick, NULL);
    //TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, AdvancedAction());

    InitJoystick();
    OpenAllJoysticks();
    SetJoystickButtonLabel();
    CloseAllJoysticks();
}

