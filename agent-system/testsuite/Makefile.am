#
# Makefile.am for y2a_system/testsuite
#

SUBDIRS = config lib ag_system.test tests

PACKAGE = ag_system
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = runag_system

runag_system_SOURCES = runag_system.cc
runag_system_LDADD = -ly2 -lycp -lscr -lcrypt -lutil
runag_system_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	../src/libpy2ag_system.la		\
	-Xlinker --no-whole-archive
