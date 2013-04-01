#include <glib.h>
#include "ccs_gnome_integration_constants.h"

const char * watchedGConfGnomeDirectories[] = {
    METACITY,
    "/desktop/gnome/applications/terminal",
    "/apps/panel/applets/window_list/prefs"
};

const CCSGNOMEIntegratedSettingNames ccsGNOMEIntegratedSettingNames =
{
    { "audible_bell", "audible_bell" },
    { "click_to_focus", "focus_mode" },
    { "raise_on_click", "raise_on_click" },
    { "autoraise_delay", "auto_raise_delay" },
    { "autoraise", "auto_raise" },
    { "current_viewport", "display_all_workspaces" },
    { "command_terminal", "exec" },
    { "command_window_screenshot", "command_window_screenshot" },
    { "command_screenshot", "command_screenshot" },
    { "rotate_right_window_key", "move_to_workspace_right" },
    { "rotate_left_window_key", "move_to_workspace_left" },
    { "rotate_to_12_window_key", "move_to_workspace_12" },
    { "rotate_to_11_window_key", "move_to_workspace_11" },
    { "rotate_to_10_window_key", "move_to_workspace_10" },
    { "rotate_to_9_window_key", "move_to_workspace_9" },
    { "rotate_to_8_window_key", "move_to_workspace_8" },
    { "rotate_to_7_window_key", "move_to_workspace_7" },
    { "rotate_to_6_window_key", "move_to_workspace_6" },
    { "rotate_to_5_window_key", "move_to_workspace_5" },
    { "rotate_to_4_window_key", "move_to_workspace_4" },
    { "rotate_to_3_window_key", "move_to_workspace_3" },
    { "rotate_to_2_window_key", "move_to_workspace_2" },
    { "rotate_to_1_window_key", "move_to_workspace_1" },
    { "put_bottom_key", "move_to_side_s" },
    { "put_top_key", "move_to_side_n" },
    { "put_right_key", "move_to_side_e" },
    { "put_left_key", "move_to_side_w" },
    { "put_bottomright_key", "move_to_corner_se" },
    { "put_bottomleft_key", "move_to_corner_sw" },
    { "put_topright_key", "move_to_corner_ne" },
    { "put_topleft_key", "move_to_corner_nw" },
    { "down_window_key", "move_to_workspace_down" },
    { "up_window_key", "move_to_workspace_up" },
    { "right_window_key", "move_to_workspace_right" },
    { "left_window_key", "move_to_workspace_left" },
    { "right_key", "switch_to_workspace_right" },
    { "left_key", "switch_to_workspace_left" },
    { "down_key", "switch_to_workspace_down" },
    { "up_key", "switch_to_workspace_up" },
    { "switch_to_12_key", "switch_to_workspace_12" },
    { "switch_to_11_key", "switch_to_workspace_11" },
    { "switch_to_10_key", "switch_to_workspace_10" },
    { "switch_to_9_key", "switch_to_workspace_9" },
    { "switch_to_8_key", "switch_to_workspace_8" },
    { "switch_to_7_key", "switch_to_workspace_7" },
    { "switch_to_6_key", "switch_to_workspace_6" },
    { "switch_to_5_key", "switch_to_workspace_5" },
    { "switch_to_4_key", "switch_to_workspace_4" },
    { "switch_to_3_key", "switch_to_workspace_3" },
    { "switch_to_2_key", "switch_to_workspace_2" },
    { "switch_to_1_key", "switch_to_workspace_1" },
    { "rotate_right_key", "switch_to_workspace_right" },
    { "rotate_left_key", "switch_to_workspace_left" },
    { "rotate_to_12_key", "switch_to_workspace_12" },
    { "rotate_to_11_key", "switch_to_workspace_11" },
    { "rotate_to_10_key", "switch_to_workspace_10" },
    { "rotate_to_9_key", "switch_to_workspace_9" },
    { "rotate_to_8_key", "switch_to_workspace_8" },
    { "rotate_to_7_key", "switch_to_workspace_7" },
    { "rotate_to_6_key", "switch_to_workspace_6" },
    { "rotate_to_5_key", "switch_to_workspace_5" },
    { "rotate_to_4_key", "switch_to_workspace_4" },
    { "rotate_to_3_key", "switch_to_workspace_3" },
    { "rotate_to_2_key", "switch_to_workspace_2" },
    { "rotate_to_1_key", "switch_to_workspace_1" },
    { "run_command11_key", "run_command_12" },
    { "run_command10_key", "run_command_11" },
    { "run_command9_key", "run_command_10" },
    { "run_command8_key", "run_command_9" },
    { "run_command7_key", "run_command_8" },
    { "run_command6_key", "run_command_7" },
    { "run_command5_key", "run_command_6" },
    { "run_command4_key", "run_command_5" },
    { "run_command3_key", "run_command_4" },
    { "run_command2_key", "run_command_3" },
    { "run_command1_key", "run_command_2" },
    { "run_command0_key", "run_command_1" },
    { "command11", "command_12" },
    { "command10", "command_11" },
    { "command9", "command_10" },
    { "command8", "command_9" },
    { "command7", "command_8" },
    { "command6", "command_7" },
    { "command5", "command_6" },
    { "command4", "command_5" },
    { "command3", "command_4" },
    { "command2", "command_3" },
    { "command1", "command_2" },
    { "command0", "command_1" },
    { "toggle_fullscreen_key", "toggle_fullscreen" },
    { "toggle_sticky_key", "toggle_on_all_workspaces" },
    { "prev_key", "switch_windows_backward" },
    { "next_key", "switch_windows" },
    { "fullscreen_visual_bell", "visual_bell_type" },
    { "visual_bell", "visual_bell" },
    { "resize_with_right_button", "resize_with_right_button" },
    { "mouse_button_modifier", "mouse_button_modifier" },
    { "window_menu_button", "activate_window_menu" },
    { "initiate_button", "begin_resize" },
    { "initiate_button", "begin_move" },
    { "window_menu_key", "activate_window_menu" },
    { "initiate_key", "begin_resize" },
    { "initiate_key", "begin_move" },
    { "show_desktop_key", "show_desktop" },
    { "toggle_window_shaded_key", "toggle_shaded" },
    { "close_window_key", "close" },
    { "lower_window_key", "lower" },
    { "raise_window_key", "raise" },
    { "maximize_window_vertically_key", "maximize_vertically" },
    { "maximize_window_horizontally_key", "maximize_horizontally" },
    { "unmaximize_window_key", "unmaximize" },
    { "maximize_window_key", "maximize" },
    { "minimize_window_key", "minimize" },
    { "toggle_window_maximized_key", "toggle_maximized" },
    { "run_command_terminal_key", "run_command_terminal" },
    { "run_command_window_screenshot_key", "run_command_window_screenshot" },
    { "run_command_screenshot_key", "run_command_screenshot" },
    { "main_menu_key", "panel_main_menu" },
    { "run_key", "panel_run_dialog" },
    { "show_hud", "show_hud" }
};

