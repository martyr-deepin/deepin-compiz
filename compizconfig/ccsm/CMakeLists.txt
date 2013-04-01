find_package (CompizConfig REQUIRED)
include (LibCompizConfigCommon)

# Hack
set (CMAKE_PROJECT_NAME ccsm)

add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/compizconfig_wrapper.c
		    COMMAND echo \"int main (void) { return 0\; }\" >> ${CMAKE_CURRENT_BINARY_DIR}/compizconfig_wrapper.c
		    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Get arch
execute_process (COMMAND uname -p OUTPUT_VARIABLE PROC_ARCH)

# Get Python Version
execute_process (COMMAND python -c "import sys; print str (sys.version_info[0]) + '.' + str (sys.version_info[1])"
		 OUTPUT_VARIABLE PY_VERSION)

string (REPLACE "\n" "" PROC_ARCH ${PROC_ARCH})
string (REPLACE "\n" "" PY_VERSION ${PY_VERSION})

set (PY_BUILD_DIR lib.linux-${PROC_ARCH}-${PY_VERSION})
set (PY_SCRIPTS_DIR scripts-${PY_VERSION})

set (PY_CCSM_LIB  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Conflicts.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Constants.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/__init__.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Pages.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Settings.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Utils.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Widgets.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/ccm/Window.py
		  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_SCRIPTS_DIR}/ccsm)

file (READ ${CMAKE_SOURCE_DIR}/VERSION COMPIZ_RELEASE_VERSION LIMIT 12 OFFSET 0)
string (STRIP ${COMPIZ_RELEASE_VERSION} COMPIZ_RELEASE_VERSION)
set (VERSION ${COMPIZ_RELEASE_VERSION})

add_custom_command (OUTPUT ${PY_CCSM_LIB}
		    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build --version=${VERSION}
		    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		    COMMENT "Generating Python Bytecode")

add_custom_target (ccsm_module ALL DEPENDS
                   ${PY_CCSM_LIB})

add_custom_target (uninstall
		   COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py uninstall --prefix=${CMAKE_INSTALL_PREFIX}
		   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		   COMMENT "Uninstalling Python Files")

install (CODE
	 "message (\"Installing python files\")
	  execute_process (COMMAND cmake -DSETUP=${CMAKE_CURRENT_SOURCE_DIR}/setup.py -DPREFIX=${CMAKE_INSTALL_PREFIX} -DWDIR=${CMAKE_CURRENT_SOURCE_DIR} -DVERSION=${VERSION} -P ${compiz_SOURCE_DIR}/compizconfig/cmake/exec_setup_py_with_destdir.cmake)")


