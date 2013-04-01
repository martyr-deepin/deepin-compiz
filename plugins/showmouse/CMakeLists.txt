find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (showmouse PLUGINDEPS composite opengl mousepoll)