const CCSGConfIntegrationCategories ccsGConfIntegrationCategories =
{
    METACITY "/general/",
    "/apps/panel/applets/window_list/prefs/",
    "/desktop/gnome/applications/terminal/",
    METACITY "/keybinding_commands/",
    METACITY "/window_keybindings/",
    METACITY "/global_keybindings/"
};

const CCSGNOMEIntegratedPluginNames ccsGNOMEIntegratedPluginNames =
{
    "core",
    "thumbnail",
    "gnomecompat",
    "rotate",
    "put",
    "wall",
    "vpswitch",
    "commands",
    "extrawm",
    "resize",
    "move",
    "staticswitcher",
    "fade",
    "unityshell",
    "__special"
};

static gpointer
ccsGNOMEIntegrationInitializeIntegratedSettingsList (gpointer data)
{
    CCSGNOMEIntegratedSettingsList *array = (CCSGNOMEIntegratedSettingsList *) data;
    const CCSGNOMEIntegratedPluginNames  *plugins = &ccsGNOMEIntegratedPluginNames;
    const CCSGNOMEIntegratedSettingNames *settings = &ccsGNOMEIntegratedSettingNames;

    array[0].pluginName = plugins->CORE;
    array[0].settingName = settings->CORE_AUDIBLE_BELL.compizName;
    array[1].pluginName = plugins->CORE;
    array[1].settingName = settings->CORE_CLICK_TO_FOCUS.compizName;
    array[2].pluginName = plugins->CORE;
    array[2].settingName = settings->CORE_RAISE_ON_CLICK.compizName;
    array[3].pluginName = plugins->CORE;
    array[3].settingName = settings->CORE_AUTORAISE_DELAY.compizName;
    array[4].pluginName = plugins->CORE;
    array[4].settingName = settings->CORE_AUTORAISE.compizName;
    array[5].pluginName = plugins->THUMBNAIL;
    array[5].settingName = settings->THUMBNAIL_CURRENT_VIEWPORT.compizName;
    array[6].pluginName = plugins->GNOMECOMPAT;
    array[6].settingName = settings->GNOMECOMPAT_COMMAND_TERMINAL.compizName;
    array[7].pluginName = plugins->GNOMECOMPAT;
    array[7].settingName = settings->GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT.compizName;
    array[8].pluginName = plugins->GNOMECOMPAT;
    array[8].settingName = settings->GNOMECOMPAT_COMMAND_SCREENSHOT.compizName;
    array[9].pluginName = plugins->ROTATE;
    array[9].settingName = settings->ROTATE_ROTATE_RIGHT_WINDOW_KEY.compizName;
    array[10].pluginName = plugins->ROTATE;
    array[10].settingName = settings->ROTATE_ROTATE_LEFT_WINDOW_KEY.compizName;
    array[11].pluginName = plugins->ROTATE;
    array[11].settingName = settings->ROTATE_ROTATE_TO_12_WINDOW_KEY.compizName;
    array[12].pluginName = plugins->ROTATE;
    array[12].settingName = settings->ROTATE_ROTATE_TO_11_WINDOW_KEY.compizName;
    array[13].pluginName = plugins->ROTATE;
    array[13].settingName = settings->ROTATE_ROTATE_TO_10_WINDOW_KEY.compizName;
    array[14].pluginName = plugins->ROTATE;
    array[14].settingName = settings->ROTATE_ROTATE_TO_9_WINDOW_KEY.compizName;
    array[15].pluginName = plugins->ROTATE;
    array[15].settingName = settings->ROTATE_ROTATE_TO_8_WINDOW_KEY.compizName;
    array[16].pluginName = plugins->ROTATE;
    array[16].settingName = settings->ROTATE_ROTATE_TO_7_WINDOW_KEY.compizName;
    array[17].pluginName = plugins->ROTATE;
    array[17].settingName = settings->ROTATE_ROTATE_TO_6_WINDOW_KEY.compizName;
    array[18].pluginName = plugins->ROTATE;
    array[18].settingName = settings->ROTATE_ROTATE_TO_5_WINDOW_KEY.compizName;
    array[19].pluginName = plugins->ROTATE;
    array[19].settingName = settings->ROTATE_ROTATE_TO_4_WINDOW_KEY.compizName;
    array[20].pluginName = plugins->ROTATE;
    array[20].settingName = settings->ROTATE_ROTATE_TO_3_WINDOW_KEY.compizName;
    array[21].pluginName = plugins->ROTATE;
    array[21].settingName = settings->ROTATE_ROTATE_TO_2_WINDOW_KEY.compizName;
    array[22].pluginName = plugins->ROTATE;
    array[22].settingName = settings->ROTATE_ROTATE_TO_1_WINDOW_KEY.compizName;
    array[23].pluginName = plugins->PUT;
    array[23].settingName = settings->PUT_PUT_BOTTOM_KEY.compizName;
    array[24].pluginName = plugins->PUT;
    array[24].settingName = settings->PUT_PUT_TOP_KEY.compizName;
    array[25].pluginName = plugins->PUT;
    array[25].settingName = settings->PUT_PUT_RIGHT_KEY.compizName;
    array[26].pluginName = plugins->PUT;
    array[26].settingName = settings->PUT_PUT_LEFT_KEY.compizName;
    array[27].pluginName = plugins->PUT;
    array[27].settingName = settings->PUT_PUT_BOTTOMRIGHT_KEY.compizName;
    array[28].pluginName = plugins->PUT;
    array[28].settingName = settings->PUT_PUT_BOTTOMLEFT_KEY.compizName;
    array[29].pluginName = plugins->PUT;
    array[29].settingName = settings->PUT_PUT_TOPRIGHT_KEY.compizName;
    array[30].pluginName = plugins->PUT;
    array[30].settingName = settings->PUT_PUT_TOPLEFT_KEY.compizName;
    array[31].pluginName = plugins->WALL;
    array[31].settingName = settings->WALL_DOWN_WINDOW_KEY.compizName;
    array[32].pluginName = plugins->WALL;
    array[32].settingName = settings->WALL_UP_WINDOW_KEY.compizName;
    array[33].pluginName = plugins->WALL;
    array[33].settingName = settings->WALL_RIGHT_WINDOW_KEY.compizName;
    array[34].pluginName = plugins->WALL;
    array[34].settingName = settings->WALL_LEFT_WINDOW_KEY.compizName;
    array[35].pluginName = plugins->WALL;
    array[35].settingName = settings->WALL_RIGHT_KEY.compizName;
    array[36].pluginName = plugins->WALL;
    array[36].settingName = settings->WALL_LEFT_KEY.compizName;
    array[37].pluginName = plugins->WALL;
    array[37].settingName = settings->WALL_DOWN_KEY.compizName;
    array[38].pluginName = plugins->WALL;
    array[38].settingName = settings->WALL_UP_KEY.compizName;
    array[39].pluginName = plugins->VPSWITCH;
    array[39].settingName = settings->VPSWITCH_SWITCH_TO_12_KEY.compizName;
    array[40].pluginName = plugins->VPSWITCH;
    array[40].settingName = settings->VPSWITCH_SWITCH_TO_11_KEY.compizName;
    array[41].pluginName = plugins->VPSWITCH;
    array[41].settingName = settings->VPSWITCH_SWITCH_TO_10_KEY.compizName;
    array[42].pluginName = plugins->VPSWITCH;
    array[42].settingName = settings->VPSWITCH_SWITCH_TO_9_KEY.compizName;
    array[43].pluginName = plugins->VPSWITCH;
    array[43].settingName = settings->VPSWITCH_SWITCH_TO_8_KEY.compizName;
    array[44].pluginName = plugins->VPSWITCH;
    array[44].settingName = settings->VPSWITCH_SWITCH_TO_7_KEY.compizName;
    array[45].pluginName = plugins->VPSWITCH;
    array[45].settingName = settings->VPSWITCH_SWITCH_TO_6_KEY.compizName;
    array[46].pluginName = plugins->VPSWITCH;
    array[46].settingName = settings->VPSWITCH_SWITCH_TO_5_KEY.compizName;
    array[47].pluginName = plugins->VPSWITCH;
    array[47].settingName = settings->VPSWITCH_SWITCH_TO_4_KEY.compizName;
    array[48].pluginName = plugins->VPSWITCH;
    array[48].settingName = settings->VPSWITCH_SWITCH_TO_3_KEY.compizName;
    array[49].pluginName = plugins->VPSWITCH;
    array[49].settingName = settings->VPSWITCH_SWITCH_TO_2_KEY.compizName;
    array[50].pluginName = plugins->VPSWITCH;
    array[50].settingName = settings->VPSWITCH_SWITCH_TO_1_KEY.compizName;
    array[51].pluginName = plugins->ROTATE;
    array[51].settingName = settings->ROTATE_ROTATE_RIGHT_KEY.compizName;
    array[52].pluginName = plugins->ROTATE;
    array[52].settingName = settings->ROTATE_ROTATE_LEFT_KEY.compizName;
    array[53].pluginName = plugins->ROTATE;
    array[53].settingName = settings->ROTATE_ROTATE_TO_12_KEY.compizName;
    array[54].pluginName = plugins->ROTATE;
    array[54].settingName = settings->ROTATE_ROTATE_TO_11_KEY.compizName;
    array[55].pluginName = plugins->ROTATE;
    array[55].settingName = settings->ROTATE_ROTATE_TO_10_KEY.compizName;
    array[56].pluginName = plugins->ROTATE;
    array[56].settingName = settings->ROTATE_ROTATE_TO_9_KEY.compizName;
    array[57].pluginName = plugins->ROTATE;
    array[57].settingName = settings->ROTATE_ROTATE_TO_8_KEY.compizName;
    array[58].pluginName = plugins->ROTATE;
    array[58].settingName = settings->ROTATE_ROTATE_TO_7_KEY.compizName;
    array[59].pluginName = plugins->ROTATE;
    array[59].settingName = settings->ROTATE_ROTATE_TO_6_KEY.compizName;
    array[60].pluginName = plugins->ROTATE;
    array[60].settingName = settings->ROTATE_ROTATE_TO_5_KEY.compizName;
    array[61].pluginName = plugins->ROTATE;
    array[61].settingName = settings->ROTATE_ROTATE_TO_4_KEY.compizName;
    array[62].pluginName = plugins->ROTATE;
    array[62].settingName = settings->ROTATE_ROTATE_TO_3_KEY.compizName;
    array[63].pluginName = plugins->ROTATE;
    array[63].settingName = settings->ROTATE_ROTATE_TO_2_KEY.compizName;
    array[64].pluginName = plugins->ROTATE;
    array[64].settingName = settings->ROTATE_ROTATE_TO_1_KEY.compizName;
    array[65].pluginName = plugins->COMMANDS;
    array[65].settingName = settings->COMMANDS_RUN_COMMAND11_KEY.compizName;
    array[66].pluginName = plugins->COMMANDS;
    array[66].settingName = settings->COMMANDS_RUN_COMMAND10_KEY.compizName;
    array[67].pluginName = plugins->COMMANDS;
    array[67].settingName = settings->COMMANDS_RUN_COMMAND9_KEY.compizName;
    array[68].pluginName = plugins->COMMANDS;
    array[68].settingName = settings->COMMANDS_RUN_COMMAND8_KEY.compizName;
    array[69].pluginName = plugins->COMMANDS;
    array[69].settingName = settings->COMMANDS_RUN_COMMAND7_KEY.compizName;
    array[70].pluginName = plugins->COMMANDS;
    array[70].settingName = settings->COMMANDS_RUN_COMMAND6_KEY.compizName;
    array[71].pluginName = plugins->COMMANDS;
    array[71].settingName = settings->COMMANDS_RUN_COMMAND5_KEY.compizName;
    array[72].pluginName = plugins->COMMANDS;
    array[72].settingName = settings->COMMANDS_RUN_COMMAND4_KEY.compizName;
    array[73].pluginName = plugins->COMMANDS;
    array[73].settingName = settings->COMMANDS_RUN_COMMAND3_KEY.compizName;
    array[74].pluginName = plugins->COMMANDS;
    array[74].settingName = settings->COMMANDS_RUN_COMMAND2_KEY.compizName;
    array[75].pluginName = plugins->COMMANDS;
    array[75].settingName = settings->COMMANDS_RUN_COMMAND1_KEY.compizName;
    array[76].pluginName = plugins->COMMANDS;
    array[76].settingName = settings->COMMANDS_RUN_COMMAND0_KEY.compizName;
    array[77].pluginName = plugins->COMMANDS;
    array[77].settingName = settings->COMMANDS_COMMAND11.compizName;
    array[78].pluginName = plugins->COMMANDS;
    array[78].settingName = settings->COMMANDS_COMMAND10.compizName;
    array[79].pluginName = plugins->COMMANDS;
    array[79].settingName = settings->COMMANDS_COMMAND9.compizName;
    array[80].pluginName = plugins->COMMANDS;
    array[80].settingName = settings->COMMANDS_COMMAND8.compizName;
    array[81].pluginName = plugins->COMMANDS;
    array[81].settingName = settings->COMMANDS_COMMAND7.compizName;
    array[82].pluginName = plugins->COMMANDS;
    array[82].settingName = settings->COMMANDS_COMMAND6.compizName;
    array[83].pluginName = plugins->COMMANDS;
    array[83].settingName = settings->COMMANDS_COMMAND5.compizName;
    array[84].pluginName = plugins->COMMANDS;
    array[84].settingName = settings->COMMANDS_COMMAND4.compizName;
    array[85].pluginName = plugins->COMMANDS;
    array[85].settingName = settings->COMMANDS_COMMAND3.compizName;
    array[86].pluginName = plugins->COMMANDS;
    array[86].settingName = settings->COMMANDS_COMMAND2.compizName;
    array[87].pluginName = plugins->COMMANDS;
    array[87].settingName = settings->COMMANDS_COMMAND1.compizName;
    array[88].pluginName = plugins->COMMANDS;
    array[88].settingName = settings->COMMANDS_COMMAND0.compizName;
    array[89].pluginName = plugins->EXTRAWM;
    array[89].settingName = settings->EXTRAWM_TOGGLE_FULLSCREEN_KEY.compizName;
    array[90].pluginName = plugins->EXTRAWM;
    array[90].settingName = settings->EXTRAWM_TOGGLE_STICKY_KEY.compizName;
    array[91].pluginName = plugins->STATICSWITCHER;
    array[91].settingName = settings->STATICSWITCHER_PREV_KEY.compizName;
    array[92].pluginName = plugins->STATICSWITCHER;
    array[92].settingName = settings->STATICSWITCHER_NEXT_KEY.compizName;
    array[93].pluginName = plugins->FADE;
    array[93].settingName = settings->FADE_FULLSCREEN_VISUAL_BELL.compizName;
    array[94].pluginName = plugins->FADE;
    array[94].settingName = settings->FADE_VISUAL_BELL.compizName;
    array[95].pluginName = plugins->SPECIAL;
    array[95].settingName = settings->NULL_RESIZE_WITH_RIGHT_BUTTON.compizName;
    array[96].pluginName = plugins->SPECIAL;
    array[96].settingName = settings->NULL_MOUSE_BUTTON_MODIFIER.compizName;
    array[97].pluginName = plugins->CORE;
    array[97].settingName = settings->CORE_WINDOW_MENU_BUTTON.compizName;
    array[98].pluginName = plugins->RESIZE;
    array[98].settingName = settings->RESIZE_INITIATE_BUTTON.compizName;
    array[99].pluginName = plugins->MOVE;
    array[99].settingName = settings->MOVE_INITIATE_BUTTON.compizName;
    array[100].pluginName = plugins->CORE;
    array[100].settingName = settings->CORE_WINDOW_MENU_KEY.compizName;
    array[101].pluginName = plugins->RESIZE;
    array[101].settingName = settings->RESIZE_INITIATE_KEY.compizName;
    array[102].pluginName = plugins->MOVE;
    array[102].settingName = settings->MOVE_INITIATE_KEY.compizName;
    array[103].pluginName = plugins->CORE;
    array[103].settingName = settings->CORE_SHOW_DESKTOP_KEY.compizName;
    array[104].pluginName = plugins->CORE;
    array[104].settingName = settings->CORE_TOGGLE_WINDOW_SHADED_KEY.compizName;
    array[105].pluginName = plugins->CORE;
    array[105].settingName = settings->CORE_CLOSE_WINDOW_KEY.compizName;
    array[106].pluginName = plugins->CORE;
    array[106].settingName = settings->CORE_LOWER_WINDOW_KEY.compizName;
    array[107].pluginName = plugins->CORE;
    array[107].settingName = settings->CORE_RAISE_WINDOW_KEY.compizName;
    array[108].pluginName = plugins->CORE;
    array[108].settingName = settings->CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY.compizName;
    array[109].pluginName = plugins->CORE;
    array[109].settingName = settings->CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY.compizName;
    array[110].pluginName = plugins->CORE;
    array[110].settingName = settings->CORE_UNMAXIMIZE_WINDOW_KEY.compizName;
    array[111].pluginName = plugins->CORE;
    array[111].settingName = settings->CORE_MAXIMIZE_WINDOW_KEY.compizName;
    array[112].pluginName = plugins->CORE;
    array[112].settingName = settings->CORE_MINIMIZE_WINDOW_KEY.compizName;
    array[113].pluginName = plugins->CORE;
    array[113].settingName = settings->CORE_TOGGLE_WINDOW_MAXIMIZED_KEY.compizName;
    array[114].pluginName = plugins->GNOMECOMPAT;
    array[114].settingName = settings->GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY.compizName;
    array[115].pluginName = plugins->GNOMECOMPAT;
    array[115].settingName = settings->GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY.compizName;
    array[116].pluginName = plugins->GNOMECOMPAT;
    array[116].settingName = settings->GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY.compizName;
    array[117].pluginName = plugins->GNOMECOMPAT;
    array[117].settingName = settings->GNOMECOMPAT_MAIN_MENU_KEY.compizName;
    array[118].pluginName = plugins->GNOMECOMPAT;
    array[118].settingName = settings->GNOMECOMPAT_RUN_KEY.compizName;
    array[119].pluginName = plugins->UNITYSHELL;
    array[119].settingName = settings->UNITYSHELL_SHOW_HUD.compizName;


    return NULL;
}

