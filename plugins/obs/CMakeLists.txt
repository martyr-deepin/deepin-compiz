find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(obs PLUGINDEPS composite opengl)