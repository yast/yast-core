#
# Makefile.am for core/scr/src
#

AM_CXXFLAGS = -DY2LOG=\"scr\" -DSUSEVERSION=\"${SUSEVERSION}\"
# to respect $prefix
AM_LDFLAGS = -L$(libdir)

serversdir = $(execcompdir)/servers
servers_PROGRAMS = scr

scr_SOURCES =						\
	scr.cc

scr_LDADD = -L$(libdir) -ly2util		\
	../../libscr/src/libscr.la		\
	../../libycp/src/libycp.la		\
	../../liby2/src/liby2.la		\
	../../liby2/src/libpy2stdio.la		\
	../../liby2/src/libpy2plugin.la

scr_DEPENDENCIES = libpy2scr.la

scr_LDFLAGS =					\
	-L../../libycp/src			\
	-L../../liby2/src			\
	-L../../libscr/src			\
	-Xlinker --whole-archive		\
	../../liby2/src/libpy2plugin.la		\
	../../liby2/src/libpy2stdio.la		\
	../../liby2/src/libpy2program.la	\
	libpy2scr.la				\
	-Xlinker --no-whole-archive

plugin_LTLIBRARIES = libpy2scr.la

libpy2scr_la_SOURCES =				\
	Y2CCSCR.cc				\
	ScriptingAgent.cc ScriptingAgent.h	\
	StdioSCRAgent.cc StdioSCRAgent.h	\
	SCRSubAgent.cc SCRSubAgent.h

libpy2scr_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)				\
	-I/usr/include/YaST2
