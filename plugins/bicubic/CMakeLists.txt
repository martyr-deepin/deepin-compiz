find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (bicubic PLUGINDEPS composite opengl)
