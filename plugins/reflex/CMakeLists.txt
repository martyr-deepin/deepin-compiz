find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (reflex PLUGINDEPS composite opengl)
