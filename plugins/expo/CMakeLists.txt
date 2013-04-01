find_package (Compiz REQUIRED)
include (CompizPlugin)

set (COMPIZ_EXPO_PLUGIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory (src/click_threshold)
include_directories (src/click_threshold/include)
add_subdirectory (src/wall_offset)
include_directories (src/wall_offset/include)
add_subdirectory (src/windows_on_viewport)
include_directories (src/windows_on_viewport/include)

compiz_plugin (expo PLUGINDEPS composite opengl LIBRARIES
	       compiz_expo_click_threshold
	       compiz_expo_wall_offset
	       compiz_expo_windows_on_viewport)
