#
# Makefile.am for core/scr/src
#

AM_CXXFLAGS = -DY2LOG=\"scr\" -DSUSEVERSION=\"${SUSEVERSION}\"

serversdir = $(execcompdir)/servers
servers_PROGRAMS = scr

scr_SOURCES =						\
	scr.cc

scr_LDADD = ${AGENT_LIBADD}				\
	$(top_builddir)/liby2/src/libpy2stdio.la	\
	$(top_builddir)/liby2/src/libpy2plugin.la

scr_DEPENDENCIES = libpy2scr.la

scr_LDFLAGS =					\
	-Xlinker --whole-archive		\
	$(top_builddir)/liby2/src/libpy2plugin.la	\
	$(top_builddir)/liby2/src/libpy2stdio.la	\
	$(top_builddir)/liby2/src/libpy2program.la	\
	libpy2scr.la				\
	-Xlinker --no-whole-archive

plugin_LTLIBRARIES = libpy2scr.la

libpy2scr_la_SOURCES =				\
	Y2CCSCR.cc				\
	ScriptingAgent.cc ScriptingAgent.h	\
	StdioSCRAgent.cc StdioSCRAgent.h	\
	SCRSubAgent.cc SCRSubAgent.h

libpy2scr_la_LDFLAGS = -version-info 2:0

INCLUDES = ${AGENT_INCLUDES}
