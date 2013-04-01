find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (scaleaddon PLUGINDEPS composite opengl text scale)
