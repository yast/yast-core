#
# Makefile.am for core/wfm/testsuite
#

SUBDIRS = config lib wfm.test tests

AM_CXXFLAGS = -DY2LOG=\"wfm\"

PACKAGE = wfm
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(srcdir)/../../libyui/src/include    \
	-I$(srcdir)/../src			\
	-I$(includedir)                         \
        -I/usr/include/YaST2

noinst_PROGRAMS = runwfm runc

runwfm_SOURCES = runwfm.cc

runwfm_LDADD = \
	../src/libpy2wfm.la


runc_SOURCES = runc.cc
runc_LDFLAGS = -L$(libdir)
runc_LDADD = $(srcdir)/../src/libpy2wfm.la     \
	$(srcdir)/../../libyui/src/libyui.la    \
	$(srcdir)/../../libscr/src/libscr.la    \
	$(srcdir)/../../libycp/src/libycp.la    \
	$(srcdir)/../../liby2/src/liby2.la      \
	-ly2util

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp	\
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
