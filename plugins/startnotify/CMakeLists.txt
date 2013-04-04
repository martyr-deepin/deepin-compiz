find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (startnotify PLUGINDEPS composite opengl mousepoll)
