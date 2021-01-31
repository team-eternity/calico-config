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

#include "textscreen.h"
#include "txt_sdl.h"
#include "../elib/elib.h"
#include "../elib/misc.h"
#include "../calico/configvars.h"
#include "../calico/j_eeprom.h"
#include "execute.h"
#include "txt_keyinput.h"
#include "mode.h"
#include "doomkeys.h"
#include "joystick.h"
#include "keyboard.h"

#define WINDOW_HELP_URL "https://www.chocolate-doom.org/setup-keyboard"


// Keys within these groups cannot have the same value.

#if DEADCODE
static int *controls[] = { &key_left, &key_right, &key_up, &key_down,
                           &key_strafeleft, &key_straferight, &key_fire,
                           &key_use, &key_strafe, &key_speed, &key_jump,
                           &key_flyup, &key_flydown, &key_flycenter,
                           &key_lookup, &key_lookdown, &key_lookcenter,
                           &key_invleft, &key_invright, &key_invquery,
                           &key_invuse, &key_invpop, &key_mission, &key_invkey,
                           &key_invhome, &key_invend, &key_invdrop,
                           &key_useartifact, &key_pause, &key_usehealth,
                           &key_weapon1, &key_weapon2, &key_weapon3,
                           &key_weapon4, &key_weapon5, &key_weapon6,
                           &key_weapon7, &key_weapon8,
                           &key_arti_quartz, &key_arti_urn, &key_arti_bomb,
                           &key_arti_tome, &key_arti_ring, &key_arti_chaosdevice,
                           &key_arti_shadowsphere, &key_arti_wings, 
                           &key_arti_torch, &key_arti_morph,
                           &key_arti_all, &key_arti_health, &key_arti_poisonbag,
                           &key_arti_blastradius, &key_arti_teleport,
                           &key_arti_teleportother, &key_arti_egg,
                           &key_arti_invulnerability,
                           &key_prevweapon, &key_nextweapon, NULL };

static int *menu_nav[] = { &key_menu_activate, &key_menu_up, &key_menu_down,
                           &key_menu_left, &key_menu_right, &key_menu_back,
                           &key_menu_forward, NULL };

static int *shortcuts[] = { &key_menu_help, &key_menu_save, &key_menu_load,
                            &key_menu_volume, &key_menu_detail, &key_menu_qsave,
                            &key_menu_endgame, &key_menu_messages, &key_spy,
                            &key_menu_qload, &key_menu_quit, &key_menu_gamma,
                            &key_menu_incscreen, &key_menu_decscreen, 
                            &key_menu_screenshot,
                            &key_message_refresh, &key_multi_msg,
                            &key_multi_msgplayer[0], &key_multi_msgplayer[1],
                            &key_multi_msgplayer[2], &key_multi_msgplayer[3] };

static int *map_keys[] = { &key_map_north, &key_map_south, &key_map_east,
                           &key_map_west, &key_map_zoomin, &key_map_zoomout,
                           &key_map_toggle, &key_map_maxzoom, &key_map_follow,
                           &key_map_grid, &key_map_mark, &key_map_clearmark,
                           NULL };
#endif

static int VarInGroup(int *variable, int **group)
{
    unsigned int i;

    for (i=0; group[i] != NULL; ++i)
    {
        if (group[i] == variable)
        {
            return 1;
        }
    }

    return 0;
}

static void CheckKeyGroup(int *variable, int **group)
{
    unsigned int i;

    // Don't check unless the variable is in this group.

    if (!VarInGroup(variable, group))
    {
        return;
    }

    // If another variable has the same value as the new value, reset it.

    for (i=0; group[i] != NULL; ++i)
    {
        if (*variable == *group[i] && group[i] != variable)
        {
            // A different key has the same value.  Clear the existing
            // value. This ensures that no two keys can have the same
            // value.

            *group[i] = 0;
        }
    }
}

// Callback invoked when a key control is set

