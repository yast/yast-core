#
# Makefile.am for core/agent-system/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-system\"

plugin_LTLIBRARIES = libpy2ag_system.la

libpy2ag_system_la_SOURCES =			\
	Y2CCSystemAgent.cc			\
	ShellCommand.cc ShellCommand.h		\
	SystemAgent.cc SystemAgent.h

libpy2ag_system_la_LDFLAGS = -version-info 2:0
libpy2ag_system_la_LIBADD = @AGENT_LIBADD@

INCLUDES = @AGENT_INCLUDES@
