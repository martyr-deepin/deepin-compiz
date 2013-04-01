find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (fadedesktop PLUGINDEPS opengl composite)