static void KeySetCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(variable))
{
#if DEADCODE
    TXT_CAST_ARG(int, variable);

    CheckKeyGroup(variable, controls);
    CheckKeyGroup(variable, menu_nav);
    CheckKeyGroup(variable, shortcuts);
    CheckKeyGroup(variable, map_keys);
#endif
}

// Add a label and keyboard input to the specified table.

static void AddKeyControl(TXT_UNCAST_ARG(table), const char *name, int key)
{
    TXT_CAST_ARG(txt_table_t, table);
    txt_key_input_t *key_input;

    TXT_AddWidget(table, TXT_NewLabel(name));
    key_input = TXT_NewKeyInput(key);
    TXT_AddWidget(table, key_input);

    //TXT_SignalConnect(key_input, "set", KeySetCallback, var);
}

static void AddSectionLabel(TXT_UNCAST_ARG(table), const char *title,
                            boolean add_space)
{
    TXT_CAST_ARG(txt_table_t, table);
    char buf[64];

    if (add_space)
    {
        TXT_AddWidgets(table,
                       TXT_NewStrut(0, 1),
                       TXT_TABLE_EOL,
                       NULL);
    }

    psnprintf(buf, sizeof(buf), " - %s - ", title);

    TXT_AddWidgets(table,
                   TXT_NewLabel(buf),
                   TXT_TABLE_EOL,
                   NULL);
}

static void ConfigExtraKeys(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_scrollpane_t *scrollpane;
    txt_table_t *table;

    window = TXT_NewWindow("Extra keyboard controls");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 21, 20);

    scrollpane = TXT_NewScrollPane(0, 13, table);
    TXT_AddWidget(window, scrollpane);

    AddSectionLabel(table, "Face buttons", false);

    AddKeyControl(table, "A", KBJK_A);
    AddKeyControl(table, "B", KBJK_B);
    AddKeyControl(table, "C", KBJK_C);

    AddSectionLabel(table, "Phone keys", true);

    AddKeyControl(table, "Pound", KBJK_NUM);
    AddKeyControl(table, "Star",  KBJK_STAR);
    AddKeyControl(table, "0",     KBJK_0);

    AddSectionLabel(table, "Weapons", true);

    AddKeyControl(table, "Weapon 1",        KBJK_1);
    AddKeyControl(table, "Weapon 2",        KBJK_2);
    AddKeyControl(table, "Weapon 3",        KBJK_3);
    AddKeyControl(table, "Weapon 4",        KBJK_4);
    AddKeyControl(table, "Weapon 5",        KBJK_5);
    AddKeyControl(table, "Weapon 6",        KBJK_6);
    AddKeyControl(table, "Weapon 7",        KBJK_7);
    AddKeyControl(table, "Weapon 8",        KBJK_8);
    AddKeyControl(table, "Previous weapon", KBJK_PWEAPN);
    AddKeyControl(table, "Next weapon",     KBJK_NWEAPN);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TXT_NewWindowAction(KEY_DEL, "Clear"));
}

static void OtherKeysDialog(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;
    txt_table_t *table;
    txt_scrollpane_t *scrollpane;

    window = TXT_NewWindow("Other keys");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    table = TXT_NewTable(2);

    TXT_SetColumnWidths(table, 25, 9);

    AddSectionLabel(table, "Menu", true);
    AddKeyControl(table, "Toggle menu", KBJK_OPTION);

    AddSectionLabel(table, "Shortcut keys", true);
    AddKeyControl(table, "Pause game", KBJK_PAUSE);

    AddSectionLabel(table, "Map", true);
    AddKeyControl(table, "Toggle map", KBJK_9);

    scrollpane = TXT_NewScrollPane(0, 13, table);

    TXT_AddWidget(window, scrollpane);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TXT_NewWindowAction(KEY_DEL, "Clear"));
}

typedef struct labelholder_t
{
    txt_label_t *lbl_a, *lbl_b, *lbl_c;
} labelholder_t;