const CCSGNOMEIntegratedSettingsList *
ccsGNOMEIntegratedSettingsList ()
{
    static GOnce initIntegratedSettings = G_ONCE_INIT;
    static const CCSGNOMEIntegratedSettingsList settings[CCS_GNOME_INTEGRATED_SETTINGS_LIST_SIZE];

    g_once (&initIntegratedSettings, ccsGNOMEIntegrationInitializeIntegratedSettingsList, (gpointer) settings);

    return settings;
}

gpointer
ccsGNOMEGSettingsIntegrationInitializeIntegratedSchemasQuarks (gpointer data)
{
    CCSGSettingsWrapperIntegratedSchemasQuarks *quarks = (CCSGSettingsWrapperIntegratedSchemasQuarks *) data;

    static const char *org_compiz_integrated = "org.compiz.integrated";
    static const char *org_gnome_desktop_wm_keybindings = "org.gnome.desktop.wm.keybindings";
    static const char *org_gnome_desktop_wm_preferences = "org.gnome.desktop.wm.preferences";
    static const char *org_gnome_settings_daemon_plugins_media_keys = "org.gnome.settings-daemon.plugins.media-keys";
    static const char *org_gnome_desktop_default_applications_terminal = "org.gnome.desktop.default-applications.terminal";

    quarks->ORG_COMPIZ_INTEGRATED = g_quark_from_string (org_compiz_integrated);
    quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS = g_quark_from_string (org_gnome_desktop_wm_keybindings);
    quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES = g_quark_from_string (org_gnome_desktop_wm_preferences);
    quarks->ORG_GNOME_SETTINGS_DAEMON_PLUGINS_MEDIA_KEYS = g_quark_from_string (org_gnome_settings_daemon_plugins_media_keys);
    quarks->ORG_GNOME_DESKTOP_DEFAULT_APPLICATIONS_TERMINAL = g_quark_from_string (org_gnome_desktop_default_applications_terminal);

    return NULL;
}

