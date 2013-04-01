cmake_minimum_required (VERSION 2.6)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message (SEND_ERROR "Building in the source directory is not supported.")
    message (FATAL_ERROR "Please remove the created \"CMakeCache.txt\" file, the \"CMakeFiles\" directory and create a build directory and call \"${CMAKE_COMMAND} <path to the sources>\".")
endif ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")

#### CTest
enable_testing()

#### policies

cmake_policy (SET CMP0000 OLD)
cmake_policy (SET CMP0002 OLD)
cmake_policy (SET CMP0003 NEW)
cmake_policy (SET CMP0005 OLD)
cmake_policy (SET CMP0011 OLD)

set (CMAKE_SKIP_RPATH FALSE)

option (BUILD_GLES "Build against GLESv2 instead of GL" OFF)
option (COMPIZ_BUILD_WITH_RPATH "Leave as ON unless building packages" ON)
option (COMPIZ_RUN_LDCONFIG "Leave OFF unless you need to run ldconfig after install")
option (COMPIZ_PACKAGING_ENABLED "Enable to manually set prefix, exec_prefix, libdir, includedir, datadir" OFF)
option (COMPIZ_BUILD_TESTING "Build Unit Tests" ON)

set (COMPIZ_DATADIR ${CMAKE_INSTALL_PREFIX}/share)
set (COMPIZ_METADATADIR ${CMAKE_INSTALL_PREFIX}/share/compiz)
set (COMPIZ_IMAGEDIR ${CMAKE_INSTALL_PREFIX}/share/compiz/images)
set (COMPIZ_PLUGINDIR ${libdir}/compiz)
set (COMPIZ_SYSCONFDIR ${sysconfdir})

set (
    VERSION ${VERSION} CACHE STRING
    "Package version that is added to a plugin pkg-version file"
)

set (
    COMPIZ_I18N_DIR ${COMPIZ_I18N_DIR} CACHE PATH "Translation file directory"
)

# Almost everything is a shared library now, so almost everything needs -fPIC
set (COMMON_FLAGS "-fPIC -Wall -Wl,-zdefs")

option (COMPIZ_DEPRECATED_WARNINGS "Warn about declarations marked deprecated" OFF)
if (NOT COMPIZ_DEPRECATED_WARNINGS)
    set (COMMON_FLAGS "${COMMON_FLAGS} -Wno-deprecated-declarations")
endif ()

option (COMPIZ_SIGN_WARNINGS "Should compiz use -Wsign-conversion during compilation." ON)
if (NOT COMPIZ_SIGN_WARNINGS)
    set (COMMON_FLAGS "${COMMON_FLAGS} -Wno-sign-conversion")
endif ()

if (${CMAKE_PROJECT_NAME} STREQUAL "compiz")
    set (COMPIZ_WERROR_DEFAULT ON)
else ()
    set (COMPIZ_WERROR_DEFAULT OFF)
endif ()
option (COMPIZ_WERROR "Treat warnings as errors" ${COMPIZ_WERROR_DEFAULT})
if (COMPIZ_WERROR)
    set (COMMON_FLAGS "${COMMON_FLAGS} -Werror")
endif ()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")

if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.bzr)
    set(IS_BZR_REPO 1)
elseif (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.bzr)
    set(IS_BZR_REPO 0)
endif (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.bzr)

set (USE_GLES ${BUILD_GLES})

if (USE_GLES)
    find_package(OpenGLES2)

    if (NOT OPENGLES2_FOUND)
	set (USE_GLES 0)
	message (SEND_ERROR "OpenGLESv2 not found")
    endif (NOT OPENGLES2_FOUND)
endif (USE_GLES)

# Parse arguments passed to a function into several lists separated by
# upper-case identifiers and options that do not have an associated list e.g.:
#
# SET(arguments
#   hello OPTION3 world
#   LIST3 foo bar
#   OPTION2
#   LIST1 fuz baz
#   )
# PARSE_ARGUMENTS(ARG "LIST1;LIST2;LIST3" "OPTION1;OPTION2;OPTION3" ${arguments})
#
# results in 7 distinct variables:
#  * ARG_DEFAULT_ARGS: hello;world
#  * ARG_LIST1: fuz;baz
#  * ARG_LIST2:
#  * ARG_LIST3: foo;bar
#  * ARG_OPTION1: FALSE
#  * ARG_OPTION2: TRUE
#  * ARG_OPTION3: TRUE
#
# taken from http://www.cmake.org/Wiki/CMakeMacroParseArguments 

MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
    SET(DEFAULT_ARGS)
    FOREACH(arg_name ${arg_names})    
        SET(${prefix}_${arg_name})
    ENDFOREACH(arg_name)
    FOREACH(option ${option_names})
        SET(${prefix}_${option} FALSE)
    ENDFOREACH(option)
    
    SET(current_arg_name DEFAULT_ARGS)
    SET(current_arg_list)
    FOREACH(arg ${ARGN})            
        SET(larg_names ${arg_names})    
        LIST(FIND larg_names "${arg}" is_arg_name)                   
        IF (is_arg_name GREATER -1)
            SET(${prefix}_${current_arg_name} ${current_arg_list})
            SET(current_arg_name ${arg})
            SET(current_arg_list)
        ELSE (is_arg_name GREATER -1)
            SET(loption_names ${option_names})    
            LIST(FIND loption_names "${arg}" is_option)            
            IF (is_option GREATER -1)
                SET(${prefix}_${arg} TRUE)
            ELSE (is_option GREATER -1)
                SET(current_arg_list ${current_arg_list} ${arg})
            ENDIF (is_option GREATER -1)
        ENDIF (is_arg_name GREATER -1)
    ENDFOREACH(arg)
    SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

function (compiz_add_to_coverage_report TARGET TEST)

    set_property (GLOBAL APPEND PROPERTY
		  COMPIZ_COVERAGE_REPORT_TARGETS
		  ${TARGET})

    set_property (GLOBAL APPEND PROPERTY
		  COMPIZ_COVERAGE_REPORT_TESTS
		  ${TARGET})

endfunction ()

