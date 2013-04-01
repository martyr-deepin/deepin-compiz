find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (shift PLUGINDEPS composite opengl text)
