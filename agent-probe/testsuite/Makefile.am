#
# Makefile.am for y2a_hwpb/testsuite
#

SUBDIRS = config lib ag_hwprobe.test tests

PACKAGE = ag_hwprobe
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = runag_hwprobe

runag_hwprobe_SOURCES = runag_hwprobe.cc
runag_hwprobe_LDADD = -ly2 -lycp -lscr -lcrypt -lutil
runag_hwprobe_LDFLAGS = -L$(libdir)		\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	../src/libpy2ag_hwprobe.la		\
	-Xlinker --no-whole-archive

