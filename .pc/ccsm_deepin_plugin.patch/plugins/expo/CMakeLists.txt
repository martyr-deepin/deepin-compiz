find_package (Compiz REQUIRED)
include (CompizPlugin)

add_subdirectory (src/click_threshold)
include_directories (src/click_threshold/include)

compiz_plugin (expo PLUGINDEPS composite opengl LIBRARIES compiz_expo_click_threshold)
