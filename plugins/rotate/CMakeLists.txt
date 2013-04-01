find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(rotate PLUGINDEPS composite opengl cube)