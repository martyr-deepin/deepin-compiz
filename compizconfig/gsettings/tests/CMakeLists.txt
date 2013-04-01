include (CompizGSettings)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}
		     ${CMAKE_CURRENT_BINARY_DIR}
		     ${CMAKE_CURRENT_SOURCE_DIR}/../mocks
		     ${CMAKE_CURRENT_SOURCE_DIR}/../testenvironments
		     ${CMAKE_CURRENT_SOURCE_DIR}/../src
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../libcompizconfig/tests
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../mocks/libcompizconfig
		     ${CMAKE_CURRENT_SOURCE_DIR}/../../tests
                     ${CMAKE_SOURCE_DIR}/tests/shared
                     ${CMAKE_SOURCE_DIR}/tests/shared/glib)

pkg_check_modules (COMPIZCONFIG_TEST_GSETTINGS libcompizconfig_internal)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

link_directories (${CMAKE_CURRENT_BINARY_DIR}/../../libcompizconfig/tests)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../testenvironments)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../mocks)

add_library (compizconfig_gsettings_settings_test_env STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_gsettings_settings_env_test.cpp)

add_executable (compizconfig_test_gsettings
		${CMAKE_CURRENT_SOURCE_DIR}/test_gsettings_tests.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/test_gsettings_wrapper.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/test_gsettings.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/test_gsettings_tests.h)

add_executable (compizconfig_test_gsettings_conformance
		${CMAKE_CURRENT_SOURCE_DIR}/test_gsettings_conformance.cpp)

target_link_libraries (compizconfig_test_gsettings
                       compizconfig_gsettings_backend
		       compizconfig_ccs_setting_mock
		       compizconfig_ccs_plugin_mock
                       compizconfig_ccs_context_mock
		       compizconfig_ccs_gsettings_backend_mock
		       compizconfig_ccs_gsettings_wrapper_mock
                       compizconfig_ccs_test_value_combinations
		       ${COMPIZCONFIG_TEST_GSETTINGS_LIBRARIES}
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/gsettings-mock-schemas-config.h.in
		${CMAKE_CURRENT_BINARY_DIR}/gsettings-mock-schemas-config.h)

add_custom_command (OUTPUT ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/org.compiz.mock.gschema.xml
		    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/org.compiz.mock.gschema.xml ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/org.compiz.mock.gschema.xml
		    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/org.compiz.mock.gschema.xml
		    VERBATIM)

add_custom_target (compiz_gsettings_mock_schema ALL
		   DEPENDS ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/org.compiz.mock.gschema.xml)

add_gsettings_schema_to_recompilation_list (compiz_gsettings_mock_schema)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/backend-conformance-config.h.in
		${CMAKE_CURRENT_BINARY_DIR}/backend-conformance-config.h)

target_link_libraries (compizconfig_test_gsettings_conformance
		       compizconfig_gsettings_settings_test_env
                       compizconfig_gsettings_backend
		       compizconfig_ccs_setting_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_context_mock
                       compizconfig_ccs_item_in_list_matcher
                       compizconfig_ccs_list_equality
                       compizconfig_ccs_list_wrapper
                       compizconfig_ccs_backend_conformance_test
                       compizconfig_ccs_setting_value_operators
                       compizconfig_ccs_test_value_combinations
		       compizconfig_ccs_integration_mock
		       ${COMPIZCONFIG_TEST_GSETTINGS_LIBRARIES}
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY})

compiz_discover_tests (compizconfig_test_gsettings COVERAGE compizconfig_gsettings_backend)
compiz_discover_tests (compizconfig_test_gsettings_conformance COVERAGE compizconfig_gsettings_backend gsettings)
