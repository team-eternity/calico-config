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

#include <inttypes.h>
#include "../elib/elib.h"
#include "../elib/misc.h"
#include "../elib/configfile.h"
#include "../hal/hal_ml.h"
#include "../hal/hal_platform.h"
#include "../calico/j_eeprom.h"
#include "../choco/doomkeys.h"
#include "textscreen.h"
#include "m_misc.h"
#include "multiplayer.h"
#include "mode.h"
#include "execute.h"

#define MULTI_START_HELP_URL "https://www.chocolate-doom.org/setup-multi-start"
#define MULTI_JOIN_HELP_URL "https://www.chocolate-doom.org/setup-multi-join"
#define MULTI_CONFIG_HELP_URL "https://www.chocolate-doom.org/setup-multi-config"
#define LEVEL_WARP_HELP_URL "https://www.chocolate-doom.org/setup-level-warp"

#define NUM_WADS 10
#define NUM_EXTRA_PARAMS 10

typedef enum
{
    WARP_ExMy,  // CALICO: NB: currently unused
    WARP_MAPxy,
} warptype_t;

typedef struct iwad_t
{
    const char *filename;
    const char *game;
} iwad_t;

// Fallback IWADs to use if no IWADs are detected.

static const iwad_t fallback_iwads[] = {
    { "./jagdoom.wad", "Doom (WAD)" },
    { "./doom.jag",    "Doom (ROM)" },
};

// Array of IWADs found to be installed

static const iwad_t **found_iwads;
static const char   **iwad_labels;

// Index of the currently selected IWAD

static int found_iwad_selected = -1;

// Filename to pass to '-iwad'.

static const char *iwadfile;

static const char *wad_extensions[] = { "wad", "jag", NULL };

static const char *doom_skills[] =
{
    "I'm A Wimp.", 
    "Not too rough.", 
    "Hurt me plenty.",
    "Ultra-Violence.",
    "Nightmare!",
};

static const char *gamemodes[] = { "Co-operative", "Deathmatch" };

static char *net_player_name;
static char *chat_macros[10];

static char *wads[NUM_WADS];
static char *extra_params[NUM_EXTRA_PARAMS];
static int character_class = 0;
static int skill = 2;
static int nomonsters = 0;
static int deathmatch = 0;
static int strife_altdeath = 0;
static int fast = 0;
static int respawn = 0;
static int udpport = 2342;
static int timer = 0;
static int privateserver = 0;

static txt_dropdown_list_t *skillbutton;
static txt_button_t *warpbutton;
static warptype_t warptype = WARP_MAPxy;
static int warpepisode = 1;
static int warpmap = 1;

// Address to connect to when joining a game

static char *connect_address = NULL;

static txt_window_t *query_window;
static int query_servers_found;

// Find an IWAD from its description

static const iwad_t *GetCurrentIWAD(void)
{
    return found_iwads[found_iwad_selected];
}

static void AddWADs(execute_context_t *exec)
{
    int have_wads = 0;
    int i;
 
    for (i=0; i<NUM_WADS; ++i)
    {
        if (wads[i] != NULL && strlen(wads[i]) > 0)
        {
            if (!have_wads)
            {
                AddCmdLineParameter(exec, "-file");
            }

            AddCmdLineParameter(exec, "\"%s\"", wads[i]);
        }
    }
}

static void AddExtraParameters(execute_context_t *exec)
{
    int i;
    
    for (i=0; i<NUM_EXTRA_PARAMS; ++i)
    {
        if (extra_params[i] != NULL && strlen(extra_params[i]) > 0)
        {
            AddCmdLineParameter(exec, "%s", extra_params[i]);
        }
    }
}

static void AddIWADParameter(execute_context_t *exec)
{
    if (iwadfile != NULL)
    {
        AddCmdLineParameter(exec, "-iwad \"%s\"", iwadfile);
    }
}

void ShowLaunchError(void)
{
    txt_window_t *window;

    window = TXT_NewWindow("Execution error");
    TXT_AddWidget(window, TXT_NewLabel("Could not start the game."));

    TXT_SetWindowAction(window, TXT_HORIZ_LEFT,  NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);

    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, TXT_NewWindowEscapeAction(window));
}

// Callback function invoked to launch the game.
// This is used when starting a server and also when starting a
// single player game via the "warp" menu.

