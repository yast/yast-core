#
# Makefile.am for core/agent-probe/testsuite
#

SUBDIRS = config lib ag_hwprobe.test tests

PACKAGE = ag_hwprobe
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include

noinst_PROGRAMS = runag_hwprobe

runag_hwprobe_SOURCES = runag_hwprobe.cc
runag_hwprobe_LDADD = -ly2 -lycp -lscr -lcrypt -lutil
runag_hwprobe_LDFLAGS = -L$(libdir)		\
	-L$(srcdir)/../../libycp/src		\
	-L$(srcdir)/../../liby2/src		\
	-L$(srcdir)/../../libscr/src		\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	../src/libpy2ag_hwprobe.la		\
	-Xlinker --no-whole-archive
