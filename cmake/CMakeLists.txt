compiz_configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/CompizDefaults.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CompizDefaults.cmake
)

set (_files
    CompizCommon.cmake
    CompizPlugin.cmake
    CompizPackage.cmake
    CompizBcop.cmake
    copy_file_install_user_env.cmake
    recompile_gsettings_schemas_in_dir_user_env.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CompizDefaults.cmake
)

list (APPEND _PluginExtensionFiles
      plugin_extensions/CompizGenInstallData.cmake)
list (APPEND _PluginExtensionFiles
      plugin_extensions/CompizGenInstallImages.cmake)
list (APPEND _PluginExtensionFiles
      plugin_extensions/CompizOpenGLFixups.cmake)

if (USE_GCONF)
    list (APPEND _files CompizGconf.cmake)
    list (APPEND _PluginExtensionFiles plugin_extensions/CompizGenGconf.cmake)
endif (USE_GCONF)

if (USE_GSETTINGS)
    list (APPEND _files CompizGSettings.cmake)
    list (APPEND _PluginExtensionFiles plugin_extensions/CompizGenGSettings.cmake)
endif (USE_GSETTINGS)

install (
    FILES ${_files}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/compiz/cmake
)

install (
    FILES ${_PluginExtensionFiles}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/compiz/cmake/plugin_extensions
)

# Do not install src/compiz for a while until we've polished it up

#install (DIRECTORY src/compiz
#	 DESTINATION ${CMAKE_INSTALL_PREFIX}/src)

if (COMPIZ_BUILD_TESTING)
    add_subdirectory (src)
endif (COMPIZ_BUILD_TESTING)
