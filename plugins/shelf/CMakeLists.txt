find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (shelf PLUGINDEPS composite opengl)
