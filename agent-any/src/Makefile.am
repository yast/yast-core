#
# Makefile.am for core/agent-any/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-any\"

plugin_LTLIBRARIES = libpy2ag_anyagent.la
noinst_LTLIBRARIES = liby2ag_anyagent.la

liby2ag_anyagent_la_SOURCES =				\
	AnyAgent.cc AnyAgent.h				\
	AnyAgentBasic.cc				\
	AnyAgentComplex.cc
liby2ag_anyagent_la_LDFLAGS = -version-info 2:0
liby2ag_anyagent_la_LIBADD = @AGENT_LIBADD@

libpy2ag_anyagent_la_SOURCES =				\
	$(liby2ag_anyagent_la_SOURCES)			\
	Y2CCAnyAgent.cc
libpy2ag_anyagent_la_LDFLAGS = -version-info 2:0
libpy2ag_anyagent_la_LIBADD = @AGENT_LIBADD@

INCLUDES = @AGENT_INCLUDES@
