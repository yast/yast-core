#
# Makefile.am for core/libscr/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"libscr\"
AM_LDFLAGS = -L$(libdir)

lib_LTLIBRARIES = libscr.la

libscr_la_SOURCES =  \
	SCRAgent.cc SCR.cc

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/include				\
	-I$(includedir)					\
        -I/usr/include/YaST2

libscr_la_LDFLAGS = -version-info 2:0
