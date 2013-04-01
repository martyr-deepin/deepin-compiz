find_package (Compiz REQUIRED)

include (CompizPlugin)

compiz_plugin(annotate PLUGINDEPS composite opengl PKGDEPS pangocairo cairo cairo-xlib-xrender)
