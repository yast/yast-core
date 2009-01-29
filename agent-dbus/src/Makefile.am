#
# Makefile.am for core/agent-dbus/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-dbus\"

plugin_LTLIBRARIES = libpy2ag_dbus.la

libpy2ag_dbus_la_SOURCES =			\
	Y2CCDbusAgent.cc			\
	DbusAgent.cc DbusAgent.h

libpy2ag_dbus_la_CPPFLAGS = @DBUS_CFLAGS@
libpy2ag_dbus_la_LDFLAGS = -version-info 2:0
libpy2ag_dbus_la_LIBADD = @AGENT_LIBADD@	\
	@DBUS_LIBS@

INCLUDES = @AGENT_INCLUDES@
