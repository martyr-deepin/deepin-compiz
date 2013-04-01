find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (showrepaint PLUGINDEPS composite opengl)
