find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (splash PLUGINDEPS composite opengl)
