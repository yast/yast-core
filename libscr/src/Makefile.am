#
# Makefile.am for libscr/src
#

SUBDIRS = include

lib_LTLIBRARIES = libscr.la

libscr_la_SOURCES =  \
	SCRAgent.cc SCRInterpreter.cc

INCLUDES = -I$(srcdir)/include -I$(includedir)

libscr_la_LDFLAGS = -version-info 2:0
