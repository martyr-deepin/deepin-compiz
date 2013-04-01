find_program (XSLTPROC_EXECUTABLE xsltproc)
mark_as_advanced (FORCE XSLTPROC_EXECUTABLE)

if (NOT XSLTPROC_EXECUTABLE)
    message (FATAL_ERROR "xsltproc not found.")
endif ()

# does the plugin require bcop
function (compiz_plugin_needs_bcop _file _return)
    file (READ ${_file} _xml_content)
    if ("${_xml_content}" MATCHES "useBcop=\"true\"")
	set (${_return} TRUE PARENT_SCOPE)
    else ()
	set (${_return} FALSE PARENT_SCOPE)
    endif ()
endfunction ()

# prepare bcop build
function (compiz_add_bcop_targets _plugin _file _sources)
    add_custom_target (${_plugin}-options
        SOURCES ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.h
                ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.cpp
    )
    add_custom_command (
        OUTPUT ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.h
        COMMAND ${XSLTPROC_EXECUTABLE}
                    -o ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.h
                    --stringparam "file" "header" ${COMPIZ_BCOP_XSLT}
                    ${_file}
        DEPENDS ${_file}
    )
    add_custom_command (
        OUTPUT ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.cpp
        COMMAND ${XSLTPROC_EXECUTABLE}
                    -o ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.cpp
                    --stringparam "file" "source" ${COMPIZ_BCOP_XSLT}
                    ${_file} > ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.cpp
        DEPENDS ${_file}
                ${CMAKE_BINARY_DIR}/generated/${_plugin}_options.h
    )
    set (${_sources} "${CMAKE_BINARY_DIR}/generated/${_plugin}_options.h;${CMAKE_BINARY_DIR}/generated/${_plugin}_options.cpp" PARENT_SCOPE)


endfunction ()