static void StartGame(int multiplayer)
{
    execute_context_t *exec;

    exec = NewExecuteContext();

    // Extra parameters come first, before all others; this way,
    // they can override any of the options set in the dialog.

    AddExtraParameters(exec);

    AddIWADParameter(exec);
    AddCmdLineParameter(exec, "-skill %i", skill + 1);

    if (nomonsters)
    {
        AddCmdLineParameter(exec, "-nomonsters");
    }

    if (fast)
    {
        AddCmdLineParameter(exec, "-fast");
    }

    // CALICO_TODO: respawn not currently supported
#if DEADCODE
    if (respawn)
    {
        AddCmdLineParameter(exec, "-respawn");
    }
#endif

    if (warptype == WARP_ExMy)
    {
        // TODO: select IWAD based on warp type
        AddCmdLineParameter(exec, "-warp %i %i", warpepisode, warpmap);
    }
    else if (warptype == WARP_MAPxy)
    {
        AddCmdLineParameter(exec, "-warp %i", warpmap);
    }

    // Multiplayer-specific options:

    if (multiplayer)
    {
        AddCmdLineParameter(exec, "-server");
        AddCmdLineParameter(exec, "-port %i", udpport);

        if (deathmatch == 1)
        {
            AddCmdLineParameter(exec, "-deathmatch");
        }
        else if (deathmatch == 2 || strife_altdeath != 0)
        {
            AddCmdLineParameter(exec, "-altdeath");
        }

        if (timer > 0)
        {
            AddCmdLineParameter(exec, "-timer %i", timer);
        }

        if (privateserver)
        {
            AddCmdLineParameter(exec, "-privateserver");
        }
    }

    AddWADs(exec);

    Cfg_WriteFile();
    WriteEEProm();
    PassThroughArguments(exec);

    if(ExecuteDoom(exec) >= 0)
    {
        TXT_Shutdown();
        hal_medialayer.exit();
    }
    else
    {
        ShowLaunchError();
    }
}

static void StartServerGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    StartGame(1);
}

static void StartSinglePlayerGame(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    StartGame(0);
}

static void UpdateWarpButton(void)
{
    char buf[10];

    if (warptype == WARP_ExMy)
    {
        psnprintf(buf, sizeof(buf), "E%iM%i", warpepisode, warpmap);
    }
    else if (warptype == WARP_MAPxy)
    {
        psnprintf(buf, sizeof(buf), "MAP%02i", warpmap);
    }

    TXT_SetButtonLabel(warpbutton, buf);
}

static void UpdateSkillButton(void)
{
    skillbutton->values = doom_skills;
}

static void SetExMyWarp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int)(intptr_t) val;

    warpepisode = l / 10;
    warpmap = l % 10;

    UpdateWarpButton();
}

static void SetMAPxyWarp(TXT_UNCAST_ARG(widget), void *val)
{
    int l;

    l = (int)(intptr_t) val;

    warpmap = l;

    UpdateWarpButton();
}

static void CloseLevelSelectDialog(TXT_UNCAST_ARG(button), TXT_UNCAST_ARG(window))
{
    TXT_CAST_ARG(txt_window_t, window);

    TXT_CloseWindow(window);
}

#define NUM_JAGDOOM_LEVELS   25
#define MAPS_PER_COLUMN      10
#define LEVEL_SELECT_COLUMNS ((NUM_JAGDOOM_LEVELS + (MAPS_PER_COLUMN - 1)) / MAPS_PER_COLUMN)
#define NUM_LEVEL_SLOTS      (MAPS_PER_COLUMN * LEVEL_SELECT_COLUMNS)

static void LevelSelectDialog(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    txt_button_t *button;
    char buf[10];
    intptr_t x, y;
    intptr_t l;
    int i;

    window = TXT_NewWindow("Select level");

    {
        TXT_SetTableColumns(window, LEVEL_SELECT_COLUMNS);

        for (i=0; i<NUM_LEVEL_SLOTS; ++i)
        {
            x = i % LEVEL_SELECT_COLUMNS;
            y = i / LEVEL_SELECT_COLUMNS;

            l = x * MAPS_PER_COLUMN + y + 1;

            if(l > NUM_JAGDOOM_LEVELS)
            {
                TXT_AddWidget(window, NULL);
                continue;
            }

            psnprintf(buf, sizeof(buf), " MAP%02" PRIiPTR " ", l);
            button = TXT_NewButton(buf);
            TXT_SignalConnect(button, "pressed", SetMAPxyWarp, (void *) l);
            TXT_SignalConnect(button, "pressed", CloseLevelSelectDialog, window);
            TXT_AddWidget(window, button);

            if (warpmap == l)
            {
                TXT_SelectWidget(window, button);
            }
        }
    }
}

