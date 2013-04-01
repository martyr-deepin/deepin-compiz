find_package (Compiz REQUIRED)

include (CompizPlugin)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/pixmapbinding/include)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/src/pixmapbinding)

compiz_plugin (composite LIBRARIES compiz_composite_pixmapbinding)

add_subdirectory (src/pixmapbinding)
