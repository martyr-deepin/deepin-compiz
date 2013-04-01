find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (stackswitch PLUGINDEPS opengl composite text)
