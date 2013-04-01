include_directories (${GTEST_INCLUDE_DIRS})
include_directories (${CMAKE_SOURCE_DIR}/include)
include_directories (${CMAKE_SOURCE_DIR}/tests/shared)
include_directories (${CMAKE_SOURCE_DIR}/tests/shared/glib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../mocks/libcompizconfig)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gsettings/tests)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../gsettings/mocks)
include_directories (${CMAKE_SOURCE_DIR}/compizconfig/tests)
link_directories (${CMAKE_INSTALL_PREFIX}/lib)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_library (compizconfig_ccs_mock_gnome_integrated_setting_composition
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_mock_gnome_integrated_setting_composition.c)

target_link_libraries (compizconfig_ccs_mock_gnome_integrated_setting_composition
		       compizconfig_ccs_integrated_setting_mock
		       compizconfig_gnome_integrated_setting
		       compizconfig)

add_executable (compizconfig_test_ccs_gnome_integration
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_gnome_integration.cpp)

target_link_libraries (compizconfig_test_ccs_gnome_integration
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_ccs_context_mock
		       compizconfig_ccs_backend_mock
		       compizconfig_ccs_setting_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_list_wrapper
		       compizconfig_ccs_setting_value_operators
		       compizconfig_ccs_setting_value_matcher
		       compizconfig_ccs_integrated_setting_mock
		       compizconfig_ccs_integrated_setting_factory_mock
		       compizconfig_ccs_integrated_setting_storage_mock
		       compizconfig_ccs_mock_gnome_integrated_setting_composition
		       compizconfig_gnome_integration)

compiz_discover_tests (compizconfig_test_ccs_gnome_integration
		       COVERAGE
		       compizconfig_gnome_integration
		       compizconfig_gnome_integration_constants)
