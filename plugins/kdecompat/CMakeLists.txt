find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (kdecompat PLUGINDEPS composite opengl)
