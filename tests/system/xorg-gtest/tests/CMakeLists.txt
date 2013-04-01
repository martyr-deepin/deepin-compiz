pkg_check_modules (X11_XI x11 xi)

if (BUILD_XORG_GTEST AND X11_XI_FOUND)

    include_directories (${COMPIZ_XORG_SYSTEM_TEST_INCLUDE_DIR}
			 ${X11_INCLUDE_DIRS}
			 ${XORG_SERVER_INCLUDE_XORG_GTEST}
			 ${XORG_SERVER_GTEST_SRC}
			 ${GTEST_INCLUDE_DIRS})

    link_directories (${X11_XI_LIBRARY_DIRS})

    add_executable (compiz_xorg_gtest_test_window_stacking
		    ${CMAKE_CURRENT_SOURCE_DIR}/compiz_xorg_gtest_test_window_stacking.cpp)

    target_link_libraries (compiz_xorg_gtest_test_window_stacking
			   compiz_xorg_gtest_system_test
			   xorg_gtest_all
			   xorg_gtest_main
			   ${GTEST_BOTH_LIBRARIES}
			   ${CMAKE_THREAD_LIBS_INIT}
			   ${XORG_SERVER_LIBRARIES}
			   ${X11_XI_LIBRARIES})

    compiz_discover_tests (compiz_xorg_gtest_test_window_stacking)

endif (BUILD_XORG_GTEST AND X11_XI_FOUND)
