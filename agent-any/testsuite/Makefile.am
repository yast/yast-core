#
# Makefile.am for y2a_anyagent/testsuite
#

SUBDIRS = config lib ag_anyagent.test tests

PACKAGE = ag_anyagent
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = runag_anyagent

runag_anyagent_SOURCES = runag_anyagent.cc
runag_anyagent_LDADD = -lscr -ly2 -lycp -lcrypt
runag_anyagent_LDFLAGS = -L$(libdir)	\
	-Xlinker --whole-archive	\
	$(plugindir)/libpy2stdio.la	\
	../src/libpy2ag_anyagent.la	\
	-Xlinker --no-whole-archive

clean-local:
	rm -f tmp.err.* tmp.out.* *.sum *.log site.*
	rm -f runag_anyagent
