#
# Makefile.am for core/wfm/src
#

AM_CXXFLAGS = -DY2LOG=\"wfm\" -DSUSEVERSION=\"${SUSEVERSION}\"	\
	-DLOCALEDIR=\"${localedir}\"  -fno-inline

plugin_LTLIBRARIES = libpy2wfm.la
noinst_LTLIBRARIES = liby2wfm.la

liby2wfm_la_SOURCES =				\
	Y2WFMComponent.cc Y2WFMComponent.h	\
	Y2CCWFM.cc Y2CCWFM.h			\
	WFM.cc WFM.h				\
	WFMSubAgent.cc WFMSubAgent.h


liby2wfm_la_LDFLAGS = -version-info 2:0
liby2wfm_la_LIBADD = \
	../../scr/src/libpy2scr.la		\
	../../libyui/src/libyui.la		\
	../../libscr/src/libscr.la		\
	../../libycp/src/libycp.la		\
	../../liby2/src/libpy2stdio.la		\
	../../liby2/src/liby2.la		\
	-ly2util

libpy2wfm_la_SOURCES =				\
	Y2ScriptComponent.cc Y2ScriptComponent.h\
	Y2CCScript.cc Y2CCScript.h

libpy2wfm_la_LDFLAGS = -version-info 2:0
# use this for KMTrace: libpy2wfm_la_LIBADD = liby2wfm.la /opt/kde3/lib/libktrace_s.a
libpy2wfm_la_LIBADD = liby2wfm.la

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libyui/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

wfmincludedir = $(includedir)/wfm

wfminclude_HEADERS =				\
	Y2WFMComponent.h			\
	WFMSubAgent.h


