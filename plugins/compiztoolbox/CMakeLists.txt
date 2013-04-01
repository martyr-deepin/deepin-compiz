find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (compiztoolbox PLUGINDEPS composite opengl LIBRARIES Xrender)
