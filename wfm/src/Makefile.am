#
# Makefile.am for y2wfm/scr
#

SUBDIRS = include

AM_CXXFLAGS = -DSUSEVERSION=\"${SUSEVERSION}\" -DLOCALEDIR=\"${localedir}\"

plugin_LTLIBRARIES = libpy2wfm.la
noinst_LTLIBRARIES = liby2wfm.la

liby2wfm_la_SOURCES =				\
	WFMInterpreter.cc WFMSubAgent.cc
liby2wfm_la_LDFLAGS = -version-info 2:0

libpy2wfm_la_SOURCES =				\
	WFMInterpreter.cc WFMSubAgent.cc	\
	Y2WFMComponent.cc Y2CCWFM.cc
libpy2wfm_la_LDFLAGS = -version-info 2:0

INCLUDES = -I$(srcdir)/include -I$(includedir)