const CCSGSettingsWrapperIntegratedSchemasQuarks *
ccsGNOMEGSettingsWrapperQuarks ()
{
    static GOnce initIntegratedSchemaQuarks = G_ONCE_INIT;
    static const CCSGSettingsWrapperIntegratedSchemasQuarks quarks;

    g_once (&initIntegratedSchemaQuarks, ccsGNOMEGSettingsIntegrationInitializeIntegratedSchemasQuarks, (gpointer) &quarks);

    return &quarks;
}

static void destroyHashTableInternal (void *ht)
{
    g_hash_table_unref ((GHashTable *) ht);
}

GHashTable *
ccsGNOMEGSettingsIntegrationPopulateSettingNameToIntegratedSchemasQuarksHashTable ()
{
    GHashTable *masterHashTable = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, destroyHashTableInternal);
    GHashTable *coreHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *thumbnailHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *gnomecompatHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *rotateHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *putHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *wallHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *vpswitchHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *commandsHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *extrawmHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *resizeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *moveHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *staticswitcherHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *fadeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *unityshellHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *specialHashTable = g_hash_table_new (g_str_hash, g_str_equal);

    const CCSGNOMEIntegratedSettingNames *names = &ccsGNOMEIntegratedSettingNames;
    const CCSGNOMEIntegratedPluginNames  *plugins = &ccsGNOMEIntegratedPluginNames;
    const CCSGSettingsWrapperIntegratedSchemasQuarks *quarks = ccsGNOMEGSettingsWrapperQuarks ();

    g_hash_table_insert (masterHashTable, (gpointer) plugins->CORE, coreHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->THUMBNAIL, thumbnailHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->GNOMECOMPAT, gnomecompatHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->ROTATE, rotateHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->PUT, putHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->WALL, wallHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->VPSWITCH, vpswitchHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->COMMANDS, commandsHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->EXTRAWM, extrawmHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->STATICSWITCHER, staticswitcherHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->FADE, fadeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->RESIZE, resizeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->MOVE, moveHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->UNITYSHELL, unityshellHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->SPECIAL, specialHashTable);

    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUDIBLE_BELL.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLICK_TO_FOCUS.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_ON_CLICK.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE_DELAY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (thumbnailHashTable, (gpointer) names->THUMBNAIL_CURRENT_VIEWPORT.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_TERMINAL.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_DEFAULT_APPLICATIONS_TERMINAL));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_SCREENSHOT.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOM_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOP_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_RIGHT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_LEFT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMRIGHT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMLEFT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPRIGHT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPLEFT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_12_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_11_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_10_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_9_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_8_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_7_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_6_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_5_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_4_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_3_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_2_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_1_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND11_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND10_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND9_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND8_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND7_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND6_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND5_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND4_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND3_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND2_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND1_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND0_KEY.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND11.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND10.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND9.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND8.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND7.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND6.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND5.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND4.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND3.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND2.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND1.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND0.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_FULLSCREEN_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_STICKY_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_PREV_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_NEXT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_FULLSCREEN_VISUAL_BELL.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_VISUAL_BELL.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_RESIZE_WITH_RIGHT_BUTTON.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_MOUSE_BUTTON_MODIFIER.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_PREFERENCES));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_BUTTON.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_BUTTON.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_BUTTON.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_SHOW_DESKTOP_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_SHADED_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLOSE_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_LOWER_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_UNMAXIMIZE_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MINIMIZE_WINDOW_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_MAXIMIZED_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_SETTINGS_DAEMON_PLUGINS_MEDIA_KEYS));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_SETTINGS_DAEMON_PLUGINS_MEDIA_KEYS));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_SETTINGS_DAEMON_PLUGINS_MEDIA_KEYS));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_MAIN_MENU_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_KEY.compizName, GINT_TO_POINTER (quarks->ORG_GNOME_DESKTOP_WM_KEYBINDINGS));
    g_hash_table_insert (unityshellHashTable, (gpointer) names->UNITYSHELL_SHOW_HUD.compizName, GINT_TO_POINTER (quarks->ORG_COMPIZ_INTEGRATED));

    return masterHashTable;
}

