#
# Makefile.am for core/agent-probe/testsuite
#

SUBDIRS = config lib ag_hwprobe.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-probe\"

PACKAGE = ag_hwprobe
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

noinst_PROGRAMS = runag_hwprobe

runag_hwprobe_SOURCES = runag_hwprobe.cc
runag_hwprobe_LDADD = -ly2 -lycp -lscr -ly2util
runag_hwprobe_LDFLAGS = -L$(libdir)			\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2ag_hwprobe.la		\
	-Xlinker --no-whole-archive
