find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (neg PLUGINDEPS composite opengl)
