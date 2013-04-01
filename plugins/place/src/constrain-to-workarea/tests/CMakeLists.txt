include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library (compiz_place_constrain_to_workarea_test
             ${CMAKE_CURRENT_SOURCE_DIR}/test-constrain-to-workarea.cpp)

add_executable (compiz_test_place_constrain_to_workarea
                ${CMAKE_CURRENT_SOURCE_DIR}/constrain-to-workarea/src/test-place-constrain-to-workarea.cpp)

target_link_libraries (compiz_test_place_constrain_to_workarea
                       compiz_place_constrain_to_workarea_test
                       compiz_place_constrain_to_workarea
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_place_constrain_to_workarea COVERAGE compiz_place_constrain_to_workarea)
