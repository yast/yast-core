#
# Makefile.am for core/agent-losetup/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-losetup\"

plugin_LTLIBRARIES = libpy2ag_losetup.la

libpy2ag_losetup_la_SOURCES =		\
	LosetupAgent.cc	LosetupAgent.h LosetupIO.c LosetupIO.h rmd160.h	\
	rmd160.c y2loop.h loop.h y2lomount.h

libpy2ag_losetup_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)
