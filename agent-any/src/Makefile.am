#
# Makefile.am for y2a_any/src
#

plugin_LTLIBRARIES = libpy2ag_anyagent.la
noinst_LTLIBRARIES = liby2ag_anyagent.la

liby2ag_anyagent_la_SOURCES =				\
	AnyAgent.cc AnyAgent.h				\
	AnyAgentBasic.cc				\
	AnyAgentComplex.cc
liby2ag_anyagent_la_LDFLAGS = -version-info 2:0
liby2ag_anyagent_la_LIBADD = -lycp -ly2 -lscr

libpy2ag_anyagent_la_SOURCES =				\
	$(liby2ag_anyagent_la_SOURCES)			\
	Y2CCAnyAgent.cc
libpy2ag_anyagent_la_LDFLAGS = -version-info 2:0
libpy2ag_anyagent_la_LIBADD = -lycp -ly2 -lscr

INCLUDES = -I$(includedir)
