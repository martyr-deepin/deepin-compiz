find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(switcher PLUGINDEPS composite opengl compiztoolbox)