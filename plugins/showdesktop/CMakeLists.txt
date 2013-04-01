find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (showdesktop PLUGINDEPS composite opengl)
