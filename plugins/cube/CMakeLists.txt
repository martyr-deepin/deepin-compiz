find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(cube PLUGINDEPS composite opengl)