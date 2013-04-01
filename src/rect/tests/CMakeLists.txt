include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_rect_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-rect.cpp)

add_executable (compiz_test_rect
                ${CMAKE_CURRENT_SOURCE_DIR}/rect/src/test-rect.cpp)

add_executable (compiz_test_rect_wraparound_point
                ${CMAKE_CURRENT_SOURCE_DIR}/wraparound_point/src/test-rect-wraparound-point.cpp)

target_link_libraries (compiz_test_rect
                       compiz_rect_test
                       compiz_rect 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

target_link_libraries (compiz_test_rect_wraparound_point
                       compiz_rect_test
                       compiz_rect 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_rect COVERAGE compiz_rect)
compiz_discover_tests (compiz_test_rect_wraparound_point COVERAGE compiz_rect)
