# Makefile.am for core/agent-resolver/testsuite

SUBDIRS = config lib ag_resolver.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-resolver\"

PACKAGE = ag_resolver
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)				\
	-I/usr/include/YaST2

noinst_PROGRAMS = runresolver

runresolver_SOURCES = runresolver.cc
runresolver_LDADD = -ly2util			\
	../../liby2/src/liby2.la		\
	../../libycp/src/libycp.la		\
	../../libscr/src/libscr.la

runresolver_LDFLAGS = 					\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-L$(libdir)					\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2ag_resolver.la		\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* site.exp *.log *.sum *.bak

