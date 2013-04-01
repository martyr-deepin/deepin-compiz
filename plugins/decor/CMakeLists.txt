find_package (Compiz REQUIRED)

include (CompizPlugin)
include (CompizCommon)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/clip-groups/include/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/pixmap-requests/include/)

compiz_plugin(decor PLUGINDEPS composite opengl LIBRARIES decoration compiz_decor_clip_groups compiz_decor_pixmap_requests)

add_subdirectory (src/clip-groups)
add_subdirectory (src/pixmap-requests)

if (COMPIZ_BUILD_WITH_RPATH AND NOT COMPIZ_DISABLE_PLUGIN_DECOR)

    set_target_properties (
	decor PROPERTIES
	INSTALL_RPATH "${COMPIZ_LIBDIR}"
    )
endif (COMPIZ_BUILD_WITH_RPATH AND NOT COMPIZ_DISABLE_PLUGIN_DECOR)

install (PROGRAMS "src/compiz-decorator" DESTINATION ${exec_prefix})
