find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (group PLUGINDEPS opengl composite text mousepoll PKGDEPS cairo cairo-xlib-xrender)
