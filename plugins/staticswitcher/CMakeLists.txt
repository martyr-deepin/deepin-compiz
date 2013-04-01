find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (staticswitcher PLUGINDEPS composite opengl compiztoolbox)
