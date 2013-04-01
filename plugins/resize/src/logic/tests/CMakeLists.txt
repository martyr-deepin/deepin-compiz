find_library (GMOCK_LIBRARY gmock)
find_library (GMOCK_MAIN_LIBRARY gmock_main)

if (NOT GMOCK_LIBRARY OR NOT GMOCK_MAIN_LIBRARY OR NOT GTEST_FOUND)
  message ("Google Mock and Google Test not found - cannot build tests!")
  set (COMPIZ_BUILD_TESTING OFF)
endif (NOT GMOCK_LIBRARY OR NOT GMOCK_MAIN_LIBRARY OR NOT GTEST_FOUND)

include_directories (${GTEST_INCLUDE_DIRS})

link_directories (${COMPIZ_LIBRARY_DIRS})

add_executable (compiz_test_resize_logic
                test-logic.cpp
		resize-options-mock.cpp
		x11-mocks.cpp)

target_link_libraries (compiz_test_resize_logic
                       resize_logic
                       compiz_core
                       ${GTEST_BOTH_LIBRARIES}
                       ${GMOCK_LIBRARY}
                       ${GMOCK_MAIN_LIBRARY}
                       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread.
                       )

compiz_discover_tests (compiz_test_resize_logic COVERAGE resize_logic)
