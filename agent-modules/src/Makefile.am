#
# Makefile.am for core/agent-modules/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-modules\"

plugin_LTLIBRARIES = libpy2ag_modules.la

libpy2ag_modules_la_SOURCES =			\
	ModulesConf.h	ModulesConf.cc		\
	ModulesAgent.h	ModulesAgent.cc		\
	Y2Logger.h				\
	Y2CCModulesAgent.cc
libpy2ag_modules_la_LDFLAGS = -version-info 2:0
libpy2ag_modules_la_LIBADD = @AGENT_LIBADD@

AM_CPPFLAGS = ${AGENT_INCLUDES}
