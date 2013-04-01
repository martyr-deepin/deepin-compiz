find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(wobbly PLUGINDEPS composite opengl)
