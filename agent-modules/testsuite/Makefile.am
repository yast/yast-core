#
# Makefile.am for y2a_mods/testsuite
#

SUBDIRS = config lib ag_modules.test tests

PACKAGE = ag_modules
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = runmodules
runmodules_SOURCES = runmodules.cc

runmodules_LDADD = -lscr -ly2 -lycp -lcrypt
runmodules_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	$(plugindir)/libpy2script.la	\
	../src/libpy2ag_modules.la	\
	-Xlinker --no-whole-archive
