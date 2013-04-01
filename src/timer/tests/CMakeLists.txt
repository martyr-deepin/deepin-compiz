include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_timer_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-timer.cpp)

add_executable (compiz_timer_callbacks
                ${CMAKE_CURRENT_SOURCE_DIR}/callbacks/src/test-timer-callbacks.cpp)

add_executable (compiz_timer_diffs
                ${CMAKE_CURRENT_SOURCE_DIR}/diffs/src/test-timer-diffs.cpp)

add_executable (compiz_timer_set-values
                ${CMAKE_CURRENT_SOURCE_DIR}/set-values/src/test-timer-set-values.cpp)

add_executable (compiz_timer_while-calling
                ${CMAKE_CURRENT_SOURCE_DIR}/while-calling/src/test-timer-set-times-while-calling.cpp)

target_link_libraries (compiz_timer_callbacks 
                       compiz_timer_test
                       compiz_timer 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

target_link_libraries (compiz_timer_diffs
                       compiz_timer_test
                       compiz_timer 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

target_link_libraries (compiz_timer_set-values
                       compiz_timer_test
                       compiz_timer 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

target_link_libraries (compiz_timer_while-calling
                       compiz_timer_test
                       compiz_timer 
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_timer_callbacks COVERAGE compiz_timer)
compiz_discover_tests (compiz_timer_diffs COVERAGE compiz_timer)
compiz_discover_tests (compiz_timer_set-values COVERAGE compiz_timer)
compiz_discover_tests (compiz_timer_while-calling COVERAGE compiz_timer)