GHashTable *
ccsGNOMEIntegrationPopulateCategoriesHashTables ()
{
    GHashTable *masterHashTable = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, destroyHashTableInternal);
    GHashTable *coreHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *thumbnailHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *gnomecompatHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *rotateHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *putHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *wallHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *vpswitchHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *commandsHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *extrawmHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *resizeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *moveHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *staticswitcherHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *fadeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *unityshellHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *specialHashTable = g_hash_table_new (g_str_hash, g_str_equal);

    const CCSGNOMEIntegratedSettingNames *names = &ccsGNOMEIntegratedSettingNames;
    const CCSGConfIntegrationCategories  *categories = &ccsGConfIntegrationCategories;
    const CCSGNOMEIntegratedPluginNames  *plugins = &ccsGNOMEIntegratedPluginNames;

    g_hash_table_insert (masterHashTable, (gpointer) plugins->CORE, coreHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->THUMBNAIL, thumbnailHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->GNOMECOMPAT, gnomecompatHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->ROTATE, rotateHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->PUT, putHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->WALL, wallHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->VPSWITCH, vpswitchHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->COMMANDS, commandsHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->EXTRAWM, extrawmHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->STATICSWITCHER, staticswitcherHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->FADE, fadeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->RESIZE, resizeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->MOVE, moveHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->UNITYSHELL, unityshellHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->SPECIAL, specialHashTable);

    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUDIBLE_BELL.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLICK_TO_FOCUS.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_ON_CLICK.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE_DELAY.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (thumbnailHashTable, (gpointer) names->THUMBNAIL_CURRENT_VIEWPORT.compizName, (gpointer) categories->APPS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_TERMINAL.compizName, (gpointer) categories->DESKTOP);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_SCREENSHOT.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOM_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOP_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_RIGHT_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_LEFT_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMRIGHT_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMLEFT_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPRIGHT_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPLEFT_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_12_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_11_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_10_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_9_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_8_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_7_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_6_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_5_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_4_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_3_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_2_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_1_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND11_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND10_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND9_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND8_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND7_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND6_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND5_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND4_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND3_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND2_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND1_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND0_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND11.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND10.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND9.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND8.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND7.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND6.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND5.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND4.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND3.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND2.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND1.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND0.compizName, (gpointer) categories->KEYBINDING_COMMANDS);
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_FULLSCREEN_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_STICKY_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_PREV_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_NEXT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_FULLSCREEN_VISUAL_BELL.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_VISUAL_BELL.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_RESIZE_WITH_RIGHT_BUTTON.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_MOUSE_BUTTON_MODIFIER.compizName, (gpointer) categories->GENERAL);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_BUTTON.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_BUTTON.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_BUTTON.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_SHOW_DESKTOP_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_SHADED_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLOSE_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_LOWER_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_UNMAXIMIZE_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MINIMIZE_WINDOW_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_MAXIMIZED_KEY.compizName, (gpointer) categories->WINDOW_KEYBINDINGS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_MAIN_MENU_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_KEY.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);
    g_hash_table_insert (unityshellHashTable, (gpointer) names->UNITYSHELL_SHOW_HUD.compizName, (gpointer) categories->GLOBAL_KEYBINDINGS);

    return masterHashTable;
}


