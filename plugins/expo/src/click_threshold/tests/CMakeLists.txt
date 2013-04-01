if (NOT GTEST_FOUND)
  message ("Google Test not found - cannot build tests!")
  set (COMPIZ_BUILD_TESTING OFF)
endif (NOT GTEST_FOUND)

include_directories (${GTEST_INCLUDE_DIRS})

link_directories (${COMPIZ_LIBRARY_DIRS})

add_executable (compiz_test_expo_click_threshold
		${CMAKE_CURRENT_SOURCE_DIR}/test-expo-click-threshold.cpp)

target_link_libraries (compiz_test_expo_click_threshold
		       compiz_expo_click_threshold
		       ${GTEST_BOTH_LIBRARIES}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
                       )

compiz_discover_tests (compiz_test_expo_click_threshold COVERAGE compiz_expo_click_threshold)
