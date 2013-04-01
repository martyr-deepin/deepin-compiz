include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_window_geometry_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-window-geometry.cpp)

add_executable (compiz_test_window_geometry
                ${CMAKE_CURRENT_SOURCE_DIR}/window-geometry/src/test-window-geometry.cpp)

target_link_libraries (compiz_test_window_geometry
                       compiz_window_geometry_test
                       compiz_window_geometry 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_window_geometry COVERAGE compiz_window_geometry)
