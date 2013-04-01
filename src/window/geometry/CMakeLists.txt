pkg_check_modules (
  GLIBMM
  REQUIRED
  glibmm-2.4 glib-2.0
)

INCLUDE_DIRECTORIES (  
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  
  ${compiz_SOURCE_DIR}/include
  ${compiz_SOURCE_DIR}/src/point/include
  ${compiz_SOURCE_DIR}/src/rect/include

  ${Boost_INCLUDE_DIRS}
  
  ${GLIBMM_INCLUDE_DIRS}
)

LINK_DIRECTORIES (${GLIBMM_LIBRARY_DIRS}) 

SET ( 
  PUBLIC_HEADERS 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/core/windowgeometry.h
)

SET ( 
  PRIVATE_HEADERS 
)

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/windowgeometry.cpp
)

ADD_LIBRARY( 
  compiz_window_geometry STATIC
  
  ${SRCS}
  
  ${PUBLIC_HEADERS}
  ${PRIVATE_HEADERS}
)

IF (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
ENDIF (COMPIZ_BUILD_TESTING)

SET_TARGET_PROPERTIES(
  compiz_window_geometry PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

install (FILES ${PUBLIC_HEADERS} DESTINATION ${COMPIZ_CORE_INCLUDE_DIR})

TARGET_LINK_LIBRARIES( 
  compiz_window_geometry
  compiz_point
  compiz_rect
  
  ${GLIBMM_LIBRARIES}
)
