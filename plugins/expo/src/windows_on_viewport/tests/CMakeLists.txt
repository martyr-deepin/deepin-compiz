if (NOT GTEST_FOUND)
  message ("Google Test not found - cannot build tests!")
  set (COMPIZ_BUILD_TESTING OFF)
endif (NOT GTEST_FOUND)

include_directories (${GTEST_INCLUDE_DIRS})
include_directories (${CMAKE_EXPO_PLUGIN_SOURCE_DIR})

link_directories (${COMPIZ_LIBRARY_DIRS})

add_executable (compiz_test_expo_windows_on_viewport
		${CMAKE_CURRENT_SOURCE_DIR}/test-windows-on-viewport.cpp)

target_link_libraries (compiz_test_expo_windows_on_viewport
		       compiz_expo_windows_on_viewport
		       ${GTEST_BOTH_LIBRARIES}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
                       ${GMOCK_LIBRARY}
                       ${GMOCK_MAIN_LIBRARY}
                       )

compiz_discover_tests (compiz_test_expo_windows_on_viewport COVERAGE compiz_expo_windows_on_viewport)