function (compiz_add_test_to_testfile CURRENT_BINARY_DIR TEST)

    message (STATUS "Will discover tests in ${TEST}")

    set (INCLUDE_STR "INCLUDE (${CURRENT_BINARY_DIR}/${TEST}_test.cmake) \n")
    set_property (GLOBAL
		  APPEND
		  PROPERTY COMPIZ_TEST_INCLUDE_FILES
		  ${INCLUDE_STR})

endfunction (compiz_add_test_to_testfile)

function (compiz_generate_testfile_target)

    # Adding a rule for the toplevel CTestTestfile.cmake
    # will cause enable_testing not to generate the file
    # for this directory, so we need to do some of the work
    # that command did for us

    file (WRITE ${CMAKE_BINARY_DIR}/CompizCTestTestfile.cmake "")

    file (GLOB ALL_DIRS "*")

    foreach (DIR ${ALL_DIRS})
	if (IS_DIRECTORY ${DIR} AND NOT ${DIR} STREQUAL ${CMAKE_BINARY_DIR})
	    file (RELATIVE_PATH RDIR ${CMAKE_CURRENT_SOURCE_DIR} ${DIR})
	    file (APPEND ${compiz_BINARY_DIR}/CompizCTestTestfile.cmake
	          "SUBDIRS (${RDIR})\n")
	endif (IS_DIRECTORY ${DIR} AND NOT ${DIR} STREQUAL ${CMAKE_BINARY_DIR})
    endforeach ()

    get_property (COMPIZ_TEST_INCLUDE_FILES_SET
		  GLOBAL PROPERTY COMPIZ_TEST_INCLUDE_FILES
		  SET)

    if (NOT COMPIZ_TEST_INCLUDE_FILES_SET)
	message (WARNING "No tests were added for discovery, not generating CTestTestfile.cmake rule")
    endif (NOT COMPIZ_TEST_INCLUDE_FILES_SET)

    get_property (COMPIZ_TEST_INCLUDE_FILES
		  GLOBAL PROPERTY COMPIZ_TEST_INCLUDE_FILES)

    foreach (INCLUDEFILE ${COMPIZ_TEST_INCLUDE_FILES})
	file (APPEND ${compiz_BINARY_DIR}/CompizCTestTestfile.cmake ${INCLUDEFILE})
    endforeach ()

    # Overwrite any existing CTestTestfile.cmake - we cannot use
    # configure_file as enable_testing () will clobber the result

    add_custom_command (OUTPUT ${CMAKE_BINARY_DIR}/CTestTestfileValid
		        COMMAND cat ${CMAKE_BINARY_DIR}/CompizCTestTestfile.cmake > ${CMAKE_BINARY_DIR}/CTestTestfile.cmake && touch ${CMAKE_BINARY_DIR}/CTestTestfileValid
		        COMMENT "Generating CTestTestfile.cmake"
		        VERBATIM)

    add_custom_target (compiz_generate_ctest_testfile ALL
		       DEPENDS ${CMAKE_BINARY_DIR}/CTestTestfileValid)

    # Invalidate the CTestTestfile.cmake
    if (EXISTS ${CMAKE_BINARY_DIR}/CTestTestfileValid)
	execute_process (COMMAND rm ${CMAKE_BINARY_DIR}/CTestTestfileValid)
    endif (EXISTS ${CMAKE_BINARY_DIR}/CTestTestfileValid)
endfunction (compiz_generate_testfile_target)

# Create target to discover tests
function (compiz_discover_tests EXECUTABLE)

    string (TOLOWER "${CMAKE_BUILD_TYPE}" COVERAGE_BUILD_TYPE)
    if (${COVERAGE_BUILD_TYPE} MATCHES "coverage")
	parse_arguments (ARG "COVERAGE" "" ${ARGN})

	foreach (COVERAGE ${ARG_COVERAGE})
	    compiz_add_to_coverage_report (${COVERAGE} ${EXECUTABLE})
	endforeach ()
    endif (${COVERAGE_BUILD_TYPE} MATCHES "coverage")

    add_custom_command (TARGET ${EXECUTABLE}
			POST_BUILD
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE} --gtest_list_tests | ${CMAKE_BINARY_DIR}/compiz_gtest/compiz_discover_gtest_tests ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Discovering Tests in ${EXECUTABLE}"
			VERBATIM)

    add_dependencies (${EXECUTABLE}
		      compiz_discover_gtest_tests)

    compiz_add_test_to_testfile (${CMAKE_CURRENT_BINARY_DIR} ${EXECUTABLE})

endfunction (compiz_discover_tests)

