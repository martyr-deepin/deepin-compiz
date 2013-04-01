include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../libcompizconfig/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/../mocks/libcompizconfig
                     ${CMAKE_SOURCE_DIR}/tests/shared
                     ${CMAKE_CURRENT_SOURCE_DIR})

add_library (compizconfig_ccs_item_in_list_matcher STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_item_in_list_matcher.cpp)

target_link_libraries (compizconfig_ccs_item_in_list_matcher
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY}
		       ${GMOCK_LIBRARY}
		       compizconfig)

add_library (compizconfig_ccs_list_equality STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_list_equality.cpp)

target_link_libraries (compizconfig_ccs_list_equality
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY}
		       ${GMOCK_LIBRARY}
		       compizconfig)

add_library (compizconfig_ccs_setting_value_matcher STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_setting_value_matcher.cpp)

target_link_libraries (compizconfig_ccs_setting_value_matcher
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY}
		       ${GMOCK_LIBRARY}
		       compizconfig)

add_library (compizconfig_ccs_list_wrapper STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_list_wrapper.cpp)

target_link_libraries (compizconfig_ccs_list_wrapper
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY}
		       ${GMOCK_LIBRARY}
		       compizconfig)

add_library (compizconfig_ccs_setting_value_operators STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_ccs_setting_value_operators.cpp)

target_link_libraries (compizconfig_ccs_setting_value_operators
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY}
		       ${GMOCK_LIBRARY}
		       compizconfig)

add_library (compizconfig_ccs_test_value_combinations STATIC
	     ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_test_value_combiners.cpp)

target_link_libraries (compizconfig_ccs_test_value_combinations
		       ${GTEST_BOTH_LIBRARIES}
		       ${GMOCK_MAIN_LIBRARY}
		       ${GMOCK_LIBRARY}
		       compizconfig)

add_library (compizconfig_ccs_backend_conformance_test STATIC
             ${CMAKE_CURRENT_SOURCE_DIR}/compizconfig_backend_concept_test.cpp)

target_link_libraries (compizconfig_ccs_backend_conformance_test
                       ${GTEST_BOTH_LIBRARIES}
                       ${GMOCK_MAIN_LIBRARY}
                       ${GMOCK_LIBRARY}
                       compizconfig
                       compizconfig_ccs_test_value_combinations
                       compizconfig_ccs_setting_value_operators
                       compizconfig_ccs_list_equality
                       compizconfig_ccs_item_in_list_matcher)
