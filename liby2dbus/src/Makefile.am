#
# Makefile.am for core/dbus/libdbus
#

AM_CXXFLAGS = -DY2LOG=\"y2dbus\" -DSUSEVERSION=\"${SUSEVERSION}\"


plugin_LTLIBRARIES = liby2dbus.la

liby2dbus_la_SOURCES =  DBusMsg.cc DBusMsg.h DBusConn.cc DBusConn.h DBusCaller.cc DBusCaller.h	\
			DBusServerBase.cc DBusServerBase.h DBusSignature.cc DBusSignature.h		\
			DBusArgument.cc DBusArgument.h
if HAVE_POLKIT
    liby2dbus_la_SOURCES += PolKit.cc PolKit.h
endif

liby2dbus_la_CPPFLAGS = $(DBUS_CFLAGS) $(POLKIT_CFLAGS)

liby2dbus_la_LIBADD = $(DBUS_LIBS) $(POLKIT_LIBS)

liby2dbus_la_LDFLAGS = -version-info 2:0

INCLUDES = ${AGENT_INCLUDES}