static void IWADSelected(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    const iwad_t *iwad;

    // Find the iwad_t selected

    iwad = GetCurrentIWAD();

    // Update iwadfile

    iwadfile = iwad->filename;
}

// Called when the IWAD button is changed, to update warptype.

static void UpdateWarpType(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(unused))
{
    const warptype_t new_warptype = WARP_MAPxy;

    // Reset to E1M1 / MAP01 when the warp type is changed.

    if (new_warptype != warptype)
    {
        warpepisode = 1;
        warpmap = 1;
    }

    warptype = new_warptype;

    UpdateWarpButton();
    UpdateSkillButton();
}

// Get an IWAD list with a default fallback IWAD that is appropriate
// for the game we are configuring (matches gamemission global variable).

static const iwad_t **GetFallbackIwadList(void)
{
    static const iwad_t *fallback_iwad_list[3];

    // Default to use if we don't find something better.

    fallback_iwad_list[0] = &fallback_iwads[0];
    fallback_iwad_list[1] = &fallback_iwads[1];
    fallback_iwad_list[2] = NULL;

    return fallback_iwad_list;
}

static const iwad_t **GetIwads(void)
{
    static const iwad_t *local_found_iwads[3];

    // CALICO_TODO: this sucks; eventually let the user choose an IWAD from the filesystem.
    size_t count = 0;
    for(size_t i = 0; i < earrlen(fallback_iwads); i++)
    {
        if(hal_platform.fileExists(fallback_iwads[i].filename))
            ++count;
    }

    if(!count)
    {
        local_found_iwads[0] = NULL;
        return local_found_iwads;
    }

    for(size_t i = 0, j = 0; i < earrlen(fallback_iwads); i++, j++)
    {
        if(j >= count)
            break;
        local_found_iwads[j] = &fallback_iwads[i];
    }
    local_found_iwads[count] = NULL;

    return local_found_iwads;
}

static txt_widget_t *IWADSelector(void)
{
    txt_dropdown_list_t *dropdown;
    txt_widget_t *result;
    unsigned int num_iwads;
    unsigned int i;

    // Find out what WADs are installed
    found_iwads = GetIwads();

    // Build a list of the descriptions for all installed IWADs

    num_iwads = 0;

    for (i=0; found_iwads[i] != NULL; ++i)
    {
         ++num_iwads;
    }

    // If no IWADs are found, provide Doom 2 as an option, but
    // we're probably screwed.
    if (num_iwads == 0)
    {
        found_iwads = GetFallbackIwadList();
        num_iwads   = earrlen(fallback_iwads);
    }

    iwad_labels = malloc(sizeof(*iwad_labels) * num_iwads);

    for (i=0; i < num_iwads; ++i)
    {
        iwad_labels[i] = found_iwads[i]->game;
    }

    // Build a dropdown list of IWADs

    if (num_iwads < 2)
    {
        // We have only one IWAD.  Show as a label.
        result = (txt_widget_t *) TXT_NewLabel(found_iwads[0]->game);
    }
    else
    {
        // Dropdown list allowing IWAD to be selected.

        dropdown = TXT_NewDropdownList(&found_iwad_selected, 
                                       iwad_labels, num_iwads);

        TXT_SignalConnect(dropdown, "changed", IWADSelected, NULL);

        result = (txt_widget_t *) dropdown;
    }

    // The first time the dialog is opened, found_iwad_selected=-1,
    // so select the first IWAD in the list. Don't lose the setting
    // if we close and reopen the dialog.

    if (found_iwad_selected < 0 || (unsigned int)found_iwad_selected >= num_iwads)
    {
        found_iwad_selected = 0;
    }

    IWADSelected(NULL, NULL);

    return result;
}

// Create the window action button to start the game.  This invokes
// a different callback depending on whether to start a multiplayer
// or single player game.

static txt_window_action_t *StartGameAction(int multiplayer)
{
    txt_window_action_t *action;
    TxtWidgetSignalFunc callback;

    action = TXT_NewWindowAction(KEY_F10, "Start");

    if (multiplayer)
    {
        callback = StartServerGame;
    }
    else
    {
        callback = StartSinglePlayerGame;
    }

    TXT_SignalConnect(action, "pressed", callback, NULL);

    return action;
}

