find_package (Compiz REQUIRED)

include (CompizPlugin)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/constrain-to-workarea/include/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/screen-size-change/include/)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src/smart/include/)

add_subdirectory (src/constrain-to-workarea)
add_subdirectory (src/screen-size-change)
add_subdirectory (src/smart)

compiz_plugin (place LIBRARIES compiz_place_constrain_to_workarea compiz_place_screen_size_change compiz_place_smart)
