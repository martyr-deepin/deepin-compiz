include (CompizCommon)

function (compiz_package_generation _name)
    include(InstallRequiredSystemLibraries)

    set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "${_name}")
    set (CPACK_PACKAGE_VENDOR "Compiz")
    set (CPACK_PACKAGE_VERSION "${VERSION}")
    set (CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION}")

    set (CPACK_RPM_PACKAGE_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
    set (CPACK_RPM_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
    set (CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
    set (CPACK_RPM_PACKAGE_RELEASE 1)
    set (CPACK_RPM_PACKAGE_LICENCE "GPL")
    set (CPACK_RPM_PACKAGE_GROUP "unknown")
    set (CPACK_RPM_PACKAGE_VENDOR ${CPACK_PACKAGE_VENDOR})
    set (CPACK_RPM_PACKAGE_DESCRIPTION "The blingiest window manager in the world")
    set (CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
    set (CPACK_SOURCE_IGNORE_FILES  "\\\\.#;/#;.*~")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "/\\\\.git")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "${CMAKE_BINARY_DIR}")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "legacy/")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.intltool-merge-cache")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "/po/POTFILES$")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "CMakeCache.txt")
    list (APPEND CPACK_SOURCE_IGNORE_FILES "CMakeFiles")
    include(CPack)

    #file (REMOVE "${CMAKE_BINARY_DIR}/CPackConfig.cmake")
endfunction ()

function (compiz_print_configure_header _name)
    compiz_format_string ("${_name}" 40 _project)
    compiz_format_string ("${VERSION}" 40 _version)
    compiz_color_message ("\n${_escape}[40;37m************************************************************${_escape}[0m")
    compiz_color_message ("${_escape}[40;37m* ${_escape}[1;31mCompiz ${_escape}[0;40;34mBuildsystem${_escape}[0m${_escape}[40;37m                                       *${_escape}[0m")
    compiz_color_message ("${_escape}[40;37m*                                                          *${_escape}[0m")
    compiz_color_message ("${_escape}[40;37m* Package : ${_escape}[32m${_project} ${_escape}[37m      *${_escape}[0m")
    compiz_color_message ("${_escape}[40;37m* Version : ${_escape}[32m${_version} ${_escape}[37m      *${_escape}[0m")
    compiz_color_message ("${_escape}[40;37m************************************************************${_escape}[0m")
endfunction ()

function (compiz_print_configure_footer)
    compiz_color_message ("${_escape}[40;37m************************************************************${_escape}[0m\n")
endfunction ()

function (compiz_print_plugin_stats _folder)
    compiz_color_message ("\n${_escape}[4mPlugin configure check results:${_escape}[0m\n")
    file (
	GLOB _plugins_in 
	RELATIVE "${_folder}"
	"${_folder}/*/CMakeLists.txt"
    )
    foreach (_plugin ${_plugins_in})
	file (READ "${_folder}/${_plugin}" _file)
	if (_file MATCHES "^.*compiz_plugin ?\\(([^\\) ]*).*$")
	    string (
		REGEX REPLACE
		"^.*compiz_plugin ?\\(([^\\) ]*).*$" "\\1"
		_plugin_name ${_file}
	    )
	else ()
	    get_filename_component (_plugin_name ${_plugin} PATH)
	endif ()

	string (TOUPPER ${_plugin_name} _PLUGIN)
	compiz_format_string (${_plugin_name} 14 _plugin_name)

	if (NOT COMPIZ_ENABLED_PLUGIN_${_PLUGIN})
	    compiz_color_message ("  ${_plugin_name}: ${_escape}[1;34mDisabled${_escape}[0m")
	else ()
	    if (COMPIZ_${_PLUGIN}_BUILD)
		compiz_color_message ("  ${_plugin_name}: ${_escape}[1;32mYes${_escape}[0m")
	    else ()
		compiz_color_message ("  ${_plugin_name}: ${_escape}[1;31mNo${_escape}[0m (Missing dependencies :${COMPIZ_${_PLUGIN}_MISSING_DEPS})")
	    endif ()
	endif ()
    endforeach ()
    message ("")
endfunction ()

function (compiz_print_result_message _name _var)
    compiz_format_string ("${_name}" 30 __name)
    if (${_var})
	set (_result "${_escape}[1;32mYes${_escape}[0m")
    else (${_var})
	set (_result "${_escape}[1;31mNo${_escape}[0m")
    endif (${_var})
    compiz_color_message ("   ${__name} : ${_result}")
endfunction (compiz_print_result_message)

function (compiz_get_version_from_file)
    file (READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" _file)
    string (
	REGEX REPLACE
	"^.*VERSION=([^\n]*).*$" "\\1"
	_version ${_file}
    )
    set (VERSION ${_version} PARENT_SCOPE)
endfunction ()
