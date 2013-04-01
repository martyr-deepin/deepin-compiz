include_directories (${GTEST_INCLUDE_DIRS})
include_directories (${CMAKE_SOURCE_DIR}/include)
include_directories (${CMAKE_SOURCE_DIR}/tests/shared)
include_directories (${CMAKE_SOURCE_DIR}/tests/shared/glib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../mocks/libcompizconfig)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gsettings/tests)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../../gsettings/mocks)
include_directories (${CMAKE_SOURCE_DIR}/compizconfig/tests)
link_directories (${CMAKE_INSTALL_PREFIX}/lib)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../../mocks/libcompizconfig)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../../tests/)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../src/)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable (compizconfig_test_ccs_gnome_gsettings_integrated_setting
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_gnome_gsettings_integrated_setting.cpp)

target_link_libraries (compizconfig_test_ccs_gnome_gsettings_integrated_setting
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_gsettings_settings_test_env
		       compizconfig_ccs_gsettings_wrapper_mock
                       compizconfig_gnome_integration_gsettings_integrated_setting
                       compizconfig_gsettings_backend_interface)

compiz_discover_tests (compizconfig_test_ccs_gnome_gsettings_integrated_setting COVERAGE compizconfig_gnome_gsettings_integrated_setting)
