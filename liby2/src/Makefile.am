#
# Makefile.am for core/liby2/src
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"liby2\" -DYAST2DIR=\"${yast2dir}\"		\
	-DPLUGINDIR=\"${plugindir}\" -DEXECCOMPDIR=\"${execcompdir}\"

lib_LTLIBRARIES = liby2.la
plugin_LTLIBRARIES = libpy2serial.la

liby2_la_SOURCES = \
	Y2Component.cc					\
	Y2ComponentBroker.cc Y2ComponentCreator.cc 	\
	Y2Namespace.cc					\
	Y2LanguageLoader.cc                             \
	genericfrontend.cc				\
	SymbolEntry.cc					\
	Y2ErrorComponent.cc				\
	Y2ProgramComponent.cc Y2CCProgram.cc		\
	Y2PluginComponent.cc Y2CCPlugin.cc		\
	Y2StdioComponent.cc Y2CCStdio.cc

liby2_la_LDFLAGS = -version-info 2:0
# pthread added (74501)
liby2_la_LIBADD = ${Y2UTIL_LIBS} -lutil -ldl -lpthread

libpy2serial_la_SOURCES =				\
	Y2SerialComponent.cc Y2CCSerial.cc
libpy2serial_la_LDFLAGS = -version-info 2:0

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/include				\
	-I$(srcdir)/include/y2				\
	-I$(srcdir)/../debugger			\
	${Y2UTIL_CFLAGS}
