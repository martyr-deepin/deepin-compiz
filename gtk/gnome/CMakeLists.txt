if (BUILD_GNOME)
    set (desktop_file ${CMAKE_CURRENT_BINARY_DIR}/compiz.desktop)
    compiz_translate_desktop_file (
        ${CMAKE_CURRENT_SOURCE_DIR}/compiz.desktop.in
        ${desktop_file}
    )
    install (
	FILES ${desktop_file}
	DESTINATION ${datadir}/applications
    )

    set (_keybindings_files
	 50-compiz-navigation.xml.in
	 50-compiz-windows.xml.in)

    set (_keybindings_files_translated "")

    foreach (_keybinding_file ${_keybindings_files})
	string (LENGTH ${_keybinding_file} _str_len)
	math (EXPR _str_len_no_in "${_str_len} - 3")
	string (SUBSTRING ${_keybinding_file} 0 ${_str_len_no_in} _keybinding_file_no_in)

	compiz_translate_xml (${CMAKE_CURRENT_SOURCE_DIR}/${_keybinding_file}
			      ${CMAKE_CURRENT_BINARY_DIR}/${_keybinding_file_no_in} NOTRANSLATIONS)

	list (APPEND _keybindings_files_translated ${CMAKE_CURRENT_BINARY_DIR}/${_keybinding_file_no_in})
    endforeach (_keybinding_file ${keybinding_files})

    add_custom_target (compiz-gnome-keybindings ALL DEPENDS
		       ${_keybindings_files_translated})

    install (
	FILES ${_keybinding_files_translated}
	DESTINATION ${datadir}/gnome-control-center/keybindings
    )

    add_custom_target (compiz.desktop ALL SOURCES ${desktop_file})
endif (BUILD_GNOME)
