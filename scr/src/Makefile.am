#
# Makefile.am for y2scr/src
#

AM_CXXFLAGS = -DSUSEVERSION=\"${SUSEVERSION}\"

serversdir = $(execcompdir)/servers
servers_PROGRAMS = scr

scr_SOURCES =					\
	Y2CCSCR.cc				\
	ScriptingAgent.cc ScriptingAgent.h	\
	SCRSubAgent.cc SCRSubAgent.h

scr_LDADD = -L$(libdir) -lscr -ly2 -lycp

scr_LDFLAGS =					\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2plugin.la		\
	$(plugindir)/libpy2stdio.la		\
	$(plugindir)/libpy2script.la		\
	$(plugindir)/libpy2program.la		\
	-Xlinker --no-whole-archive

plugin_LTLIBRARIES = libpy2scr.la

libpy2scr_la_SOURCES =				\
	Y2CCSCR.cc				\
	ScriptingAgent.cc ScriptingAgent.h	\
	SCRSubAgent.cc SCRSubAgent.h

libpy2scr_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(includedir)
