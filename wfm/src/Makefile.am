#
# Makefile.am for core/wfm/scr
#

SUBDIRS = include

AM_CXXFLAGS = -DY2LOG=\"wfm\" -DSUSEVERSION=\"${SUSEVERSION}\"	\
	-DLOCALEDIR=\"${localedir}\"

plugin_LTLIBRARIES = libpy2wfm.la
noinst_LTLIBRARIES = liby2wfm.la

liby2wfm_la_SOURCES =				\
	WFMInterpreter.cc WFMSubAgent.cc
liby2wfm_la_LDFLAGS = -version-info 2:0

libpy2wfm_la_SOURCES =				\
	WFMInterpreter.cc WFMSubAgent.cc	\
	Y2WFMComponent.cc Y2CCWFM.cc
libpy2wfm_la_LDFLAGS = -version-info 2:0

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(srcdir)/include