static void OpenWadsWindow(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Add WADs");

    for (i=0; i<NUM_WADS; ++i)
    {
        TXT_AddWidget(
            window,
            TXT_NewFileSelector(&wads[i], 60, "Select a WAD file", wad_extensions)
        );
    }
}

static void OpenExtraParamsWindow(TXT_UNCAST_ARG(widget), 
                                  TXT_UNCAST_ARG(user_data))
{
    txt_window_t *window;
    int i;

    window = TXT_NewWindow("Extra command line parameters");
    
    for (i=0; i<NUM_EXTRA_PARAMS; ++i)
    {
        TXT_AddWidget(window, TXT_NewInputBox(&extra_params[i], 70));
    }
}

// CALICO_TODO: if -file becomes supported
#if DEADCODE
static txt_window_action_t *WadWindowAction(void)
{
    txt_window_action_t *action;

    action = TXT_NewWindowAction('w', "Add WADs");
    TXT_SignalConnect(action, "pressed", OpenWadsWindow, NULL);

    return action;
}
#endif

static txt_dropdown_list_t *GameTypeDropdown(void)
{
    return TXT_NewDropdownList(&deathmatch, gamemodes, earrlen(gamemodes));
}

// "Start game" menu.  This is used for the start server window
// and the single player warp menu.  The parameters specify
// the window title and whether to display multiplayer options.

static void StartGameMenu(const char *window_title, int multiplayer)
{
    txt_window_t *window;
    txt_widget_t *iwad_selector;

    window = TXT_NewWindow(window_title);
    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 12, 6);

    if (multiplayer)
    {
        TXT_SetWindowHelpURL(window, MULTI_START_HELP_URL);
    }
    else
    {
        TXT_SetWindowHelpURL(window, LEVEL_WARP_HELP_URL);
    }

    // CALICO_TODO: if -file becomes supported
    //TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, StartGameAction(multiplayer));

    TXT_AddWidgets(
        window,
        TXT_NewLabel("Game"),
        iwad_selector = IWADSelector(),
        NULL
    );

    TXT_AddWidgets(
        window,
        TXT_NewLabel("Skill"),
        skillbutton = TXT_NewDropdownList(&skill, doom_skills, 5),
        TXT_NewLabel("Level warp"),
        warpbutton = TXT_NewButton2("?", LevelSelectDialog, NULL),
        NULL
    );

#if DEADCODE
    if (multiplayer)
    {
        TXT_AddWidgets(
            window,
            TXT_NewLabel("Game type"),
            GameTypeDropdown(),
            TXT_NewLabel("Time limit"),
            TXT_NewHorizBox(TXT_NewIntInputBox(&timer, 2),
                            TXT_NewLabel("minutes"),
                            NULL),
            NULL
        );
    }
#endif

    TXT_AddWidgets(
        window,
        TXT_NewSeparator("Monster options"),
        TXT_NewInvertedCheckBox("Monsters enabled", &nomonsters),
        TXT_TABLE_OVERFLOW_RIGHT,
        TXT_NewCheckBox("Fast monsters", &fast),
        TXT_TABLE_OVERFLOW_RIGHT,
        // CALICO_TODO: not currently supported (Jag never had respawning monsters)
#if DEADCODE
        TXT_NewCheckBox("Respawning monsters", &respawn),
        TXT_TABLE_OVERFLOW_RIGHT,
#endif
        NULL
    );

#if DEADCODE
    if (multiplayer)
    {
        TXT_AddWidgets(
            window,
            TXT_NewSeparator("Advanced"),
            TXT_NewLabel("UDP port"),
            TXT_NewIntInputBox(&udpport, 5),
            TXT_NewInvertedCheckBox("Register with master server",
            &privateserver),
            TXT_TABLE_OVERFLOW_RIGHT,
            NULL
        );
    }
#endif

    TXT_AddWidgets(
        window,
        TXT_NewButton2("Add extra parameters...", OpenExtraParamsWindow, NULL),
        TXT_TABLE_OVERFLOW_RIGHT,
        NULL
    );

    TXT_SignalConnect(iwad_selector, "changed", UpdateWarpType, NULL);

    UpdateWarpType(NULL, NULL);
    UpdateWarpButton();
}

void StartMultiGame(TXT_UNCAST_ARG(widget), void *user_data)
{
    StartGameMenu("Start multiplayer game", 1);
}

void WarpMenu(TXT_UNCAST_ARG(widget), void *user_data)
{
    StartGameMenu("Level Warp", 0);
}

