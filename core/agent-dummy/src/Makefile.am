#
# Makefile.am for core/agent-dummy/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-dummy\"

plugin_LTLIBRARIES = libpy2ag_dummy.la

libpy2ag_dummy_la_SOURCES =			\
	DummyAgent.cc DummyAgent.h		\
	Y2CCDummyAgent.cc

libpy2ag_dummy_la_LDFLAGS = -version-info 2:0
libpy2ag_dummy_la_LIBADD = @AGENT_LIBADD@

INCLUDES =					\
	-I$(top_srcdir)/libycp/src/include	\
	-I$(top_srcdir)/liby2/src/include	\
	-I$(top_srcdir)/libscr/src/include	\
	${Y2UTIL_CFLAGS}
