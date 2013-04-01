find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (clone PLUGINDEPS composite opengl)