static void OnControlDropdownChanged(TXT_UNCAST_ARG(widget), void *user_data)
{
    const labelholder_t *const pLabels = (labelholder_t *)user_data;

    TXT_SetLabel(pLabels->lbl_a, buttona[controltype]);
    TXT_SetLabel(pLabels->lbl_b, buttonb[controltype]);
    TXT_SetLabel(pLabels->lbl_c, buttonc[controltype]);
}

static void ControlSchemeDialog(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    txt_window_t *window;

    window = TXT_NewWindow("Configure A/B/C buttons");

    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 16, 10);

    static labelholder_t labels;
    txt_dropdown_list_t *list;
    TXT_AddWidgets(
        window,
        TXT_NewLabel("Control scheme"),
        (list = TXT_NewDropdownList(&controltype, buttons_abc, earrlen(buttons_abc))),
        TXT_NewStrut(24, 1),
        TXT_TABLE_EOL,
        TXT_NewSeparator("Control diagram"),
        TXT_NewLabel("A"),
        (labels.lbl_a = TXT_NewLabel(buttona[controltype])),
        TXT_NewLabel("B"),
        (labels.lbl_b = TXT_NewLabel(buttonb[controltype])),
        TXT_NewLabel("C"),
        (labels.lbl_c = TXT_NewLabel(buttonc[controltype])),
        NULL
    );

    TXT_SetFGColor(labels.lbl_a, TXT_COLOR_BRIGHT_GREEN);
    TXT_SetFGColor(labels.lbl_b, TXT_COLOR_BRIGHT_GREEN);
    TXT_SetFGColor(labels.lbl_c, TXT_COLOR_BRIGHT_GREEN);

    TXT_SignalConnect(list, "changed", OnControlDropdownChanged, &labels);
}

void ConfigKeyboard(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_checkbox_t *run_control;

    // CALICO: initialize keyboard configuration
    Calico_InitKeyboardKeys();

    window = TXT_NewWindow("Keyboard configuration");

    TXT_SetWindowHelpURL(window, WINDOW_HELP_URL);

    // The window is on a 5-column grid layout that looks like:
    // Label | Control | | Label | Control
    // There is a small gap between the two conceptual "columns" of
    // controls, just for spacing.
    TXT_SetTableColumns(window, 5);
    TXT_SetColumnWidths(window, 15, 8, 2, 15, 8);

    TXT_AddWidget(window, TXT_NewSeparator("Movement"));

    AddKeyControl(window, "Move forward", KBJK_UP);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Strafe left",  KBJK_SLEFT);

    AddKeyControl(window, "Move backward", KBJK_DOWN);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Strafe right", KBJK_SRIGHT);

    AddKeyControl(window, "Turn left", KBJK_LEFT);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Speed on", KBJK_SPEED);

    AddKeyControl(window, "Turn right", KBJK_RIGHT);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Strafe on", KBJK_STRAFE);

    TXT_AddWidget(window, TXT_NewSeparator("Action"));

    AddKeyControl(window, "Fire/Attack", KBJK_ATTACK);
    TXT_AddWidget(window, TXT_TABLE_EMPTY);
    AddKeyControl(window, "Use", KBJK_USE);

    TXT_AddWidgets(window,
                   TXT_NewButton2("More controls...", ConfigExtraKeys, NULL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_TABLE_EMPTY,
                   TXT_NewButton2("Other keys...", OtherKeysDialog, NULL),
                   TXT_TABLE_OVERFLOW_RIGHT,

                   TXT_NewSeparator("Misc."),
                   TXT_NewButton2("Set A/B/C button functions...", ControlSchemeDialog, NULL),
                   TXT_TABLE_EOL,
                   run_control = TXT_NewCheckBox("Always run", &autorun),
                   TXT_TABLE_EOL,
                   NULL);

    //TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TestConfigAction());
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TXT_NewWindowAction(KEY_DEL, "Clear"));
}
