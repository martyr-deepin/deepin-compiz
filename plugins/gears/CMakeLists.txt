find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (gears PLUGINDEPS composite opengl cube)
