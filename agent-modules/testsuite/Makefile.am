#
# Makefile.am for core/agent-modules/testsuite
#

SUBDIRS = config lib ag_modules.test tests

PACKAGE = ag_modules
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include

noinst_PROGRAMS = runmodules
runmodules_SOURCES = runmodules.cc

runmodules_LDADD = -lscr -ly2 -lycp -lcrypt
runmodules_LDFLAGS = -L$(libdir)	\
	-L$(srcdir)/../../libycp/src	\
	-L$(srcdir)/../../liby2/src	\
	-L$(srcdir)/../../libscr/src	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_modules.la	\
	-Xlinker --no-whole-archive
