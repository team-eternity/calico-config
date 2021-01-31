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

#include "../elib/elib.h"
#include "../elib/configfile.h"
#include "../hal/hal_init.h"
#include "../hal/hal_ml.h"
#include "../sdl/sdl_hal.h"
#include "../calico/j_eeprom.h"
#include "../calico/configvars.h"
#include "doomkeys.h"
#include "textscreen.h"
#include "execute.h"
#include "setup_icon.c"
#include "mode.h"
#include "compatibility.h"
#include "display.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup"

#if DEADCODE
static const int cheat_sequence[] =
{
    KEY_UPARROW, KEY_UPARROW, KEY_DOWNARROW, KEY_DOWNARROW,
    KEY_LEFTARROW, KEY_RIGHTARROW, KEY_LEFTARROW, KEY_RIGHTARROW,
    'b', 'a', KEY_ENTER, 0
};

static unsigned int cheat_sequence_index = 0;
#endif

// I think these are good "sensible" defaults:

static void SensibleDefaults(void)
{
    autorun                 = 1;
    rb_linear_filtering     = 0;
    useGamepad              = 1;
    gamepadInvertY          = 0;
    gamepadThreshold        = 8000;
    gamepadTriggerThreshold = 3000;
    gamepadLTriggerAction   = GPT_NONE;
    gamepadRTriggerAction   = GPT_ATTACK;
    useMouse                = 1;
    mouseAcceleration       = 2.0;
    mouseThreshold          = 10;
    mouseSensitivity        = 5;
    s_lowfreq               = 880.0;
    s_highfreq              = 5000.0;
    s_preampmul             = 0.93896;
    s_lowgain               = 1.2;
    s_midgain               = 1.0;
    s_highgain              = 0.8;
    screenwidth             = 320;
    screenheight            = 224;
    fullscreen              = 0;
    monitornum              = 0;
    aspectNum               = 4;
    aspectDenom             = 3;
    renderer                = RENDERER_GL4;
    sfxvolume               = 200;
    musicvolume             = 128;
    controltype             = 0;

    Calico_InitKeyboardKeys();

    Calico_SetKeyboardKey(KBJK_A,      SDLK_UNKNOWN);
    Calico_SetKeyboardKey(KBJK_B,      SDLK_UNKNOWN);
    Calico_SetKeyboardKey(KBJK_C,      SDLK_UNKNOWN);
    Calico_SetKeyboardKey(KBJK_UP,     SDLK_w);
    Calico_SetKeyboardKey(KBJK_DOWN,   SDLK_s);
    Calico_SetKeyboardKey(KBJK_LEFT,   SDLK_LEFT);
    Calico_SetKeyboardKey(KBJK_RIGHT,  SDLK_RIGHT);
    Calico_SetKeyboardKey(KBJK_OPTION, SDLK_ESCAPE);
    Calico_SetKeyboardKey(KBJK_PAUSE,  SDLK_PAUSE);
    Calico_SetKeyboardKey(KBJK_NUM,    SDLK_KP_DIVIDE);
    Calico_SetKeyboardKey(KBJK_STAR,   SDLK_KP_MULTIPLY);
    Calico_SetKeyboardKey(KBJK_0,      SDLK_0);
    Calico_SetKeyboardKey(KBJK_1,      SDLK_1);
    Calico_SetKeyboardKey(KBJK_2,      SDLK_2);
    Calico_SetKeyboardKey(KBJK_3,      SDLK_3);
    Calico_SetKeyboardKey(KBJK_4,      SDLK_4);
    Calico_SetKeyboardKey(KBJK_5,      SDLK_5);
    Calico_SetKeyboardKey(KBJK_6,      SDLK_6);
    Calico_SetKeyboardKey(KBJK_7,      SDLK_7);
    Calico_SetKeyboardKey(KBJK_8,      SDLK_8);
    Calico_SetKeyboardKey(KBJK_9,      SDLK_TAB);
    Calico_SetKeyboardKey(KBJK_STRAFE, SDLK_RALT);
    Calico_SetKeyboardKey(KBJK_SLEFT,  SDLK_a);
    Calico_SetKeyboardKey(KBJK_SRIGHT, SDLK_d);
    Calico_SetKeyboardKey(KBJK_USE,    SDLK_SPACE);
    Calico_SetKeyboardKey(KBJK_PWEAPN, SDLK_LEFTBRACKET);
    Calico_SetKeyboardKey(KBJK_NWEAPN, SDLK_RIGHTBRACKET);
    Calico_SetKeyboardKey(KBJK_ATTACK, SDLK_RCTRL);
    Calico_SetKeyboardKey(KBJK_SPEED,  SDLK_LSHIFT);

    Calico_InitMouseButtons();

    Calico_UpdateMouseButtonToValue(&sdlMouseToJagButton[CMB_LEFT],   KBJK_ATTACK);
    Calico_UpdateMouseButtonToValue(&sdlMouseToJagButton[CMB_MIDDLE], KBJK_STRAFE);
    Calico_UpdateMouseButtonToValue(&sdlMouseToJagButton[CMB_RIGHT],  KBJK_USE);
    Calico_UpdateMouseButtonToValue(&sdlMouseToJagButton[CMB_X1],     KBJK_9);
    Calico_UpdateMouseButtonToValue(&sdlMouseToJagButton[CMB_X2],     KBJK_PAUSE);

    Calico_InitGamepadButtons();

    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_A],             KBJK_USE);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_B],             KBJK_SPEED);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_X],             KBJK_STRAFE);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_Y],             KBJK_9);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_BACK],          KBJK_PAUSE);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_START],         KBJK_OPTION);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSTICK],     KBJK_STAR);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK],    KBJK_NUM);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER],  KBJK_PWEAPN);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER], KBJK_NWEAPN);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_UP],       KBJK_UP);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN],     KBJK_DOWN);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT],     KBJK_LEFT);
    Calico_UpdateGamepadButtonToValue(&sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT],    KBJK_RIGHT);
}

