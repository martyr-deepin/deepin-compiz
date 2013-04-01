find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (td PLUGINDEPS composite opengl cube)
