#
# Makefile.am for core/agent-dumpto/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-dumpto\"

plugin_LTLIBRARIES = libpy2ag_dumpto.la

libpy2ag_dumpto_la_SOURCES =			\
	DumpTo.cc DumpTo.h			\
	Y2CCDumpTo.cc

libpy2ag_dumpto_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)
