#
# Makefile.am for core/libscr/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"libscr\"

INCLUDES =						\
	-I$(top_srcdir)/libycp/src/include		\
	-I$(top_srcdir)/liby2/src/include		\
	-I$(srcdir)/../src/include			\
	${Y2UTIL_CFLAGS}

noinst_PROGRAMS = runscr
runscr_SOURCES = runscr.cc
runscr_LDADD = ../src/libscr.la ../../liby2/src/liby2.la ../../libycp/src/libycp.la ${Y2UTIL_LIBS}

PACKAGE = libscr
AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
