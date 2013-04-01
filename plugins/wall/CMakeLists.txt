find_package (Compiz REQUIRED)

include (CompizPlugin)
include (CompizDefaults)

if (NOT COMPIZ_DISABLE_PLUGIN_WALL)
    add_subdirectory (src/offset_movement)
    include_directories (src/offset_movement/include)
endif (NOT COMPIZ_DISABLE_PLUGIN_WALL)

compiz_plugin (wall PLUGINDEPS composite opengl mousepoll PKGDEPS cairo cairo-xlib-xrender LIBRARIES compiz_wall_offset_movement dl)
