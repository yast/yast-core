#
# Makefile.am for y2wfm/testsuite
#

PACKAGE = wfm
AUTOMAKE_OPTIONS = dejagnu
SUBDIRS = config lib wfm.test tests

INCLUDES= -I$(srcdir)/../src/include/wfm -I$(includedir)

noinst_PROGRAMS = runwfm
runwfm_SOURCES = runwfm.cc
runwfm_LDADD = ../src/liby2wfm.la -ly2 -lycp -lutil
runwfm_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	../src/liby2wfm.la			\
	-Xlinker --no-whole-archive

bin_PROGRAMS = y2wfm
y2wfm_SOURCES = y2wfm.cc
y2wfm_LDADD = -ly2 -lycp -lutil
y2wfm_LDFLAGS = -L$(libdir)			\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	$(plugindir)/libpy2script.la		\
	../src/libpy2wfm.la			\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* ycp.log ycp.sum site.exp libycp.log libycp.sum site.bak

EXTRA_DIST = README