function (compiz_ensure_linkage)
    find_program (LDCONFIG_EXECUTABLE ldconfig)
    mark_as_advanced (FORCE LDCONFIG_EXECUTABLE)

    if (LDCONFIG_EXECUTABLE AND ${COMPIZ_RUN_LDCONFIG})

    install (
        CODE "message (\"Running \" ${LDCONFIG_EXECUTABLE} \" \" ${CMAKE_INSTALL_PREFIX} \"/lib\")
	      exec_program (${LDCONFIG_EXECUTABLE} ARGS \"-v\" ${CMAKE_INSTALL_PREFIX}/lib)"
        )

    endif (LDCONFIG_EXECUTABLE AND ${COMPIZ_RUN_LDCONFIG})
endfunction ()

macro (compiz_add_git_dist)

	add_custom_target (dist
			   COMMAND bzr export --root=${CMAKE_PROJECT_NAME}-${VERSION} ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2
			   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endmacro ()

macro (compiz_add_distcheck)
	add_custom_target (distcheck 
			   COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist-build
			   && cp ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2 ${CMAKE_BINARY_DIR}/dist-build
			   && cd ${CMAKE_BINARY_DIR}/dist-build
			   && tar xvf ${CMAKE_BINARY_DIR}/dist-build/${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2
			   && mkdir -p ${CMAKE_BINARY_DIR}/dist-build/${CMAKE_PROJECT_NAME}-${VERSION}/build
			   && cd ${CMAKE_BINARY_DIR}/dist-build/${CMAKE_PROJECT_NAME}-${VERSION}/build
			   && cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dist-build/buildroot -DCOMPIZ_PLUGIN_INSTALL_TYPE='package' .. -DCMAKE_MODULE_PATH=/usr/share/cmake -DCOMPIZ_DISABLE_PLUGIN_KDE=ON -DBUILD_KDE4=OFF
			   && make
			   && make test
			   && make install
			   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	add_dependencies (distcheck dist)
endmacro ()

macro (compiz_add_release_signoff)

	set (AUTO_VERSION_UPDATE "" CACHE STRING "Automatically update VERSION to this number")

	if (AUTO_VERSION_UPDATE)
		message ("-- Next version will be " ${AUTO_VERSION_UPDATE})
	endif (AUTO_VERSION_UPDATE)

	add_custom_target (release-signoff)

	add_custom_target (release-update-working-tree
			   COMMAND cp NEWS ${CMAKE_SOURCE_DIR} && bzr add ${CMAKE_SOURCE_DIR}/NEWS &&
				   cp AUTHORS ${CMAKE_SOURCE_DIR} && bzr add ${CMAKE_SOURCE_DIR}/AUTHORS
			   COMMENT "Updating working tree"
			   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}) 

	# TODO
	add_custom_target (release-commits)
	add_custom_target (release-tags)
	add_custom_target (release-branch)
	add_custom_target (release-update-dist)
	add_custom_target (release-version-bump)

	add_custom_target (release-sign-tarballs
		   COMMAND gpg --armor --sign --detach-sig ${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2
	   COMMENT "Signing tarball"
	   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
	add_custom_target (release-sha1-tarballs
		   COMMAND sha1sum ${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2 > ${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2.sha1
		   COMMENT "SHA1Summing tarball"
		   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
	add_custom_target (release-sign-sha1-tarballs
		   COMMAND gpg --armor --sign --detach-sig ${CMAKE_PROJECT_NAME}-${VERSION}.tar.bz2.sha1
		   COMMENT "Signing SHA1Sum checksum"
		   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

	add_dependencies (release-commits release-update-working-tree)
	add_dependencies (release-tags release-commits)
	add_dependencies (release-branch release-tags)
	add_dependencies (release-update-dist release-branch)
	add_dependencies (release-version-bump release-update-dist)
	add_dependencies (release-sign-tarballs release-version-bump)
	add_dependencies (release-sha1-tarballs release-sign-tarballs)
	add_dependencies (release-sign-sha1-tarballs release-sha1-tarballs)

	add_dependencies (release-signoff release-sign-sha1-tarballs)

	# Actually pushes the release
	add_custom_target (push-master)
	add_custom_target (push-release-branch)
	add_custom_target (push-tag)

	add_custom_target (release-push)

	add_dependencies (release-push push-release-branch)
	add_dependencies (push-release-branch push-tag)
	add_dependencies (push-tag push-master)

	# Push the tarball to releases.compiz.org

	# Does nothing for now
	add_custom_target (release-upload-component)
	add_custom_target (release-upload)

	add_dependencies (release-upload-component release-upload-version)
	add_dependencies (release-upload release-upload-component)

endmacro ()

macro (compiz_add_release)

	set (AUTO_NEWS_UPDATE "" CACHE STRING "Value to insert into NEWS file, leave blank to get an editor when running make news-update")

	if (AUTO_NEWS_UPDATE)
		message ("-- Using auto news update: " ${AUTO_NEWS_UPDATE})
	endif (AUTO_NEWS_UPDATE)

	if (NOT EXISTS ${CMAKE_SOURCE_DIR}/.AUTHORS.sed)
		file (WRITE ${CMAKE_SOURCE_DIR}/.AUTHORS.sed "")
	endif (NOT EXISTS ${CMAKE_SOURCE_DIR}/.AUTHORS.sed)

	add_custom_target (authors
			   COMMAND bzr log --long --levels=0 | grep -e "^\\s*author:" -e "^\\s*committer:" | cut -d ":" -f 2 | sed -r -f ${CMAKE_SOURCE_DIR}/.AUTHORS.sed  | sort -u > AUTHORS
			   COMMENT "Generating AUTHORS")

	if (AUTO_NEWS_UPDATE)

		add_custom_target (news-header echo > ${CMAKE_BINARY_DIR}/NEWS.update
				   COMMAND echo 'Release ${VERSION} ('`date +%Y-%m-%d`' '`bzr config email`')' > ${CMAKE_BINARY_DIR}/NEWS.update && seq -s "=" `cat ${CMAKE_BINARY_DIR}/NEWS.update | wc -c` | sed 's/[0-9]//g' >> ${CMAKE_BINARY_DIR}/NEWS.update && echo '${AUTO_NEWS_UPDATE}' >> ${CMAKE_BINARY_DIR}/NEWS.update && echo >> ${CMAKE_BINARY_DIR}/NEWS.update
				   COMMENT "Generating NEWS Header"
				   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	else (AUTO_NEWS_UPDATE)
		add_custom_target (news-header echo > ${CMAKE_BINARY_DIR}/NEWS.update
				   COMMAND echo 'Release ${VERSION} ('`date +%Y-%m-%d`' '`bzr config email`')' > ${CMAKE_BINARY_DIR}/NEWS.update && seq -s "=" `cat ${CMAKE_BINARY_DIR}/NEWS.update | wc -c` | sed 's/[0-9]//g' >> ${CMAKE_BINARY_DIR}/NEWS.update && $ENV{EDITOR} ${CMAKE_BINARY_DIR}/NEWS.update && echo >> ${CMAKE_BINARY_DIR}/NEWS.update
				   COMMENT "Generating NEWS Header"
				   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	endif (AUTO_NEWS_UPDATE)

	add_custom_target (news
			   COMMAND cat ${CMAKE_SOURCE_DIR}/NEWS > NEWS.old &&
				   cat NEWS.old >> NEWS.update &&
				   cat NEWS.update > NEWS
			   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

	add_dependencies (news-header authors)
	add_dependencies (news news-header)

	add_custom_target (release-prep)
	add_dependencies (release-prep news)

endmacro (compiz_add_release)

# unsets the given variable
macro (compiz_unset var)
    set (${var} "" CACHE INTERNAL "")
endmacro ()

# sets the given variable
macro (compiz_set var value)
    set (${var} ${value} CACHE INTERNAL "")
endmacro ()


macro (compiz_format_string str length return)
    string (LENGTH "${str}" _str_len)
    math (EXPR _add_chr "${length} - ${_str_len}")
    set (${return} "${str}")
    while (_add_chr GREATER 0)
	set (${return} "${${return}} ")
	math (EXPR _add_chr "${_add_chr} - 1")
    endwhile ()
endmacro ()

string (ASCII 27 _escape)
function (compiz_color_message _str)
    if (CMAKE_COLOR_MAKEFILE)
	message (${_str})
    else ()
	string (REGEX REPLACE "${_escape}.[0123456789;]*m" "" __str ${_str})
	message (${__str})
    endif ()
endfunction ()

function (compiz_configure_file _src _dst)
    foreach (_val ${ARGN})
        set (_${_val}_sav ${${_val}})
        set (${_val} "")
	foreach (_word ${_${_val}_sav})
	    set (${_val} "${${_val}}${_word} ")
	endforeach (_word ${_${_val}_sav})
    endforeach (_val ${ARGN})

    configure_file (${_src} ${_dst} @ONLY)

    foreach (_val ${ARGN})
	set (${_val} ${_${_val}_sav})
        set (_${_val}_sav "")
    endforeach (_val ${ARGN})
endfunction ()

macro (compiz_add_plugins_in_folder folder)
    set (COMPIZ_PLUGIN_PACK_BUILD 1)
    file (
        GLOB _plugins_in
        RELATIVE "${folder}"
        "${folder}/*/CMakeLists.txt"
    )

    foreach (_plugin ${_plugins_in})
	get_filename_component (_plugin_dir ${_plugin} PATH)
	string (TOUPPER ${_plugin_dir} _plugin_upper)
	if (NOT COMPIZ_DISABLE_PLUGIN_${_plugin_upper})
	    add_subdirectory (${folder}/${_plugin_dir})
	    set (COMPIZ_ENABLED_PLUGIN_${_plugin_upper} Y CACHE INTERNAL "")
	endif ()
    endforeach ()
endmacro ()

#### pkg-config handling

include (FindPkgConfig)

function (compiz_pkg_check_modules _var _req)
    if (NOT ${_var})
        pkg_check_modules (${_var} ${_req} ${ARGN})
	if (${_var}_FOUND)
	    set (${_var} 1 CACHE INTERNAL "" FORCE)
	endif ()
	set(__pkg_config_checked_${_var} 0 CACHE INTERNAL "" FORCE)
    endif ()
endfunction ()

#### translations

# translate metadata file
function (compiz_translate_xml _src _dst)
    find_program (INTLTOOL_MERGE_EXECUTABLE intltool-merge)
    mark_as_advanced (FORCE INTLTOOL_MERGE_EXECUTABLE)

    set (_additional_arg
	 -x
	 -u
	 ${COMPIZ_I18N_DIR})

    foreach (_arg ${ARGN})
	if ("${_arg}" STREQUAL "NOTRANSLATIONS")
	    set (_additional_arg
		 --no-translations
		 -x
		 -u)
	endif ("${_arg}" STREQUAL "NOTRANSLATIONS")
    endforeach (_arg ${ARGN})

    if (INTLTOOL_MERGE_EXECUTABLE
	AND COMPIZ_I18N_DIR
	AND EXISTS ${COMPIZ_I18N_DIR})
	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND ${INTLTOOL_MERGE_EXECUTABLE}
		    -c
		    ${CMAKE_BINARY_DIR}/.intltool-merge-cache
		    ${_additional_arg}
		    ${_src}
		    ${_dst}
	    DEPENDS ${_src}
	)
    else ()
    	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND cat ${_src} |
		    sed -e 's:<_:<:g' -e 's:</_:</:g' > 
		    ${_dst}
	    DEPENDS ${_src}
	)
    endif ()
endfunction ()

function (compiz_translate_desktop_file _src _dst)
    find_program (INTLTOOL_MERGE_EXECUTABLE intltool-merge)
    mark_as_advanced (FORCE INTLTOOL_MERGE_EXECUTABLE)

    if (INTLTOOL_MERGE_EXECUTABLE
	AND COMPIZ_I18N_DIR
	AND EXISTS ${COMPIZ_I18N_DIR})
	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND ${INTLTOOL_MERGE_EXECUTABLE} -d -u -c
		    ${CMAKE_BINARY_DIR}/.intltool-merge-cache
		    ${COMPIZ_I18N_DIR}
		    ${_src}
		    ${_dst}
	    DEPENDS ${_src}
	)
    else ()
    	add_custom_command (
	    OUTPUT ${_dst}
	    COMMAND cat ${_src} |
		    sed -e 's:^_::g' >
		    ${_dst}
	    DEPENDS ${_src}
	)
    endif ()
endfunction ()

#### modules / tests
macro (_get_parameters _prefix)
    set (_current_var _foo)
    set (_supported_var PKGDEPS PLUGINDEPS MODULES LDFLAGSADD CFLAGSADD LIBRARIES LIBDIRS INCDIRS DEFSADD)
    foreach (_val ${_supported_var})
	set (${_prefix}_${_val})
    endforeach (_val)
    foreach (_val ${ARGN})
	set (_found FALSE)
	foreach (_find ${_supported_var})
	    if ("${_find}" STREQUAL "${_val}")
		set (_found TRUE)
	    endif ("${_find}" STREQUAL "${_val}")
	endforeach (_find)

	if (_found)
	    set (_current_var ${_prefix}_${_val})
	else (_found)
	    list (APPEND ${_current_var} ${_val})
	endif (_found)
    endforeach (_val)
endmacro (_get_parameters)

macro (_check_pkg_deps _prefix)
    set (${_prefix}_HAS_PKG_DEPS TRUE)
    foreach (_val ${ARGN})
        string (REGEX REPLACE "[<>=\\.]" "_" _name ${_val})
	string (TOUPPER ${_name} _name)

	compiz_pkg_check_modules (_${_name} ${_val})

	if (_${_name}_FOUND)
	    list (APPEND ${_prefix}_PKG_LIBDIRS "${_${_name}_LIBRARY_DIRS}")
	    list (APPEND ${_prefix}_PKG_LIBRARIES "${_${_name}_LIBRARIES}")
	    list (APPEND ${_prefix}_PKG_INCDIRS "${_${_name}_INCLUDE_DIRS}")
	else (_${_name}_FOUND)
	    set (${_prefix}_HAS_PKG_DEPS FALSE)
	    compiz_set (${_prefix}_MISSING_DEPS "${${_prefix}_MISSING_DEPS} ${_val}")
	    set(__pkg_config_checked__${_name} 0 CACHE INTERNAL "" FORCE)
	endif (_${_name}_FOUND)
    endforeach ()
endmacro (_check_pkg_deps)

macro (_build_include_flags _prefix)
    foreach (_include ${ARGN})
	if (NOT ${_prefix}_INCLUDE_CFLAGS)
	    compiz_set (${_prefix}_INCLUDE_CFLAGS "" PARENT_SCOPE)
	endif (NOT ${_prefix}_INCLUDE_CFLAGS)
	list (APPEND ${_prefix}_INCLUDE_CFLAGS -I${_include})
    endforeach (_include)
endmacro (_build_include_flags)

macro (_build_definitions_flags _prefix)
    foreach (_def ${ARGN})
	if (NOT ${_prefix}_DEFINITIONS_CFLAGS)
	    compiz_set (${_prefix}_DEFINITIONS_CFLAGS "")
	endif (NOT ${_prefix}_DEFINITIONS_CFLAGS)
	list (APPEND ${_prefix}_DEFINITIONS_CFLAGS -D${_def})
    endforeach (_def)
endmacro (_build_definitions_flags)

macro (_build_link_dir_flags _prefix)
    foreach (_link_dir ${ARGN})
	if (NOT ${_prefix}_LINK_DIR_LDFLAGS)
	    compiz_set (${_prefix}_LINK_DIR_LDFLAGS "")
	endif (NOT ${_prefix}_LINK_DIR_LDFLAGS)
	list (APPEND ${_prefix}_LINK_DIR_LDFLAGS -L${_link_dir})
    endforeach (_link_dir)
endmacro (_build_link_dir_flags)

macro (_build_library_flags _prefix)
    foreach (_library ${ARGN})
	if (NOT ${_prefix}_LIBRARY_LDFLAGS)
	    compiz_set (${_prefix}_LIBRARY_LDFLAGS "")
	endif (NOT ${_prefix}_LIBRARY_LDFLAGS)
	list (APPEND ${_prefix}_LIBRARY_LDFLAGS -l${_library})
    endforeach (_library)
endmacro (_build_library_flags)

function (_build_compiz_module _prefix _name _full_prefix)

    if (${_full_prefix}_INCLUDE_DIRS)
	_build_include_flags (${_full_prefix} ${${_full_prefix}_INCLUDE_DIRS})
    endif (${_full_prefix}_INCLUDE_DIRS)
    _build_include_flags (${_full_prefix} ${${_full_prefix}_SOURCE_DIR})
    _build_include_flags (${_full_prefix} ${${_full_prefix}_INCLUDE_DIR})

    if (${_full_prefix}_DEFSADD)
	_build_definitions_flags (${_full_prefix} ${${_full_prefix}_DEFSADD})
    endif (${_full_prefix}_DEFSADD)

    if (${_full_prefix}_LIBRARY_DIRS)
	_build_link_dir_flags (${_full_prefix} ${${_full_prefix}_LIBRARY_DIRS})
    endif (${_full_prefix}_LIBRARY_DIRS)

    if (${_full_prefix}_LIBRARIES)
	_build_library_flags (${_full_prefix} ${${_full_prefix}_LIBRARIES})
    endif (${_full_prefix}_LIBRARIES)			      

    file (GLOB _cpp_files "${${_full_prefix}_SOURCE_DIR}/*.cpp")

    add_library (${_prefix}_${_name}_internal STATIC ${_cpp_files})

    target_link_libraries (${_prefix}_${_name}_internal
			   ${${_full_prefix}_LIBRARIES} m pthread dl)

    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_INCLUDE_CFLAGS})
    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_DEFINITIONS_CFLAGS})
    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_CFLAGSADD})

    set (${_full_prefix}_COMPILE_FLAGS_STR " ")
    foreach (_flag ${${_full_prefix}_COMPILE_FLAGS})
	set (${_full_prefix}_COMPILE_FLAGS_STR "${_flag} ${${_full_prefix}_COMPILE_FLAGS_STR}")
    endforeach (_flag)

    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LINK_LDFLAGS})
    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LDFLAGSADD})
    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LIBARY_FLAGS})

    set (${_full_prefix}_LINK_FLAGS_STR " ")
    foreach (_flag ${${_full_prefix}_LINK_FLAGS})
	set (${_full_prefix}_LINK_FLAGS_STR "${_flag} ${${_full_prefix}_LINK_FLAGS_STR}")
    endforeach (_flag)

    set_target_properties (${_prefix}_${_name}_internal PROPERTIES
			   COMPILE_FLAGS ${${_full_prefix}_COMPILE_FLAGS_STR}
			   LINK_FLAGS ${${_full_prefix}_LINK_FLAGS_STR})

    file (GLOB _h_files "${_full_prefix}_INCLUDE_DIR/*.h")

    foreach (_file ${_h_files})

	install (
	    FILES ${_file}
	    DESTINATION ${includedir}/compiz/${_prefix}
	)

    endforeach (_file)

