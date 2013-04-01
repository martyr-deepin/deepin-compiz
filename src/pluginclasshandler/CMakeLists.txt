
INCLUDE_DIRECTORIES(  
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}../logmessage/include
  
  ${compiz_SOURCE_DIR}/include
    
  ${Boost_INCLUDE_DIRS}
)

SET( 
  PUBLIC_HEADERS 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/core/pluginclasses.h 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/core/pluginclasshandler.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/core/valueholder.h
)

SET( 
  PRIVATE_HEADERS 
)

SET( 
  SRCS 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/valueholder.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pluginclasses.cpp
)

ADD_LIBRARY( 
  compiz_pluginclasshandler STATIC
  
  ${SRCS}
  
  ${PUBLIC_HEADERS}
  ${PRIVATE_HEADERS}
)

IF (COMPIZ_BUILD_TESTING)
ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
ENDIF (COMPIZ_BUILD_TESTING)

SET_TARGET_PROPERTIES(
  compiz_pluginclasshandler PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

install (FILES ${PUBLIC_HEADERS} DESTINATION ${COMPIZ_CORE_INCLUDE_DIR})
