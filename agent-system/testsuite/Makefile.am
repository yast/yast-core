#
# Makefile.am for core/agent-system/testsuite
#

SUBDIRS = config lib ag_system.test tests

CLEANFILES = $(wildcard tmp.out.* tmp.err.* tmp.write.* *.sum *.log)

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
runag_system_LDADD =                            \
	-ly2util                                \
	../../liby2/src/liby2.la                \
	../../libycp/src/libycp.la              \
	../../libscr/src/libscr.la

runag_system_LDFLAGS = 				\
	-L../../libycp/src			\
	-L../../liby2/src			\
	-L../../libscr/src			\
	-L$(libdir)				\
	-Xlinker --whole-archive		\
	../../liby2/src/libpy2stdio.la		\
	../src/libpy2ag_system.la		\
	-Xlinker --no-whole-archive
