#
# Makefile.am for core/wfm/src
#

AM_CXXFLAGS = -DY2LOG=\"wfm\" -DSUSEVERSION=\"${SUSEVERSION}\"	\
	-DLOCALEDIR=\"${localedir}\"

# to look for the packagemanager in the prefix first (really first?)
AM_LDFLAGS = -L${libdir}

plugin_LTLIBRARIES = libpy2wfm.la
noinst_LTLIBRARIES = liby2wfm.la

liby2wfm_la_SOURCES =				\
	Y2WFMComponent.cc Y2WFMComponent.h	\
	Y2CCWFM.cc Y2CCWFM.h			\
	Y2PkgComponent.cc Y2PkgComponent.h	\
	Y2CCPkg.cc Y2CCPkg.h			\
	WFM.cc WFM.h				\
	WFMSubAgent.cc WFMSubAgent.h		\
	ycpTools.cc ycpTools.h			\
	PkgModule.cc PkgModule.h		\
	PkgModuleError.cc PkgModuleError.h	\
	PkgModuleFunctions.h			\
	PkgModuleFunctions.cc			\
	PkgModuleFunctionsPackage.cc		\
	PkgModuleFunctionsPatch.cc		\
	PkgModuleFunctionsSelection.cc		\
	PkgModuleFunctionsTarget.cc		\
	PkgModuleFunctionsSource.cc		\
	PkgModuleCallbacks.h			\
	PkgModuleCallbacks.YCP.h		\
	PkgModuleCallbacks.cc

liby2wfm_la_LDFLAGS = -version-info 2:0
liby2wfm_la_LIBADD = \
	$(srcdir)/../../scr/src/libpy2scr.la		\
	$(srcdir)/../../libyui/src/libyui.la		\
	$(srcdir)/../../libscr/src/libscr.la		\
	$(srcdir)/../../libycp/src/libycp.la		\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/liby2.la		\
	-ly2pm -ly2util

libpy2wfm_la_SOURCES =				\
	Y2ScriptComponent.cc Y2ScriptComponent.h\
	Y2CCScript.cc Y2CCScript.h

libpy2wfm_la_LDFLAGS = -version-info 2:0
libpy2wfm_la_LIBADD = liby2wfm.la

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libyui/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

