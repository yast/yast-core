#
# Makefile.am for core/libyui/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"libyui\"

INCLUDES =						\
	-I$(top_srcdir)/libycp/src/include		\
	-I$(top_srcdir)/liby2/src/include		\
	-I$(srcdir)/../src/include			\
	${Y2UTIL_CFLAGS}

noinst_PROGRAMS = runyui
runyui_SOURCES = runyui.cc
runyui_LDADD = ../src/libpy2UI.la ../../liby2/src/liby2.la ../../libycp/src/libycp.la ${Y2UTIL_LIBS} -lpthread

#This is needed for the testsuite to run
#But it has been broken for ages now
#PACKAGE=libyui

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
