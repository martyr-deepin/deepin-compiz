include (CompizCommon)
include (FindPkgConfig)

pkg_check_modules (COMPIZ_XIG_TEST xig-0 glib-2.0 gobject-2.0)

if (COMPIZ_XIG_TEST_FOUND)

	compiz_set (COMPIZ_XIG_TEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	compiz_set (COMPIZ_XIG_TEST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

	add_subdirectory (src)

	add_test (compiz-xig-test-startup
		  ${CMAKE_CURRENT_BINARY_DIR}/src/compiz-xig-test-runner startup)

	add_test (compiz-xig-test-restart
		  ${CMAKE_CURRENT_BINARY_DIR}/src/compiz-xig-test-runner restart)

	add_test (compiz-xig-test-xserver-quit
		  ${CMAKE_CURRENT_BINARY_DIR}/src/compiz-xig-test-runner xserver-quit)

	add_test (compiz-xig-test-new-window
		  ${CMAKE_CURRENT_BINARY_DIR}/src/compiz-xig-test-runner new-window)

	add_test (compiz-xig-test-existing-window
		  ${CMAKE_CURRENT_BINARY_DIR}/src/compiz-xig-test-runner existing-window)

else (COMPIZ_XIG_TEST_FOUND)

	message (WARNING "Xig not found, you will not be able to run X Server integration tests")
endif (COMPIZ_XIG_TEST_FOUND)
