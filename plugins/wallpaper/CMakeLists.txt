find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (wallpaper PLUGINDEPS composite opengl)
