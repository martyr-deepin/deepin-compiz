find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (scalefilter PLUGINDEPS composite opengl scale text)
