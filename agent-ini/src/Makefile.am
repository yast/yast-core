#
# Makefile.am for y2a_ini/src
#

AM_CXXFLAGS = -Wall # -g # -pedantic # -W

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

libpy2ag_ini_la_SOURCES =                           \
        $(liby2ag_ini_la_SOURCES)                   \
        Y2CCIniAgent.cc         Y2CCIniAgent.h  \
        Y2IniAgentComponent.cc  Y2IniAgentComponent.h
libpy2ag_ini_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(includedir)

