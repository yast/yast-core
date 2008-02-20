#
# Makefile.am for .../agent-process/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-process\" -Wall

plugin_LTLIBRARIES = libpy2ag_process.la
noinst_LTLIBRARIES = liby2ag_process.la

liby2ag_process_la_SOURCES =				\
	ProcessAgent.cc					\
	ProcessAgent.h					\
	Process.cc					\
	Process.h

liby2ag_process_la_LDFLAGS = -version-info 2:0
liby2ag_process_la_LIBADD = @AGENT_LIBADD@

libpy2ag_process_la_SOURCES =				\
        $(liby2ag_process_la_SOURCES)			\
        Y2CCProcessAgent.cc
libpy2ag_process_la_LDFLAGS = -version-info 2:0
libpy2ag_process_la_LIBADD = @AGENT_LIBADD@

INCLUDES = @AGENT_INCLUDES@
