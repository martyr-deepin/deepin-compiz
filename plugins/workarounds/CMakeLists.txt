find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (workarounds PLUGINDEPS composite opengl)
