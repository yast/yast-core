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
	WFMInterpreter.cc WFMInterpreter.h	\
	WFMSubAgent.cc WFMSubAgent.h		\
	PerlModule.cc PerlModule.h		\
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
# this makes it use the version from our prefix instead of /usr/lib
liby2wfm_la_LIBADD = -ly2pm

libpy2wfm_la_SOURCES =				\
	$(liby2wfm_la_SOURCES)			\
	Y2WFMComponent.cc Y2WFMComponent.h	\
	Y2CCWFM.cc Y2CCWFM.h
libpy2wfm_la_LDFLAGS = -version-info 2:0
libpy2wfm_la_LIBADD = -ly2pm

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

