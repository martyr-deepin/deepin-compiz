set (_COMPIZCONFIG_INTERNAL ON)
set (LIBCOMPIZCONFIG_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/libcompizconfig/cmake )
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LIBCOMPIZCONFIG_CMAKE_MODULE_PATH})
set (COMPIZCONFIG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libcompizconfig/include)
set (COMPIZCONFIG_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libcompizconfig)

option (USE_GCONF "Enable legacy GNOME 2.x option integration with GConf" ON)

add_subdirectory (libcompizconfig)
add_subdirectory (compizconfig-python)
add_subdirectory (integration)
add_subdirectory (ccsm)

if (USE_GCONF)
    add_subdirectory (gconf)
endif (USE_GCONF)

if (USE_GSETTINGS)
    add_subdirectory (gsettings)
endif (USE_GSETTINGS)

if (COMPIZ_BUILD_TESTING)
    add_subdirectory (mocks)
    add_subdirectory (tests)
endif (COMPIZ_BUILD_TESTING)

add_custom_target (pyclean)

add_dependencies (pyclean pyclean_ccsm pyclean_compizconfig)
