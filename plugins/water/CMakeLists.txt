find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(water PLUGINDEPS composite opengl)