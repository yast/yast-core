#
# Makefile.am for y2scr/testsuite
#

SUBDIRS = config lib scr.test tests

PACKAGE = scr
AUTOMAKE_OPTIONS = dejagnu

INCLUDES = -I$(includedir)

noinst_PROGRAMS = runscr y2scr

runscr_SOURCES = runscr.cc
runscr_LDADD = ../src/libpy2scr.la -ly2 -lycp -lscr -lutil
runscr_LDFLAGS = -L$(libdir)			\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	../ag_dummy/src/libpy2ag_dummy.la	\
	-Xlinker --no-whole-archive

y2scr_SOURCES = y2scr.cc
y2scr_LDADD = -L$(libdir) -lscr -ly2 -lycp
y2scr_LDFLAGS = \
	$(all_libraries)			\
	-Xlinker --whole-archive		\
	../ag_dummy/src/libpy2ag_dummy.la	\
	../ag_dumpto/src/libpy2ag_dumpto.la	\
	../src/libpy2scr.la			\
	$(plugindir)/libpy2stdio.la		\
	$(plugindir)/libpy2script.la		\
	-Xlinker --no-whole-archive
