project (po)

find_program (_intltool_update intltool-update)
if (NOT EXISTS "${_intltool_update}")
    message (SEND_ERROR "Missing intltool-update (try: sudo apt-get install intltool)")
endif ()

find_program (_xgettext xgettext)
mark_as_advanced (_xgettext)

find_program (_msgmerge msgmerge)
mark_as_advanced (_msgmerge)

find_program(_msgfmt msgfmt)
mark_as_advanced (k3d_msgfmt)

set (compiz_catalogs ${ALL_LINGUAS})

set (compiz_catalog_targets)

add_custom_target (nls-update-sources
    ${_intltool_update} -m
    WORKING_DIRECTORY ${po_SOURCE_DIR}
)

add_custom_command (
    OUTPUT ${po_SOURCE_DIR}/compiz.pot
    COMMAND ${_intltool_update} --gettext-package compiz --pot
    DEPENDS POTFILES.in
    WORKING_DIRECTORY ${po_SOURCE_DIR}
)

foreach(catalog ${compiz_catalogs})
    add_custom_target (nls-update-${catalog}.po
	${_msgmerge} ${catalog} compiz -o ${catalog}.po --no-wrap
	DEPENDS ${po_SOURCE_DIR}/${catalog}.po
	WORKING_DIRECTORY ${po_SOURCE_DIR}
    )

    file (MAKE_DIRECTORY "${po_BINARY_DIR}/${catalog}/LC_MESSAGES")

    add_custom_command (
	OUTPUT ${po_BINARY_DIR}/${catalog}/LC_MESSAGES/compiz.mo
	COMMAND ${_msgfmt} -c -o ${po_BINARY_DIR}/${catalog}/LC_MESSAGES/compiz.mo ${po_SOURCE_DIR}/${catalog}.po
	DEPENDS ${po_SOURCE_DIR}/${catalog}.po
    )

    install (
	FILES ${po_BINARY_DIR}/${catalog}/LC_MESSAGES/compiz.mo
	DESTINATION ${datadir}/locale/${catalog}/LC_MESSAGES
    )

    set (compiz_catalog_targets ${compiz_catalog_targets} ${po_BINARY_DIR}/${catalog}/LC_MESSAGES/compiz.mo)
endforeach(catalog)

add_custom_target(nls ALL
    DEPENDS ${po_SOURCE_DIR}/compiz.pot ${compiz_catalog_targets}
)
