#
# Makefile.am for core/agent-dummy/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-dummy\"

plugin_LTLIBRARIES = libpy2ag_dummy.la

libpy2ag_dummy_la_SOURCES =			\
	DummyAgent.cc DummyAgent.h		\
	Y2CCDummyAgent.cc

libpy2ag_dummy_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)
