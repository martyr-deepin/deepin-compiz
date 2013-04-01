find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (cubeaddon PLUGINDEPS composite opengl cube)

