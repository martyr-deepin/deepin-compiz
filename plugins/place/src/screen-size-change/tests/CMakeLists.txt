include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_place_screen_size_change_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-screen-size-change.cpp)

add_executable (compiz_test_place_screen_size_change
                ${CMAKE_CURRENT_SOURCE_DIR}/screen-size-change/src/test-place-screen-size-change.cpp)

target_link_libraries (compiz_test_place_screen_size_change
                       compiz_place_screen_size_change_test
                       compiz_place_screen_size_change
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_place_screen_size_change COVERAGE compiz_place_screen_size_change)
