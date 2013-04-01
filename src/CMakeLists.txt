include (CompizBcop)


add_subdirectory( string )
add_subdirectory( logmessage )
add_subdirectory( timer )
add_subdirectory( pluginclasshandler )
add_subdirectory( point )
add_subdirectory( rect )
add_subdirectory( region )
add_subdirectory( window )
add_subdirectory( servergrab )

IF (COMPIZ_BUILD_TESTING)
add_subdirectory( privatescreen/tests )
add_subdirectory( wrapsystem/tests )
add_subdirectory( plugin )
add_subdirectory( option/tests )
ENDIF (COMPIZ_BUILD_TESTING)

compiz_add_bcop_targets (
    core
    ${compiz_BINARY_DIR}/generated/core.xml.in
    _bcop_sources
)

get_property (CORE_MOD_INCLUDE_DIRS
	      GLOBAL
	      PROPERTY CORE_MOD_INCLUDE_DIRS)

include_directories (
    ${compiz_SOURCE_DIR}/include
    ${compiz_BINARY_DIR}
    ${compiz_BINARY_DIR}/generated
    ${COMPIZ_INCLUDE_DIRS}
    ${CMAKE_PREFIX_PATH}/include
    ${CMAKE_INCLUDE_PATH}
    ${CORE_MOD_INCLUDE_DIRS}
    
    ${CMAKE_CURRENT_SOURCE_DIR}
    
    # Module specific include dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/string/include
    ${CMAKE_CURRENT_SOURCE_DIR}/string/src
    
    ${CMAKE_CURRENT_SOURCE_DIR}/logmessage/include
    ${CMAKE_CURRENT_SOURCE_DIR}/logmessage/src
    
    ${CMAKE_CURRENT_SOURCE_DIR}/timer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/timer/src
    
    ${CMAKE_CURRENT_SOURCE_DIR}/pluginclasshandler/include
    ${CMAKE_CURRENT_SOURCE_DIR}/pluginclasshandler/src

    ${CMAKE_CURRENT_SOURCE_DIR}/point/include
    ${CMAKE_CURRENT_SOURCE_DIR}/point/src

    ${CMAKE_CURRENT_SOURCE_DIR}/rect/include
    ${CMAKE_CURRENT_SOURCE_DIR}/rect/src

    ${CMAKE_CURRENT_SOURCE_DIR}/servergrab/include
    ${CMAKE_CURRENT_SOURCE_DIR}/servergrab/src

    ${CMAKE_CURRENT_SOURCE_DIR}/region/include
    ${CMAKE_CURRENT_SOURCE_DIR}/region/src

    ${CMAKE_CURRENT_SOURCE_DIR}/window/geometry/include
    ${CMAKE_CURRENT_SOURCE_DIR}/window/geometry/src

    ${CMAKE_CURRENT_SOURCE_DIR}/window/geometry-saver/include
    ${CMAKE_CURRENT_SOURCE_DIR}/window/geometry-saver/src

    ${CMAKE_CURRENT_SOURCE_DIR}/window/extents/include
    ${CMAKE_CURRENT_SOURCE_DIR}/window/extents/src

    ${CMAKE_CURRENT_SOURCE_DIR}/window/constrainment/include
    ${CMAKE_CURRENT_SOURCE_DIR}/window/constrainment/src
)

add_definitions (
    -DHAVE_CONFIG_H
    -DPLUGINDIR=\\\"${compiz_plugindir}\\\"
    -DSHAREDIR=\\\"${compiz_sharedir}\\\"
    -DMETADATADIR=\\\"${compiz_metadatadir}\\\"
)

get_property (CORE_MOD_LIBRARY_DIRS
	      GLOBAL
	      PROPERTY CORE_MOD_LIBRARY_DIRS)

link_directories (
    ${COMPIZ_LINK_DIRS}
    ${CORE_MOD_LIBRARY_DIRS}
    ${libdir}
)

add_library (compiz_core SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/global.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/atoms.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/screen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/option.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/match.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/output.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/size.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/windowgeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/icon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/modifierhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/propertywriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eventsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signalsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stackdebugger.cpp

    ${_bcop_sources}
)

add_dependencies (core-options core-xml-file)
add_dependencies (compiz_core core-options)

#
# Normal libraries use the major version number of the library as their
# ABI version. But compiz uses funky date numbers for ABI revisions instead.
#
set (abiversion_h "${compiz_SOURCE_DIR}/include/core/abiversion.h")
file (STRINGS ${abiversion_h} abiline REGEX "^#define +CORE_ABIVERSION .+$")
string (REGEX REPLACE "#define +CORE_ABIVERSION +" "" abiversion "${abiline}")
set_target_properties (compiz_core PROPERTIES
    VERSION ${VERSION}
    SOVERSION "ABI-${abiversion}"
)

add_executable (compiz
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# workaround for build race
add_dependencies (compiz core-xml-file)

get_property (CORE_MOD_LIBRARIES
	      GLOBAL
	      PROPERTY CORE_MOD_LIBRARIES)

target_link_libraries (
    compiz_core
    
    ${COMPIZ_LIBRARIES} 
    
    m 
    pthread 
    dl
    
    -Wl,-whole-archive
    compiz_string
    compiz_timer
    compiz_logmessage
    compiz_pluginclasshandler
    compiz_point
    compiz_rect
    compiz_region
    compiz_window_geometry
    compiz_window_geometry_saver
    compiz_window_extents
    compiz_window_constrainment
    compiz_servergrab
    -Wl,-no-whole-archive
#    ${CORE_MOD_LIBRARIES}
)

#
# Tell CMake that targets using compiz_core should NOT re-import the
# libraries that compiz_core depends on (contains).
#
set_target_properties (compiz_core PROPERTIES
    LINK_INTERFACE_LIBRARIES ""
)

target_link_libraries (
    compiz
    compiz_core
)

install (
    TARGETS compiz_core
  	LIBRARY DESTINATION ${libdir}
)

install (
    TARGETS compiz
    RUNTIME DESTINATION ${exec_prefix}
)

enable_coverage_report( TARGETS compiz )
