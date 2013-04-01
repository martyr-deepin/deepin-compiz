include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_window_constrainment_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-window-constrainment.cpp)

add_executable (compiz_test_window_constrainment_to_hints
                ${CMAKE_CURRENT_SOURCE_DIR}/to-hints/src/test-window-constrainment-to-hints.cpp
		${compiz_SOURCE_DIR}/src/size.cpp)

target_link_libraries (compiz_test_window_constrainment_to_hints
                       compiz_window_constrainment_test
                       compiz_window_constrainment 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_window_constrainment_to_hints COVERAGE compiz_window_constrainment)
