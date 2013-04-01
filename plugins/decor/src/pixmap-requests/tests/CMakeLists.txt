include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable (compiz_test_decor_pixmap_requests
                ${CMAKE_CURRENT_SOURCE_DIR}/pixmap-requests/src/test-decor-pixmap-requests.cpp)

target_link_libraries (compiz_test_decor_pixmap_requests
		       compiz_decor_pixmap_requests
		       decoration
                       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT} # Link in pthread. 
                       )

compiz_discover_tests (compiz_test_decor_pixmap_requests COVERAGE compiz_decor_pixmap_requests)
