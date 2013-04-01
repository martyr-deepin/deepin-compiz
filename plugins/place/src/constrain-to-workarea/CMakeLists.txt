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
  ${CMAKE_CURRENT_SOURCE_DIR}/include/constrain-to-workarea.h
)

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/constrain-to-workarea.cpp
  ${compiz_SOURCE_DIR}/src/size.cpp
)

ADD_LIBRARY( 
  compiz_place_constrain_to_workarea STATIC
  
  ${SRCS}
  
  ${PUBLIC_HEADERS}
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

SET_TARGET_PROPERTIES(
  compiz_place_constrain_to_workarea PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

TARGET_LINK_LIBRARIES(
  compiz_place_constrain_to_workarea

  compiz_core
  ${GLIBMM_LIBRARIES}
)
