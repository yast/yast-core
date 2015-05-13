#
# Makefile.am for .../agent-process/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-process\" -Wall

plugin_LTLIBRARIES = libpy2ag_process.la

libpy2ag_process_la_SOURCES =				\
	ProcessAgent.cc					\
	ProcessAgent.h					\
        Y2CCProcessAgent.cc
libpy2ag_process_la_LDFLAGS = -version-info 2:0
libpy2ag_process_la_LIBADD = @AGENT_LIBADD@

AM_CPPFLAGS = @AGENT_INCLUDES@
