#
# Makefile.am for core/agent-ini/src
#

AM_CXXFLAGS = -DY2LOG=\"agent-ini\"
AM_LDFLAGS = -L$(libdir)

plugin_LTLIBRARIES = libpy2ag_ini.la
noinst_LTLIBRARIES = liby2ag_ini.la

liby2ag_ini_la_SOURCES =	\
	IniAgent.cc		\
	IniAgent.h		\
	IniParser.cc		\
	IniParser.h		\
	IniFile.cc		\
	IniFile.h

liby2ag_ini_la_LDFLAGS = -version-info 2:0
liby2ag_ini_la_LIBADD = @AGENT_LIBADD@

libpy2ag_ini_la_SOURCES =			\
        $(liby2ag_ini_la_SOURCES)		\
        Y2CCIniAgent.cc

libpy2ag_ini_la_LDFLAGS = -version-info 2:0
libpy2ag_ini_la_LIBADD =			\
	@AGENT_LIBADD@				\
	-ly2util

INCLUDES = @AGENT_INCLUDES@
