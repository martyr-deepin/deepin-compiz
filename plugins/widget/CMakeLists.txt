find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (widget PLUGINDEPS composite opengl)
