#ifndef _CCS_GNOME_INTEGRATION_CONSTANTS_H
#define _CCS_GNOME_INTEGRATION_CONSTANTS_H

#include <ccs-defs.h>
#include <glib.h>

COMPIZCONFIG_BEGIN_DECLS

#include <ccs_gnome_integration_types.h>

#define METACITY "/apps/metacity"
#define NUM_WATCHED_DIRS 3

typedef struct _CCSSettingIntegratedSettingPair CCSSettingIntegratedSettingPair;
struct _CCSSettingIntegratedSettingPair
{
    const char *compizName;
    const char *gnomeName;
};

typedef struct _CCSGNOMEIntegratedSettingNames CCSGNOMEIntegratedSettingNames;
struct _CCSGNOMEIntegratedSettingNames
{
    CCSSettingIntegratedSettingPair CORE_AUDIBLE_BELL;
    CCSSettingIntegratedSettingPair CORE_CLICK_TO_FOCUS;
    CCSSettingIntegratedSettingPair CORE_RAISE_ON_CLICK;
    CCSSettingIntegratedSettingPair CORE_AUTORAISE_DELAY;
    CCSSettingIntegratedSettingPair CORE_AUTORAISE;
    CCSSettingIntegratedSettingPair THUMBNAIL_CURRENT_VIEWPORT;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_COMMAND_TERMINAL;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_COMMAND_SCREENSHOT;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_RIGHT_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_LEFT_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_12_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_11_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_10_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_9_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_8_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_7_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_6_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_5_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_4_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_3_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_2_WINDOW_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_1_WINDOW_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_BOTTOM_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_TOP_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_RIGHT_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_LEFT_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_BOTTOMRIGHT_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_BOTTOMLEFT_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_TOPRIGHT_KEY;
    CCSSettingIntegratedSettingPair PUT_PUT_TOPLEFT_KEY;
    CCSSettingIntegratedSettingPair WALL_DOWN_WINDOW_KEY;
    CCSSettingIntegratedSettingPair WALL_UP_WINDOW_KEY;
    CCSSettingIntegratedSettingPair WALL_RIGHT_WINDOW_KEY;
    CCSSettingIntegratedSettingPair WALL_LEFT_WINDOW_KEY;
    CCSSettingIntegratedSettingPair WALL_RIGHT_KEY;
    CCSSettingIntegratedSettingPair WALL_LEFT_KEY;
    CCSSettingIntegratedSettingPair WALL_DOWN_KEY;
    CCSSettingIntegratedSettingPair WALL_UP_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_12_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_11_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_10_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_9_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_8_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_7_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_6_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_5_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_4_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_3_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_2_KEY;
    CCSSettingIntegratedSettingPair VPSWITCH_SWITCH_TO_1_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_RIGHT_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_LEFT_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_12_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_11_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_10_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_9_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_8_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_7_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_6_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_5_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_4_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_3_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_2_KEY;
    CCSSettingIntegratedSettingPair ROTATE_ROTATE_TO_1_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND11_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND10_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND9_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND8_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND7_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND6_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND5_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND4_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND3_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND2_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND1_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_RUN_COMMAND0_KEY;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND11;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND10;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND9;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND8;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND7;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND6;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND5;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND4;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND3;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND2;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND1;
    CCSSettingIntegratedSettingPair COMMANDS_COMMAND0;
    CCSSettingIntegratedSettingPair EXTRAWM_TOGGLE_FULLSCREEN_KEY;
    CCSSettingIntegratedSettingPair EXTRAWM_TOGGLE_STICKY_KEY;
    CCSSettingIntegratedSettingPair STATICSWITCHER_PREV_KEY;
    CCSSettingIntegratedSettingPair STATICSWITCHER_NEXT_KEY;
    CCSSettingIntegratedSettingPair FADE_FULLSCREEN_VISUAL_BELL;
    CCSSettingIntegratedSettingPair FADE_VISUAL_BELL;
    CCSSettingIntegratedSettingPair NULL_RESIZE_WITH_RIGHT_BUTTON;
    CCSSettingIntegratedSettingPair NULL_MOUSE_BUTTON_MODIFIER;
    CCSSettingIntegratedSettingPair CORE_WINDOW_MENU_BUTTON;
    CCSSettingIntegratedSettingPair RESIZE_INITIATE_BUTTON;
    CCSSettingIntegratedSettingPair MOVE_INITIATE_BUTTON;
    CCSSettingIntegratedSettingPair CORE_WINDOW_MENU_KEY;
    CCSSettingIntegratedSettingPair RESIZE_INITIATE_KEY;
    CCSSettingIntegratedSettingPair MOVE_INITIATE_KEY;
    CCSSettingIntegratedSettingPair CORE_SHOW_DESKTOP_KEY;
    CCSSettingIntegratedSettingPair CORE_TOGGLE_WINDOW_SHADED_KEY;
    CCSSettingIntegratedSettingPair CORE_CLOSE_WINDOW_KEY;
    CCSSettingIntegratedSettingPair CORE_LOWER_WINDOW_KEY;
    CCSSettingIntegratedSettingPair CORE_RAISE_WINDOW_KEY;
    CCSSettingIntegratedSettingPair CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY;
    CCSSettingIntegratedSettingPair CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY;
    CCSSettingIntegratedSettingPair CORE_UNMAXIMIZE_WINDOW_KEY;
    CCSSettingIntegratedSettingPair CORE_MAXIMIZE_WINDOW_KEY;
    CCSSettingIntegratedSettingPair CORE_MINIMIZE_WINDOW_KEY;
    CCSSettingIntegratedSettingPair CORE_TOGGLE_WINDOW_MAXIMIZED_KEY;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_MAIN_MENU_KEY;
    CCSSettingIntegratedSettingPair GNOMECOMPAT_RUN_KEY;
    CCSSettingIntegratedSettingPair UNITYSHELL_SHOW_HUD;
};

