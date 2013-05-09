find_package (Compiz REQUIRED)

include (CompizPlugin)

include_directories (src/grabhandler/include)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/src/grabhandler)

compiz_plugin (grid PLUGINDEPS composite opengl LIBRARIES compiz_grid_grabhandler)

add_subdirectory (src/grabhandler)
