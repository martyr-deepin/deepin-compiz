INCLUDE_DIRECTORIES (  
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
    
  ${Boost_INCLUDE_DIRS}
  
  ${GLIBMM_INCLUDE_DIRS}
)

LINK_DIRECTORIES (${GLIBMM_LIBRARY_DIRS} ${COMPIZ_LIBRARY_DIRS}) 

SET ( 
  PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/offset-movement.h
)

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/offset-movement.cpp
)

ADD_LIBRARY( 
  compiz_wall_offset_movement STATIC
  
  ${SRCS}
  
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

TARGET_LINK_LIBRARIES(
  compiz_wall_offset_movement

  compiz_rect
  compiz_region
)
