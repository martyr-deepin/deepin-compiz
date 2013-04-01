find_package (Compiz REQUIRED)

include (CompizPlugin)

add_definitions (
    -DHAVE_CONFIG_H
    -DPLUGINDIR=\\\"${compiz_plugindir}\\\"
    -DMETADATADIR=\\\"${compiz_metadatadir}\\\"
)

compiz_plugin (dbus PKGDEPS dbus-1 libxml-2.0)



#AC_ARG_ENABLE(dbus,
#  [  --disable-dbus          Disable dbus plugin],
#  [use_dbus=$enableval], [use_dbus=yes])
#if test "x$use_dbus" = "xyes"; then
#  PKG_CHECK_MODULES(DBUS, dbus-1 libxml-2.0, [use_dbus=yes], [use_dbus=no])
#fi

#AM_CONDITIONAL(DBUS_PLUGIN, test "x$use_dbus" = "xyes")
#if test "$use_dbus" = yes; then
#  AC_DEFINE(USE_DBUS, 1, [Build dbus plugin])
#fi

#AC_ARG_ENABLE(dbus-glib,
#  [  --disable-dbus-glib     Disable dbus-glib support],
#  [use_dbus_glib=$enableval], [use_dbus_glib=yes])

#if test "x$use_dbus_glib" = "xyes"; then
#  PKG_CHECK_MODULES(DBUS_GLIB, dbus-glib-1, [use_dbus_glib=yes], [use_dbus_glib=no])
#fi

#if test "$use_dbus" = yes; then
#  AC_DEFINE(USE_DBUS_GLIB, 1, [Build dbus glib support])
#fi