GHashTable *
ccsGNOMEIntegrationPopulateSpecialTypesHashTables ()
{
    GHashTable *masterHashTable = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, destroyHashTableInternal);
    GHashTable *coreHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *thumbnailHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *gnomecompatHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *rotateHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *putHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *wallHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *vpswitchHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *commandsHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *extrawmHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *resizeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *moveHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *staticswitcherHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *fadeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *unityshellHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *specialHashTable = g_hash_table_new (g_str_hash, g_str_equal);

    const CCSGNOMEIntegratedSettingNames *names = &ccsGNOMEIntegratedSettingNames;
    const CCSGNOMEIntegratedPluginNames  *plugins = &ccsGNOMEIntegratedPluginNames;

    g_hash_table_insert (masterHashTable, (gpointer) plugins->CORE, coreHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->THUMBNAIL, thumbnailHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->GNOMECOMPAT, gnomecompatHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->ROTATE, rotateHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->PUT, putHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->WALL, wallHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->VPSWITCH, vpswitchHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->COMMANDS, commandsHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->EXTRAWM, extrawmHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->STATICSWITCHER, staticswitcherHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->FADE, fadeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->RESIZE, resizeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->MOVE, moveHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->UNITYSHELL, unityshellHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->SPECIAL, specialHashTable);

    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUDIBLE_BELL.compizName, GINT_TO_POINTER (OptionBool));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLICK_TO_FOCUS.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_ON_CLICK.compizName, GINT_TO_POINTER (OptionBool));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE_DELAY.compizName, GINT_TO_POINTER (OptionInt));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE.compizName, GINT_TO_POINTER (OptionBool));
    g_hash_table_insert (thumbnailHashTable, (gpointer) names->THUMBNAIL_CURRENT_VIEWPORT.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_TERMINAL.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_SCREENSHOT.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOM_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOP_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_RIGHT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_LEFT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMRIGHT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMLEFT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPRIGHT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPLEFT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_12_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_11_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_10_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_9_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_8_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_7_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_6_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_5_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_4_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_3_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_2_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_1_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND11_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND10_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND9_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND8_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND7_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND6_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND5_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND4_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND3_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND2_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND1_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND0_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND11.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND10.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND9.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND8.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND7.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND6.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND5.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND4.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND3.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND2.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND1.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND0.compizName, GINT_TO_POINTER (OptionString));
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_FULLSCREEN_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_STICKY_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_PREV_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_NEXT_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_FULLSCREEN_VISUAL_BELL.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_VISUAL_BELL.compizName, GINT_TO_POINTER (OptionBool));
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_RESIZE_WITH_RIGHT_BUTTON.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_MOUSE_BUTTON_MODIFIER.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_BUTTON.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_BUTTON.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_BUTTON.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_SHOW_DESKTOP_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_SHADED_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLOSE_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_LOWER_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_UNMAXIMIZE_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MINIMIZE_WINDOW_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_MAXIMIZED_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY.compizName, GINT_TO_POINTER (OptionSpecial));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_MAIN_MENU_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_KEY.compizName, GINT_TO_POINTER (OptionKey));
    g_hash_table_insert (unityshellHashTable, (gpointer) names->UNITYSHELL_SHOW_HUD.compizName, GINT_TO_POINTER (OptionKey));

    return masterHashTable;
}

