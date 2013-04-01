INCLUDE_DIRECTORIES (  
  ${compiz_SOURCE_DIR}/src/servergrab/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
    
  ${Boost_INCLUDE_DIRS}
)

LINK_DIRECTORIES (${COMPIZ_LIBRARY_DIRS}) 

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/glx-tfp-bind.cpp
)

ADD_LIBRARY( 
  compiz_opengl_glx_tfp_bind STATIC
  
  ${SRCS}
)

if (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

TARGET_LINK_LIBRARIES(
  compiz_opengl_glx_tfp_bind

  compiz_servergrab
)
