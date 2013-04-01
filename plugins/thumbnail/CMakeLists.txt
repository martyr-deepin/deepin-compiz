find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (thumbnail PLUGINDEPS composite opengl text mousepoll)
