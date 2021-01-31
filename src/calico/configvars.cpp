/*
  CALICO
  
  Configuration variables for Calico Configurator
  
  The MIT License (MIT)
  
  Copyright (c) 2021 James Haley
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "SDL.h"

#include "../elib/elib.h"
#include "../elib/configfile.h"
#include "../hal/hal_video.h"
#include "configvars.h"

static cfgrange_t<int> boolRange { 0, 1 };

// === g_options ==============================================================

int autorun;

static CfgItem cfgAutorun("g_autorun", &autorun, &boolRange);

// === rb_texture =============================================================

int rb_linear_filtering = 0;

static CfgItem cfgLinearFiltering("linear_filtering", &rb_linear_filtering, &boolRange);

// === sdl_input ==============================================================

const char *kbJoyKeyJagNames[KBJK_MAX+1] =
{
   "a",
   "b",
   "c",
   "up",
   "down",
   "left",
   "right",
   "option",
   "pause",
   "num",
   "star",
   "0",
   "1",
   "2",
   "3",
   "4",
   "5",
   "6",
   "7",
   "8",
   "9",
   "strafeon",
   "strafeleft",
   "straferight",
   "use",
   "prevweapon",
   "nextweapon",
   "attack",
   "speed",
   "unbound"
};

char *kbKeyNames[KBJK_MAX];
static SDL_Keycode kbKeyCodes[KBJK_MAX] =
{
   SDLK_RSHIFT,
   SDLK_RCTRL,
   SDLK_RALT,
   SDLK_UP,
   SDLK_DOWN,
   SDLK_LEFT,
   SDLK_RIGHT,
   SDLK_ESCAPE,
   SDLK_PAUSE,
   SDLK_KP_DIVIDE,
   SDLK_KP_MULTIPLY,
   SDLK_0,
   SDLK_1,
   SDLK_2,
   SDLK_3,
   SDLK_4,
   SDLK_5,
   SDLK_6,
   SDLK_7,
   SDLK_8,
   SDLK_9,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN,
   SDLK_UNKNOWN
};

static CfgItem cfgKeyNameA("kb_key_a",      &kbKeyNames[KBJK_A]);
static CfgItem cfgKeyNameB("kb_key_b",      &kbKeyNames[KBJK_B]);
static CfgItem cfgKeyNameC("kb_key_c",      &kbKeyNames[KBJK_C]);
static CfgItem cfgKeyNameU("kb_key_up",     &kbKeyNames[KBJK_UP]);
static CfgItem cfgKeyNameD("kb_key_down",   &kbKeyNames[KBJK_DOWN]);
static CfgItem cfgKeyNameL("kb_key_left",   &kbKeyNames[KBJK_LEFT]);
static CfgItem cfgKeyNameR("kb_key_right",  &kbKeyNames[KBJK_RIGHT]);
static CfgItem cfgKeyNameO("kb_key_option", &kbKeyNames[KBJK_OPTION]);
static CfgItem cfgKeyNameP("kb_key_pause",  &kbKeyNames[KBJK_PAUSE]);
static CfgItem cfgKeyNameN("kb_key_num",    &kbKeyNames[KBJK_NUM]);
static CfgItem cfgKeyNameS("kb_key_star",   &kbKeyNames[KBJK_STAR]);
static CfgItem cfgKeyName0("kb_key_0",      &kbKeyNames[KBJK_0]);
static CfgItem cfgKeyName1("kb_key_1",      &kbKeyNames[KBJK_1]);
static CfgItem cfgKeyName2("kb_key_2",      &kbKeyNames[KBJK_2]);
static CfgItem cfgKeyName3("kb_key_3",      &kbKeyNames[KBJK_3]);
static CfgItem cfgKeyName4("kb_key_4",      &kbKeyNames[KBJK_4]);
static CfgItem cfgKeyName5("kb_key_5",      &kbKeyNames[KBJK_5]);
static CfgItem cfgKeyName6("kb_key_6",      &kbKeyNames[KBJK_6]);
static CfgItem cfgKeyName7("kb_key_7",      &kbKeyNames[KBJK_7]);
static CfgItem cfgKeyName8("kb_key_8",      &kbKeyNames[KBJK_8]);
static CfgItem cfgKeyName9("kb_key_9",      &kbKeyNames[KBJK_9]);

static CfgItem cfgKeyNameStrafeOn   ("kb_key_strafeon",    &kbKeyNames[KBJK_STRAFE]);
static CfgItem cfgKeyNameStrafeLeft ("kb_key_strafeleft",  &kbKeyNames[KBJK_SLEFT ]);
static CfgItem cfgKeyNameStrafeRight("kb_key_straferight", &kbKeyNames[KBJK_SRIGHT]);
static CfgItem cfgKeyNameUse        ("kb_key_use",         &kbKeyNames[KBJK_USE   ]);
static CfgItem cfgKeyNamePrevWeapon ("kb_key_prevweapon",  &kbKeyNames[KBJK_PWEAPN]);
static CfgItem cfgKeyNameNextWeapon ("kb_key_nextweapon",  &kbKeyNames[KBJK_NWEAPN]);
static CfgItem cfgKeyNameAttack     ("kb_key_attack",      &kbKeyNames[KBJK_ATTACK]);
static CfgItem cfgKeyNameSpeed      ("kb_key_speed",       &kbKeyNames[KBJK_SPEED ]);

int   useGamepad;
char *gamepadDevice;
int   gamepadInvertY;
int   gamepadThreshold = 8000;
int   gamepadTriggerThreshold = 3000;
int   gamepadLTriggerAction = GPT_NONE;
int   gamepadRTriggerAction = GPT_ATTACK;

static cfgrange_t<int> triggerRange = { GPT_NONE, GPT_MAX - 1 };

static CfgItem cfgUseGamepad      ("use_gamepad",              &useGamepad,            &boolRange);
static CfgItem cfgGamepad         ("gamepad",                  &gamepadDevice          );
static CfgItem cfgGamepadInvertY  ("gamepad_inverty",          &gamepadInvertY,        &boolRange);
static CfgItem cfgGamepadThreshold("gamepad_xythreshold",      &gamepadThreshold       );
static CfgItem cfgGamepadTThresh  ("gamepad_triggerthreshold", &gamepadTriggerThreshold);

static CfgItem cfgGamepadLTrigger("gamepad_ltrigger", &gamepadLTriggerAction, &triggerRange);
static CfgItem cfgGamepadRTrigger("gamepad_rtrigger", &gamepadRTriggerAction, &triggerRange);

gpbutton_t sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_MAX] =
{
   { SDL_CONTROLLER_BUTTON_A,             KBJK_USE,    nullptr, false }, // default: use
   { SDL_CONTROLLER_BUTTON_B,             KBJK_SPEED,  nullptr, false }, // default: speed-on
   { SDL_CONTROLLER_BUTTON_X,             KBJK_STRAFE, nullptr, false }, // default: strafe-on
   { SDL_CONTROLLER_BUTTON_Y,             KBJK_9,      nullptr, false }, // default: toggle map
   { SDL_CONTROLLER_BUTTON_BACK,          KBJK_PAUSE,  nullptr, false }, // default: pause
   { SDL_CONTROLLER_BUTTON_GUIDE,         KBJK_MAX,    nullptr, false }, // default: unassigned (not usable on Windows)
   { SDL_CONTROLLER_BUTTON_START,         KBJK_OPTION, nullptr, false }, // default: toggle menu
   { SDL_CONTROLLER_BUTTON_LEFTSTICK,     KBJK_STAR,   nullptr, false }, // default: jag * key
   { SDL_CONTROLLER_BUTTON_RIGHTSTICK,    KBJK_NUM,    nullptr, false }, // default: jag # key
   { SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  KBJK_PWEAPN, nullptr, false }, // default: previous weapon
   { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, KBJK_NWEAPN, nullptr, false }, // default: next weapon
   { SDL_CONTROLLER_BUTTON_DPAD_UP,       KBJK_UP,     nullptr, false }, // default: move up
   { SDL_CONTROLLER_BUTTON_DPAD_DOWN,     KBJK_DOWN,   nullptr, false }, // default: move down
   { SDL_CONTROLLER_BUTTON_DPAD_LEFT,     KBJK_LEFT,   nullptr, false }, // default: move left
   { SDL_CONTROLLER_BUTTON_DPAD_RIGHT,    KBJK_RIGHT,  nullptr, false }, // default: move right
};

static CfgItem cfgGamepadButtonA      { "gamepad_button_a",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_A            ].name };
static CfgItem cfgGamepadButtonB      { "gamepad_button_b",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_B            ].name };
static CfgItem cfgGamepadButtonX      { "gamepad_button_x",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_X            ].name };
static CfgItem cfgGamepadButtonY      { "gamepad_button_y",      &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_Y            ].name };
static CfgItem cfgGamepadButtonBack   { "gamepad_button_back",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_BACK         ].name };
static CfgItem cfgGamepadButtonGuide  { "gamepad_button_guide",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_GUIDE        ].name };
static CfgItem cfgGamepadButtonStart  { "gamepad_button_start",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_START        ].name };
static CfgItem cfgGamepadButtonLStick { "gamepad_button_lstick", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSTICK    ].name };
static CfgItem cfgGamepadButtonRStick { "gamepad_button_rstick", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSTICK   ].name };
static CfgItem cfgGamepadButtonLShldr { "gamepad_button_lshldr", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_LEFTSHOULDER ].name };
static CfgItem cfgGamepadButtonRShldr { "gamepad_button_rshldr", &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER].name };
static CfgItem cfgGamepadButtonUp     { "gamepad_button_up",     &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_UP      ].name };
static CfgItem cfgGamepadButtonDown   { "gamepad_button_down",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_DOWN    ].name };
static CfgItem cfgGamepadButtonLeft   { "gamepad_button_left",   &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_LEFT    ].name };
static CfgItem cfgGamepadButtonRight  { "gamepad_button_right",  &sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_DPAD_RIGHT   ].name };

int    useMouse          = 1;
double mouseAcceleration = 2.0;
int    mouseThreshold    = 10;
int    mouseSensitivity  = 5;

static cfgrange_t<double> mouseAccelRng  { 1.0, 5.0 };
static cfgrange_t<int>    mouseThreshRng { 0,   32  };
static cfgrange_t<int>    mouseSenseRng  { 1,   256 };

static CfgItem cfgUseMouse         { "useMouse",          &useMouse,          &boolRange      };
static CfgItem cfgMouseAccel       { "mouseAcceleration", &mouseAcceleration, &mouseAccelRng  };
static CfgItem cfgMouseThreshold   { "mouseThreshold",    &mouseThreshold,    &mouseThreshRng };
static CfgItem cfgMouseSensitivity { "mouseSensitivity",  &mouseSensitivity,  &mouseSenseRng  };

mbutton_t sdlMouseToJagButton[NUM_CALICO_MBS] =
{
   { SDL_BUTTON_LEFT,   KBJK_ATTACK, nullptr, false }, // default: attack
   { SDL_BUTTON_MIDDLE, KBJK_STRAFE, nullptr, false }, // default: strafe
   { SDL_BUTTON_RIGHT,  KBJK_USE,    nullptr, false }, // default: use
   { SDL_BUTTON_X1,     KBJK_9,      nullptr, false }, // default: toggle map
   { SDL_BUTTON_X2,     KBJK_PAUSE,  nullptr, false }, // default: pause
};

static CfgItem cfgMouseButtonLeft   { "mouse_button_left",   &sdlMouseToJagButton[CMB_LEFT  ].name };
static CfgItem cfgMouseButtonMiddle { "mouse_button_middle", &sdlMouseToJagButton[CMB_MIDDLE].name };
static CfgItem cfgMouseButtonRight  { "mouse_button_right",  &sdlMouseToJagButton[CMB_RIGHT ].name };
static CfgItem cfgMouseButtonX1     { "mouse_button_x1",     &sdlMouseToJagButton[CMB_X1    ].name };
static CfgItem cfgMouseButtonX2     { "mouse_button_x2",     &sdlMouseToJagButton[CMB_X2    ].name };

void Calico_InitKeyboardKeys()
{
    // initialize keyboard key bindings
    for(size_t i = 0; i < KBJK_MAX; i++)
    {
        if(estrempty(kbKeyNames[i]))
            kbKeyNames[i] = estrdup(SDL_GetKeyName(kbKeyCodes[i]));
        else
            kbKeyCodes[i] = SDL_GetKeyFromName(kbKeyNames[i]);
    }
}

void Calico_SetKeyboardKey(int key, SDL_Keycode k)
{
    if(key >= 0 && key < KBJK_MAX)
    {
        kbKeyCodes[key] = k;
        if(kbKeyNames[key])
            efree(kbKeyNames[key]);
        kbKeyNames[key] = estrdup(SDL_GetKeyName(kbKeyCodes[key]));
    }
}

const char *Calico_GetCurrentKeyBinding(int key)
{
    const char *ret = "Error";
    if(key >= 0 && key < KBJK_MAX)
    {
        ret = SDL_GetKeyName(kbKeyCodes[key]);
    }
    return ret;
}

void Calico_InitMouseButtons()
{
    // intialize mouse button bindings
    for(mbutton_t &mbutton : sdlMouseToJagButton)
    {
        if(estrempty(mbutton.name))
        {
            // unconfigured name
            if(mbutton.kbButton >= 0 && mbutton.kbButton < KBJK_MAX)
                mbutton.name = estrdup(kbJoyKeyJagNames[mbutton.kbButton]);
            else
                mbutton.name = estrdup("unbound");
        }
        else
        {
            // name is configured; find the corresponding KBJK enum value and setup the button correspondence
            bool found = false;

            for(size_t j = 0; j < KBJK_MAX; j++)
            {
                if(!strcasecmp(kbJoyKeyJagNames[j], mbutton.name))
                {
                    mbutton.kbButton = int(j);
                    found = true;
                    break;
                }
            }

            // must be unbound
            if(!found)
                mbutton.kbButton = KBJK_MAX;
        }
    }
}

void Calico_UpdateMouseButton(mbutton_t *mbutton)
{
    if(mbutton->name)
    {
        efree(mbutton->name);
        mbutton->name = nullptr;
    }

    if(mbutton->kbButton >= 0 && mbutton->kbButton < KBJK_MAX)
        mbutton->name = estrdup(kbJoyKeyJagNames[mbutton->kbButton]);
    else
        mbutton->name = estrdup("unbound");
}

void Calico_UpdateMouseButtonToValue(mbutton_t *mbutton, int kbButton)
{
    mbutton->kbButton = kbButton;
    Calico_UpdateMouseButton(mbutton);
}

void Calico_InitGamepadButtons()
{
    // intialize gamepad button bindings
    for(gpbutton_t &gpbutton : sdlButtonToJagButton)
    {
        if(estrempty(gpbutton.name))
        {
            // unconfigured name
            if(gpbutton.kbButton >= 0 && gpbutton.kbButton < KBJK_MAX)
                gpbutton.name = estrdup(kbJoyKeyJagNames[gpbutton.kbButton]);
            else
                gpbutton.name = estrdup("unbound");
        }
        else
        {
            // name is configured; find the corresponding KBJK enum value and setup the button correspondence
            bool found = false;

            for(size_t j = 0; j < KBJK_MAX; j++)
            {
                if(!strcasecmp(kbJoyKeyJagNames[j], gpbutton.name))
                {
                    gpbutton.kbButton = int(j);
                    found = true;
                    break;
                }
            }

            // must be unbound
            if(!found)
                gpbutton.kbButton = KBJK_MAX;
        }
    }
}

void Calico_UpdateGamepadButton(gpbutton_t *gpbutton)
{
    if(gpbutton->name)
    {
        efree(gpbutton->name);
        gpbutton->name = nullptr;
    }

    if(gpbutton->kbButton >= 0 && gpbutton->kbButton < KBJK_MAX)
        gpbutton->name = estrdup(kbJoyKeyJagNames[gpbutton->kbButton]);
    else
        gpbutton->name = estrdup("unbound");
}

void Calico_UpdateGamepadButtonToValue(gpbutton_t *gpbutton, int kbButton)
{
    gpbutton->kbButton = kbButton;
    Calico_UpdateGamepadButton(gpbutton);
}

// === sdl_sound ==============================================================

double s_lowfreq   = 880.0;
double s_highfreq  = 5000.0;
double s_preampmul = 0.93896;
double s_lowgain   = 1.2;
double s_midgain   = 1.0;
double s_highgain  = 0.8;

static cfgrange_t<double> gainRange   = { 0.0, 3.0 };
static cfgrange_t<double> preAmpRange = { 0.0, 1.0 };

static CfgItem cfgSLowFreq  ("s_lowfreq",   &s_lowfreq);
static CfgItem cfgSHighFreq ("s_highfreq",  &s_highfreq);
static CfgItem cfgSPreampMul("s_preampmul", &s_preampmul, &preAmpRange);
static CfgItem cfgSLowGain  ("s_lowgain",   &s_lowgain,   &gainRange);
static CfgItem cfgSMidGain  ("s_midgain",   &s_midgain,   &gainRange);
static CfgItem cfgSHighGain ("s_highgain",  &s_highgain,  &gainRange);

// === sdl_video ==============================================================

int screenwidth     = CALICO_ORIG_SCREENWIDTH;
int screenheight    = CALICO_ORIG_SCREENHEIGHT;
int fullscreen      = 0;
int monitornum      = 0;
int aspectNum       = 4;
int aspectDenom     = 3;
int renderer        = RENDERER_GL1_1;

static cfgrange_t<int> swRange = { 320, 32768 };
static cfgrange_t<int> shRange = { 224, 32768 };
static cfgrange_t<int> fsRange = { -1,  1     };
static cfgrange_t<int> anRange = {  3,  100   };
static cfgrange_t<int> adRange = {  2,  100   };

static cfgrange_t<int> rnRange = { RENDERER_MIN, RENDERER_MAX };

static CfgItem cfgScreenWidth  { "screenwidth",  &screenwidth,  &swRange };
static CfgItem cfgScreenHeight { "screenheight", &screenheight, &shRange };
static CfgItem cfgFullScreen   { "fullscreen",   &fullscreen,   &fsRange };
static CfgItem cfgMonitorNum   { "monitornum",   &monitornum };
static CfgItem cfgAspectNum    { "aspectnum",    &aspectNum,    &anRange };
static CfgItem cfgAspectDenom  { "aspectdenom",  &aspectDenom,  &adRange };
static CfgItem cfgRenderer     { "renderer",     &renderer,     &rnRange };

// EOF
