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
	-I$(includedir)				\
	-I$(includedir)/y2util/qt		\
	-I$(srcdir)/../src

noinst_PROGRAMS = runwfm
runwfm_SOURCES = runwfm.cc
runwfm_LDADD = -ly2 -lycp -ly2pm
runwfm_LDFLAGS = -L$(libdir)				\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2wfm.la			\
	-Xlinker --no-whole-archive

bin_PROGRAMS = y2wfm
y2wfm_SOURCES = y2wfm.cc
y2wfm_LDADD = -ly2 -lycp -ly2pm
y2wfm_LDFLAGS = -L$(libdir)				\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../../liby2/src/libpy2script.la	\
	$(srcdir)/../src/libpy2wfm.la			\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp	\
	libycp.log libycp.sum site.bak

EXTRA_DIST = README
