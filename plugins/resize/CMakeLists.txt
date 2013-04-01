find_package (Compiz REQUIRED)

include (CompizPlugin)

add_subdirectory (src/logic)
include_directories (src/logic/include)

compiz_plugin(resize PLUGINDEPS composite opengl LIBRARIES resize_logic)
