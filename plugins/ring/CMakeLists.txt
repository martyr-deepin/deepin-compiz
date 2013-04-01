find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (ring PLUGINDEPS composite opengl text)
