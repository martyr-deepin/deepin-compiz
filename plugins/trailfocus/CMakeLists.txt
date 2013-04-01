find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (trailfocus PLUGINDEPS composite opengl)
