#
# Makefile.am for core/libscr/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libscr\"

lib_LTLIBRARIES = libscr.la

libscr_la_SOURCES =  \
	SCRAgent.cc SCR.cc

AM_CPPFLAGS =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/include				\
	${Y2UTIL_CFLAGS}

libscr_la_LDFLAGS = -version-info 3:0
libscr_la_LIBADD = \
	$(top_srcdir)/libycp/src/libycp.la \
	$(top_srcdir)/libycp/src/libycpvalues.la \
	${Y2UTIL_LIBS}
