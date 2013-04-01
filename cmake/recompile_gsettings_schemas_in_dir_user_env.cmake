set (SCHEMADIR_USER "" CACHE FORCE "Installation dir if user")
set (SCHEMADIR_ROOT "" CACHE FORCE "Installation dir if root")

if (ENV{USER})
    set (USERNAME $ENV{USER})
else (ENV${USER})
    set (USERNAME "user")
endif (ENV{USER})

if (${USERNAME} STREQUAL "root")
    set (SCHEMADIR ${SCHEMADIR_ROOT})
else (${USERNAME} STREQUAL "root")
    set (SCHEMADIR ${SCHEMADIR_USER})
endif (${USERNAME} STREQUAL "root")

find_program (GLIB_COMPILE_SCHEMAS glib-compile-schemas)

if (GLIB_COMPILE_SCHEMAS)

    message ("-- Recompiling GSettings schemas in ${SCHEMADIR}")
    execute_process (COMMAND ${GLIB_COMPILE_SCHEMAS} ${SCHEMADIR})

endif (GLIB_COMPILE_SCHEMAS)
