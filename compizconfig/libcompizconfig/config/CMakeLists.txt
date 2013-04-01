set (_config_file
     config)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config_test
		${CMAKE_CURRENT_BINARY_DIR}/compiz-1/compizconfig/config)

compiz_opt_install_file (${CMAKE_CURRENT_SOURCE_DIR}/${_config_file} ${COMPIZ_SYSCONFDIR}/compizconfig/${_config_file})

add_custom_target (
    config_install
    ${CMAKE_COMMAND} -E make_directory ${CMAKE_ROOT}/Modules &&
    ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/config/${_config_file} ${COMPIZ_SYSCONFDIR}/compizconfig/${_config_file}
    )
