#
# Makefile.am for core/scr/testsuite
#

SUBDIRS = config lib scr.test tests

PACKAGE = scr
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include

noinst_PROGRAMS = runscr y2scr

runscr_SOURCES = runscr.cc
runscr_LDADD = ../src/libpy2scr.la -ly2 -lycp -lscr -lutil
runscr_LDFLAGS = -L$(libdir)			\
	-L$(srcdir)/../../libycp/src		\
	-L$(srcdir)/../../liby2/src		\
	-L$(srcdir)/../../libscr/src		\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2stdio.la		\
	$(plugindir)/libpy2ag_dummy.la		\
	-Xlinker --no-whole-archive

y2scr_SOURCES = y2scr.cc
y2scr_LDADD = -L$(libdir) -lscr -ly2 -lycp
y2scr_LDFLAGS =					\
	-L$(srcdir)/../../libycp/src		\
	-L$(srcdir)/../../liby2/src		\
	-L$(srcdir)/../../libscr/src		\
	$(all_libraries)			\
	-Xlinker --whole-archive		\
	$(plugindir)/libpy2ag_dummy.la	\
	$(plugindir)/libpy2ag_dumpto.la	\
	../src/libpy2scr.la			\
	$(plugindir)/libpy2stdio.la		\
	$(plugindir)/libpy2script.la		\
	-Xlinker --no-whole-archive
