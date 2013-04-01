pkg_check_modules (
  GLIBMM
  REQUIRED
  glibmm-2.4 glib-2.0
)

INCLUDE_DIRECTORIES (  
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src

  ${compiz_SOURCE_DIR}/src/point/include
  ${compiz_SOURCE_DIR}/src/rect/include
  ${compiz_SOURCE_DIR}/src/window/geometry/include
  ${compiz_SOURCE_DIR}/src/window/geometry-saver/include
  ${compiz_SOURCE_DIR}/src/window/extents/include
  ${compiz_SOURCE_DIR}/include
    
  ${Boost_INCLUDE_DIRS}
  
  ${GLIBMM_INCLUDE_DIRS}
)

LINK_DIRECTORIES (${GLIBMM_LIBRARY_DIRS}) 

SET ( 
  PUBLIC_HEADERS 
)

SET ( 
  PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/clip-groups.h
)

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/clip-groups.cpp
)

ADD_LIBRARY( 
  compiz_decor_clip_groups STATIC
  
  ${SRCS}
  
  ${PUBLIC_HEADERS}
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

SET_TARGET_PROPERTIES(
  compiz_decor_clip_groups PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

TARGET_LINK_LIBRARIES(
  compiz_decor_clip_groups

  compiz_core
  ${GLIBMM_LIBRARIES}
)
