#
# Makefile.am for core/agent-modules/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-modules\"

plugin_LTLIBRARIES = libpy2ag_modules.la
noinst_LTLIBRARIES = liby2ag_modules.la

liby2ag_modules_la_SOURCES =			\
	ModulesConf.h	ModulesConf.cc		\
	ModulesAgent.h	ModulesAgent.cc		\
	Y2Logger.h
liby2ag_modules_la_LDFLAGS = -version-info 2:0

libpy2ag_modules_la_SOURCES =				\
	$(liby2ag_modules_la_SOURCES)			\
	Y2CCModulesAgent.cc         Y2CCModulesAgent.h	\
	Y2ModulesAgentComponent.cc  Y2ModulesAgentComponent.h
libpy2ag_modules_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)
