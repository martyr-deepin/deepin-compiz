include (FindPkgConfig)

PKG_CHECK_MODULES (X11 x11)

INCLUDE_DIRECTORIES (  
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
    
  ${Boost_INCLUDE_DIRS}
  
  ${X11_INCLUDE_DIRS}
)

LINK_DIRECTORIES (${GLIBMM_LIBRARY_DIRS} ${COMPIZ_LIBRARY_DIRS}) 

SET ( 
  PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/pixmapbinding.h
)

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pixmapbinding.cpp
  ${compiz_SOURCE_DIR}/src/size.cpp # XXX: Remove once we've merged this into a general geometry lib
)

ADD_LIBRARY( 
  compiz_composite_pixmapbinding STATIC
  
  ${SRCS}
  
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

TARGET_LINK_LIBRARIES(
  compiz_composite_pixmapbinding
  compiz_servergrab
  ${X11_LIBRARIES}
)
