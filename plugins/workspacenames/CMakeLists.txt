find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (workspacenames PLUGINDEPS composite opengl text)