endfunction (_build_compiz_module)

macro (compiz_module _prefix _name)

    string (TOUPPER ${_prefix} _PREFIX)
    string (TOUPPER ${_name} _NAME)
    set (_FULL_PREFIX ${_PREFIX}_${_NAME})

    _get_parameters (${_FULL_PREFIX} ${ARGN})
    _check_pkg_deps (${_FULL_PREFIX} ${${_FULL_PREFIX}_PKGDEPS})

    if (${_FULL_PREFIX}_HAS_PKG_DEPS)

	list (APPEND ${_FULL_PREFIX}_LIBRARIES ${${_FULL_PREFIX}_PKG_LIBRARIES})
	list (APPEND ${_FULL_PREFIX}_INCLUDE_DIRS ${${_FULL_PREFIX}_INCDIRS})
	list (APPEND ${_FULL_PREFIX}_INCLUDE_DIRS ${${_FULL_PREFIX}_PKG_INCDIRS})
	list (APPEND ${_FULL_PREFIX}_LIBRARY_DIRS ${${_FULL_PREFIX}_LIBDIRS})
	list (APPEND ${_FULL_PREFIX}_LIBRARY_DIRS ${${_FULL_PREFIX}_PKG_LIBDIRS})

	# also add modules
	foreach (_module ${${_FULL_PREFIX}_MODULES})
	    string (TOUPPER ${_module} _MODULE)
	    list (APPEND ${_FULL_PREFIX}_INCLUDE_DIRS ${${_MODULE}_INCLUDE_DIR})
	    list (APPEND ${_FULL_PREFIX}_LIBRARY_DIRS ${${_MODULE}_BINARY_DIR})
	    list (APPEND ${_FULL_PREFIX}_LIBRARIES ${_module}_internal)
	endforeach (_module)

	compiz_set (${_FULL_PREFIX}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${_name})
	compiz_set (${_FULL_PREFIX}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${_name}/src)
	compiz_set (${_FULL_PREFIX}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${_name}/include)
	compiz_set (${_FULL_PREFIX}_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${_name}tests)

	# Need to abuse set_property here since set () with CACHE INTERNAL will save the 
	# value to the cache which we will just read right back (but we need to regenerate that)
	set_property (GLOBAL APPEND PROPERTY ${_PREFIX}_MOD_LIBRARY_DIRS ${${_FULL_PREFIX}_BINARY_DIR})
	set_property (GLOBAL APPEND PROPERTY ${_PREFIX}_MOD_INCLUDE_DIRS ${${_FULL_PREFIX}_INCLUDE_DIR})
	set_property (GLOBAL APPEND PROPERTY ${_PREFIX}_MOD_INCLUDE_DIRS ${${_FULL_PREFIX}_SOURCE_DIR})
	set_property (GLOBAL APPEND PROPERTY ${_PREFIX}_MOD_LIBRARIES ${_prefix}_${_name}_internal)

	_build_compiz_module (${_prefix} ${_name} ${_FULL_PREFIX})

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/${_name}/tests)

    else (${_FULL_PREFIX}_HAS_PKG_DEPS)
	message (STATUS "[WARNING] One or more dependencies for module ${_name} for ${_prefix} not found. Skipping module.")
	message (STATUS "Missing dependencies :${${_FULL_PREFIX}_MISSING_DEPS}")
	compiz_set (${_FULL_PREFIX}_BUILD FALSE)
    endif (${_FULL_PREFIX}_HAS_PKG_DEPS)

    
