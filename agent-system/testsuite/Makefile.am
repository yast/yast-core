#
# Makefile.am for core/agent-system/testsuite
#

SUBDIRS = config lib ag_system.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-system\"

PACKAGE = ag_system
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

noinst_PROGRAMS = runag_system

runag_system_SOURCES = runag_system.cc
runag_system_LDADD = -ly2 -lycp -lscr -ly2util
runag_system_LDFLAGS = -L$(libdir)			\
	-L$(srcdir)/../../libycp/src			\
	-L$(srcdir)/../../liby2/src			\
	-L$(srcdir)/../../libscr/src			\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2ag_system.la		\
	-Xlinker --no-whole-archive
