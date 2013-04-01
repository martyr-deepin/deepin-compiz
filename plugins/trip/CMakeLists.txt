find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (trip PLUGINDEPS composite opengl)