static void DoJoinGame(void *unused1, void *unused2)
{
    execute_context_t *exec;

    if (connect_address == NULL || strlen(connect_address) <= 0)
    {
        TXT_MessageBox(NULL, "Please enter a server address\n"
                             "to connect to.");
        return;
    }

    exec = NewExecuteContext();

    AddCmdLineParameter(exec, "-connect %s", connect_address);

#if DEADCODE
    if (gamemission == hexen)
    {
        AddCmdLineParameter(exec, "-class %i", character_class);
    }
#endif

    // Extra parameters come first, so that they can be used to override
    // the other parameters.

    AddExtraParameters(exec);
    AddIWADParameter(exec);
    AddWADs(exec);

    Cfg_WriteFile();
    WriteEEProm();

    PassThroughArguments(exec);

    if(ExecuteDoom(exec) >= 0)
    {
        TXT_Shutdown();
        hal_medialayer.exit();
    }
    else
    {
        ShowLaunchError();
    }
}

static txt_window_action_t *JoinGameAction(void)
{
#if DEADCODE
    txt_window_action_t *action;

    action = TXT_NewWindowAction(KEY_F10, "Connect");
    TXT_SignalConnect(action, "pressed", DoJoinGame, NULL);

    return action;
#else
    return NULL;
#endif
}

static void SelectQueryAddress(TXT_UNCAST_ARG(button),
                               TXT_UNCAST_ARG(querydata))
{
    TXT_CAST_ARG(txt_button_t, button);
#if DEADCODE
    TXT_CAST_ARG(net_querydata_t, querydata);
    int i;

    if (querydata->server_state != 0)
    {
        TXT_MessageBox("Cannot connect to server",
                       "Gameplay is already in progress\n"
                       "on this server.");
        return;
    }
#endif

    // Set address to connect to:

    free(connect_address);
    connect_address = M_StringDuplicate(button->label);

    // Auto-choose IWAD if there is already a player connected.

#if DEADCODE
    if (querydata->num_players > 0)
    {
        for (i = 0; found_iwads[i] != NULL; ++i)
        {
            if (found_iwads[i]->mode == querydata->gamemode
             && found_iwads[i]->mission == querydata->gamemission)
            {
                found_iwad_selected = i;
                iwadfile = found_iwads[i]->name;
                break;
            }
        }

        if (found_iwads[i] == NULL)
        {
            TXT_MessageBox(NULL,
                           "The game on this server seems to be:\n"
                           "\n"
                           "   %s\n"
                           "\n"
                           "but the IWAD file %s is not found!\n"
                           "Without the required IWAD file, it may not be\n"
                           "possible to join this game.",
                           D_SuggestGameName(querydata->gamemission,
                                             querydata->gamemode),
                           D_SuggestIWADName(querydata->gamemission,
                                             querydata->gamemode));
        }
    }
#endif
    // Finished with search.

    TXT_CloseWindow(query_window);
}

#if DEADCODE
static void QueryResponseCallback(net_addr_t *addr,
                                  net_querydata_t *querydata,
                                  unsigned int ping_time,
                                  TXT_UNCAST_ARG(results_table))
{
    TXT_CAST_ARG(txt_table_t, results_table);
    char ping_time_str[16];
    char description[47];

    // When we connect we'll have to negotiate a common protocol that we
    // can agree upon between the client and server. If we can't then we
    // won't be able to connect, so it's pointless to include it in the
    // results list. If protocol==NET_PROTOCOL_UNKNOWN then this may be
    // an old, pre-3.0 Chocolate Doom server that doesn't support the new
    // protocol negotiation mechanism, or it may be an incompatible fork.
    if (querydata->protocol == NET_PROTOCOL_UNKNOWN)
    {
        return;
    }

    M_snprintf(ping_time_str, sizeof(ping_time_str), "%ims", ping_time);

    // Build description from server name field. Because there is limited
    // space, we only include the player count if there are already players
    // connected to the server.
    if (querydata->num_players > 0)
    {
        M_snprintf(description, sizeof(description), "(%d/%d) ",
                   querydata->num_players, querydata->max_players);
    }
    else
    {
        M_StringCopy(description, "", sizeof(description));
    }

    M_StringConcat(description, querydata->description, sizeof(description));

    TXT_AddWidgets(results_table,
                   TXT_NewLabel(ping_time_str),
                   TXT_NewButton2(NET_AddrToString(addr),
                                  SelectQueryAddress, querydata),
                   TXT_NewLabel(description),
                   NULL);

    ++query_servers_found;
}
#endif

