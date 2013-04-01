find_package (Compiz REQUIRED)
find_package (KDE4 4.2)

include (CompizPlugin)

if (KDE4_FOUND)
    include(KDE4Defaults)
    include(MacroLibrary)

    string (REGEX REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string (REGEX REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string (REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS})

    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

    QT4_AUTOMOC (src/dispatcher.cpp)

    compiz_plugin (kde LIBRARIES ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS}
	LIBDIRS ${KDE4_LIB_DIR}
	INCDIRS ${KDE4_INCLUDES}
    )

endif ()
