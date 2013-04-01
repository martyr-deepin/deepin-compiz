set (WDIR "" CACHE FORCE "Working Directory")
set (PREFIX "" CACHE FORCE "Prefix")
set (SETUP "" CACHE FORCE "Path to setup.py")
set (VERSION "" CACHE FORCE "Version")

set (EXTRAARGS "")
set (BUILD_DEB $ENV{COMPIZ_DEB_BUILD})

set (INSTALL_ROOT $ENV{DESTDIR})

if (INSTALL_ROOT)
    set (INSTALL_ROOT_ARGS "--root=$ENV{DESTDIR}")
else (INSTALL_ROOT)
    set (INSTALL_ROOT_ARGS "")
endif (INSTALL_ROOT)

cmake_policy (SET CMP0012 NEW)

if (BUILD_DEB)
    message ("Installing Debian Format")
    set (EXTRAARGS "--install-layout=deb")
endif (BUILD_DEB)

execute_process (COMMAND python ${SETUP} install ${EXTRAARGS} --prefix=${PREFIX} --version=${VERSION} ${INSTALL_ROOT_ARGS}
	 		   WORKING_DIRECTORY ${WDIR})
