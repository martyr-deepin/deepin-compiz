include_directories (${COMPIZ_XIG_TEST_INCLUDE_DIRS}
		     ${CMAKE_CURRENT_BINARY_DIR})
link_directories (${COMPIZ_XIG_TEST_LIBRARY_DIRS})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
		${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_executable (compiz-xig-test-runner
		test-runner.c)

target_link_libraries (compiz-xig-test-runner
		       ${COMPIZ_XIG_TEST_LIBRARIES})
