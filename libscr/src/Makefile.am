#
# Makefile.am for core/libscr/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libscr\"
AM_LDFLAGS = -L$(libdir)

lib_LTLIBRARIES = libscr.la

libscr_la_SOURCES =  \
	SCRAgent.cc SCRInterpreter.cc

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/include				\
	-I$(includedir)

libscr_la_LDFLAGS = -version-info 2:0
