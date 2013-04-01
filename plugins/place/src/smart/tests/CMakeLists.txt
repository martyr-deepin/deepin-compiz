include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable (compiz_test_place_smart_on_screen
                ${CMAKE_CURRENT_SOURCE_DIR}/offscreen/src/test-place-smart-on-screen.cpp)

target_link_libraries (compiz_test_place_smart_on_screen
		       compiz_place_smart
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_place_smart_on_screen COVERAGE compiz_place_smart)
