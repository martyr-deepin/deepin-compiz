find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(screenshot PLUGINDEPS composite opengl compiztoolbox)
