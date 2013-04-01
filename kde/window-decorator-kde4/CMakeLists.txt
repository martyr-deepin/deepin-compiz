project (kde4-window-decorator)

option (BUILD_KDE4 "Build KDE 4 window decorator" 1)

compiz_set (USE_KDE4 ${BUILD_KDE4})

if (USE_KDE4)

    find_package(KDE4 4.3.0)
    find_package(KDE4Workspace)

    if (KDE4_FOUND AND KDE4Workspace_FOUND)
        include(KDE4Defaults)
	include(FindPkgConfig)
	include(MacroLibrary)

        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

        add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

	qt4_add_dbus_adaptor (kwd4_SRCS org.kde.KWin.xml decorator.h KWD::Decorator kwinadaptor)

	pkg_check_modules (KWD4 REQUIRED xdamage xrender xcomposite xext)

	link_directories(
	    ${KWD4_LIBRARY_DIRS}
	)

	include_directories (
	    ${compiz_SOURCE_DIR}/include
	    ${KWD4_INCLUDE_DIRS}
	    ${KDE4_INCLUDES}
	    ${KDE4_INCLUDE_DIR}
	    ${KDE4_PLASMA_INCLUDE_DIR}
	    ${QT_INCLUDES}
	)

	set (kwd4_SRCS
	    main.cpp
	    utils.cpp
	    decorator.cpp
	    window.cpp
	    options.cpp
	    kdecoration_plugins.cpp
	    switcher.cpp
	    paintredirector.cpp
	    ${kwd4_SRCS}
	)

	kde4_add_executable (
	    kde4-window-decorator
	    ${kwd4_SRCS}
	)

	set_target_properties (
	    kde4-window-decorator PROPERTIES
	    INSTALL_RPATH_USE_LINK_PATH 0
	)

	target_link_libraries (
	    kde4-window-decorator
	    decoration
	    ${KWD4_LIBRARIES}
            ${KDE4_KDECORE_LIBS}
	    ${KDE4_KDEUI_LIBS}
	    ${KDE4_PLASMA_LIBS}
	    kdecorations
	)

	install (
	    TARGETS kde4-window-decorator
	    DESTINATION ${exec_prefix}
	)

    else (KDE4_FOUND AND KDE4Workspace_FOUND)
	compiz_set (USE_KDE4 0)
    endif (KDE4_FOUND AND KDE4Workspace_FOUND)

endif (USE_KDE4)