extern const CCSGNOMEIntegratedSettingNames ccsGNOMEIntegratedSettingNames;

typedef struct _CCSGConfIntegrationCategoires CCSGConfIntegrationCategories;

struct _CCSGConfIntegrationCategoires
{
    const char *GENERAL;
    const char *APPS;
    const char *DESKTOP;
    const char *KEYBINDING_COMMANDS;
    const char *WINDOW_KEYBINDINGS;
    const char *GLOBAL_KEYBINDINGS;
};

extern const CCSGConfIntegrationCategories ccsGConfIntegrationCategories;

typedef struct _CCSGNOMEIntegratedPluginNames CCSGNOMEIntegratedPluginNames;

struct _CCSGNOMEIntegratedPluginNames
{
    const char *CORE;
    const char *THUMBNAIL;
    const char *GNOMECOMPAT;
    const char *ROTATE;
    const char *PUT;
    const char *WALL;
    const char *VPSWITCH;
    const char *COMMANDS;
    const char *EXTRAWM;
    const char *RESIZE;
    const char *MOVE;
    const char *STATICSWITCHER;
    const char *FADE;
    const char *UNITYSHELL;
    const char *SPECIAL;
};

extern const CCSGNOMEIntegratedPluginNames ccsGNOMEIntegratedPluginNames;

extern const char* watchedGConfGnomeDirectories[];

typedef struct _CCSGSettingsWrapperIntegratedSchemasQuarks
{
    GQuark ORG_GNOME_DESKTOP_WM_PREFERENCES;
    GQuark ORG_GNOME_DESKTOP_WM_KEYBINDINGS;
    GQuark ORG_GNOME_SETTINGS_DAEMON_PLUGINS_MEDIA_KEYS;
    GQuark ORG_GNOME_DESKTOP_DEFAULT_APPLICATIONS_TERMINAL;
    GQuark ORG_COMPIZ_INTEGRATED;
} CCSGSettingsWrapperIntegratedSchemasQuarks;

const CCSGSettingsWrapperIntegratedSchemasQuarks *ccsGNOMEGSettingsWrapperQuarks ();

GHashTable * ccsGNOMEIntegrationPopulateCategoriesHashTables ();
GHashTable * ccsGNOMEIntegrationPopulateSpecialTypesHashTables ();
GHashTable * ccsGNOMEIntegrationPopulateSettingNameToGNOMENameHashTables ();
GHashTable * ccsGNOMEGSettingsIntegrationPopulateSettingNameToIntegratedSchemasQuarksHashTable ();

/* We only have to use the #define here because
 * C doesn't have a concept of "constants" setting
 * the array size ...
 */
#define CCS_GNOME_INTEGRATED_SETTINGS_LIST_SIZE 120

typedef struct _CCSGNOMEIntegratedSettingsList
{
    const char *pluginName;
    const char *settingName;
} CCSGNOMEIntegratedSettingsList;

const CCSGNOMEIntegratedSettingsList * ccsGNOMEIntegratedSettingsList ();

COMPIZCONFIG_END_DECLS

#endif
