#
# Makefile.am for core/wfm/src
#

AM_CXXFLAGS = -DY2LOG=\"wfm\" -DSUSEVERSION=\"${SUSEVERSION}\"	\
	-DLOCALEDIR=\"${localedir}\"

plugin_LTLIBRARIES = libpy2wfm.la
noinst_LTLIBRARIES = liby2wfm.la

liby2wfm_la_SOURCES =				\
	WFMInterpreter.cc WFMInterpreter.h	\
	WFMSubAgent.cc WFMSubAgent.h		\
	PerlModule.cc PerlModule.h		\
	PkgModule.cc PkgModule.h		\
	PkgModuleFunctions.h			\
	PkgModuleFunctions.cc			\
	PkgModuleFunctionsPackage.cc		\
	PkgModuleFunctionsPatch.cc		\
	PkgModuleFunctionsSelection.cc		\
	PkgModuleFunctionsTarget.cc		\
	PkgModuleFunctionsSource.cc		\
	PkgModuleCallbacks.cc
liby2wfm_la_LDFLAGS = -version-info 2:0

libpy2wfm_la_SOURCES =				\
	$(liby2wfm_la_SOURCES)			\
	Y2WFMComponent.cc Y2WFMComponent.h	\
	Y2CCWFM.cc Y2CCWFM.h
libpy2wfm_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

