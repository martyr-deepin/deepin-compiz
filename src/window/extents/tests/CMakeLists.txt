include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_window_extents_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-window-extents.cpp)

add_executable (compiz_test_window_extents_shift
                ${CMAKE_CURRENT_SOURCE_DIR}/shift/src/test-window-extents-shift.cpp)

target_link_libraries (compiz_test_window_extents_shift
                       compiz_window_extents_test
                       compiz_window_extents 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_window_extents_shift COVERAGE compiz_window_extents)
