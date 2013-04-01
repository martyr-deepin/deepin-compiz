find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (addhelper PLUGINDEPS composite opengl)
