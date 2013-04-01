include (CompizDefaults)
include (CompizCommon)

INCLUDE_DIRECTORIES(
  ${compiz_BINARY_DIR}/generated
  ${compiz_SOURCE_DIR}/include

  ${compiz_SOURCE_DIR}/src/timer/include
  ${compiz_SOURCE_DIR}/src/timer/src

  ${compiz_SOURCE_DIR}/src/rect/include
  ${compiz_SOURCE_DIR}/src/region/include
  ${compiz_SOURCE_DIR}/src/window/geometry/include
  ${compiz_SOURCE_DIR}/src/window/extents/include

  ${compiz_SOURCE_DIR}/src/pluginclasshandler/include
  ${compiz_SOURCE_DIR}/src/servergrab/include

  ${COMPIZ_INCLUDE_DIRS}

  ${Boost_INCLUDE_DIRS}
)

SET( PUBLIC_HEADERS )
SET( PRIVATE_HEADERS )

add_definitions (
    -DHAVE_CONFIG_H
    -DPLUGINDIR=\\\"${compiz_plugindir}\\\"
    -DSHAREDIR=\\\"${compiz_sharedir}\\\"
    -DMETADATADIR=\\\"${compiz_metadatadir}\\\"
)

SET( SRCS ${compiz_SOURCE_DIR}/src/plugin.cpp )

ADD_LIBRARY( 
  compiz_plugin STATIC
 		  
  ${SRCS}
  
  ${PUBLIC_HEADERS}
  ${PRIVATE_HEADERS}
)

add_dependencies (compiz_plugin core-options)

ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )

SET_TARGET_PROPERTIES(
  compiz_plugin PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
)