#if DEADCODE
static int MainMenuKeyPress(txt_window_t *window, int key, void *user_data)
{
    if (key == cheat_sequence[cheat_sequence_index])
    {
        ++cheat_sequence_index;

        if (cheat_sequence[cheat_sequence_index] == 0)
        {
            SensibleDefaults();
            cheat_sequence_index = 0;

            window = TXT_MessageBox(NULL, "    \x01    ");

            return 1;
        }
    }
    else
    {
        cheat_sequence_index = 0;
    }

    return 0;
}
#endif

static void DoQuit(void *widget, void *dosave)
{
    if (dosave != NULL)
    {
        Cfg_WriteFile();
        WriteEEProm();
    }

    TXT_Shutdown();

    hal_medialayer.exit();
}

static void QuitConfirm(void *unused1, void *unused2)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_button_t *yes_button;
    txt_button_t *no_button;

    window = TXT_NewWindow(NULL);

    TXT_AddWidgets(window, 
                   label = TXT_NewLabel("Exiting setup.\nSave settings?"),
                   TXT_NewStrut(24, 0),
                   yes_button = TXT_NewButton2("  Yes  ", DoQuit, DoQuit),
                   no_button = TXT_NewButton2("  No   ", DoQuit, NULL),
                   NULL);

    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(yes_button, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(no_button, TXT_HORIZ_CENTER);

    // Only an "abort" button in the middle.
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
}

static void LaunchDoom(void *unused1, void *unused2)
{
    execute_context_t *exec;
    
    // Save configuration first

    Cfg_WriteFile();
    WriteEEProm();

    // Launch Doom

    exec = NewExecuteContext();
    PassThroughArguments(exec);
    if(ExecuteDoom(exec) >= 0)
    {
        // Shut down textscreen GUI
        TXT_Shutdown();
        hal_medialayer.exit();
    }
    else
    {
        ShowLaunchError();
    }
}

static txt_button_t *GetLaunchButton(void)
{
    return TXT_NewButton2("Save parameters and launch game", LaunchDoom, NULL);
}

static void OnCfgCancel(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window = (txt_window_t *)user_data;
    if(window)
        TXT_CloseWindow(window);
}

static void OnDoReloadConfig(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window = (txt_window_t *)user_data;
    
    Cfg_LoadFile();
    ReadEEProm();
    
    if(window)
        TXT_CloseWindow(window);
}

