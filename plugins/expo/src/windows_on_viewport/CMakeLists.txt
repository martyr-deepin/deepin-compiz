include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${COMPIZ_INCLUDE_DIRS}
  ${COMPIZ_EXPO_PLUGIN_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
)

link_directories (${GLIBMM_LIBRARY_DIRS} ${COMPIZ_LIBRARY_DIRS})

set (
  PRIVATE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/windows-on-viewport.h
)

set (
  SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/windows-on-viewport.cpp
)

add_library (
  compiz_expo_windows_on_viewport STATIC
  ${SRCS}
  ${PRIVATE_HEADERS}
)

if (COMPIZ_BUILD_TESTING)
  add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif (COMPIZ_BUILD_TESTING)

target_link_libraries (
  compiz_expo_windows_on_viewport
  compiz_window_geometry
  compiz_core
)
