#
# Makefile.am for core/agent-modules/testsuite
#

SUBDIRS = config lib ag_modules.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-modules\"

PACKAGE = ag_modules
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)				\
	-I/usr/include/YaST2

noinst_PROGRAMS = runmodules

runmodules_SOURCES = runmodules.cc
runmodules_LDADD = -ly2util			\
	../../liby2/src/liby2.la		\
	../../libycp/src/libycp.la		\
	../../libscr/src/libscr.la

runmodules_LDFLAGS = 					\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-L$(libdir)					\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2ag_modules.la		\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* site.exp *.log *.sum *.bak

