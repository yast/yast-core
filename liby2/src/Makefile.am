#
# Makefile.am for liby2/src
#

SUBDIRS = include

AM_CXXFLAGS = -DYAST2DIR=\"${yast2dir}\" -DPLUGINDIR=\"${plugindir}\" 	\
	 -DEXECCOMPDIR=\"${execcompdir}\"

lib_LTLIBRARIES = liby2.la
plugin_LTLIBRARIES = 	\
	libpy2stdio.la		\
	libpy2script.la		\
	libpy2remote.la		\
	libpy2serial.la		\
	libpy2program.la	\
	libpy2plugin.la

liby2_la_SOURCES = \
	ExternalDataSource.cc ExternalProgram.cc	\
	Y2Component.cc					\
	Y2ComponentBroker.cc Y2ComponentCreator.cc 	\
	genericfrontend.cc pathsearch.cc
liby2_la_LDFLAGS = -version-info 2:0
liby2_la_LIBADD = -lutil -ldl

libpy2stdio_la_SOURCES =				\
	Y2StdioComponent.cc Y2CCStdio.cc
libpy2stdio_la_LDFLAGS = -version-info 2:0

libpy2script_la_SOURCES =				\
	Y2ScriptComponent.cc Y2CCScript.cc
libpy2script_la_LDFLAGS = -version-info 2:0

libpy2remote_la_SOURCES =				\
	Y2RemoteComponent.cc Y2CCRemote.cc		\
	RemoteProtocol.cc				\
	TelnetProtocol.cc				\
	RloginProtocol.cc				\
	RshProtocol.cc					\
	SuProtocol.cc					\
	SudoProtocol.cc
libpy2remote_la_LDFLAGS = -version-info 2:0

libpy2serial_la_SOURCES =				\
	Y2SerialComponent.cc Y2CCSerial.cc
libpy2serial_la_LDFLAGS = -version-info 2:0

libpy2program_la_SOURCES =	\
	Y2ProgramComponent.cc Y2CCProgram.cc
libpy2program_la_LDFLAGS = -version-info 2:0

libpy2plugin_la_SOURCES =	\
	Y2PluginComponent.cc Y2CCPlugin.cc
libpy2plugin_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(srcdir)/include -I$(srcdir)/include/y2 -I$(includedir)
