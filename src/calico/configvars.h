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

#pragma once

#include "SDL.h"

enum kbjoykeys_e
{
   KBJK_A,
   KBJK_B,
   KBJK_C,
   KBJK_UP,
   KBJK_DOWN,
   KBJK_LEFT,
   KBJK_RIGHT,
   KBJK_OPTION,
   KBJK_PAUSE,
   KBJK_NUM,
   KBJK_STAR,
   KBJK_0,
   KBJK_1,
   KBJK_2,
   KBJK_3,
   KBJK_4,
   KBJK_5,
   KBJK_6,
   KBJK_7,
   KBJK_8,
   KBJK_9,
   KBJK_STRAFE,
   KBJK_SLEFT,
   KBJK_SRIGHT,
   KBJK_USE,
   KBJK_PWEAPN,
   KBJK_NWEAPN,
   KBJK_ATTACK,
   KBJK_SPEED,
   KBJK_MAX
};

enum gptriggeraction_e
{
   GPT_NONE,
   GPT_LSTRAFE,
   GPT_RSTRAFE,
   GPT_PREVWEAPON,
   GPT_NEXTWEAPON,
   GPT_ATTACK,
   GPT_USE,
   GPT_MAX
};

enum calico_mb_e
{
    CMB_LEFT,
    CMB_MIDDLE,
    CMB_RIGHT,
    CMB_X1,
    CMB_X2,
    NUM_CALICO_MBS
};

enum renderer_e
{
    RENDERER_GL1_1, // GL 1.1 renderer
    RENDERER_GL4,   // GL 4 renderer

    RENDERER_MIN = RENDERER_GL1_1,
    RENDERER_MAX = RENDERER_GL4
};

typedef struct mbutton_t
{
   uint8_t button;   // SDL controller button for this slot
   int     kbButton; // KBJ button to which the SDL controller button is assigned
   char   *name;     // name of Jag controller button for config
   int     down;     // if true, button is down
} mbutton_t;

typedef struct gpbutton_t
{
   int   button;   // SDL controller button for this slot
   int   kbButton; // KBJ button to which the SDL controller button is assigned
   char *name;     // name of Jag controller button for config
   int   down;     // if true, button is down
} gpbutton_t;

#if defined(__cplusplus)
extern "C" {
#endif

extern char       *kbKeyNames[KBJK_MAX];
extern const char *kbJoyKeyJagNames[KBJK_MAX+1];
extern mbutton_t   sdlMouseToJagButton[NUM_CALICO_MBS];
extern gpbutton_t  sdlButtonToJagButton[SDL_CONTROLLER_BUTTON_MAX];

extern int     autorun;                 // *
extern int     rb_linear_filtering;     // *
extern int     useGamepad;              // *
extern char   *gamepadDevice;           // *
extern int     gamepadInvertY;          // *
extern int     gamepadThreshold;        // *
extern int     gamepadTriggerThreshold; // *
extern int     gamepadLTriggerAction;   // *
extern int     gamepadRTriggerAction;   // *
extern int     useMouse;                // *
extern double  mouseAcceleration;       // *
extern int     mouseThreshold;          // *
extern int     mouseSensitivity;        // *
extern double  s_lowfreq;               // *
extern double  s_highfreq;              // *
extern double  s_preampmul;             // *
extern double  s_lowgain;               // *
extern double  s_midgain;               // *
extern double  s_highgain;              // *
extern int     screenwidth;             // *
extern int     screenheight;            // *
extern int     fullscreen;              // *
extern int     monitornum;              // *
extern int     aspectNum;               // *
extern int     aspectDenom;             // *
extern int     renderer;                // *

void Calico_InitKeyboardKeys(void);
void Calico_SetKeyboardKey(int key, SDL_Keycode k);
const char *Calico_GetCurrentKeyBinding(int key);

void Calico_InitMouseButtons(void);
void Calico_UpdateMouseButton(mbutton_t *mbutton);
void Calico_UpdateMouseButtonToValue(mbutton_t *mbutton, int kbButton);

void Calico_InitGamepadButtons(void);
void Calico_UpdateGamepadButton(gpbutton_t *gpbutton);
void Calico_UpdateGamepadButtonToValue(gpbutton_t *gpbutton, int kbButton);

#if defined(__cplusplus)
}
#endif

// EOF
