include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable (compiz_test_servergrab
                ${CMAKE_CURRENT_SOURCE_DIR}/test-servergrab.cpp)

target_link_libraries (compiz_test_servergrab
                       compiz_servergrab
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
		      )

compiz_discover_tests (compiz_test_servergrab COVERAGE compiz_servergrab)
