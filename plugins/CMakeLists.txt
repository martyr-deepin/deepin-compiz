set (COMPIZ_CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${COMPIZ_CMAKE_MODULE_PATH})
include (CompizDefaults)
include (CompizCommon)

set (COMPIZ_FOUND "true")

add_definitions ( -DHAVE_CONFIG_H)

include_directories( 
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/timer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/string/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/pluginclasshandler/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/point/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/rect/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/servergrab/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/region/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/window/geometry/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/window/geometry-saver/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/window/extents/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/window/constrainment/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../logmessage/include
)

compiz_add_plugins_in_folder (${CMAKE_CURRENT_SOURCE_DIR})
