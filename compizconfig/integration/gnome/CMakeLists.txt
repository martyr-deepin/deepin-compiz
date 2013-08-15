include (FindPkgConfig)

add_subdirectory (gconf)
add_subdirectory (gsettings)

set (_deps
     glib-2.0>=2.20.0
     gio-2.0>=2.25.0)
