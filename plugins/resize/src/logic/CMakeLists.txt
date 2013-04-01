INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/generated
  ${COMPIZ_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

LINK_DIRECTORIES (${GLIBMM_LIBRARY_DIRS} ${COMPIZ_LIBRARY_DIRS})

SET (
  PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/composite-screen-interface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gl-screen-interface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/resize-logic.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/resize-window-interface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/screen-interface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/window-interface.h
)

SET(
  SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/resize-logic.cpp
)

ADD_LIBRARY(
  resize_logic STATIC
  ${SRCS}
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
  ADD_SUBDIRECTORY( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

TARGET_LINK_LIBRARIES(
  resize_logic
  compiz_rect
  compiz_region
)

ADD_DEPENDENCIES (resize_logic resize-options)
