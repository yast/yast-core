#
# Makefile.am for core/debugger
#
SUBDIRS=client

AM_CXXFLAGS = -DY2LOG=\"debugger\"

noinst_LTLIBRARIES = liby2debug.la

liby2debug_la_SOURCES =	\
	Debugger.h	\
	Debugger.cc

#liby2debug_la_LIBADD = @AGENT_LIBADD@

INCLUDES =	\
	-I$(srcdir)/../libycp/src/include \
	-I$(srcdir)/../liby2/src/include \
	-I$(srcdir)/../liby2util-r/src/include \
	${Y2UTIL_CFLAGS}
