#
# Makefile.am for core/libscr/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libscr\"

lib_LTLIBRARIES = libscr.la

libscr_la_SOURCES =  \
	SCRAgent.cc SCR.cc

libscr_la_CPPFLAGS = $(DBUS_CFLAGS)

libscr_la_LIBADD = $(srcdir)/../../liby2dbus/src/liby2dbus.la

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/../../liby2dbus/src			\
	-I$(srcdir)/../../scr/src			\
	-I$(srcdir)/include				\
	${Y2UTIL_CFLAGS}

libscr_la_LDFLAGS = -version-info 2:0