static void QueryPeriodicCallback(TXT_UNCAST_ARG(results_table))
{
    TXT_CAST_ARG(txt_table_t, results_table);

#if DEADCODE
    if (!NET_Query_Poll(QueryResponseCallback, results_table))
    {
        TXT_SetPeriodicCallback(NULL, NULL, 0);

        if (query_servers_found == 0)
        {
            TXT_AddWidgets(results_table,
                TXT_TABLE_EMPTY,
                TXT_NewLabel("No compatible servers found."),
                NULL
            );
        }
    }
#endif
}

static void QueryWindowClosed(TXT_UNCAST_ARG(window), void *unused)
{
    TXT_SetPeriodicCallback(NULL, NULL, 0);
}

static void ServerQueryWindow(const char *title)
{
    txt_table_t *results_table;

    query_servers_found = 0;

    query_window = TXT_NewWindow(title);

    TXT_AddWidget(query_window,
                  TXT_NewScrollPane(70, 10,
                                    results_table = TXT_NewTable(3)));

    TXT_SetColumnWidths(results_table, 7, 22, 40);
    TXT_SetPeriodicCallback(QueryPeriodicCallback, results_table, 1);

    TXT_SignalConnect(query_window, "closed", QueryWindowClosed, NULL);
}

static void FindInternetServer(TXT_UNCAST_ARG(widget),
                               TXT_UNCAST_ARG(user_data))
{
#if DEADCODE
    NET_StartMasterQuery();
    ServerQueryWindow("Find Internet server");
#endif
}

static void FindLANServer(TXT_UNCAST_ARG(widget),
                          TXT_UNCAST_ARG(user_data))
{
#if DEADCODE
    NET_StartLANQuery();
    ServerQueryWindow("Find LAN server");
#endif
}

void JoinMultiGame(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_inputbox_t *address_box;

    window = TXT_NewWindow("Join multiplayer game");
    TXT_SetTableColumns(window, 2);
    TXT_SetColumnWidths(window, 12, 12);

    TXT_SetWindowHelpURL(window, MULTI_JOIN_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewLabel("Game"),
                   IWADSelector(),
                   NULL);

#if DEADCODE
    if (gamemission == hexen)
    {
        TXT_AddWidgets(window,
                       TXT_NewLabel("Character class "),
                       TXT_NewDropdownList(&character_class,
                                           character_classes, 3),
                       NULL);
    }
#endif

    TXT_AddWidgets(window,
                   TXT_NewSeparator("Server"),
                   TXT_NewLabel("Connect to address: "),
                   address_box = TXT_NewInputBox(&connect_address, 30),

                   TXT_NewButton2("Find server on Internet...",
                                  FindInternetServer, NULL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewButton2("Find server on local network...",
                                  FindLANServer, NULL),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewStrut(0, 1),
                   TXT_TABLE_OVERFLOW_RIGHT,
                   TXT_NewButton2("Add extra parameters...",
                                  OpenExtraParamsWindow, NULL),
                   NULL);

    TXT_SelectWidget(window, address_box);

    // CALICO_TODO: if -file becomes supported
    //TXT_SetWindowAction(window, TXT_HORIZ_CENTER, WadWindowAction());
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, JoinGameAction());
}

void MultiplayerConfig(TXT_UNCAST_ARG(widget), void *user_data)
{
    txt_window_t *window;
    txt_label_t *label;
    txt_table_t *table;
    char buf[10];
    int i;

    window = TXT_NewWindow("Multiplayer Configuration");
    TXT_SetWindowHelpURL(window, MULTI_CONFIG_HELP_URL);

    TXT_AddWidgets(window,
                   TXT_NewStrut(0, 1),
                   TXT_NewHorizBox(TXT_NewLabel("Player name:  "),
                                   TXT_NewInputBox(&net_player_name, 25),
                                   NULL),
                   TXT_NewStrut(0, 1),
                   TXT_NewSeparator("Chat macros"),
                   NULL);

    table = TXT_NewTable(2);

    for (i=0; i<10; ++i)
    {
        psnprintf(buf, sizeof(buf), "#%i ", i + 1);

        label = TXT_NewLabel(buf);
        TXT_SetFGColor(label, TXT_COLOR_BRIGHT_CYAN);

        TXT_AddWidgets(table,
                       label,
                       TXT_NewInputBox(&chat_macros[(i + 1) % 10], 40),
                       NULL);
    }

    TXT_AddWidget(window, table);
}
