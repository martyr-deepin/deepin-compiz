find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (opacify PLUGINDEPS composite opengl)
