#
# Makefile.am for core/agent-any/testsuite
#

SUBDIRS = config lib ag_anyagent.test tests

AM_CXXFLAGS = -DY2LOG=\"agent-any\"

PACKAGE = ag_anyagent
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =						\
	-I$(srcdir)/../../libycp/src/include		\
	-I$(srcdir)/../../liby2/src/include		\
	-I$(srcdir)/../../libscr/src/include		\
	-I$(includedir)

noinst_PROGRAMS = runag_anyagent

runag_anyagent_SOURCES = runag_anyagent.cc
runag_anyagent_LDADD = -ly2util
# why is whole-archive necessary?
runag_anyagent_LDFLAGS = -L$(libdir)			\
	$(srcdir)/../../libscr/src/libscr.la		\
	$(srcdir)/../../liby2/src/liby2.la		\
	$(srcdir)/../../libycp/src/libycp.la		\
	-Xlinker --whole-archive			\
	$(srcdir)/../../liby2/src/libpy2stdio.la	\
	$(srcdir)/../src/libpy2ag_anyagent.la		\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* *.sum *.log site.*
	rm -f runag_anyagent