static void ReloadConfig(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_label_t  *label;
    txt_button_t *yes_button, *no_button;

    window = TXT_NewWindow("Confirm reload");

    TXT_AddWidgets(
        window, 
        label = TXT_NewLabel("Unsaved changes will be lost.\nAre you sure?"),
        TXT_NewStrut(24, 0),
        yes_button = TXT_NewButton2("  OK  ", OnDoReloadConfig, window),
        no_button  = TXT_NewButton2("Cancel", OnCfgCancel,      window),
        NULL
    );

    TXT_SetWidgetAlign(label,      TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(yes_button, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(no_button,  TXT_HORIZ_CENTER);

    // Only an "abort" button in the middle.
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT,   NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT,  NULL);
}

static void OnDoResetToDefaults(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window = (txt_window_t *)user_data;

    SensibleDefaults();
    
    if(window)
        TXT_CloseWindow(window);
}

static void ResetToDefaults(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_label_t  *label;
    txt_button_t *yes_button, *no_button;

    window = TXT_NewWindow("Confirm reset to defaults");

    TXT_AddWidgets(
        window, 
        label = TXT_NewLabel("All settings will be reset to defaults.\nAre you sure?"),
        TXT_NewStrut(24, 0),
        yes_button = TXT_NewButton2("  OK  ", OnDoResetToDefaults, window),
        no_button  = TXT_NewButton2("Cancel", OnCfgCancel,         window),
        NULL
    );

    TXT_SetWidgetAlign(label,      TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(yes_button, TXT_HORIZ_CENTER);
    TXT_SetWidgetAlign(no_button,  TXT_HORIZ_CENTER);

    // Only an "abort" button in the middle.
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT,   NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT,  NULL);
}

static void ManageConfig(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;

    window = TXT_NewWindow("Manage configuration files");

    TXT_AddWidgets(
        window,
        TXT_NewStrut(26, 0),
        TXT_NewButton2("Reload configuration...", ReloadConfig,    NULL),
        TXT_NewButton2("Reset to defaults...",    ResetToDefaults, NULL),
        NULL
    );
}

void MainMenu(void)
{
    txt_window_t *window;
    txt_window_action_t *quit_action;
    txt_window_action_t *warp_action;

    window = TXT_NewWindow("Main Menu");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    TXT_AddWidgets(
        window,
        TXT_NewButton2("Configure Display",  ConfigDisplay,  NULL),
        TXT_NewButton2("Configure Sound",    ConfigSound,    NULL),
        TXT_NewButton2("Configure Keyboard", ConfigKeyboard, NULL),
        TXT_NewButton2("Configure Mouse",    ConfigMouse,    NULL),
        TXT_NewButton2("Configure Gamepad",  ConfigJoystick, NULL),
        TXT_NewStrut(0, 1),
        GetLaunchButton(),
        TXT_NewButton2("Manage configuration files...", ManageConfig, NULL),
        NULL
    );

    quit_action = TXT_NewWindowAction(KEY_ESCAPE, "Quit");
    warp_action = TXT_NewWindowAction(KEY_F2, "Warp");
    TXT_SignalConnect(quit_action, "pressed", QuitConfirm, NULL);
    TXT_SignalConnect(warp_action, "pressed",
                      (TxtWidgetSignalFunc) WarpMenu, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, quit_action);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, warp_action);

    //TXT_SetKeyListener(window, MainMenuKeyPress, NULL);
}

//
// Initialize all configuration variables, load config file, etc
//

static void InitConfig(void)
{
    Cfg_LoadFile();
    ReadEEProm();
}

//
// Application icon
//

static void SetIcon(void)
{
    extern SDL_Window *TXT_SDLWindow;
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurfaceFrom((void *) setup_icon_data, setup_icon_w,
                                       setup_icon_h, 32, setup_icon_w * 4,
                                       0xff << 24, 0xff << 16,
                                       0xff << 8, 0xff << 0);

    SDL_SetWindowIcon(TXT_SDLWindow, surface);
    SDL_FreeSurface(surface);
}

static void SetWindowTitle(void)
{
    TXT_SetDesktopTitle("Calico Configurator");
}

// Initialize the textscreen library.

static void InitTextscreen(void)
{
    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialize GUI\n");
        hal_medialayer.exit();
    }

    // Set Romero's "funky blue" color:
    // <https://doomwiki.org/wiki/Romero_Blue>
    TXT_SetColor(TXT_COLOR_BLUE, 0x04, 0x14, 0x40);

    SetIcon();
    SetWindowTitle();
}

// Restart the textscreen library.  Used when the video_driver variable
// is changed.

void RestartTextscreen(void)
{
    TXT_Shutdown();
    InitTextscreen();
}

// 
// Initialize and run the textscreen GUI.
//

static void RunGUI(void)
{
    InitTextscreen();

    TXT_GUIMainLoop();
}

static void MissionSet(void)
{
    SetWindowTitle();
    InitConfig();
    MainMenu();
}

void D_DoomMain(void)
{
    // CALICO: init HAL
    HAL_Init();
    SDL2_InitHAL();

    MissionSet();

    RunGUI();
}
