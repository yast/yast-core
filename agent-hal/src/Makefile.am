#
# Makefile.am for core/agent-hal/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-hal\"

plugin_LTLIBRARIES = libpy2ag_hal.la

libpy2ag_hal_la_SOURCES =			\
	Y2CCHalAgent.cc				\
	HalAgent.cc HalAgent.h

libpy2ag_hal_la_LDFLAGS = -version-info 2:0	\
	`pkg-config --libs hal dbus-glib-1`
libpy2ag_hal_la_LIBADD = @AGENT_LIBADD@

INCLUDES = @AGENT_INCLUDES@			\
	`pkg-config --cflags hal dbus-glib-1`
