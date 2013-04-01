include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable (compiz_test_decor_clip_groups
                ${CMAKE_CURRENT_SOURCE_DIR}/clip-groups/src/test-decor-clip-groups.cpp)

target_link_libraries (compiz_test_decor_clip_groups
		       compiz_decor_clip_groups
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_decor_clip_groups COVERAGE compiz_decor_clip_groups)
