#
# Makefile.am for y2a_system/src
#

plugin_LTLIBRARIES = libpy2ag_system.la

libpy2ag_system_la_SOURCES =			\
	Y2CCSystemAgent.cc			\
	ShellCommand.cc ShellCommand.h		\
	SystemAgent.cc SystemAgent.h

libpy2ag_system_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(top_srcdir) -I$(includedir)
