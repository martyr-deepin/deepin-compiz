include_directories (${GTEST_INCLUDE_DIRS})
include_directories (${CMAKE_SOURCE_DIR}/include)
include_directories (${CMAKE_SOURCE_DIR}/tests/shared)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../mocks/libcompizconfig)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories (${CMAKE_SOURCE_DIR}/compizconfig/tests)
link_directories (${CMAKE_INSTALL_PREFIX}/lib)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../../mocks/libcompizconfig)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../../tests/)
link_directories (${CMAKE_CURRENT_BINARY_DIR}/../src/)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable (compizconfig_test_ccs_object
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_object.cpp)

add_executable (compizconfig_test_ccs_context
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_context.cpp)

add_executable (compizconfig_test_ccs_plugin
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_plugin.cpp)

add_executable (compizconfig_test_ccs_setting
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_setting.cpp)

add_executable (compizconfig_test_ccs_backend
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_backend.cpp)

add_executable (compizconfig_test_ccs_text_file
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_text_file.cpp)

add_executable (compizconfig_test_ccs_mock_backend_conformance
		${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_mock_backend_conformance.cpp)

add_executable (compizconfig_test_ccs_upgrade_internal
    ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_ccs_settings_upgrade_internal.cpp)

if (HAVE_PROTOBUF)
    set (LIBCOMPIZCONFIG_LIBRARIES
	 ${LIBCOMPIZCONFIG_LIBRARIES}
	 protobuf)
endif (HAVE_PROTOBUF)

target_link_libraries (compizconfig_test_ccs_object
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig)

target_link_libraries (compizconfig_test_ccs_context
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_ccs_context_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_setting_mock)

target_link_libraries (compizconfig_test_ccs_plugin
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_ccs_context_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_setting_mock)

target_link_libraries (compizconfig_test_ccs_backend
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_ccs_context_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_setting_mock
		       compizconfig_ccs_backend_mock)

target_link_libraries (compizconfig_test_ccs_mock_backend_conformance
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_ccs_context_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_setting_mock
                       compizconfig_ccs_backend_mock
                       compizconfig_ccs_backend_conformance_test
                       compizconfig_ccs_item_in_list_matcher
                       compizconfig_ccs_list_equality
                       compizconfig_ccs_list_wrapper
                       compizconfig_ccs_setting_value_operators
                       compizconfig_ccs_test_value_combinations)

target_link_libraries (compizconfig_test_ccs_setting
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       compizconfig
		       compizconfig_ccs_context_mock
		       compizconfig_ccs_plugin_mock
		       compizconfig_ccs_setting_mock)

target_link_libraries (compizconfig_test_ccs_text_file
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
		       ${LIBCOMPIZCONFIG_LIBRARIES}
		       ccs_text_file_interface
                       compizconfig_ccs_text_file_mock
                       compizconfig)

target_link_libraries (compizconfig_test_ccs_upgrade_internal
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_LIBRARY}
		       ${GMOCK_MAIN_LIBRARY}
		       ${CMAKE_THREAD_LIBS_INIT}
                       ccs_settings_upgrade_internal
                       compizconfig
                       compizconfig_ccs_context_mock
                       compizconfig_ccs_plugin_mock
                       compizconfig_ccs_setting_mock
                       compizconfig_ccs_item_in_list_matcher
		       compizconfig_ccs_setting_value_matcher
)

compiz_discover_tests (compizconfig_test_ccs_object COVERAGE compizconfig)
compiz_discover_tests (compizconfig_test_ccs_context COVERAGE compizconfig_ccs_context_mock)
compiz_discover_tests (compizconfig_test_ccs_plugin COVERAGE compizconfig_ccs_plugin_mock)
compiz_discover_tests (compizconfig_test_ccs_setting COVERAGE compizconfig_ccs_setting_mock)
compiz_discover_tests (compizconfig_test_ccs_mock_backend_conformance COVERAGE compizconfig_ccs_backend_mock)
compiz_discover_tests (compizconfig_test_ccs_text_file COVERAGE ccs_text_file_interface compizconfig_ccs_text_file_mock)
compiz_discover_tests (compizconfig_test_ccs_upgrade_internal COVERAGE ccs_settings_upgrade_internal)
