find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (loginout PLUGINDEPS composite opengl)
