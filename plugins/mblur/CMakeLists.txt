find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (mblur PLUGINDEPS composite opengl)
