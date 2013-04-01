find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(gnomecompat PLUGINDEPS composite opengl)