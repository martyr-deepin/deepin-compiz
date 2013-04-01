find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (bench PLUGINDEPS composite opengl)

