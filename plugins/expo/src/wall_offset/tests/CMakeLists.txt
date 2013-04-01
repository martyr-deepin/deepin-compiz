if (NOT GTEST_FOUND)
  message ("Google Test not found - cannot build tests!")
  set (COMPIZ_BUILD_TESTING OFF)
endif (NOT GTEST_FOUND)

include_directories (${GTEST_INCLUDE_DIRS})

link_directories (${COMPIZ_LIBRARY_DIRS})

add_executable (compiz_test_expo_wall_offset
		${CMAKE_CURRENT_SOURCE_DIR}/test-expo-wall-offset.cpp)

target_link_libraries (compiz_test_expo_wall_offset
		       compiz_expo_wall_offset
		       ${GTEST_BOTH_LIBRARIES}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
                       )

compiz_discover_tests (compiz_test_expo_wall_offset COVERAGE compiz_expo_wall_offset)
