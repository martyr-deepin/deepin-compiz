include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_point_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-point.cpp)

add_executable (compiz_test_point
                ${CMAKE_CURRENT_SOURCE_DIR}/point/src/test-point.cpp)

target_link_libraries (compiz_test_point
                       compiz_point_test
                       compiz_point 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_point COVERAGE compiz_point)
