#
# Makefile.am for core/scr/testsuite
#

SUBDIRS = config lib scr.test tests

AM_CXXFLAGS = -DY2LOG=\"scr\"

PACKAGE = scr
AUTOMAKE_OPTIONS = dejagnu

INCLUDES =					\
	-I$(srcdir)/../../libycp/src/include	\
	-I$(srcdir)/../../liby2/src/include	\
	-I$(srcdir)/../../libscr/src/include	\
	-I$(includedir)

noinst_PROGRAMS = runscr y2scr

runscr_SOURCES = runscr.cc
runscr_LDADD = -ly2 -lycp -lscr -ly2util
runscr_LDFLAGS = 				\
	-L../../libycp/src			\
	-L../../liby2/src			\
	-L../../libscr/src			\
	-L$(libdir)				\
	-Xlinker --whole-archive		\
	../../liby2/src/libpy2stdio.la		\
	../../agent-dummy/src/libpy2ag_dummy.la	\
	../src/libpy2scr.la			\
	-Xlinker --no-whole-archive

y2scr_SOURCES = y2scr.cc
y2scr_LDADD = -lscr -ly2 -lycp
y2scr_LDFLAGS =					\
	-L../../libycp/src			\
	-L../../liby2/src			\
	-L../../libscr/src			\
	-L$(libdir)				\
	-Xlinker --whole-archive		\
	../../agent-dummy/src/libpy2ag_dummy.la	\
	../../liby2/src/libpy2stdio.la		\
	../src/libpy2scr.la			\
	-Xlinker --no-whole-archive
