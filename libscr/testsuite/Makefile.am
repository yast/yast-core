#
# Makefile.am for core/libscr/testsuite
#

AM_CXXFLAGS = -DY2LOG=\"libscr\"

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/../src/include			\
	-I$(includedir)					\
	-I/usr/include/YaST2

noinst_PROGRAMS = runscr
runscr_SOURCES = runscr.cc
runscr_LDADD = ../src/libscr.la ../../liby2/src/liby2.la -lycp -ly2util
runscr_LDFLAGS = -L$(srcdir)/../../libycp/src -L$(libdir)

PACKAGE = libscr
AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
