find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (colorfilter PLUGINDEPS composite opengl)
