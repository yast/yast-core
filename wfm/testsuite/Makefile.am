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
	-I$(srcdir)/../src			\
	-I$(includedir)                         \
        -I/usr/include/YaST2

noinst_PROGRAMS = runwfm

runwfm_SOURCES = runwfm.cc
runwfm_LDADD = -ly2 -lycp -ly2util -ly2pm -lscr

runwfm_LDFLAGS =					\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../scr/src			\
	-L$(srcdir)/../../libscr/src			\
	-L$(libdir)					\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../scr/src/libpy2scr.la		\
	$(srcdir)/../../liby2/src/liby2.la		\
	$(srcdir)/../../libyui/src/libyui.la		\
	$(srcdir)/../src/libpy2wfm.la			\
	$(srcdir)/../src/libpy2wfm.la			\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp	\
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
