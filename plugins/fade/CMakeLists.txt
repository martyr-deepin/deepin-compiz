find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(fade PLUGINDEPS composite opengl)