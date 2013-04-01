find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(scale PLUGINDEPS composite opengl)