endmacro (compiz_module)

function (_build_compiz_test_base _prefix _module _full_prefix)

    file (GLOB _cpp_files "${${_FULL_TEST_BASE_PREFIX}_SOURCE_DIR}/*.cpp")

    if (${_full_prefix}_INCLUDE_DIRS)
	_build_include_flags (${_full_prefix} ${${_full_prefix}_INCLUDE_DIRS})
    endif (${_full_prefix}_INCLUDE_DIRS)
    _build_include_flags (${_full_prefix} ${${_full_prefix}_SOURCE_DIR})
    _build_include_flags (${_full_prefix} ${${_full_prefix}_INCLUDE_DIR})

    if (${_full_prefix}_DEFSADD)
	_build_definitions_flags (${_full_prefix} ${${_full_prefix}_DEFSADD})
    endif (${_full_prefix}_DEFSADD)

    if (${_full_prefix}_LIBRARY_DIRS)
	_build_link_dir_flags (${_full_prefix} ${${_full_prefix}_LIBRARY_DIRS})
    endif (${_full_prefix}_LIBRARY_DIRS)

    if (${_full_prefix}_LIBRARIES)
	_build_library_flags (${_full_prefix} ${${_full_prefix}_LIBRARIES})
    endif (${_full_prefix}_LIBRARIES)

    add_library (${_prefix}_${_module}_test_internal STATIC
		 ${_cpp_files})

    target_link_libraries (${_prefix}_${_module}_test_internal
			   ${${_full_prefix}_LIBRARIES}
			   ${_prefix}_${_module}_internal)


    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_INCLUDE_CFLAGS})
    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_DEFINITIONS_CFLAGS})
    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_CFLAGSADD})

    set (${_full_prefix}_COMPILE_FLAGS_STR "  ")
    foreach (_flag ${${_full_prefix}_COMPILE_FLAGS})
	set (${_full_prefix}_COMPILE_FLAGS_STR "${_flag} ${${_full_prefix}_COMPILE_FLAGS_STR}")
    endforeach (_flag)

    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LINK_LDFLAGS})
    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LDFLAGSADD})
    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LIBARY_FLAGS})

    set (${_full_prefix}_LINK_FLAGS_STR " ")
    foreach (_flag ${${_full_prefix}_LINK_FLAGS})
	set (${_full_prefix}_LINK_FLAGS_STR "${_flag} ${${_full_prefix}_LINK_FLAGS_STR}")
    endforeach (_flag)

    set_target_properties (${_prefix}_${_module}_test_internal PROPERTIES
			   COMPILE_FLAGS "${${_full_prefix}_COMPILE_FLAGS_STR}"
			   LINK_FLAGS "${${_full_prefix}_LINK_FLAGS_STR}")
