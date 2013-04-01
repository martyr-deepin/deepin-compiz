find_package (CompizConfig REQUIRED)
include (LibCompizConfigCommon)

# Hack
set (CMAKE_PROJECT_NAME compizconfig-python)

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

set (PY_COMPIZCONFIG_SRC ${CMAKE_CURRENT_BINARY_DIR}/compizconfig.c)

add_custom_command (OUTPUT ${PY_COMPIZCONFIG_SRC}
		    COMMAND pyrexc -o ${PY_COMPIZCONFIG_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/src/compizconfig.pyx
		    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (PY_COMPIZCONFIG_LIB  ${CMAKE_CURRENT_BINARY_DIR}/build/${PY_BUILD_DIR}/compizconfig.so)

file (READ ${CMAKE_SOURCE_DIR}/VERSION COMPIZ_RELEASE_VERSION LIMIT 12 OFFSET 0)
string (STRIP ${COMPIZ_RELEASE_VERSION} COMPIZ_RELEASE_VERSION)
set (VERSION ${COMPIZ_RELEASE_VERSION})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/compizconfig-python.pc.in
                ${CMAKE_CURRENT_BINARY_DIR}/compizconfig-python.pc)

add_custom_command (OUTPUT ${PY_COMPIZCONFIG_LIB}
		    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build --version=${VERSION}
		    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		    DEPENDS ${PY_COMPIZCONFIG_SRC} ${PY_COMPIZCONFIG_VERSION} compizconfig
		    COMMENT "Generating Python Bytecode")

add_custom_target (compizconfig_python_module ALL DEPENDS
                   ${PY_COMPIZCONFIG_LIB})

add_custom_target (uninstall
		   COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/setup.py uninstall --prefix=${CMAKE_INSTALL_PREFIX}
		   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		   COMMENT "Uninstalling Python Files")

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/compizconfig-python.pc
         DESTINATION ${libdir}/pkgconfig)

install (CODE
	 "message (\"Installing python files\")
	  execute_process (COMMAND cmake -DSETUP=${CMAKE_CURRENT_SOURCE_DIR}/setup.py -DPREFIX=${CMAKE_INSTALL_PREFIX} -DWDIR=${CMAKE_CURRENT_BINARY_DIR} -DVERSION=${VERSION} -P ${compiz_SOURCE_DIR}/compizconfig/cmake/exec_setup_py_with_destdir.cmake)")

if (COMPIZ_BUILD_TESTING)

        find_program (BASH_EXECUTABLE bash)
	find_program (PYTHON_EXECUTABLE python)
	mark_as_advanced (FORCE PYTHON_EXECUTABLE)

	file (GLOB PYTHON_TESTS "tests/test_*.py")

        if (PYTHON_EXECUTABLE AND
            BASH_EXECUTABLE)
		foreach (PYTHON_TEST_FILE ${PYTHON_TESTS})
			get_filename_component (PYTHON_TEST ${PYTHON_TEST_FILE} NAME_WE)
			message (STATUS "Adding test " ${PYTHON_TEST})
                        add_test (NAME CompizConfigPython.${PYTHON_TEST}
                                  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                                  COMMAND ${BASH_EXECUTABLE}
                                  ${CMAKE_SOURCE_DIR}/tests/util/run_with_ld_library_path.sh
                                  ${CMAKE_BINARY_DIR}/compizconfig/libcompizconfig/src
                                  ${PYTHON_EXECUTABLE} ${PYTHON_TEST_FILE})
		endforeach (PYTHON_TEST_FILE)
        endif (PYTHON_EXECUTABLE AND
               BASH_EXECUTABLE)

endif (COMPIZ_BUILD_TESTING)
