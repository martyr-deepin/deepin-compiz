set (_headers
    action.h
    atoms.h
    core.h
    countedlist.h
    global.h
    icon.h
    match.h
    modifierhandler.h
    option.h
    output.h
    plugin.h
    propertywriter.h
    privateunion.h
    screen.h
    session.h
    size.h
    string.h
    window.h
    wrapsystem.h
    abiversion.h
)

install (FILES ${_headers} DESTINATION ${COMPIZ_CORE_INCLUDE_DIR})
