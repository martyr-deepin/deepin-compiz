find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (firepaint PLUGINDEPS composite opengl)