endfunction (_build_compiz_test_base)

macro (compiz_test_base _prefix _module)

    string (TOUPPER ${_prefix} _PREFIX)
    string (TOUPPER ${_module} _MODULE)

    set (_FULL_MODULE_PREFIX ${_PREFIX}_${_NAME})
    set (_FULL_TEST_BASE_PREFIX ${_FULL_MODULE_PREFIX}_TEST_BASE)

    _get_parameters (${_FULL_TEST_BASE_PREFIX} ${ARGN})
    _check_pkg_deps (${_FULL_TEST_BASE_PREFIX} ${${_FULL_TEST_BASE_PREFIX}_PKGDEPS})

    if (${_FULL_TEST_BASE_PREFIX}_HAS_PKG_DEPS)

	list (APPEND ${_FULL_TEST_BASE_PREFIX}_LIBRARIES ${${_FULL_TEST_BASE_PREFIX}_PKG_LIBDIRS})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_BASE_PREFIX}_INCDIRS})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_BASE_PREFIX}_PKG_INCDIRS})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_LIBRARY_DIRS ${${_FULL_TEST_BASE_PREFIX}_LIBDIRS})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_LIBRARY_DIRS ${${_FULL_TEST_BASE_PREFIX}_PKG_LIBDIRS})

	compiz_set (${_FULL_TEST_BASE_PREFIX}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	compiz_set (${_FULL_TEST_BASE_PREFIX}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	compiz_set (${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

	list (APPEND ${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIRS ${${_FULL_MODULE_PREFIX}_INCLUDE_DIRS})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIRS ${${_FULL_MODULE_PREFIX}_INCLUDE_DIR})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIRS ${${_FULL_MODULE_PREFIX}_SOURCE_DIR})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_LIBRARY_DIRS ${${_FULL_MODULE_PREFIX}_LIBRARY_DIRS})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_LIBRARY_DIRS ${${_FULL_MODULE_PREFIX}_BINARY_DIR})
	list (APPEND ${_FULL_TEST_BASE_PREFIX}_LIBRARIES ${${_FULL_MODULE_PREFIX}_LIBRARIES})

	_build_compiz_test_base (${_prefix} ${_module} ${_FULL_TEST_BASE_PREFIX})
    else (${_FULL_TEST_BASE_PREFIX}_HAS_PKG_DEPS)
	message (STATUS "[WARNING] One or more dependencies for test base on module ${_module} for ${_prefix} not found. Skipping test base.")
	message (STATUS "Missing dependencies :${${_FULL_TEST_BASE_PREFIX}_MISSING_DEPS}")
	compiz_set (${_FULL_TEST_BASE_PREFIX}_BUILD FALSE)
    endif (${_FULL_TEST_BASE_PREFIX}_HAS_PKG_DEPS)
endmacro (compiz_test_base)

function (_build_compiz_test _prefix _module _test _full_prefix)
    file (GLOB _cpp_files "${${_FULL_TEST_PREFIX}_SOURCE_DIR}/*.cpp")

    if (${_full_prefix}_INCLUDE_DIRS)
	_build_include_flags (${_full_prefix} ${${_full_prefix}_INCLUDE_DIRS})
    endif (${_full_prefix}_INCLUDE_DIRS)
    _build_include_flags (${_full_prefix} ${${_full_prefix}_SOURCE_DIR})
    _build_include_flags (${_full_prefix} ${${_full_prefix}_INCLUDE_DIR})

    if (${_full_prefix}_DEFSADD)
	_build_definitions_flags (${_full_prefix} ${${_full_prefix}_DEFSADD})
    endif (${_full_prefix}_DEFSADD)

    if (${_full_prefix}_LIBRARY_DIRS)
	_build_link_dir_flags (${_full_prefix} ${${_full_prefix}_LIBRARY_DIRS})
    endif (${_full_prefix}_LIBRARY_DIRS)

    if (${_full_prefix}_LIBRARIES)
	_build_library_flags (${_full_prefix} ${${_full_prefix}_LIBRARIES})
    endif (${_full_prefix}_LIBRARIES)

    add_executable (${_prefix}_${_module}_${_test}_test
		    ${_cpp_files})

    target_link_libraries (${_prefix}_${_module}_${_test}_test
			   ${${_full_prefix}_LIBRARIES}
			   ${_prefix}_${_module}_internal
			   ${_prefix}_${_module}_test_internal)

    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_INCLUDE_CFLAGS})
    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_DEFINITIONS_CFLAGS})
    list (APPEND ${_full_prefix}_COMPILE_FLAGS ${${_full_prefix}_CFLAGSADD})

    set (${_full_prefix}_COMPILE_FLAGS_STR " ")
    foreach (_flag ${${_full_prefix}_COMPILE_FLAGS})
	set (${_full_prefix}_COMPILE_FLAGS_STR "${_flag} ${${_full_prefix}_COMPILE_FLAGS_STR}")
    endforeach (_flag)

    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LINK_LDFLAGS})
    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LDFLAGSADD})
    list (APPEND ${_full_prefix}_LINK_FLAGS ${${_full_prefix}_LIBARY_FLAGS})

    set (${_full_prefix}_LINK_FLAGS_STR " ")
    foreach (_flag ${${_full_prefix}_LINK_FLAGS})
	set (${_full_prefix}_LINK_FLAGS_STR "${_flag} ${${_full_prefix}_LINK_FLAGS_STR}")
    endforeach (_flag)

    set_target_properties (${_prefix}_${_module}_${_test}_test PROPERTIES
			   COMPILE_FLAGS "${${_full_prefix}_COMPILE_FLAGS_STR}"
			   LINK_FLAGS "${${_full_prefix}_LINK_FLAGS_STR}")

    add_test (test-${_prefix}-${_module}-${_test}
	      ${CMAKE_CURRENT_BINARY_DIR}/${_prefix}_${_module}_${_test}_test)
