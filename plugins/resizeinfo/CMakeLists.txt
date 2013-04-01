find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin (resizeinfo PLUGINDEPS opengl composite PKGDEPS pango pangocairo cairo-xlib-xrender)
