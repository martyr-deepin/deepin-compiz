include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

link_directories (${GLIBMM_LIBRARY_DIRS} ${COMPIZ_LIBRARY_DIRS})

set (
  PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/wall-offset.h
)

set (
  SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/wall-offset.cpp
)

add_library (
  compiz_expo_wall_offset STATIC
  ${SRCS}
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
  add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

target_link_libraries (
  compiz_expo_wall_offset
  compiz_core
)
