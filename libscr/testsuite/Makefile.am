#
# Makefile.am for libscr/testsuite
#

INCLUDES = -I../src/include -I$(includedir)

bin_PROGRAMS = runscr
runscr_SOURCES = runscr.cc
runscr_LDADD = ../src/libscr.la -lycp
runscr_LDFLAGS = -L$(libdir)

AUTOMAKE_OPTIONS = dejagnu

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp \
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
