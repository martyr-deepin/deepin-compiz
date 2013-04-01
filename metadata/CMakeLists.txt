set (
    COMPIZ_DEFAULT_PLUGINS "core" CACHE STRING
    "Default plugins list that will be added to core metadata file"
)

find_program (XSLTPROC_EXECUTABLE xsltproc)
mark_as_advanced (FORCE XSLTPROC_EXECUTABLE)

if (XSLTPROC_EXECUTABLE)

    add_custom_command (
	OUTPUT ${compiz_BINARY_DIR}/generated/core.xml.in
	COMMAND ${XSLTPROC_EXECUTABLE}
		-o ${compiz_BINARY_DIR}/generated/core.xml.in
		--param default_plugins "\"'${COMPIZ_DEFAULT_PLUGINS}'\""
		${CMAKE_SOURCE_DIR}/xslt/default_plugins.xslt
		${CMAKE_CURRENT_SOURCE_DIR}/core.xml.in
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/core.xml.in
    )
    set (_core_file "${CMAKE_BINARY_DIR}/generated/core.xml.in")
else ()
    set (_core_file "${CMAKE_CURRENT_SOURCE_DIR}/core.xml.in")
endif ()

set (_xml_targets)
set (_schema_targets)

set (_core_xml_file ${CMAKE_BINARY_DIR}/generated/core.xml)
compiz_translate_xml (
    ${_core_file}
    ${_core_xml_file}
)

install (
    FILES ${_core_xml_file}
    DESTINATION ${compiz_metadatadir}
)

add_custom_target (core-xml-file ALL
    DEPENDS ${_core_xml_file}
)

if (USE_GCONF)
    compiz_gconf_schema (
	${_core_xml_file}
	${CMAKE_BINARY_DIR}/generated/compiz-core.schemas
	${SCHEMADIR}
    )
    add_custom_target (core-gconf-schema ALL
	DEPENDS ${CMAKE_BINARY_DIR}/generated/compiz-core.schemas
	        core-xml-file
    )
endif ()
if (USE_GSETTINGS)
    set (_core_gschema_path ${CMAKE_BINARY_DIR}/generated/glib-2.0/schemas/org.compiz.core.gschema.xml)
    compiz_gsettings_schema (
	compiz
	${_core_xml_file}
	${_core_gschema_path}
	${datadir}/glib-2.0/schemas
    )
    add_custom_target (core-settings-schema ALL
	DEPENDS ${_core_gschema_path}
	        core-xml-file
    )
endif ()