endfunction (_build_compiz_test)

macro (compiz_test _prefix _module _test)

    set (_supported_var PKGDEPS LDFLAGSADD CFLAGSADD LIBRARIES LIBDIRS INCDIRS DEFSADD)

    set (_FULL_TEST_PREFIX ${_FULL_MODULE_PREFIX}_TEST)

    _get_parameters (${_FULL_TEST_PREFIX} ${ARGN})
    _check_pkg_deps (${_FULL_TEST_PREFIX} ${${_FULL_TEST_PREFIX}_PKGDEPS})

    if (${_FULL_TEST_PREFIX}_HAS_PKG_DEPS)
	list (APPEND ${_FULL_TEST_PREFIX}_LIBRARIES ${${_FULL_TEST_PREFIX}_PKG_LIBDIRS})
	list (APPEND ${_FULL_TEST_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_PREFIX}_INCDIRS})
	list (APPEND ${_FULL_TEST_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_PREFIX}_PKG_INCDIRS})
	list (APPEND ${_FULL_TEST_PREFIX}_LIBRARY_DIRS ${${_FULL_TEST_PREFIX}_LIBDIRS})
	list (APPEND ${_FULL_TEST_PREFIX}_LIBRARY_DIRS ${${_FULL_TEST_PREFIX}_PKG_LIBDIRS})

	compiz_set (${_FULL_TEST_PREFIX}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	compiz_set (${_FULL_TEST_PREFIX}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
	compiz_set (${_FULL_TEST_PREFIX}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

	list (APPEND ${_FULL_TEST_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIRS})
	list (APPEND ${_FULL_TEST_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_BASE_PREFIX}_INCLUDE_DIR})
	list (APPEND ${_FULL_TEST_PREFIX}_INCLUDE_DIRS ${${_FULL_TEST_BASE_PREFIX}_SOURCE_DIR})
	list (APPEND ${_FULL_TEST_PREFIX}_LIBRARY_DIRS ${${_FULL_TEST_BASE_PREFIX}_LIBRARY_DIRS})
	list (APPEND ${_FULL_TEST_PREFIX}_LIBRARY_DIRS ${${_FULL_TEST_BASE_PREFIX}_BINARY_DIR})
	list (APPEND ${_FULL_TEST_PREFIX}_LIBRARIES ${${_FULL_TEST_BASE_PREFIX}_LIBRARIES})

	_build_compiz_test (${_prefix} ${_module} ${_test} ${_FULL_TEST_PREFIX})

    else (${_FULL_TEST_PREFIX}_HAS_PKG_DEPS)
	message (STATUS "[WARNING] One or more dependencies for test ${_test} on module ${_name} for ${_prefix} not found. Skipping test.")
	message (STATUS "Missing dependencies :${${_FULL_TEST_PREFIX}_MISSING_DEPS}")
	compiz_set (${_FULL_TEST_PREFIX}_BUILD FALSE)
    endif (${_FULL_TEST_PREFIX}_HAS_PKG_DEPS)

endmacro (compiz_test)

#### optional file install

function (compiz_opt_install_file _src _dst)
    install (CODE
        "message (\"-- Installing: ${_dst}\")
         execute_process (
	    COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_src}\" \"$ENV{DESTDIR}${_dst}\"
	    RESULT_VARIABLE _result
	    OUTPUT_QUIET ERROR_QUIET
	 )
	 if (_result)
	     message (\"-- Failed to install: ${_dst}\")
	 endif ()
        "
    )
endfunction ()

#### uninstall

macro (compiz_add_uninstall)
   if (NOT _compiz_uninstall_rule_created)
	compiz_set(_compiz_uninstall_rule_created TRUE)

	set (_file "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

	file (WRITE  ${_file} "if (NOT EXISTS \"${CMAKE_BINARY_DIR}/install_manifest.txt\")\n")
	file (APPEND ${_file} "  message (FATAL_ERROR \"Cannot find install manifest: \\\"${CMAKE_BINARY_DIR}/install_manifest.txt\\\"\")\n")
	file (APPEND ${_file} "endif (NOT EXISTS \"${CMAKE_BINARY_DIR}/install_manifest.txt\")\n\n")
	file (APPEND ${_file} "file (READ \"${CMAKE_BINARY_DIR}/install_manifest.txt\" files)\n")
	file (APPEND ${_file} "string (REGEX REPLACE \"\\n\" \";\" files \"\${files}\")\n")
	file (APPEND ${_file} "foreach (file \${files})\n")
	file (APPEND ${_file} "  message (STATUS \"Uninstalling \\\"\${file}\\\"\")\n")
	file (APPEND ${_file} "  if (EXISTS \"\${file}\")\n")
	file (APPEND ${_file} "    exec_program(\n")
	file (APPEND ${_file} "      \"${CMAKE_COMMAND}\" ARGS \"-E remove \\\"\${file}\\\"\"\n")
	file (APPEND ${_file} "      OUTPUT_VARIABLE rm_out\n")
	file (APPEND ${_file} "      RETURN_VALUE rm_retval\n")
	file (APPEND ${_file} "      )\n")
	file (APPEND ${_file} "    if (\"\${rm_retval}\" STREQUAL 0)\n")
	file (APPEND ${_file} "    else (\"\${rm_retval}\" STREQUAL 0)\n")
	file (APPEND ${_file} "      message (FATAL_ERROR \"Problem when removing \\\"\${file}\\\"\")\n")
	file (APPEND ${_file} "    endif (\"\${rm_retval}\" STREQUAL 0)\n")
	file (APPEND ${_file} "  else (EXISTS \"\${file}\")\n")
	file (APPEND ${_file} "    message (STATUS \"File \\\"\${file}\\\" does not exist.\")\n")
	file (APPEND ${_file} "  endif (EXISTS \"\${file}\")\n")
	file (APPEND ${_file} "endforeach (file)\n")

	add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

    endif ()
endmacro ()

#posix 2008 scandir check
if (CMAKE_CXX_COMPILER)
	include (CheckCXXSourceCompiles)
	CHECK_CXX_SOURCE_COMPILES (
	  "# include <dirent.h>
	   int func (const char *d, dirent ***list, void *sort)
	   {
	     int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
	     return n;
	   }

	   int main (int, char **)
	   {
	     return 0;
	   }
	  "
	  HAVE_SCANDIR_POSIX)
endif (CMAKE_CXX_COMPILER)

if (HAVE_SCANDIR_POSIX)
  add_definitions (-DHAVE_SCANDIR_POSIX)
endif ()