GHashTable *
ccsGNOMEIntegrationPopulateSettingNameToGNOMENameHashTables ()
{
    GHashTable *masterHashTable = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, destroyHashTableInternal);
    GHashTable *coreHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *thumbnailHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *gnomecompatHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *rotateHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *putHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *wallHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *vpswitchHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *commandsHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *extrawmHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *resizeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *moveHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *staticswitcherHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *fadeHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *unityshellHashTable = g_hash_table_new (g_str_hash, g_str_equal);
    GHashTable *specialHashTable = g_hash_table_new (g_str_hash, g_str_equal);

    const CCSGNOMEIntegratedSettingNames *names = &ccsGNOMEIntegratedSettingNames;
    const CCSGNOMEIntegratedPluginNames  *plugins = &ccsGNOMEIntegratedPluginNames;

    g_hash_table_insert (masterHashTable, (gpointer) plugins->CORE, coreHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->THUMBNAIL, thumbnailHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->GNOMECOMPAT, gnomecompatHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->ROTATE, rotateHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->PUT, putHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->WALL, wallHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->VPSWITCH, vpswitchHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->COMMANDS, commandsHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->EXTRAWM, extrawmHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->STATICSWITCHER, staticswitcherHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->FADE, fadeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->RESIZE, resizeHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->MOVE, moveHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->UNITYSHELL, unityshellHashTable);
    g_hash_table_insert (masterHashTable, (gpointer) plugins->SPECIAL, specialHashTable);

    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUDIBLE_BELL.compizName, (gpointer) names->CORE_AUDIBLE_BELL.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLICK_TO_FOCUS.compizName, (gpointer) names->CORE_CLICK_TO_FOCUS.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_ON_CLICK.compizName, (gpointer) names->CORE_RAISE_ON_CLICK.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE_DELAY.compizName, (gpointer) names->CORE_AUTORAISE_DELAY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_AUTORAISE.compizName, (gpointer) names->CORE_AUTORAISE.gnomeName);
    g_hash_table_insert (thumbnailHashTable, (gpointer) names->THUMBNAIL_CURRENT_VIEWPORT.compizName, (gpointer) names->THUMBNAIL_CURRENT_VIEWPORT.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_TERMINAL.compizName, (gpointer) names->GNOMECOMPAT_COMMAND_TERMINAL.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT.compizName, (gpointer) names->GNOMECOMPAT_COMMAND_WINDOW_SCREENSHOT.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_COMMAND_SCREENSHOT.compizName, (gpointer) names->GNOMECOMPAT_COMMAND_SCREENSHOT.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_RIGHT_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_LEFT_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_12_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_11_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_10_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_9_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_8_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_7_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_6_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_5_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_4_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_3_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_2_WINDOW_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_WINDOW_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_1_WINDOW_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOM_KEY.compizName, (gpointer) names->PUT_PUT_BOTTOM_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOP_KEY.compizName, (gpointer) names->PUT_PUT_TOP_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_RIGHT_KEY.compizName, (gpointer) names->PUT_PUT_RIGHT_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_LEFT_KEY.compizName, (gpointer) names->PUT_PUT_LEFT_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMRIGHT_KEY.compizName, (gpointer) names->PUT_PUT_BOTTOMRIGHT_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_BOTTOMLEFT_KEY.compizName, (gpointer) names->PUT_PUT_BOTTOMLEFT_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPRIGHT_KEY.compizName, (gpointer) names->PUT_PUT_TOPRIGHT_KEY.gnomeName);
    g_hash_table_insert (putHashTable, (gpointer) names->PUT_PUT_TOPLEFT_KEY.compizName, (gpointer) names->PUT_PUT_TOPLEFT_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_WINDOW_KEY.compizName, (gpointer) names->WALL_DOWN_WINDOW_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_WINDOW_KEY.compizName, (gpointer) names->WALL_UP_WINDOW_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_WINDOW_KEY.compizName, (gpointer) names->WALL_RIGHT_WINDOW_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_WINDOW_KEY.compizName, (gpointer) names->WALL_LEFT_WINDOW_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_RIGHT_KEY.compizName, (gpointer) names->WALL_RIGHT_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_LEFT_KEY.compizName, (gpointer) names->WALL_LEFT_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_DOWN_KEY.compizName, (gpointer) names->WALL_DOWN_KEY.gnomeName);
    g_hash_table_insert (wallHashTable, (gpointer) names->WALL_UP_KEY.compizName, (gpointer) names->WALL_UP_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_12_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_12_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_11_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_11_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_10_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_10_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_9_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_9_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_8_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_8_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_7_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_7_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_6_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_6_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_5_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_5_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_4_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_4_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_3_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_3_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_2_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_2_KEY.gnomeName);
    g_hash_table_insert (vpswitchHashTable, (gpointer) names->VPSWITCH_SWITCH_TO_1_KEY.compizName, (gpointer) names->VPSWITCH_SWITCH_TO_1_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_RIGHT_KEY.compizName, (gpointer) names->ROTATE_ROTATE_RIGHT_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_LEFT_KEY.compizName, (gpointer) names->ROTATE_ROTATE_LEFT_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_12_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_12_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_11_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_11_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_10_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_10_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_9_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_9_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_8_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_8_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_7_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_7_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_6_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_6_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_5_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_5_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_4_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_4_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_3_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_3_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_2_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_2_KEY.gnomeName);
    g_hash_table_insert (rotateHashTable, (gpointer) names->ROTATE_ROTATE_TO_1_KEY.compizName, (gpointer) names->ROTATE_ROTATE_TO_1_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND11_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND11_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND10_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND10_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND9_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND9_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND8_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND8_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND7_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND7_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND6_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND6_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND5_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND5_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND4_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND4_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND3_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND3_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND2_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND2_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND1_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND1_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_RUN_COMMAND0_KEY.compizName, (gpointer) names->COMMANDS_RUN_COMMAND0_KEY.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND11.compizName, (gpointer) names->COMMANDS_COMMAND11.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND10.compizName, (gpointer) names->COMMANDS_COMMAND10.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND9.compizName, (gpointer) names->COMMANDS_COMMAND9.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND8.compizName, (gpointer) names->COMMANDS_COMMAND8.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND7.compizName, (gpointer) names->COMMANDS_COMMAND7.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND6.compizName, (gpointer) names->COMMANDS_COMMAND6.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND5.compizName, (gpointer) names->COMMANDS_COMMAND5.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND4.compizName, (gpointer) names->COMMANDS_COMMAND4.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND3.compizName, (gpointer) names->COMMANDS_COMMAND3.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND2.compizName, (gpointer) names->COMMANDS_COMMAND2.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND1.compizName, (gpointer) names->COMMANDS_COMMAND1.gnomeName);
    g_hash_table_insert (commandsHashTable, (gpointer) names->COMMANDS_COMMAND0.compizName, (gpointer) names->COMMANDS_COMMAND0.gnomeName);
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_FULLSCREEN_KEY.compizName, (gpointer) names->EXTRAWM_TOGGLE_FULLSCREEN_KEY.gnomeName);
    g_hash_table_insert (extrawmHashTable, (gpointer) names->EXTRAWM_TOGGLE_STICKY_KEY.compizName, (gpointer) names->EXTRAWM_TOGGLE_STICKY_KEY.gnomeName);
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_PREV_KEY.compizName, (gpointer) names->STATICSWITCHER_PREV_KEY.gnomeName);
    g_hash_table_insert (staticswitcherHashTable, (gpointer) names->STATICSWITCHER_NEXT_KEY.compizName, (gpointer) names->STATICSWITCHER_NEXT_KEY.gnomeName);
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_FULLSCREEN_VISUAL_BELL.compizName, (gpointer) names->FADE_FULLSCREEN_VISUAL_BELL.gnomeName);
    g_hash_table_insert (fadeHashTable, (gpointer) names->FADE_VISUAL_BELL.compizName, (gpointer) names->FADE_VISUAL_BELL.gnomeName);
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_RESIZE_WITH_RIGHT_BUTTON.compizName, (gpointer) names->NULL_RESIZE_WITH_RIGHT_BUTTON.gnomeName);
    g_hash_table_insert (specialHashTable, (gpointer) names->NULL_MOUSE_BUTTON_MODIFIER.compizName, (gpointer) names->NULL_MOUSE_BUTTON_MODIFIER.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_BUTTON.compizName, (gpointer) names->CORE_WINDOW_MENU_BUTTON.gnomeName);
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_BUTTON.compizName, (gpointer) names->RESIZE_INITIATE_BUTTON.gnomeName);
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_BUTTON.compizName, (gpointer) names->MOVE_INITIATE_BUTTON.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_WINDOW_MENU_KEY.compizName, (gpointer) names->CORE_WINDOW_MENU_KEY.gnomeName);
    g_hash_table_insert (resizeHashTable, (gpointer) names->RESIZE_INITIATE_KEY.compizName, (gpointer) names->RESIZE_INITIATE_KEY.gnomeName);
    g_hash_table_insert (moveHashTable, (gpointer) names->MOVE_INITIATE_KEY.compizName, (gpointer) names->MOVE_INITIATE_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_SHOW_DESKTOP_KEY.compizName, (gpointer) names->CORE_SHOW_DESKTOP_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_SHADED_KEY.compizName, (gpointer) names->CORE_TOGGLE_WINDOW_SHADED_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_CLOSE_WINDOW_KEY.compizName, (gpointer) names->CORE_CLOSE_WINDOW_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_LOWER_WINDOW_KEY.compizName, (gpointer) names->CORE_LOWER_WINDOW_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_RAISE_WINDOW_KEY.compizName, (gpointer) names->CORE_RAISE_WINDOW_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY.compizName, (gpointer) names->CORE_MAXIMIZE_WINDOW_VERTICALLY_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY.compizName, (gpointer) names->CORE_MAXIMIZE_WINDOW_HORIZONTALLY_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_UNMAXIMIZE_WINDOW_KEY.compizName, (gpointer) names->CORE_UNMAXIMIZE_WINDOW_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MAXIMIZE_WINDOW_KEY.compizName, (gpointer) names->CORE_MAXIMIZE_WINDOW_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_MINIMIZE_WINDOW_KEY.compizName, (gpointer) names->CORE_MINIMIZE_WINDOW_KEY.gnomeName);
    g_hash_table_insert (coreHashTable, (gpointer) names->CORE_TOGGLE_WINDOW_MAXIMIZED_KEY.compizName, (gpointer) names->CORE_TOGGLE_WINDOW_MAXIMIZED_KEY.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY.compizName, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_TERMINAL_KEY.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY.compizName, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_WINDOW_SCREENSHOT_KEY.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY.compizName, (gpointer) names->GNOMECOMPAT_RUN_COMMAND_SCREENSHOT_KEY.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_MAIN_MENU_KEY.compizName, (gpointer) names->GNOMECOMPAT_MAIN_MENU_KEY.gnomeName);
    g_hash_table_insert (gnomecompatHashTable, (gpointer) names->GNOMECOMPAT_RUN_KEY.compizName, (gpointer) names->GNOMECOMPAT_RUN_KEY.gnomeName);
    g_hash_table_insert (unityshellHashTable, (gpointer) names->UNITYSHELL_SHOW_HUD.compizName, (gpointer) names->UNITYSHELL_SHOW_HUD.gnomeName);

    return masterHashTable;
}
