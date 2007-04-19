#
# Makefile.am for core/wfm/src
#

AM_CXXFLAGS = -DY2LOG=\"wfm\" -DSUSEVERSION=\"${SUSEVERSION}\"	\
	-DLOCALEDIR=\"${localedir}\"

plugin_LTLIBRARIES = libpy2wfm.la

libpy2wfm_la_SOURCES =				\
	Y2WFMComponent.cc Y2WFMComponent.h	\
	Y2CCWFM.cc Y2CCWFM.h			\
	WFM.cc WFM.h				\
	WFMSubAgent.cc WFMSubAgent.h		\
	Y2SystemNamespace.cc			\
	Y2SystemNamespace.h			\
	Y2StdioFunction.cc			\
	Y2StdioFunction.h			\
	Y2SystemFunction.h			\
	Y2SystemFunction.cc


# use this for KMTrace: libpy2wfm_la_LIBADD = liby2wfm.la /opt/kde3/lib/libktrace_s.a
libpy2wfm_la_LDFLAGS = -version-info 2:0
libpy2wfm_la_LIBADD = \
	../../scr/src/libpy2scr.la		\
	../../libyui/src/libyui.la		\
	../../libscr/src/libscr.la		\
	../../libycp/src/libycp.la		\
	../../liby2/src/liby2.la		\
	${Y2UTIL_LIBS}

INCLUDES =					\
	-I$(top_srcdir)/libycp/src/include	\
	-I$(top_srcdir)/liby2/src/include	\
	-I$(top_srcdir)/libyui/src		\
	-I$(top_srcdir)/libyui/src/include	\
	-I$(top_srcdir)/libscr/src/include	\
	${Y2UTIL_CFLAGS}

wfmincludedir = $(includedir)/wfm

wfminclude_HEADERS =				\
	Y2WFMComponent.h			\
	WFMSubAgent.h


