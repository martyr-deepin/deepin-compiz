find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(move PLUGINDEPS composite opengl)