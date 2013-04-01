include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_window_geometry_saver_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-window-geometry-saver.cpp)

add_executable (compiz_test_window_geometry_saver
                ${CMAKE_CURRENT_SOURCE_DIR}/window-geometry-saver/src/test-window-geometry-saver.cpp)

target_link_libraries (compiz_test_window_geometry_saver
                       compiz_window_geometry_saver_test
                       compiz_window_geometry_saver 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_window_geometry_saver COVERAGE compiz_window_geometry_saver)
