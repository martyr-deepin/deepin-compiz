find_package (Compiz REQUIRED)
include (CompizPlugin)

compiz_plugin (animation PLUGINDEPS composite opengl)

